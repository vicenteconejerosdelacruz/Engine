#pragma once

#include "OverridePass.h"

struct MinMaxChainResultPass : public OverridePass
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE depthGpuHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapChainGpuHandle1;
	CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapChainGpuHandle2;

	MinMaxChainResultPass(SceneUnitId id, JUUID cam, unsigned int rpI, JUUID rpT, JUUID rp);
	virtual ~MinMaxChainResultPass() {};
	virtual void CreatePrevPassDependentResources();
	virtual void Pass(SceneUnitId unit);
	void CreateFSQuad(std::string material);
	void Render(SceneUnitId id);

	std::string materialName;
};


