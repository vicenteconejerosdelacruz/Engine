#pragma once
#include "../ComputeInterface.h"
#include <filesystem>
#include <DirectXTex.h>
#include <d3dx12.h>
#include <wrl.h>
#include <wrl/client.h>
#include <atlbase.h>

namespace ComputeShader
{
	struct PreFilteredEnvironmentMap : public ComputeInterface
	{
		unsigned int faceWidth = 0U;
		unsigned int faceHeight = 0U;
		unsigned int numMipMaps = 0U;
		unsigned int numFaces = 6U;
		const unsigned int pixelSize = 4U * sizeof(float);
		const DXGI_FORMAT dataFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

		std::filesystem::path outputFile;

		JUUID envMap;
		CD3DX12_CPU_DESCRIPTOR_HANDLE envMapCubeCpuHandle; //SRV, (T0)
		CD3DX12_GPU_DESCRIPTOR_HANDLE envMapCubeGpuHandle; //SRV, (T0)

		std::vector<D3D12_RESOURCE_DESC> resourcesDesc;
		std::vector<CComPtr<ID3D12Resource>> resources;
		std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> mipsResultsCpuHandle; //UAV, (U0) 
		std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> mipsResultsGpuHandle; //UAV, (U0)
		std::vector<JUUID> mipsResultsCB; //CBV, (C0)

		std::vector<size_t> readBackSizes;
		std::vector<CComPtr<ID3D12Resource>> readBackResources;

		PreFilteredEnvironmentMap(JUUID envMapUUID, std::filesystem::path iblPreFilteredEnvironmentMapFile);
		~PreFilteredEnvironmentMap();

		virtual void Compute(SceneUnitId unit);
		virtual void Solution(SceneUnitId unit);
		void WriteFile(std::vector<Image>& imgs) const;
	};
};

