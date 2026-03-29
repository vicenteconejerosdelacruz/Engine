#include "pch.h"
#include "RenderPass.h"
#include <Renderer.h>
#include <DeviceUtils/DescriptorHeap/DescriptorHeap.h>
#include <DeviceUtils/RenderPass/SwapChainPass.h>
#include <DeviceUtils/RenderPass/RenderToTexturePass.h>
#include <SceneObject.h>
#include "Override/OverridePass.h"

extern std::unique_ptr<JRenderer> renderer;

namespace Templates
{
#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <RenderPassAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <RenderPassAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <RenderPassAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <RenderPassAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <RenderPassAtt.h>
#include <JEnd.h>

#endif

	namespace RenderPass
	{
		std::unique_ptr<DeviceUtils::DescriptorHeap> mainHeap;

		void CreateRenderPassMainHeap()
		{
			mainHeap = std::make_unique<DeviceUtils::DescriptorHeap>();
			mainHeap->CreateDescriptorHeap(renderer->d3dDevice, renderer->numFrames);
		}

		void DestroyRenderPassMainHeap()
		{
			mainHeap->DestroyDescriptorHeap();
			mainHeap = nullptr;
		}
	}

	RenderPassJson::RenderPassJson(nlohmann::json& json) : JTemplate(json)
	{
#include <Attributes/JInit.h>
#include <RenderPassAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <RenderPassAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void RenderPassJson::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <RenderPassAtt.h>
#include <JEnd.h>
	}
#endif

	void RenderPassJson::SetPipelineStateCallback(size_t hash, std::function<void()> callback)
	{
		if (pipelineChangeCallbacks.contains(hash)) return;
		pipelineChangeCallbacks.insert_or_assign(hash, callback);
	}

	TEMPDEF_FULL(RenderPass);
	TEMPDEF_REFTRACKER(RenderPass);

	std::unordered_map<RenderPassJsonID, std::set<RenderPassInstanceID>> renderPassTemplatesInstances;
	std::set<RenderPassInstanceID> renderPassesInstancesToDelete;
	void RenderPassJsonStep()
	{
		std::set<RenderPassJsonID> passes;
		std::transform(RenderPasstemplates.begin(), RenderPasstemplates.end(), std::inserter(passes, passes.begin()), [](auto& temps)
			{
				return temps.first;
			}
		);

		std::set<RenderPassJsonID> rebuildPipelineState;
		std::copy_if(passes.begin(), passes.end(), std::inserter(rebuildPipelineState, rebuildPipelineState.begin()), [](auto pass)
			{
				return (
					pass->dirty(RenderPassJson::Update_renderTargetFormats) ||
					pass->dirty(RenderPassJson::Update_depthStencilFormat)) &&
					renderPassTemplatesInstances.contains(pass) &&
					renderPassTemplatesInstances.at(pass).size() > 0ULL;
			}
		);

		std::unordered_map<RenderPassJsonID, std::set<RenderPassInstanceID>> changes;
		std::for_each(rebuildPipelineState.begin(), rebuildPipelineState.end(), [&](auto pass)
			{
				changes.insert_or_assign(pass, renderPassTemplatesInstances.at(pass));
			}
		);

		if (changes.size() > 0ULL)
		{
			UpdateRenderPassInstances(changes);
		}

		std::for_each(rebuildPipelineState.begin(), rebuildPipelineState.end(), [&](auto pass)
			{
				for (auto& [hash, callback] : pass->pipelineChangeCallbacks)
				{
					callback();
				}
				pass->clean(RenderPassJson::Update_renderTargetFormats);
				pass->clean(RenderPassJson::Update_depthStencilFormat);
			}
		);

		if (renderPassesInstancesToDelete.size() > 0)
		{
			for (auto pass : renderPassesInstancesToDelete)
			{
				auto passJ = pass->renderPassTemplate;
				if (renderPassTemplatesInstances.contains(passJ))
				{
					renderPassTemplatesInstances.at(passJ).erase(pass);
				}
				DestroyRenderPassInstance(pass());
			}
			renderPassesInstancesToDelete.clear();
		}
	}

	void UpdateRenderPassInstances(std::unordered_map<RenderPassJsonID, std::set<RenderPassInstanceID>> changes)
	{
		for (auto& [pass, instances] : changes)
		{
			for (auto& instance : instances)
			{
				instance->CreateRenderTargets();
			}
		}
	}

	RenderPassInstanceID CreateRenderPassInstance(CameraID camera, RenderPassJsonID renderPassTemplate, unsigned int renderPassIndex, unsigned int width, unsigned int height)
	{
		RenderPassInstanceID rpi = getUUID();
		CreateRenderPassInstance(
			renderPassTemplate(), rpi(), [&]()
			{
				std::unique_ptr<RenderPassInstance> instance = std::make_unique<RenderPassInstance>(camera, renderPassTemplate, rpi, renderPassIndex, width, height);
				return instance;
			}
		);
		rpi->InitRenderPass();
		return rpi;
	}

