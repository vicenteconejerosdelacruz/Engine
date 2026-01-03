#include "pch.h"
#include "ToneMappingPass.h"
//#include <Renderer.h>
//#include <StepTimer.h>
//#include <DeviceUtils/Resources/Resources.h>
//#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
//#include <DeviceUtils/RenderToTexture/RenderToTexture.h>
//#include <RenderPass/RenderPass.h>
//#include <Camera/Camera.h>
//#include <HDR/LuminanceHistogram.h>
//#include <HDR/LuminanceHistogramAverage.h>
//#include <Mesh/Mesh.h>
//#if defined(_EDITOR)
//#include <Editor.h>
//#endif

//extern std::unique_ptr<Renderer> renderer;

ToneMappingPass::ToneMappingPass(JUUID cam, unsigned int rpI, JUUID rp) : OverridePass(cam, rpI, rp)
{
	//using namespace Scene;

	//JUUID prevPassRTTUUID = GetPrevPassRenderToTexture();
	//auto& prevPassRTT = GetRenderToTexture(prevPassRTTUUID);
	//auto& camera = GetCameraSceneObject(cam);

	//hdrHistogram = std::make_unique<ComputeShader::LuminanceHistogram>(prevPassRTTUUID);
	//hdrHistogram->UpdateLuminanceHistogramParams(prevPassRTT->width, prevPassRTT->height, camera->minLogLuminance(), camera->maxLogLuminance());

	//luminanceHistogramAverage = std::make_unique<ComputeShader::LuminanceHistogramAverage>(hdrHistogram->resultCpuHandle, hdrHistogram->resultGpuHandle);
	//luminanceHistogramAverage->UpdateLuminanceHistogramAverageParams(prevPassRTT->width * prevPassRTT->height,
	//	camera->minLogLuminance(), camera->maxLogLuminance(), 0.016f, camera->tau()
	//);

	//CreateFsQuadResources("ToneMap", camera->renderPasses().at(rpI));
}

ToneMappingPass::~ToneMappingPass()
{
	//hdrHistogram = nullptr;
	//luminanceHistogramAverage = nullptr;
}

//extern DX::StepTimer timer;
void ToneMappingPass::Pass(SceneUnitId unit)
{
	//	using namespace Scene;
	//	auto& prevPassRTT = GetRenderToTexture(GetPrevPassRenderToTexture());
	//	CameraUUID cam = camera;
	//
	//	float dt = static_cast<float>(timer.GetElapsedSeconds());
	//#if defined(_EDITOR)
	//	if (!Editor::IsPlaying() || Editor::IsPaused())
	//		dt = 0.0f;
	//#endif
	//
	//	//update the histogram and & luminance average parameters
	//	hdrHistogram->UpdateLuminanceHistogramParams(prevPassRTT->width, prevPassRTT->height, cam->minLogLuminance(), cam->maxLogLuminance());
	//	luminanceHistogramAverage->UpdateLuminanceHistogramAverageParams(prevPassRTT->width * prevPassRTT->height,
	//		cam->minLogLuminance(), cam->maxLogLuminance(), dt, cam->tau());
	//
	//	//run the compute shaders for hdr histogram & luminance average calculation
	//	hdrHistogram->Compute();
	//	luminanceHistogramAverage->Compute();
	//
	//	//render the scene using the applied luminane correction
	//	auto& rttPass = renderPassInstance->renderToTexturePass;
	//	rttPass->BeginRenderPass();
	//	Render();
	//	rttPass->EndRenderPass();
}

void ToneMappingPass::Render()
{
	//	using namespace Scene;
	//	using namespace DeviceUtils;
	//
	//	auto& commandList = renderer->commandList;
	//	auto& prevPassRTT = GetRenderToTexture(GetPrevPassRenderToTexture());
	//	auto& fsQuadMesh = GetMeshInstance(fsQuad);
	//
	//#if defined(_DEVELOPMENT)
	//	PIXBeginEvent(commandList.p, 0, "ResolvePassQuad");
	//#endif
	//
	//	DeviceUtils::UAVResource(commandList, luminanceHistogramAverage->average);
	//	DeviceUtils::TransitionResource(commandList, luminanceHistogramAverage->average,
	//		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS
	//	);
	//
	//	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//	commandList->SetGraphicsRootSignature(rootSignature);
	//	commandList->SetPipelineState(pipelineState);
	//
	//	//commandList->SetGraphicsRootDescriptorTable(0, fsQuadConstantsBuffer->gpu_xhandle.at(renderer->backBufferIndex));
	//	unsigned int camSlot = 0U;
	//	camera->cameraCb->SetRootDescriptorTable(commandList, camSlot, renderer->backBufferIndex);
	//	commandList->SetGraphicsRootDescriptorTable(1, prevPassRTT->gpuTextureHandle);
	//	commandList->SetGraphicsRootDescriptorTable(2, luminanceHistogramAverage->averageReadGpuHandle);
	//
	//	commandList->IASetVertexBuffers(0, 1, &fsQuadMesh->vbvData.vertexBufferView);
	//	commandList->IASetIndexBuffer(&fsQuadMesh->ibvData.indexBufferView);
	//	commandList->DrawIndexedInstanced(fsQuadMesh->ibvData.indexBufferView.SizeInBytes / sizeof(unsigned int), 1, 0, 0, 0);
	//
	//	DeviceUtils::TransitionResource(commandList, luminanceHistogramAverage->average,
	//		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON
	//	);
	//
	//#if defined(_DEVELOPMENT)
	//	PIXEndEvent(commandList.p);
	//#endif
}
