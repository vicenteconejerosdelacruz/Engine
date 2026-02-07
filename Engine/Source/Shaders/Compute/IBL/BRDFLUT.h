#pragma once

#include "../ComputeInterface.h"
#include <DirectXMath.h>
#include <filesystem>
#include <wrl.h>
#include <wrl/client.h>
#include <atlbase.h>
#include <d3dx12.h>

namespace ComputeShader
{
	struct BRDFLUT : public ComputeInterface
	{
		const unsigned int faceWidth = 128U;
		const unsigned int faceHeight = 128U;
		const unsigned int pixelSize = 4U * sizeof(float);
		const unsigned int faceSize = faceWidth * faceHeight * pixelSize;
		const unsigned int dataSize = static_cast<unsigned int>(faceSize);
		const DXGI_FORMAT dataFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

		std::filesystem::path outputFile;

		D3D12_RESOURCE_DESC resourceDesc;
		CComPtr<ID3D12Resource> resource;
		CD3DX12_CPU_DESCRIPTOR_HANDLE resultCpuHandle; //UAV, (U0) 
		CD3DX12_GPU_DESCRIPTOR_HANDLE resultGpuHandle; //UAV, (U0)

		CComPtr<ID3D12Resource> readBackResource;

		BRDFLUT(std::filesystem::path iblBRDFLUTPath);
		~BRDFLUT();

		virtual void Compute(SceneUnitId unit);
		virtual void Solution(SceneUnitId unit);
		void WriteFile(XMFLOAT4* data) const;
	};
}