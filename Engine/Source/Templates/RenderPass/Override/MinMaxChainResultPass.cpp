#include "pch.h"
#include "MinMaxChainResultPass.h"
//#include <Renderer.h>
//#include <Material/Material.h>
//#include <Shader/Shader.h>
//#include <Mesh/Mesh.h>
//#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
//#include <RenderPass/RenderPass.h>

//extern std::unique_ptr<Renderer> renderer;

MinMaxChainResultPass::MinMaxChainResultPass(JUUID cam, unsigned int rpI, JUUID rp) : OverridePass(cam, rpI, rp)
{
}

void MinMaxChainResultPass::CreateFSQuad(std::string materialName)
{
	/*using namespace DeviceUtils;

	auto& renderPassI = renderPassInstance;
	JUUID renderPassTemplateUUID = renderPassI->renderPassJson();
	CreateFsQuadResources(materialName, renderPassTemplateUUID);*/
}

void MinMaxChainResultPass::Pass(SceneUnitId unit)
{
	/*auto& renderPass = renderPassInstance;
	RenderToTexturePassUUID rttPass = renderPass->renderToTexturePass;
	rttPass->BeginRenderPass();
	Render();
	rttPass->EndRenderPass();*/
}

void MinMaxChainResultPass::Render()
{
	/*auto& commandList = renderer->commandList;
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
#endif*/
}
