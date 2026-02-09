#include "pch.h"
#include "MinMaxChainResultPass.h"

MinMaxChainResultPass::MinMaxChainResultPass(CameraID cam, unsigned int rpI, RenderPassJsonID rpT, RenderPassInstanceID rp) : OverridePass(cam, rpI, rpT, rp)
{
}

void MinMaxChainResultPass::CreatePrevPassDependentResources()
{
	CreateFsQuadResources(camera.unit(), materialName, renderPassTemplate());
}

void MinMaxChainResultPass::CreateFSQuad(std::string materialName)
{
	this->materialName = materialName;
	CreatePrevPassDependentResources();
}

void MinMaxChainResultPass::Pass(SceneUnitId unit)
{
	auto& renderPass = renderPassInstance;
	RenderToTexturePassID rttPass = renderPass->renderToTexturePass;
	rttPass->BeginRenderPass(unit);
	Render(unit);
	rttPass->EndRenderPass(unit);
}

void MinMaxChainResultPass::Render(SceneUnitId id)
{
	auto& scene = GetSceneUnit(id);
	auto& commandList = scene->GetCommandList();
	auto& fsQuadMesh = GetMeshInstance(fsQuad);

#if defined(_DEVELOPMENT)
	PIXBeginEvent(commandList.p, 0, "MinMaxChainResultPassQuad");
#endif

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->SetPipelineState(pipelineState);

	commandList->SetGraphicsRootDescriptorTable(0, depthGpuHandle);
	commandList->SetGraphicsRootDescriptorTable(1, shadowMapChainGpuHandle1);
	commandList->SetGraphicsRootDescriptorTable(2, shadowMapChainGpuHandle2);

	commandList->IASetVertexBuffers(0, 1, &fsQuadMesh->vbvData.vertexBufferView);
	commandList->IASetIndexBuffer(&fsQuadMesh->ibvData.indexBufferView);
	commandList->DrawIndexedInstanced(fsQuadMesh->ibvData.indexBufferView.SizeInBytes / sizeof(unsigned int), 1, 0, 0, 0);

#if defined(_DEVELOPMENT)
	PIXEndEvent(commandList.p);
#endif
}
