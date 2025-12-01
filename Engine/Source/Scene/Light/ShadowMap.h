#pragma once
#include <DirectXMath.h>
#include "../../Common/d3dx12.h"

namespace Scene { struct Light; }

namespace Scene {

	using namespace DirectX;

	struct ShadowMapAttributes {
		XMMATRIX atts0;
		XMMATRIX atts1;
		XMMATRIX atts2;
		XMMATRIX atts3;
		XMMATRIX atts4;
		XMMATRIX atts5;
		XMMATRIX atts6;
		XMFLOAT4 atts7;
	};

	void CreateShadowMapResources();
	void DestroyShadowMapResources();
	void AllocShadowMapSlot(unsigned int slot);
	void FreeShadowMapSlot(unsigned int slot);
	unsigned int GetNextAvailableShadowMapSlot();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetShadowMapGpuDescriptorHandleStart();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetShadowMapGpuDescriptorHandle(unsigned int index);

};

