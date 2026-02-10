#pragma once

#include "OverridePass.h"

namespace Templates
{
	struct MinMaxChainPass : public OverridePass
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapChainGpuHandle1;
		CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapChainGpuHandle2;

		MinMaxChainPass(CameraID cam, unsigned int rpI, RenderPassJsonID rpT, RenderPassInstanceID rp);
		virtual ~MinMaxChainPass() {};
		virtual void Initialize();
		virtual void CreatePrevPassDependentResources();
		virtual void Pass(SceneUnitId unit);
		void Render(SceneUnitId id);
	};
};
