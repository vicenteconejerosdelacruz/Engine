#pragma once

#include <dxgi1_6.h>

using namespace Microsoft::WRL;
using namespace DirectX;

struct Renderer
{
	~Renderer() {}
	static const constexpr unsigned int numFrames = 3;
	static const constexpr float fovAngleY = (70.0f * XM_PI / 180.0f);
	static inline const std::map<DXGI_FORMAT, DXGI_FORMAT> depthFallback = {
		{ DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT }
	};
	static inline const std::map<DXGI_FORMAT, DXGI_FORMAT> depthFormatTexConversion = {
		{ DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_TYPELESS },
		{ DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS }
	};
	static inline const std::map<DXGI_FORMAT, DXGI_FORMAT> depthFormatSRVConversion = {
		{ DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT },
		{ DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS }
	};

	//app window reference
	HWND hwnd;

	//d3d device resources in order of creation
	CComPtr<ID3D12Device2> d3dDevice;
	CComPtr<ID3D12CommandQueue> commandQueue;
	CComPtr<IDXGISwapChain4> swapChain;
	DXGI_FORMAT swapChainFormat;

	//command
	CComPtr<ID3D12CommandAllocator> commandAllocators[numFrames];
	CComPtr<ID3D12GraphicsCommandList2> commandList;

	//GPU <-> CPU synchronization 
	CComPtr<ID3D12Fence> fence;
	unsigned long long fenceValue = 0;
	unsigned long long frameFenceValues[numFrames] = {};
	HANDLE fenceEvent;

	//window based values
	D3D12_VIEWPORT screenViewport;
	D3D12_RECT scissorRect;

	unsigned int backBufferIndex;
	bool d32FSupported;

	//the swap chain pass
	RenderPassInstanceUUID swapChainPass;

	//CREATE
	void Initialize(HWND hwnd);
	void CreateComputeEngine();
	void CreateSwapChainPass();

	//READ&GET

	//UPDATE
	void UpdateViewportPerspective();
	void Resize(unsigned int width, unsigned int height);
	void ResetCommands() const;
	void SetCSUDescriptorHeap() const;
	void CloseCommandsAndFlush();
	void RenderCriticalFrame(std::function<void()> callback = []() {}, bool flush = true);
	void ExecuteCommands() const;
	void Present();
	void Flush();

	//DESTROY
	void DestroySwapChainPass();
	void Destroy();
};

