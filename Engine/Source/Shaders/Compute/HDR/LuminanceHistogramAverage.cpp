#include "pch.h"
#include "LuminanceHistogramAverage.h"

#include <Renderer.h>
#include <DeviceUtils/Resources/Resources.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <DirectXHelper.h>

extern std::unique_ptr<JRenderer> renderer;

using namespace DeviceUtils;

namespace ComputeShader
{
	LuminanceHistogramAverage::LuminanceHistogramAverage(
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle
	) : ComputeInterface("LuminanceHistogramAverage_cs")
	{
		//cpu/gpu handles of the previously computed histogram, this is an UAV of 256 uints
		histogramCpuHandle = cpuHandle;
		histogramGpuHandle = gpuHandle;

		//create the luminicance histogram buffer containing the calculation parameters (C0)
		constantsBuffers = CreateConstantsBuffer(sizeof(LuminanceHistogramAverageBuffer), JRenderer::numFrames, "LuminanceHistogramAverageBuffer");

		//create the uav resource for the calculation results, this is a single float
		unsigned int dataSize = static_cast<unsigned int>(sizeof(float));
		D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		renderer->d3dDevice->CreateCommittedResource(
			&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&average)
		);
		CCNAME_D3D12_OBJECT_N(average, std::string("LuminanceHistogramAverage"));
		LogCComPtrAddress("LuminanceHistogramAverage", average);

		//create a uav desc/view for writing to a float
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDescWrite = {
			.Format = DXGI_FORMAT_UNKNOWN, .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
			.Buffer = {
				.FirstElement = 0ULL, .NumElements = 1U, .StructureByteStride = sizeof(float),
				.CounterOffsetInBytes = 0ULL, .Flags = D3D12_BUFFER_UAV_FLAG_NONE
			}
		};
		DeviceUtils::AllocCSUDescriptor(averageCpuHandle, averageGpuHandle);
		renderer->d3dDevice->CreateUnorderedAccessView(average, nullptr, &uavDescWrite, averageCpuHandle);

		//createa srv desc/view for reading the float
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDescRead = {
			.Format = DXGI_FORMAT_UNKNOWN, .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			.Buffer = {
				.FirstElement = 0, .NumElements = 1U, .StructureByteStride = sizeof(float), .Flags = D3D12_BUFFER_SRV_FLAG_NONE
			}
		};
		DeviceUtils::AllocCSUDescriptor(averageReadCpuHandle, averageReadGpuHandle);
		renderer->d3dDevice->CreateShaderResourceView(average, &srvDescRead, averageReadCpuHandle);
	}

	LuminanceHistogramAverage::~LuminanceHistogramAverage()
	{
		DestroyConstantsBuffer(constantsBuffers);
		FreeCSUDescriptor(averageCpuHandle, averageGpuHandle);
		average = nullptr;
	}

	void LuminanceHistogramAverage::UpdateLuminanceHistogramAverageParams(unsigned int pixelCount, float minLogLuminance, float maxLogLuminance, float timeDelta, float tau) const
	{
		LuminanceHistogramAverageBuffer params
		{
			.pixelCount = pixelCount,
			.minLogLuminance = minLogLuminance,
			.logLuminanceRange = (maxLogLuminance - minLogLuminance),
			.timeDelta = timeDelta,
			.tau = tau
		};
		constantsBuffers->push(params, 0);
	}

	void LuminanceHistogramAverage::Compute(SceneUnitId unit)
	{
		auto& scene = GetSceneUnit(unit);
		CComPtr<ID3D12GraphicsCommandList2>& commandList = scene->GetComputeCommandList();

#if defined(_DEVELOPMENT)
		{
			PIXScopedEvent(commandList.p, 0, L"LuminanceHistogramAverage Compute");
#endif

			//after clearing the uav we can compute
			shader.SetComputeState(unit);

			commandList->SetComputeRootDescriptorTable(0, constantsBuffers->gpu_xhandle[0]);
			commandList->SetComputeRootDescriptorTable(1, histogramGpuHandle);
			commandList->SetComputeRootDescriptorTable(2, averageGpuHandle);
			commandList->Dispatch(1, 1, 1);

#if defined(_DEVELOPMENT)
		}
#endif
	}
}