#include "pch.h"
#include "MinMaxChainPass.h"
//#include <Renderer.h>
//#include <Material/Material.h>
//#include <Shader/Shader.h>
//#include <Mesh/Mesh.h>
//#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
//#include <RenderPass/RenderPass.h>

//extern std::unique_ptr<Renderer> renderer;

MinMaxChainPass::MinMaxChainPass(JUUID cam, unsigned int rpI, JUUID rp) : OverridePass(cam, rpI, rp)
{
}

void MinMaxChainPass::Initialize()
{
	/*using namespace DeviceUtils;

	JUUID renderPassTemplateUUID = renderPassInstance->renderPassJson();
	RenderToTexturePassUUID rttPass = renderPassInstance->renderToTexturePass;
	XMFLOAT2 texelInvSize = {
		1.0f / rttPass->screenViewport.Width,
		1.0f / rttPass->screenViewport.Height
	};

	CreateFsQuadResources("DepthMinMax", renderPassTemplateUUID,
		[this, texelInvSize](std::string name, ShaderConstantsBufferVariable& var)
		{
			auto& fsCB = fsQuadConstantsBuffer;

			for (unsigned int n = 0; n < renderer->numFrames; n++)
			{
				if (name == "texelInvSize")
				{
					fsCB->push(texelInvSize, n, var.offset);
				}
			}
		}
	);*/
}

void MinMaxChainPass::Pass(SceneUnitId unit)
{
	/*RenderPassInstanceUUID renderPass = renderPassInstance;
	RenderToTexturePassUUID rttPass = renderPass->renderToTexturePass;
	rttPass->BeginRenderPass();
	Render();
	rttPass->EndRenderPass();*/
}

void MinMaxChainPass::Render()
{
	/*auto& commandList = renderer->commandList;
	auto& fsCB = fsQuadConstantsBuffer;
	auto& fsQuadMesh = GetMeshInstance(fsQuad);

#if defined(_DEVELOPMENT)
	PIXBeginEvent(commandList.p, 0, "MinMaxChainPassQuad");
#endif

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->SetPipelineState(pipelineState);

	commandList->SetGraphicsRootDescriptorTable(0, fsCB->gpu_xhandle.at(renderer->backBufferIndex));
	commandList->SetGraphicsRootDescriptorTable(1, shadowMapChainGpuHandle1);
	commandList->SetGraphicsRootDescriptorTable(2, shadowMapChainGpuHandle2);

	commandList->IASetVertexBuffers(0, 1, &fsQuadMesh->vbvData.vertexBufferView);
	commandList->IASetIndexBuffer(&fsQuadMesh->ibvData.indexBufferView);
	commandList->DrawIndexedInstanced(fsQuadMesh->ibvData.indexBufferView.SizeInBytes / sizeof(unsigned int), 1, 0, 0, 0);

#if defined(_DEVELOPMENT)
	PIXEndEvent(commandList.p);
#endif*/
}
