#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

namespace DeviceUtils {
	CComPtr<IDXGIAdapter4> GetAdapter();
	CComPtr<ID3D12Device2> CreateDevice(CComPtr<IDXGIAdapter4> adapter);
	CComPtr<ID3D12CommandQueue> CreateCommandQueue(CComPtr<ID3D12Device2> device);
	CComPtr<IDXGISwapChain4> CreateSwapChain(HWND hwnd, CComPtr<ID3D12CommandQueue> commandQueue, UINT bufferCount, bool& supportsTearing);
	CComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(CComPtr<ID3D12Device2> device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	CComPtr<ID3D12CommandAllocator> CreateCommandAllocator(CComPtr<ID3D12Device2> device);
	CComPtr<ID3D12GraphicsCommandList2> CreateCommandList(CComPtr<ID3D12Device2> device, CComPtr<ID3D12CommandAllocator> commandAllocator);
	CComPtr<ID3D12Fence> CreateFence(CComPtr<ID3D12Device2> device, std::string name);
	HANDLE CreateEventHandle();
	bool D32FSupported(CComPtr<ID3D12Device2> device);
	DXGI_FORMAT GetSwapChainFormat(CComPtr<IDXGISwapChain4> swapChain);
}

