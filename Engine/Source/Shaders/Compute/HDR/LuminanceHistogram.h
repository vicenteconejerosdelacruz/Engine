#pragma once
#include "../ComputeInterface.h"
#include <wrl.h>
#include <wrl/client.h>
#include <atlbase.h>
#include <d3dx12.h>
#include <DeviceUtils/DescriptorHeap/DescriptorHeap.h>

namespace ComputeShader
{
	struct LuminanceHistogramBuffer
	{
		unsigned int inputWidth;
		unsigned int inputHeight;
		float minLogLuminance;
		float oneOverLogLuminanceRange;
	};

	struct LuminanceHistogram : public ComputeInterface
	{
		//histogram
		JUUID rttUUID; // HDR BaseTexture, (T0)
		CComPtr<ID3D12Resource> resource; //LuminanceHistogram (U0)
		ConstantsBufferID constantsBuffers; //LuminanceHistogramBuffer CBV (C0)
		CD3DX12_CPU_DESCRIPTOR_HANDLE resultCpuHandle;	//UAV, (U0) 
		CD3DX12_GPU_DESCRIPTOR_HANDLE resultGpuHandle; //UAV, (U0)

		//UAV Clearing
		std::unique_ptr<DeviceUtils::DescriptorHeap> resultClearHeap; //UAV (U0)
		CD3DX12_CPU_DESCRIPTOR_HANDLE resultClearCpuHandle; //UAV (U0)

		LuminanceHistogram(JUUID RenderToTextureID);
		~LuminanceHistogram();

		void UpdateLuminanceHistogramParams(unsigned int width, unsigned int height, float minLogLuminance, float maxLogLuminance) const;

		virtual void Compute(SceneUnitId unit);
		virtual void Solution(SceneUnitId unit) {};
	};
}

