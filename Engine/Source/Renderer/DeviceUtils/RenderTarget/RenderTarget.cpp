#include "pch.h"
#include "RenderTarget.h"
#include <DirectXHelper.h>

namespace DeviceUtils {

	void UpdateRenderTargetViews(CComPtr<ID3D12Device2>& device, CComPtr<IDXGISwapChain4>& swapChain, CComPtr<ID3D12DescriptorHeap>& descriptorHeap, CComPtr<ID3D12Resource>* renderTargets, UINT bufferCount)
	{
		auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());

		for (UINT i = 0; i < bufferCount; ++i)
		{
			CComPtr<ID3D12Resource> backBuffer;
			DX::ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
			device->CreateRenderTargetView(backBuffer, nullptr, rtvHandle);
			renderTargets[i] = backBuffer;
			rtvHandle.Offset(rtvDescriptorSize);
		}
	}

	void UpdateDepthStencilView(CComPtr<ID3D12Device2>& device, CComPtr<ID3D12DescriptorHeap>& descriptorHeap, CComPtr<ID3D12Resource>& depthStencil, DXGI_FORMAT format, UINT width, UINT height)
	{
		D3D12_CLEAR_VALUE optimizedClearValue = {};
		optimizedClearValue.Format = format;
		optimizedClearValue.DepthStencil = { 1.0f, 0 };

		const CD3DX12_HEAP_PROPERTIES depthHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		const CD3DX12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			JRenderer::depthFormatTexConversion.contains(format) ? JRenderer::depthFormatTexConversion.at(format) : format,
			width, height,
			1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		DX::ThrowIfFailed(device->CreateCommittedResource(
			&depthHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&optimizedClearValue,
			IID_PPV_ARGS(&depthStencil)
		));
		CCNAME_D3D12_OBJECT(depthStencil);
		LogCComPtrAddress("depthStencil", depthStencil);

		D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
		dsv.Format = format;
		dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsv.Texture2D.MipSlice = 0;
		dsv.Flags = D3D12_DSV_FLAG_NONE;

		device->CreateDepthStencilView(depthStencil, &dsv, descriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}
}