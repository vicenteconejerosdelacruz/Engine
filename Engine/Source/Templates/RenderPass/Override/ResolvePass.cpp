#include "pch.h"
#include "ResolvePass.h"
#include <Scene.h>
#include <SceneObject.h>
#include <DeviceUtils/RenderToTexture/RenderToTexture.h>
#include <DeviceUtils/RenderPass/SwapChainPass.h>

namespace Templates
{
	ResolvePass::ResolvePass(CameraID cam, unsigned int rpI, RenderPassJsonID rpT, RenderPassInstanceID rp) : OverridePass(cam, rpI, rpT, rp)
	{
		CreatePrevPassDependentResources();
	}

	void ResolvePass::CreatePrevPassDependentResources()
	{
		if (renderPassTemplate->usePrevPassTexture())
		{
			auto prevPassJ = GetPrevRenderPassTemplate();
			mode = ResolveMode_CopyFromRenderToTexture;

			if (prevPassJ->renderTargetFormats().at(0) == DXGI_FORMAT_R8G8B8A8_UNORM)
			{
				mode = ResolveMode_FullScreenQuad;
			}
		}
		else
		{
			mode = ResolveMode_CopyFromRenderToTexture;
		}

		if (mode == ResolveMode_CopyFromRenderToTexture)
		{
			CreateFsQuadResources(camera.unit(), "FullScreenQuad", renderPassTemplate(), [this](std::string name, ShaderConstantsBufferVariable& var)
				{
					auto& fsCB = fsQuadConstantsBuffer;

					for (unsigned int n = 0; n < JRenderer::numFrames; n++)
					{
						if (name == "alpha")
						{
							float data = 1.0f;
							fsCB->push(data, n, var.offset);
						}
					}
				}
			);
		}
	}

	void ResolvePass::Pass(SceneUnitId unit)
	{
		auto& swapChain = renderPassInstance->swapChainPass;
		swapChain->BeginRenderPass(unit, swapChain->depthStencilViewDescriptorHeap, clearRTV);
		if (mode == ResolveMode_FullScreenQuad)
		{
			swapChain->CopyFromRenderToTexture(unit, GetPrevPassRenderToTexture());
		}
		else
		{
			Render(unit);
		}
		swapChain->EndRenderPass(unit);
	}

	void ResolvePass::Render(SceneUnitId unit)
	{
		using namespace Scene;
		auto& scene = GetSceneUnit(unit);
		auto& commandList = scene->GetCommandList();
		auto& fsCB = fsQuadConstantsBuffer;
		auto& fsQuadMesh = GetMeshInstance(fsQuad);
		auto& prevPassRTT = renderPassTemplate->usePrevPassTexture() ? GetRenderToTexture(GetPrevPassRenderToTexture()) : GetRenderToTexture(rt_texture());

#if defined(_DEVELOPMENT)
		PIXBeginEvent(commandList.p, 0, "ResolvePassQuad");
#endif

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->SetGraphicsRootSignature(rootSignature);
		commandList->SetPipelineState(pipelineState);

		commandList->SetGraphicsRootDescriptorTable(0, fsCB->gpu_xhandle.at(scene->Frame()));
		commandList->SetGraphicsRootDescriptorTable(1, prevPassRTT->gpuTextureHandle);

		commandList->IASetVertexBuffers(0, 1, &fsQuadMesh->vbvData.vertexBufferView);
		commandList->IASetIndexBuffer(&fsQuadMesh->ibvData.indexBufferView);
		commandList->DrawIndexedInstanced(fsQuadMesh->ibvData.indexBufferView.SizeInBytes / sizeof(unsigned int), 1, 0, 0, 0);

#if defined(_DEVELOPMENT)
		PIXEndEvent(commandList.p);
#endif
	}
}
