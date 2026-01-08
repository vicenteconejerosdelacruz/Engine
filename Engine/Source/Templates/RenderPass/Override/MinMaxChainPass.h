#pragma once

#include "OverridePass.h"

struct MinMaxChainPass : public OverridePass
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapChainGpuHandle1;
	CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapChainGpuHandle2;

	MinMaxChainPass(SceneUnitId id, JUUID cam, unsigned int rpI, JUUID rp);
	virtual ~MinMaxChainPass() {};
	virtual void Initialize();
	virtual void Pass(SceneUnitId unit);
	void Render(SceneUnitId id);
};