	RenderPassInstance::RenderPassInstance(CameraID camera, RenderPassJsonID renderPassTemplate, RenderPassInstanceID renderPassInstance, unsigned int renderPassIndex, unsigned int width, unsigned int height)
	{
		this->camera = camera;
		this->renderPassTemplate = renderPassTemplate;
		this->renderPassInstance = renderPassInstance;
		this->renderPassIndex = renderPassIndex;
		this->width = width;
		this->height = height;

		renderPassTemplatesInstances[renderPassTemplate].insert(renderPassInstance);

		type = renderPassTemplate->type();
		CreateRenderTargets();

		materialOverride = renderPassTemplate->materialOverride();
		renderCallbackOverride = renderPassTemplate->renderCallbackOverride();
		const std::map<RenderPassRenderCallbackOverride, std::function<
			std::unique_ptr<OverridePass>(CameraID, unsigned int, RenderPassJsonID, RenderPassInstanceID
			)>> RenderCallbackOverriders =
		{
			{ RenderPassRenderCallbackOverride_None, [](auto c,auto rpindex, auto rpTemplate, auto rpInstance) { return nullptr; }},
			{ RenderPassRenderCallbackOverride_ToneMapping, [&](auto c, auto rpindex, auto rpTemplate, auto rpInstance) { return std::make_unique<ToneMappingPass>(c,rpindex, rpTemplate, rpInstance); } },
			{ RenderPassRenderCallbackOverride_Resolve, [&](auto c, auto rpindex, auto rpTemplate, auto rpInstance) { return rpindex > 0 ? std::make_unique<ResolvePass>(c,rpindex, rpTemplate, rpInstance) : nullptr; } },
			{ RenderPassRenderCallbackOverride_MinMaxChain, [&](auto c, auto rpindex, auto rpTemplate, auto rpInstance) { return std::make_unique<MinMaxChainPass>(c,rpindex, rpTemplate, rpInstance); } },
			{ RenderPassRenderCallbackOverride_MinMaxChainResult, [&](auto c, auto rpindex, auto rpTemplate, auto rpInstance) { return std::make_unique<MinMaxChainResultPass>(c,rpindex, rpTemplate, rpInstance); } }
		};
		overridePass = RenderCallbackOverriders.at(renderCallbackOverride)(camera, renderPassIndex, renderPassTemplate, renderPassInstance);
	}

	void DestroyRenderPassInstance(RenderPassInstanceID renderPassInstanceID)
	{
		if (renderPassInstanceID.empty()) return;

		DeleteRenderPassInstance(renderPassInstanceID());
	}

	RenderPassInstance::~RenderPassInstance()
	{
		if (!swapChainPass.empty()) {
			swapChainPass->ReleaseResources();
			DeleteSwapChainPass(swapChainPass());
		}
		if (!renderToTexturePass.empty()) {
			renderToTexturePass->ReleaseResources();
			DeleteRenderToTexturePass(renderToTexturePass());
		}
		renderPassTemplatesInstances[renderPassTemplate].erase(renderPassInstance);
	}

	void RenderPassInstance::Pass(SceneUnitId unit, std::function<void(SceneUnitId)> renderCallback, bool clearRTV, XMVECTORF32 clearColor)
	{
		if (overridePass) return overridePass->Pass(unit);

		switch (type)
		{
		case RenderPassType_RenderToTexturePass:
		{
			renderToTexturePass->Pass(unit, renderCallback, clearColor);
		}
		break;
		case RenderPassType_SwapChainPass:
		{
			swapChainPass->Pass(unit, renderCallback, clearRTV, clearColor);
		}
		break;
		}
	}

