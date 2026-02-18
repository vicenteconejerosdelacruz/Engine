#include "pch.h"
#include "OverridePass.h"
#include <JTemplate.h>
#include <Renderer.h>
#include <Camera/Camera.h>
#include <DeviceUtils/RootSignature/RootSignature.h>
#include <DeviceUtils/PipelineState/PipelineState.h>
#include <DeviceUtils/RenderPass/RenderToTexturePass.h>

namespace Templates
{
	OverridePass::~OverridePass()
	{
		if (!fsQuad.empty() && MeshInstanceExists(fsQuad))
		{
			DestroyMeshInstance(fsQuad);
		}
		if (!fsQuadMaterial.empty())
		{
			DestroyMaterialInstance(fsQuadMaterial());
		}
		if (!fsQuadConstantsBuffer.empty())
		{
			DestroyConstantsBuffer(fsQuadConstantsBuffer());
		}
	}

	void OverridePass::CreateFsQuadResources(SceneUnitId id, std::string materialName, JUUID renderPassTemplate, std::function<void(std::string, ShaderConstantsBufferVariable&)> constantsBufferPusher)
	{
		using namespace Scene;

		nlohmann::json decalMeshJson = GetMeshJsonByName("decal");
		auto& fsQuadMesh = GetMeshInstance(id, decalMeshJson);
		fsQuad = fsQuadMesh->uuid;
		fsQuadMaterial = GetMaterialUUIDByName(materialName);
		VertexClass vertexClass = fsQuadMesh->vertexClass;
		CreateMaterialInstance(fsQuadMaterial(), [this, id, vertexClass]()
			{
				return std::make_unique<MaterialInstance>(id, fsQuadMaterial(), fsQuadMaterial(), vertexClass, false, false);
			}
		);
		auto& fsQuadMat = fsQuadMaterial;

		if (fsQuadMat->variablesBufferSize.size() > 0ULL)
		{
			size_t size = fsQuadMat->variablesBufferSize.at(0);
			fsQuadConstantsBuffer = CreateConstantsBuffer(size, Renderer::numFrames, materialName + ":cbv");

			auto& vsVars = fsQuadMat->vertexShaderInstanceID->constantsBuffersVariables;
			auto& psVars = fsQuadMat->pixelShaderInstanceID->constantsBuffersVariables;

			for (auto& [name, var] : vsVars) { constantsBufferPusher(name, var); }
			for (auto& [name, var] : psVars) { constantsBufferPusher(name, var); }
		}

		auto& miVS = fsQuadMat->vertexShaderInstanceID;
		auto& miPS = fsQuadMat->pixelShaderInstanceID;

		auto& vsCBparams = miVS->constantsBuffersParameters;
		auto& psCBparams = miPS->constantsBuffersParameters;
		auto& uavParams = miPS->uavParameters;
		auto& psSRVCSparams = miPS->srvCSParameters;
		auto& psSRVTexparams = miPS->srvTexParameters;
		auto& psSamplersParams = miPS->samplersParameters;
		auto& samplers = fsQuadMat->samplers;

		std::string rsName = "rootSignature:" + materialName;
		rootSignature = CreateRootSignature(rsName, vsCBparams, psCBparams, uavParams, psSRVCSparams, psSRVTexparams, psSamplersParams, samplers);

		auto& vsLayout = vertexInputLayoutsMap[vertexClass];
		auto& vsByteCode = miVS->byteCode;
		auto& psByteCode = miPS->byteCode;

		MaterialJsonID material = fsQuadMaterial();
		BlendDesc blendDesc = material->blendState();
		RasterizerDesc rasterizerDesc = material->rasterizerState();

		D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		std::string plName = "pipelineState:" + materialName;
		auto& renderPass = GetRenderPassTemplate(renderPassTemplate);
		auto rtf = renderPass->renderTargetFormats();
		auto df = renderPass->depthStencilFormat();

		pipelineState = CreateGraphicsPipelineState(plName, vsLayout, vsByteCode, psByteCode, rootSignature, blendDesc, rasterizerDesc, primitiveTopologyType, rtf, df);
	}

	RenderPassInstanceID OverridePass::GetPrevRenderPass()
	{
		if (renderPassIndex == 0U) return JUUID();
		return camera->renderPassesUUID.at(renderPassIndex - 1);
	}

	RenderPassJsonID OverridePass::GetPrevRenderPassTemplate()
	{
		if (renderPassIndex == 0U) return JUUID();
		return GetPrevRenderPass()->renderPassTemplate;
	}

	JUUID OverridePass::GetPrevPassRenderToTexture(unsigned int index)
	{
		return GetPrevRenderPass()->renderToTexturePass->renderToTexture.at(index)();
	}
}