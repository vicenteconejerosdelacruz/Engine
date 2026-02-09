#pragma once
#include <DeviceUtils/RenderToTexture/RenderToTexture.h>
#include <DeviceUtils/DescriptorHeap/DescriptorHeap.h>
#include <DirectXColors.h>
#include <functional>

using namespace DeviceUtils;
using namespace DirectX;

namespace DeviceUtils
{
	struct RenderToTexturePass
	{
		std::string name;
		D3D12_VIEWPORT screenViewport;
		D3D12_RECT scissorRect;
		std::vector<RenderToTextureID> renderToTexture;
		DXGI_FORMAT depthStencilFormat;
		CComPtr<ID3D12DescriptorHeap> depthStencilViewDescriptorHeap;
		CComPtr<ID3D12Resource> depthStencilTexture;
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDepthStencilTextureHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDepthStencilTextureHandle;

		~RenderToTexturePass() { Destroy(); }
		void Pass(SceneUnitId unit, std::function<void(SceneUnitId)> renderCallback, XMVECTORF32 clearColor = DirectX::Colors::Black);
		void BeginRenderPass(SceneUnitId unit, XMVECTORF32 clearColor = DirectX::Colors::Black);
		void EndRenderPass(SceneUnitId unit);
		void Destroy();
		void ReleaseResources();
		void Resize(unsigned int width, unsigned int height);
	};

	RenderToTexturePassID CreateRenderToTexturePass(const std::string name, std::vector<DXGI_FORMAT> renderTargetsFormats, DXGI_FORMAT depthStencilFormat, unsigned int width, unsigned int height);
	std::unique_ptr<RenderToTexturePass>& GetRenderToTexturePass(JUUID uuid);
	void DeleteRenderToTexturePass(RenderToTexturePassID renderToTexturePassId);
};

