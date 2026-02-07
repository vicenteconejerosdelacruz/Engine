#pragma once
#include <DirectXCollision.h>
#include <vector>
#include <atlbase.h>
#include <wrl/client.h>
#include <d3dx12.h>
#include <DirectXHelper.h>
#include "../ComputeInterface.h"

namespace ComputeShader
{
	struct RenderableBoundingBox : public ComputeInterface
	{
		ConstantsBufferUUID bonesCbv;
		BoundingBox boundingBox;

		//Animable/Compute Shader stuff
		std::vector<CComPtr<ID3D12Resource>> resources;
		std::vector<CComPtr<ID3D12Resource>> readBackResources;
		std::vector<ConstantsBufferUUID> constantsBuffers; //CBV, 0
		std::vector<::CD3DX12_CPU_DESCRIPTOR_HANDLE> verticesCpuHandles; //SRV, 3
		std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> verticesGpuHandles;	//SRV, 3
		std::vector<::CD3DX12_CPU_DESCRIPTOR_HANDLE> resultCpuHandle;	//UAV, 2
		std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> resultGpuHandle; //UAV, 2

		RenderableBoundingBox(SceneUnitId id, JUUID renderableUUID);
		~RenderableBoundingBox();

		virtual void Compute(SceneUnitId unit);
		virtual void Solution(SceneUnitId unit);
	};

	JUUID CreateRenderableBoundingBox(RenderableSUUUID renderable);
	std::unique_ptr<RenderableBoundingBox>& GetRenderableBoundingBox(JUUID compUUID);
	void DeleteRenderableBoundingBox(JUUID compUUID);
}

