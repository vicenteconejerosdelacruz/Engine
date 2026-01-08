#pragma once

#include "OverridePass.h"

struct MinMaxChainResultPass : public OverridePass
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE depthGpuHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapChainGpuHandle1;
	CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapChainGpuHandle2;

	MinMaxChainResultPass(SceneUnitId id, JUUID cam, unsigned int rpI, JUUID rp);
	virtual ~MinMaxChainResultPass() {};
	void CreateFSQuad(std::string material);
	virtual void Pass(SceneUnitId unit);
	void Render(SceneUnitId id);
};


