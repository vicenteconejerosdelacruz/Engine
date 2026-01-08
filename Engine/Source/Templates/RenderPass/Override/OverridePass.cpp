#include "pch.h"
#include "OverridePass.h"
#include <JTemplate.h>
//#include <Scene.h>
#include <Renderer.h>
//#include <Material/Material.h>
//#include <Shader/Shader.h>
#include <Camera/Camera.h>
//#include <Mesh/Mesh.h>
#include <DeviceUtils/RootSignature/RootSignature.h>
#include <DeviceUtils/PipelineState/PipelineState.h>
#include <DeviceUtils/RenderPass/RenderToTexturePass.h>
//#include <RenderPass/RenderPass.h>

//extern std::unique_ptr<Renderer> renderer;

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

void OverridePass::CreateFsQuadResources(SceneUnitId id, std::string materialName, JUUID renderPassJson, std::function<void(std::string, ShaderConstantsBufferVariable&)> constantsBufferPusher)
{
	using namespace Scene;

	auto& fsQuadMesh = GetMeshInstance(id, GetMeshUUIDByName("decal"));
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

		auto& vsVars = fsQuadMat->vertexShaderInstanceUUID->constantsBuffersVariables;
		auto& psVars = fsQuadMat->pixelShaderInstanceUUID->constantsBuffersVariables;

		for (auto& [name, var] : vsVars) { constantsBufferPusher(name, var); }
		for (auto& [name, var] : psVars) { constantsBufferPusher(name, var); }
	}

	auto& miVS = fsQuadMat->vertexShaderInstanceUUID;
	auto& miPS = fsQuadMat->pixelShaderInstanceUUID;

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

	MaterialJsonUUID material = fsQuadMaterial();
	BlendDesc blendDesc = material->blendState();
	RasterizerDesc rasterizerDesc = material->rasterizerState();

	D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	std::string plName = "pipelineState:" + materialName;
	auto& renderPass = GetRenderPassTemplate(renderPassJson);
	auto rtf = renderPass->renderTargetFormats();
	auto df = renderPass->depthStencilFormat();

	pipelineState = CreateGraphicsPipelineState(plName, vsLayout, vsByteCode, psByteCode, rootSignature, blendDesc, rasterizerDesc, primitiveTopologyType, rtf, df);
}

JUUID OverridePass::GetPrevPassRenderToTexture(unsigned int index)
{
	using namespace Scene;
	auto& prevPass = camera->renderPassesUUID.at(renderPassIndex - 1);
	auto& rttPass = prevPass->renderToTexturePass;
	return rttPass->renderToTexture.at(index)();
}