#include "pch.h"
#include "SwapChainPass.h"
#include <Scene.h>
#include <Renderer.h>
#include <DirectXHelper.h>
#include <DeviceUtils/RenderTarget/RenderTarget.h>
#include <DeviceUtils/DescriptorHeap/DescriptorHeap.h>
#include <DeviceUtils/RenderToTexture/RenderToTexture.h>
#include <NoStd.h>
#if defined(_DEVELOPMENT)
#include <pix3.h>
#endif

extern std::unique_ptr<JRenderer> renderer;

namespace DeviceUtils
{
	static std::unordered_map<SwapChainPassID, std::unique_ptr<SwapChainPass>> swapChainPasses;

	SwapChainPassID CreateSwapChainPass(const std::string name, std::unique_ptr<DeviceUtils::DescriptorHeap>& descriptorHeap, DXGI_FORMAT depthStencilFormat)
	{
		SwapChainPassID swapChainPassId = getUUID();
		auto& d3dDevice = renderer->d3dDevice;

		unsigned int bufferCount = renderer->numFrames;
		std::unique_ptr<SwapChainPass> swapChainPass = std::make_unique<SwapChainPass>();
		swapChainPass->name = name;
		swapChainPass->rtvDescriptorHeap = descriptorHeap.get();
		nostd::VecN_push_back(bufferCount, swapChainPass->renderTargets);
		UpdateRenderTargetViews(d3dDevice, renderer->swapChain, descriptorHeap->descriptorHeap, swapChainPass->renderTargets.data(), bufferCount);
		for (unsigned int i = 0U; i < bufferCount; i++)
		{
			std::string passName = name + "[" + std::to_string(i) + "]";
			CCNAME_D3D12_OBJECT_N(swapChainPass->renderTargets[i], passName);
			LogCComPtrAddress(passName, swapChainPass->renderTargets[i]);
		}
		swapChainPass->screenViewport = renderer->screenViewport;
		swapChainPass->scissorRect = renderer->scissorRect;
		swapChainPass->width = static_cast<unsigned int>(abs(renderer->scissorRect.right - renderer->scissorRect.left));
		swapChainPass->height = static_cast<unsigned int>(abs(renderer->scissorRect.bottom - renderer->scissorRect.top));

		if (JRenderer::depthFallback.contains(depthStencilFormat) && !renderer->d32FSupported)
			depthStencilFormat = JRenderer::depthFallback.at(depthStencilFormat);

		swapChainPass->depthStencilFormat = depthStencilFormat;
		if (depthStencilFormat != DXGI_FORMAT_UNKNOWN)
		{
			swapChainPass->depthStencilViewDescriptorHeap = CreateDescriptorHeap(d3dDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			CCNAME_D3D12_OBJECT(swapChainPass->depthStencilViewDescriptorHeap);

			UpdateDepthStencilView(d3dDevice, swapChainPass->depthStencilViewDescriptorHeap, swapChainPass->depthStencilTexture, depthStencilFormat, swapChainPass->width, swapChainPass->height);
			CCNAME_D3D12_OBJECT_N(swapChainPass->depthStencilTexture, name);
		}

		swapChainPasses.insert_or_assign(swapChainPassId, std::move(swapChainPass));
		return swapChainPassId;
	}

	std::unique_ptr<SwapChainPass>& GetSwapChainPass(JUUID uuid)
	{
		return swapChainPasses.at(uuid);
	}

	void DeleteSwapChainPass(SwapChainPassID swapChainPassId)
	{
		swapChainPassId->ReleaseResources();
		swapChainPasses.erase(swapChainPassId);
	}

	void SwapChainPass::Pass(SceneUnitId unit, std::function<void(SceneUnitId)> renderCallback, bool clearRTV, XMVECTORF32 clearColor)
	{
#if defined(_DEVELOPMENT)
		{
			auto& sceneUnit = GetSceneUnit(unit);
			auto& commandList = sceneUnit->GetCommandList();
			PIXScopedEvent(commandList.p, 0, name.c_str());
#endif
			BeginRenderPass(unit, depthStencilViewDescriptorHeap, clearRTV, clearColor);
			renderCallback(unit);
			EndRenderPass(unit);
#if defined(_DEVELOPMENT)
		}
#endif
	}

	void SwapChainPass::BeginRenderPass(SceneUnitId unit, CComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap, bool clearRTV, XMVECTORF32 clearColor)
	{
		using namespace Scene;
		auto& sceneUnit = GetSceneUnit(unit);

		unsigned int frame = sceneUnit->Frame();
		auto& commandList = sceneUnit->GetCommandList();
		auto& backBuffer = renderTargets[frame];

		//transition the back buffer from present to render target so it's allowed to draw
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &barrier);

		commandList->RSSetViewports(1, &screenViewport);
		commandList->RSSetScissorRects(1, &scissorRect);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(rtvDescriptorHeap->descriptorHeap->GetCPUDescriptorHandleForHeapStart(), frame, rtvDescriptorHeap->descriptorSize);

		if (dsvDescriptorHeap)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
			commandList->OMSetRenderTargets(1, &rtv, false, &dsv);
			commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		}
		else
		{
			commandList->OMSetRenderTargets(1, &rtv, false, nullptr);
		}

