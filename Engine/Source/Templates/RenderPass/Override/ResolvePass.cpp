#include "pch.h"
#include "ResolvePass.h"
#include <Scene.h>
#include <SceneObject.h>
//#include <Renderer.h>
//#include <Material/Material.h>
//#include <Shader/Shader.h>
//#include <Camera/Camera.h>
//#include <Mesh/Mesh.h>
//#include <RenderPass/RenderPass.h>
#include <DeviceUtils/RenderToTexture/RenderToTexture.h>
#include <DeviceUtils/RenderPass/SwapChainPass.h>

//extern std::unique_ptr<Renderer> renderer;

ResolvePass::ResolvePass(SceneUnitId id, JUUID cam, unsigned int rpI, JUUID rpT, JUUID rp) : OverridePass(id, cam, rpI, rpT, rp)
{
	using namespace Scene;

	//auto& camSO = GetCameraSUSceneObject(id, cam);
	//CameraSUUUID camSO = MAKESUUUID(id, cam);
	//auto& prevPassJ = GetRenderPassTemplate(camera->renderPasses().at(rpI - 1));
	CreatePrevPassDependentResources();
}

void ResolvePass::CreatePrevPassDependentResources()
{
	auto prevPassJ = GetPrevRenderPassTemplate();
	mode = ResolveMode_CopyFromRenderToTexture;

	if (prevPassJ->renderTargetFormats().at(0) == DXGI_FORMAT_R8G8B8A8_UNORM)
	{
		mode = ResolveMode_FullScreenQuad;
	}

	if (mode == ResolveMode_CopyFromRenderToTexture)
	{
		CreateFsQuadResources(camera.unit(), "FullScreenQuad", renderPassTemplate(), [this](std::string name, ShaderConstantsBufferVariable& var)
			{
				auto& fsCB = fsQuadConstantsBuffer;

				for (unsigned int n = 0; n < Renderer::numFrames; n++)
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
	swapChain->BeginRenderPass(unit, swapChain->depthStencilViewDescriptorHeap);
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
	//auto& commandList = renderer->commandList;
	auto& scene = GetSceneUnit(unit);
	auto& commandList = scene->GetCommandList();
	auto& fsCB = fsQuadConstantsBuffer;
	auto& fsQuadMesh = GetMeshInstance(fsQuad);
	auto& prevPassRTT = GetRenderToTexture(GetPrevPassRenderToTexture());

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
