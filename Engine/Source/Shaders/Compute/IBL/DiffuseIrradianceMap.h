#pragma once
#include "../ComputeInterface.h"
#include <filesystem>
#include <wrl.h>
#include <wrl/client.h>
#include <atlbase.h>
#include <d3dx12.h>
#include <SimpleMath.h>

namespace ComputeShader
{
	using namespace Templates;

	struct DiffuseIrradianceMap : public ComputeInterface
	{
		const unsigned int faceWidth = 64U;
		const unsigned int faceHeight = 64U;
		const unsigned int numFaces = 6U;
		const unsigned int pixelSize = 4U * sizeof(float);
		const unsigned int faceSize = faceWidth * faceHeight * pixelSize;
		const unsigned int dataSize = static_cast<unsigned int>(faceSize * numFaces);
		const DXGI_FORMAT dataFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

		std::filesystem::path outputFile;

		JUUID envMap;
		CD3DX12_CPU_DESCRIPTOR_HANDLE envMapCubeCpuHandle; //SRV, (T0)
		CD3DX12_GPU_DESCRIPTOR_HANDLE envMapCubeGpuHandle; //SRV, (T0)

		D3D12_RESOURCE_DESC resourceDesc;
		CComPtr<ID3D12Resource> resource;
		CD3DX12_CPU_DESCRIPTOR_HANDLE resultCpuHandle; //UAV, (U0) 
		CD3DX12_GPU_DESCRIPTOR_HANDLE resultGpuHandle; //UAV, (U0)

		CComPtr<ID3D12Resource> readBackResource;

		DiffuseIrradianceMap(JUUID envMapTemplateUUID, std::filesystem::path iblDiffuseFile);
		~DiffuseIrradianceMap();

		virtual void Compute(SceneUnitId unit);
		virtual void Solution(SceneUnitId unit);
		void WriteFile(XMFLOAT4* data) const;
	};
};

