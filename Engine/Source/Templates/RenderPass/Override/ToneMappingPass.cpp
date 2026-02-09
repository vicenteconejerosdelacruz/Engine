#include "pch.h"
#include "ToneMappingPass.h"
#include <DeviceUtils/Resources/Resources.h>
#include <DeviceUtils/RenderToTexture/RenderToTexture.h>
#include <Camera/Camera.h>
#include <HDR/LuminanceHistogram.h>
#include <HDR/LuminanceHistogramAverage.h>

ToneMappingPass::ToneMappingPass(CameraID cam, unsigned int rpI, RenderPassJsonID rpT, RenderPassInstanceID rp) : OverridePass(cam, rpI, rpT, rp)
{
	CreatePrevPassDependentResources();
}

ToneMappingPass::~ToneMappingPass()
{
	hdrHistogram = nullptr;
	luminanceHistogramAverage = nullptr;
}

void ToneMappingPass::CreatePrevPassDependentResources()
{
	using namespace Scene;
	using namespace DeviceUtils;
	using namespace ComputeShader;

	JUUID prevPassRTTUUID = GetPrevPassRenderToTexture();
	auto& prevPassRTT = GetRenderToTexture(prevPassRTTUUID);

	hdrHistogram = std::make_unique<LuminanceHistogram>(prevPassRTTUUID);
	hdrHistogram->UpdateLuminanceHistogramParams(prevPassRTT->width, prevPassRTT->height, camera->minLogLuminance(), camera->maxLogLuminance());

	luminanceHistogramAverage = std::make_unique<LuminanceHistogramAverage>(hdrHistogram->resultCpuHandle, hdrHistogram->resultGpuHandle);
	luminanceHistogramAverage->UpdateLuminanceHistogramAverageParams(prevPassRTT->width * prevPassRTT->height,
		camera->minLogLuminance(), camera->maxLogLuminance(), 0.016f, camera->tau()
	);

	CreateFsQuadResources(camera.unit(), "ToneMap", renderPassTemplate());
}

extern DX::StepTimer timer;
void ToneMappingPass::Pass(SceneUnitId id)
{
	using namespace Scene;
	auto& prevPassRTT = GetRenderToTexture(GetPrevPassRenderToTexture());
	CameraID cam = camera;

	float dt = static_cast<float>(timer.GetElapsedSeconds());
	//not a solution, without dt the algorithm explodes in pure colors
	//#if defined(_EDITOR)
	//	if (!Editor::IsPlaying(id) || Editor::IsPaused(id))
	//		dt = 0.0f;
	//#endif

	//update the histogram and & luminance average parameters
	hdrHistogram->UpdateLuminanceHistogramParams(prevPassRTT->width, prevPassRTT->height, cam->minLogLuminance(), cam->maxLogLuminance());
	luminanceHistogramAverage->UpdateLuminanceHistogramAverageParams(prevPassRTT->width * prevPassRTT->height,
		cam->minLogLuminance(), cam->maxLogLuminance(), dt, cam->tau());

	//run the compute shaders for hdr histogram & luminance average calculation
	hdrHistogram->Compute(id);
	luminanceHistogramAverage->Compute(id);

	//render the scene using the applied luminane correction
	auto& rttPass = renderPassInstance->renderToTexturePass;
	rttPass->BeginRenderPass(id);
	Render(id);
	rttPass->EndRenderPass(id);
}

void ToneMappingPass::Render(SceneUnitId id)
{
	using namespace Scene;
	using namespace DeviceUtils;

	auto& scene = GetSceneUnit(id);
	auto& commandList = scene->GetCommandList();
	auto& prevPassRTT = GetRenderToTexture(GetPrevPassRenderToTexture());
	auto& fsQuadMesh = GetMeshInstance(fsQuad);

#if defined(_DEVELOPMENT)
	PIXBeginEvent(commandList.p, 0, "ResolvePassQuad");
#endif

	DeviceUtils::UAVResource(commandList, luminanceHistogramAverage->average);
	DeviceUtils::TransitionResource(commandList, luminanceHistogramAverage->average,
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS
	);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->SetPipelineState(pipelineState);

	unsigned int camSlot = 0U;
	camera->cameraCb->SetRootDescriptorTable(commandList, camSlot, scene->Frame());
	commandList->SetGraphicsRootDescriptorTable(1, prevPassRTT->gpuTextureHandle);
	commandList->SetGraphicsRootDescriptorTable(2, luminanceHistogramAverage->averageReadGpuHandle);

	commandList->IASetVertexBuffers(0, 1, &fsQuadMesh->vbvData.vertexBufferView);
	commandList->IASetIndexBuffer(&fsQuadMesh->ibvData.indexBufferView);
	commandList->DrawIndexedInstanced(fsQuadMesh->ibvData.indexBufferView.SizeInBytes / sizeof(unsigned int), 1, 0, 0, 0);

	DeviceUtils::TransitionResource(commandList, luminanceHistogramAverage->average,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON
	);

#if defined(_DEVELOPMENT)
	PIXEndEvent(commandList.p);
#endif
}
