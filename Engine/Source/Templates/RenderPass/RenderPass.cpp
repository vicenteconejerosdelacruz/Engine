#include "pch.h"
#include "RenderPass.h"
//#include <Templates.h>
//#include <TemplateDef.h>
#include <Renderer.h>
#include <DeviceUtils/DescriptorHeap/DescriptorHeap.h>
#include <DeviceUtils/RenderPass/SwapChainPass.h>
#include <DeviceUtils/RenderPass/RenderToTexturePass.h>
#include <SceneObject.h>
//#include <Material/Material.h>
//#include <Mesh/Mesh.h>
//#include <Camera/Camera.h>
#include "Override/OverridePass.h"
//#include "Override/ResolvePass.h"
//#include "Override/ToneMappingPass.h"
//#include "Override/MinMaxChainPass.h"
//#include "Override/MinMaxChainResultPass.h"

extern std::unique_ptr<Renderer> renderer;

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

		void ResizeRelease()
		{
			//using namespace Scene;
			//auto& cameras = GetWindowCameras();
			//for (auto& uuid : cameras)
			//{
			//	auto& cam = GetFromWindowCameras(uuid);
			//	cam->ResizeReleasePasses();
			//}
		}

		void Resize(unsigned int width, unsigned int height)
		{
			//using namespace Scene;
			//auto& cameras = GetWindowCameras();
			//for (auto& uuid : cameras)
			//{
			//	auto& cam = GetFromWindowCameras(uuid);
			//	cam->ResizePasses(width, height);
			//}
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

	TEMPDEF_FULL(RenderPass);
	TEMPDEF_REFTRACKER(RenderPass);

	JUUID CreateRenderPassInstance(SceneUnitId id, JUUID cameraUUID, JUUID renderPassTemplateUUID, unsigned int renderPassIndex, unsigned int width, unsigned int height)
	{
		RenderPassInstanceUUID rpiUUID = getUUID();
		CreateRenderPassInstance(
			renderPassTemplateUUID, rpiUUID(),
			[&]()
			{
				std::unique_ptr<RenderPassInstance> instance = std::make_unique<RenderPassInstance>(id, cameraUUID, renderPassTemplateUUID, rpiUUID(), renderPassIndex, width, height);
				return instance;
			}
		);
		rpiUUID->InitRenderPass();
		return rpiUUID();
	}

	RenderPassInstance::RenderPassInstance(SceneUnitId id, JUUID cameraUUID, JUUID renderPassTemplateUUID, JUUID renderPassInstanceUUID, unsigned int renderPassIndex, unsigned int width, unsigned int height)
	{
		using namespace RenderPass;
		camera = MAKESUUUID(id, cameraUUID);
		renderPassJson = renderPassTemplateUUID;

		type = renderPassJson->type();
		switch (renderPassJson->type())
		{
		case RenderPassType_SwapChainPass:
		{
			swapChainPass = DeviceUtils::CreateSwapChainPass(GetRenderPassName(renderPassTemplateUUID), mainHeap, renderPassJson->depthStencilFormat());
		}
		break;
		case RenderPassType_RenderToTexturePass:
		{
			assert(width != 0U); assert(height != 0U);
			renderToTexturePass = DeviceUtils::CreateRenderToTexturePass(GetRenderPassName(renderPassTemplateUUID),
				renderPassJson->renderTargetFormats(), renderPassJson->depthStencilFormat(),
				width, height
			);
		}
		break;
		}

		materialOverride = renderPassJson->materialOverride();
		renderCallbackOverride = renderPassJson->renderCallbackOverride();
		const std::map<RenderPassRenderCallbackOverride, std::function<
			std::unique_ptr<OverridePass>(
				JUUID camUUID,
				unsigned int,
				JUUID rpIUUID
			)>> RenderCallbackOverriders =
		{
			{ RenderPassRenderCallbackOverride_None, [](auto c,auto rpindex, auto rpInstance) { return nullptr; }},
			{ RenderPassRenderCallbackOverride_ToneMapping, [&](auto c, auto rpindex, auto rpInstance) { return std::make_unique<ToneMappingPass>(id,c,rpindex, rpInstance); } },
			{ RenderPassRenderCallbackOverride_Resolve, [&](auto c, auto rpindex, auto rpInstance) { return rpindex > 0 ? std::make_unique<ResolvePass>(id,c,rpindex, rpInstance) : nullptr; } },
			{ RenderPassRenderCallbackOverride_MinMaxChain, [&](auto c, auto rpindex, auto rpInstance) { return std::make_unique<MinMaxChainPass>(id,c,rpindex, rpInstance); } },
			{ RenderPassRenderCallbackOverride_MinMaxChainResult, [&](auto c, auto rpindex, auto rpInstance) { return std::make_unique<MinMaxChainResultPass>(id,c,rpindex, rpInstance); } }
		};
		overridePass = RenderCallbackOverriders.at(renderCallbackOverride)(cameraUUID, renderPassIndex, renderPassInstanceUUID);
		if (!camera.empty() && type == RenderPassType_RenderToTexturePass &&
			materialOverride == RenderPassMaterialOverride_None &&
			renderCallbackOverride == RenderPassRenderCallbackOverride_None &&
			camera->HasIBL())
		{
			camera->CreateIBLTextures();
		}
	}

	void DestroyRenderPassInstance(JUUID renderPassInstanceUUID)
	{
		if (renderPassInstanceUUID.empty()) return;

		DeleteRenderPassInstance(renderPassInstanceUUID);
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
		SceneUnitId id,
		MaterialJsonUUID material,
		MeshInstanceUUID mesh,
		bool shadowed,
		std::vector<PassMaterialOverride> passMaterialOverride,
		JUUID bindingUUID
		/*,
		JObjectChangeCallback materialChangeCallback,
		JObjectChangePostCallback materialChangePostCallback
		*/
	)
	{
		using namespace RenderPass;

		VertexClass vertexClass = mesh->vertexClass;
		std::string vertexType = VertexClassToString.at(vertexClass);
		bool hasIbl = camera.uuid().empty() ? false : camera->HasIBL();

		auto noOverride = [id, &material, vertexClass, vertexType, shadowed, hasIbl, bindingUUID/*, materialChangeCallback, materialChangePostCallback*/]()
			{
				MaterialInstanceUUID instanceUUID = material() + "-" + vertexType;
				CreateMaterialInstance(instanceUUID(), [id, &instanceUUID, &material, vertexClass, shadowed, hasIbl, bindingUUID/*, materialChangeCallback, materialChangePostCallback*/]()
					{
						return std::make_unique<MaterialInstance>(id, instanceUUID(), material(), vertexClass, shadowed, hasIbl, TextureShaderUsageMap(),
							bindingUUID/*, materialChangeCallback, materialChangePostCallback*/);
					}
				);
				return instanceUUID();
			};

		auto shadowMapOverride = [id, &material, vertexClass, vertexType]()
			{
				JUUID smMatUUID = GetMaterialUUIDByName(shadowMapMaterialName);
				MaterialInstanceUUID instanceUUID = smMatUUID + "-" + vertexType;
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
					if (pmo.renderPass == renderPassJson->uuid())
					{
						pickMaterialUUID = pmo.material;
						break;
					}
				}
				MaterialInstanceUUID instanceUUID = pickMaterialUUID + "-" + vertexType;
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
};