	JUUID RenderPassInstance::GetRenderPassMaterialInstance(
		SceneUnitId id, MaterialJsonID material, MeshInstanceID mesh,
		bool shadowed, std::vector<PassMaterialOverride> passMaterialOverride, JUUID bindingUUID
	)
	{
		using namespace RenderPass;

		VertexClass vertexClass = mesh->vertexClass;
		std::string vertexType = VertexClassToString.at(vertexClass);
		bool hasIbl = camera.uuid().empty() ? false : camera->HasIBL();

		auto noOverride = [id, &material, vertexClass, vertexType, shadowed, hasIbl, bindingUUID]()
			{
				MaterialInstanceID instanceUUID = material() + "-" + vertexType;
				CreateMaterialInstance(instanceUUID(), [id, &instanceUUID, &material, vertexClass, shadowed, hasIbl, bindingUUID]()
					{
						return std::make_unique<MaterialInstance>(id, instanceUUID(), material(), vertexClass, shadowed, hasIbl, TextureShaderUsageMap(), bindingUUID);
					}
				);
				return instanceUUID();
			};

		auto shadowMapOverride = [id, &material, vertexClass, vertexType]()
			{
				JUUID smMatUUID = GetMaterialUUIDByName(shadowMapMaterialName);
				MaterialInstanceID instanceUUID = smMatUUID + "-" + vertexType;
				TextureShaderUsageMap overrideTextures;
				if (material->textures_contains(TextureShaderUsage_Base))
				{
					overrideTextures.insert_or_assign(TextureShaderUsage_Base, material->textures().at(TextureShaderUsage_Base));
				}
				CreateMaterialInstance(instanceUUID(), [id, &instanceUUID, smMatUUID, vertexClass, overrideTextures]()
					{
						return std::make_unique<MaterialInstance>(id, instanceUUID(), smMatUUID, vertexClass, false, false, overrideTextures);
					}
				);
				return instanceUUID();
			};

		auto pickingOverride = [id, this, &passMaterialOverride, vertexClass, vertexType]
			{
				JUUID pickMaterialUUID = GetMaterialUUIDByName(pickingMaterialName);
				for (auto& pmo : passMaterialOverride)
				{
					if (pmo.renderPass == renderPassTemplate->uuid())
					{
						pickMaterialUUID = pmo.material;
						break;
					}
				}
				MaterialInstanceID instanceUUID = pickMaterialUUID + "-" + vertexType;
				CreateMaterialInstance(instanceUUID(), [id, &instanceUUID, pickMaterialUUID, vertexClass]()
					{
						return std::make_unique<MaterialInstance>(id, instanceUUID(), pickMaterialUUID, vertexClass, false, false);
					}
				);
				return instanceUUID();
			};

		const std::map<RenderPassMaterialOverride, std::function<JUUID()>> overrideMaterial =
		{
			{ RenderPassMaterialOverride_None, noOverride },
			{ RenderPassMaterialOverride_ShadowMap, shadowMapOverride },
			{ RenderPassMaterialOverride_Picking, pickingOverride },
		};

		return overrideMaterial.at(materialOverride)();
	}

	void RenderPassInstance::InitRenderPass()
	{
		if (overridePass)
		{
			overridePass->Initialize();
		}
	}

	void RenderPassInstance::ResizeRelease()
	{
		switch (type)
		{
		case RenderPassType_SwapChainPass:
		{
			swapChainPass->ReleaseResources();
		}
		break;
		case RenderPassType_RenderToTexturePass:
		{
			renderToTexturePass->ReleaseResources();
		}
		break;
		}
	}

	void RenderPassInstance::Resize(unsigned int width, unsigned int height)
	{
		switch (type)
		{
		case RenderPassType_SwapChainPass:
		{
			swapChainPass->Resize(width, height);
		}
		break;
		case RenderPassType_RenderToTexturePass:
		{
			renderToTexturePass->Resize(width, height);
		}
		break;
		}
	}

	void RenderPassInstance::CreateRenderTargets()
	{
		using namespace RenderPass;

		switch (type)
		{
		case RenderPassType_SwapChainPass:
		{
			swapChainPass = DeviceUtils::CreateSwapChainPass(GetRenderPassName(renderPassTemplate()), mainHeap, renderPassTemplate->depthStencilFormat());
		}
		break;
		case RenderPassType_RenderToTexturePass:
		{
			assert(width != 0U); assert(height != 0U);
			renderToTexturePass = DeviceUtils::CreateRenderToTexturePass(GetRenderPassName(renderPassTemplate()),
				renderPassTemplate->renderTargetFormats(), renderPassTemplate->depthStencilFormat(),
				width, height
			);
		}
		break;
		}
	}

	std::vector<DXGI_FORMAT> RenderPassInstance::GetRenderTargetsFormats()
	{
		std::vector<DXGI_FORMAT> formats;
		switch (type)
		{
		case RenderPassType_SwapChainPass:
		{
			formats.push_back(renderer->swapChainFormat);
		}
		break;
		case RenderPassType_RenderToTexturePass:
		{
			auto& renderToTexture = renderToTexturePass->renderToTexture;
			for (auto& rtt : renderToTexture)
			{
				formats.push_back(rtt->format);
			}
		}
		break;
		}
		return formats;
	}

	DXGI_FORMAT RenderPassInstance::GetDepthStencilFormat()
	{
		return (type == RenderPassType_SwapChainPass) ?
			swapChainPass->depthStencilFormat :
			renderToTexturePass->depthStencilFormat;
	}

	void RenderPassInstance::MarkForDelete()
	{
		renderPassesInstancesToDelete.insert(renderPassInstance);
	}
};