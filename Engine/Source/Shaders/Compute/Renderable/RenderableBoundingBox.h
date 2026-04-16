#pragma once
#include <SimpleMath.h>
#include <vector>
#include <atlbase.h>
#include <wrl/client.h>
#include <d3dx12.h>
#include <DirectXHelper.h>
#include <UUID.h>
#include "../ComputeInterface.h"

namespace ComputeShader
{
	struct RenderableBoundingBox : public ComputeInterface
	{
		bool canCompute;
		bool hasSolution;
		unsigned int solutionCounter = JRenderer::numFrames + 1;
		ConstantsBufferID bonesCbv;
		BoundingBox boundingBox;

		//Animable/Compute Shader stuff
		std::vector<CComPtr<ID3D12Resource>> resources;
		std::vector<CComPtr<ID3D12Resource>> readBackResources;
		std::vector<ConstantsBufferID> constantsBuffers; //CBV, 0
		std::vector<::CD3DX12_CPU_DESCRIPTOR_HANDLE> verticesCpuHandles; //SRV, 3
		std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> verticesGpuHandles;	//SRV, 3
		std::vector<::CD3DX12_CPU_DESCRIPTOR_HANDLE> resultCpuHandle;	//UAV, 2
		std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> resultGpuHandle; //UAV, 2

		RenderableBoundingBox(RenderableID renderable);
		~RenderableBoundingBox();

		virtual void Compute(SceneUnitId unit);
		virtual void Solution(SceneUnitId unit);
	};

	std::unique_ptr<RenderableBoundingBox>& GetRenderableBoundingBox(JUUID compUUID);
	DEF_TEMPLATE_ID(RenderableBoundingBox, GetRenderableBoundingBox);

	JUUID CreateRenderableBoundingBox(RenderableID renderable);
	void DeleteRenderableBoundingBox(JUUID compUUID);
};

DEF_TEMPLATE_ID_HASH(RenderableBoundingBox);