		if (clearRTV)
		{
			commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
		}
	}

	void SwapChainPass::CopyFromRenderToTexture(SceneUnitId unit, JUUID RenderToTextureID)
	{
		using namespace Scene;
		auto& sceneUnit = GetSceneUnit(unit);

		unsigned int frame = sceneUnit->Frame();
		auto& commandList = sceneUnit->GetCommandList();
		auto& backBuffer = renderTargets[frame];

		auto& renderToTexture = GetRenderToTexture(RenderToTextureID);
		auto& rtt = renderToTexture->renderToTexture;

		std::vector<CD3DX12_RESOURCE_BARRIER> hold = {
			CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST),
			CD3DX12_RESOURCE_BARRIER::Transition(rtt, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE),
		};
		commandList->ResourceBarrier((unsigned int)hold.size(), hold.data());

		D3D12_TEXTURE_COPY_LOCATION src = { .pResource = backBuffer, .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, .SubresourceIndex = 0U };
		D3D12_TEXTURE_COPY_LOCATION dst = { .pResource = renderToTexture->renderToTexture, .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, .SubresourceIndex = 0U };
		commandList->CopyTextureRegion(&src, 0, 0, 0, &dst, nullptr);

		std::vector<CD3DX12_RESOURCE_BARRIER> release = {
			CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET),
			CD3DX12_RESOURCE_BARRIER::Transition(rtt, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
		};
		commandList->ResourceBarrier((unsigned int)release.size(), release.data());
	}

	void SwapChainPass::EndRenderPass(SceneUnitId unit)
	{
		using namespace Scene;
		auto& sceneUnit = GetSceneUnit(unit);

		unsigned int frame = sceneUnit->Frame();
		auto& commandList = sceneUnit->GetCommandList();
		auto& backBuffer = renderTargets[frame];

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		commandList->ResourceBarrier(1, &barrier);
	}

	void SwapChainPass::ReleaseResources()
	{
		renderTargets.clear();
	}

	void SwapChainPass::Resize(unsigned int width, unsigned int height)
	{
		using namespace std;
		this->width = width;
		this->height = height;
		unsigned int bufferCount = renderer->numFrames;
		nostd::VecN_push_back(bufferCount, renderTargets);
		UpdateRenderTargetViews(renderer->d3dDevice, renderer->swapChain, rtvDescriptorHeap->descriptorHeap, renderTargets.data(), bufferCount);
		for (unsigned int i = 0U; i < bufferCount; i++)
		{
			std::string passName = name + "[" + std::to_string(i) + "]";
			CCNAME_D3D12_OBJECT_N(renderTargets[i], passName);
		}
		screenViewport = renderer->screenViewport;
		screenViewport.Width = min(static_cast<float>(width), screenViewport.Width);
		screenViewport.Height = min(static_cast<float>(height), screenViewport.Height);
		scissorRect = renderer->scissorRect;
		scissorRect.right = min(static_cast<LONG>(width), scissorRect.right);
		scissorRect.bottom = min(static_cast<LONG>(height), scissorRect.bottom);
	}
}