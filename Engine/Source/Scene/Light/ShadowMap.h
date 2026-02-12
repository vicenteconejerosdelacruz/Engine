#pragma once
#include <SimpleMath.h>
#include <d3dx12.h>

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

	void CreateShadowMapResources(SceneUnitId unit);
	void DestroyShadowMapResources(SceneUnitId unit);
	void AllocShadowMapSlot(SceneUnitId unit, unsigned int slot);
	void FreeShadowMapSlot(SceneUnitId unit, unsigned int slot);
	unsigned int GetNextAvailableShadowMapSlot(SceneUnitId unit);
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetShadowMapGpuDescriptorHandleStart(SceneUnitId unit);
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetShadowMapGpuDescriptorHandle(SceneUnitId unit, unsigned int index);
};

