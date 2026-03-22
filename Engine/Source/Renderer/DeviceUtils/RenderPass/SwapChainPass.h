#pragma once
//#include <DeviceUtils/RenderToTexture/RenderToTexture.h>
#include <DeviceUtils/DescriptorHeap/DescriptorHeap.h>
#include <DirectXColors.h>
#include <functional>

namespace DeviceUtils
{
	struct SwapChainPass
	{
		std::string name;
		unsigned int width;
		unsigned int height;
		D3D12_VIEWPORT screenViewport;
		D3D12_RECT scissorRect;
		DeviceUtils::DescriptorHeap* rtvDescriptorHeap;
		std::vector<CComPtr<ID3D12Resource>> renderTargets;

		DXGI_FORMAT depthStencilFormat;
		CComPtr<ID3D12DescriptorHeap> depthStencilViewDescriptorHeap;
		CComPtr<ID3D12Resource> depthStencilTexture;

		void Pass(SceneUnitId unit, std::function<void(SceneUnitId)> renderCallback, bool clearRTV = true, XMVECTORF32 clearColor = DirectX::Colors::Black);
		void BeginRenderPass(SceneUnitId unit, CComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap, bool clearRTV = true, XMVECTORF32 clearColor = DirectX::Colors::Black);
		void CopyFromRenderToTexture(SceneUnitId unit, JUUID RenderToTextureID);
		void EndRenderPass(SceneUnitId unit);
		void ReleaseResources();
		void Resize(unsigned int width, unsigned int height);
	};

	std::unique_ptr<SwapChainPass>& GetSwapChainPass(JUUID uuid);
	DEF_TEMPLATE_ID(SwapChainPass, GetSwapChainPass);

	SwapChainPassID CreateSwapChainPass(const std::string name, std::unique_ptr<DeviceUtils::DescriptorHeap>& descriptorHeap, DXGI_FORMAT depthStencilFormat);
	void DeleteSwapChainPass(SwapChainPassID swapChainPassId);
};

using namespace DeviceUtils;
DEF_TEMPLATE_ID_HASH(SwapChainPass);