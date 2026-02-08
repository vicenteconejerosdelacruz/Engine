#include "pch.h"
#include "MinMaxChainPass.h"

MinMaxChainPass::MinMaxChainPass(SceneUnitId id, JUUID cam, unsigned int rpI, JUUID rpT, JUUID rp) : OverridePass(id, cam, rpI, rpT, rp)
{
}

void MinMaxChainPass::Initialize()
{
	CreatePrevPassDependentResources();
}

void MinMaxChainPass::CreatePrevPassDependentResources()
{
	using namespace DeviceUtils;

	JUUID renderPassTemplateUUID = renderPassInstance->renderPassTemplate();
	RenderToTexturePassID rttPass = renderPassInstance->renderToTexturePass;
	XMFLOAT2 texelInvSize = {
		1.0f / rttPass->screenViewport.Width,
		1.0f / rttPass->screenViewport.Height
	};

	CreateFsQuadResources(camera.unit(), "DepthMinMax", renderPassTemplateUUID,
		[this, texelInvSize](std::string name, ShaderConstantsBufferVariable& var)
		{
			auto& fsCB = fsQuadConstantsBuffer;

			for (unsigned int n = 0; n < Renderer::numFrames; n++)
			{
				if (name == "texelInvSize")
				{
					fsCB->push(texelInvSize, n, var.offset);
				}
			}
		}
	);
}

void MinMaxChainPass::Pass(SceneUnitId unit)
{
	RenderPassInstanceID renderPass = renderPassInstance;
	RenderToTexturePassID rttPass = renderPass->renderToTexturePass;
	rttPass->BeginRenderPass(unit);
	Render(unit);
	rttPass->EndRenderPass(unit);
}

void MinMaxChainPass::Render(SceneUnitId id)
{
	auto& scene = GetSceneUnit(id);
	auto& commandList = scene->GetCommandList();
	auto& fsCB = fsQuadConstantsBuffer;
	auto& fsQuadMesh = GetMeshInstance(fsQuad);

#if defined(_DEVELOPMENT)
	PIXBeginEvent(commandList.p, 0, "MinMaxChainPassQuad");
#endif

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->SetPipelineState(pipelineState);

	commandList->SetGraphicsRootDescriptorTable(0, fsCB->gpu_xhandle.at(scene->Frame()));
	commandList->SetGraphicsRootDescriptorTable(1, shadowMapChainGpuHandle1);
	commandList->SetGraphicsRootDescriptorTable(2, shadowMapChainGpuHandle2);

	commandList->IASetVertexBuffers(0, 1, &fsQuadMesh->vbvData.vertexBufferView);
	commandList->IASetIndexBuffer(&fsQuadMesh->ibvData.indexBufferView);
	commandList->DrawIndexedInstanced(fsQuadMesh->ibvData.indexBufferView.SizeInBytes / sizeof(unsigned int), 1, 0, 0, 0);

#if defined(_DEVELOPMENT)
	PIXEndEvent(commandList.p);
#endif
}
