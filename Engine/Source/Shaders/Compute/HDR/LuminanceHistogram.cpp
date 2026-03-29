#include "pch.h"
#include "LuminanceHistogram.h"

#include <Renderer.h>
#include <DeviceUtils/Resources/Resources.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <DeviceUtils/RenderToTexture/RenderToTexture.h>
#include <DirectXHelper.h>

extern std::unique_ptr<JRenderer> renderer;

using namespace DeviceUtils;

namespace ComputeShader
{
	LuminanceHistogram::LuminanceHistogram(JUUID RenderToTextureID) : ComputeInterface("LuminanceHistogram_cs")
	{
		//hold a copy to the render to texture used for HDR rendering (T0)
		rttUUID = RenderToTextureID;

		//create the luminicance histogram buffer containing the calculation parameters (C0)
		constantsBuffers = CreateConstantsBuffer(sizeof(LuminanceHistogramBuffer), JRenderer::numFrames, "LuminanceHistogramBuffer");

		//create the uav resource for the calculation results, this is table of 256 unsigned ints (U0)
		unsigned int dataSize = static_cast<unsigned int>(sizeof(unsigned int[256]));
		D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		renderer->d3dDevice->CreateCommittedResource(
			&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resource)
		);
		CCNAME_D3D12_OBJECT_N(resource, std::string("LuminanceHistogram"));
		LogCComPtrAddress("LuminanceHistogram", resource);
		DeviceUtils::AllocCSUDescriptor(resultCpuHandle, resultGpuHandle);

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {
			.Format = DXGI_FORMAT_R32_TYPELESS, .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
			.Buffer = {
				.FirstElement = 0ULL, .NumElements = 256U, .StructureByteStride = 0,
				.CounterOffsetInBytes = 0ULL, .Flags = D3D12_BUFFER_UAV_FLAG_RAW
			}
		};
		renderer->d3dDevice->CreateUnorderedAccessView(resource, nullptr, &uavDesc, resultCpuHandle);

		//create a gpu handle to be able to clear the uav result
		resultClearHeap = std::make_unique<DeviceUtils::DescriptorHeap>();
		resultClearHeap->CreateDescriptorHeap(renderer->d3dDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		resultClearHeap->AllocCPUDescriptor(resultClearCpuHandle);
		renderer->d3dDevice->CreateUnorderedAccessView(resource, nullptr, &uavDesc, resultClearCpuHandle);
	}

	LuminanceHistogram::~LuminanceHistogram()
	{
		resultClearHeap->FreeCPUDescriptor(resultClearCpuHandle);
		resultClearHeap->DestroyDescriptorHeap();
		resultClearHeap = nullptr;
		DeviceUtils::FreeCSUDescriptor(resultCpuHandle, resultGpuHandle);
		resource = nullptr;
		DestroyConstantsBuffer(constantsBuffers);
	}

	//minLogLuminance, I use -10.0, and a max of 2.0, making the luminance range 12.0, and oneOverLogLuminanceRange = 1.0 / 12.0.
	void LuminanceHistogram::UpdateLuminanceHistogramParams(unsigned int width, unsigned int height, float minLogLuminance, float maxLogLuminance) const
	{
		LuminanceHistogramBuffer params
		{
			.inputWidth = width,
			.inputHeight = height,
			.minLogLuminance = minLogLuminance,
			.oneOverLogLuminanceRange = 1.0f / (maxLogLuminance - minLogLuminance)
		};
		constantsBuffers->push(params, 0);
	}

	void LuminanceHistogram::Compute(SceneUnitId unit)
	{
		auto& scene = GetSceneUnit(unit);
		CComPtr<ID3D12GraphicsCommandList2>& commandList = scene->GetComputeCommandList();

#if defined(_DEVELOPMENT)
		PIXBeginEvent(commandList.p, 0, L"LuminanceHistogram Compute");
#endif

		//Clear UAV
		unsigned int clearValue[] = { 0U,0U,0U,0U };
		commandList->ClearUnorderedAccessViewUint(resultGpuHandle, resultClearCpuHandle, resource, clearValue, 0, nullptr);

		//after clearing the uav we can compute
		shader.SetComputeState(unit);

		auto& rtt = GetRenderToTexture(rttUUID);

		commandList->SetComputeRootDescriptorTable(0, constantsBuffers->gpu_xhandle[0]);
		commandList->SetComputeRootDescriptorTable(1, resultGpuHandle);
		commandList->SetComputeRootDescriptorTable(2, rtt->gpuTextureHandle);
		unsigned int numDispatchX = (rtt->width / 16U) + ((rtt->width % 16U) ? 1U : 0U);
		unsigned int numDispatchY = (rtt->height / 16U) + ((rtt->height % 16U) ? 1U : 0U);
		commandList->Dispatch(numDispatchX, numDispatchY, 1);

#if defined(_DEVELOPMENT)
		PIXEndEvent(commandList.p);
#endif
	}
}
