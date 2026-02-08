#include "pch.h"
#include <map>
#include <Scene.h>
#include "RenderToTexturePass.h"
#include <DirectXHelper.h>
#include <DeviceUtils/RenderTarget/RenderTarget.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <d3dx12.h>
#if defined(_DEVELOPMENT)
#include <pix3.h>
#endif

extern std::unique_ptr<Renderer> renderer;

namespace DeviceUtils {

	static std::unordered_map<JUUID, std::unique_ptr<RenderToTexturePass>> renderToTexturePasses;

	JUUID CreateRenderToTexturePass(const std::string name, std::vector<DXGI_FORMAT> renderTargetsFormats, DXGI_FORMAT depthStencilFormat, unsigned int width, unsigned int height)
	{
		JUUID uuid = getUUID();
		auto& d3dDevice = renderer->d3dDevice;
		std::unique_ptr<RenderToTexturePass> renderPass = std::make_unique<RenderToTexturePass>();

		renderPass->name = name;
		renderPass->screenViewport = {
			.TopLeftX = 0.0f, .TopLeftY = 0.0f,
			.Width = static_cast<float>(width), .Height = static_cast<float>(height),
			.MinDepth = 0.0f, .MaxDepth = 1.0f
		};
		renderPass->scissorRect = {
			.left = 0L, .top = 0L,
			.right = static_cast<long>(width),
			.bottom = static_cast<long>(height)
		};

		for (auto format : renderTargetsFormats)
		{
			RenderToTextureID rtt = CreateRenderToTexture();
			rtt->name = name + "[" + std::to_string(renderPass->renderToTexture.size()) + "]";
			rtt->format = format;
			rtt->width = width;
			rtt->height = height;

			renderPass->renderToTexture.push_back(rtt);
			rtt->Create();
			AllocCSUDescriptor(rtt->cpuTextureHandle, rtt->gpuTextureHandle);

			D3D12_SHADER_RESOURCE_VIEW_DESC rttSRVDesc = {
				.Format = format,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = {.MostDetailedMip = 0, .MipLevels = 1U, .ResourceMinLODClamp = 0.0f },
			};

			d3dDevice->CreateShaderResourceView(rtt->renderToTexture, &rttSRVDesc, rtt->cpuTextureHandle);
		}

		if (Renderer::depthFallback.contains(depthStencilFormat) && !renderer->d32FSupported)
			depthStencilFormat = Renderer::depthFallback.at(depthStencilFormat);

		renderPass->depthStencilFormat = depthStencilFormat;
		if (depthStencilFormat != DXGI_FORMAT_UNKNOWN)
		{
			renderPass->depthStencilViewDescriptorHeap = CreateDescriptorHeap(d3dDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			CCNAME_D3D12_OBJECT(renderPass->depthStencilViewDescriptorHeap);

			UpdateDepthStencilView(d3dDevice, renderPass->depthStencilViewDescriptorHeap, renderPass->depthStencilTexture, depthStencilFormat, width, height);
			CCNAME_D3D12_OBJECT_N(renderPass->depthStencilTexture, name);

			AllocCSUDescriptor(renderPass->cpuDepthStencilTextureHandle, renderPass->gpuDepthStencilTextureHandle);

			D3D12_SHADER_RESOURCE_VIEW_DESC depthStencilSRVDesc = {
				.Format = Renderer::depthFormatSRVConversion.contains(depthStencilFormat) ? Renderer::depthFormatSRVConversion.at(depthStencilFormat) : depthStencilFormat,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = {.MostDetailedMip = 0, .MipLevels = 1U, .ResourceMinLODClamp = 0.0f },
			};

			d3dDevice->CreateShaderResourceView(renderPass->depthStencilTexture, &depthStencilSRVDesc, renderPass->cpuDepthStencilTextureHandle);
		}

		renderToTexturePasses.insert_or_assign(uuid, std::move(renderPass));
		return uuid;
	}

	std::unique_ptr<RenderToTexturePass>& GetRenderToTexturePass(JUUID uuid)
	{
		return renderToTexturePasses.at(uuid);
	}

	void DeleteRenderToTexturePass(JUUID uuid)
	{
		if (renderToTexturePasses.contains(uuid))
		{
			renderToTexturePasses.at(uuid)->ReleaseResources();
			renderToTexturePasses.erase(uuid);
		}
	}

	void RenderToTexturePass::Pass(SceneUnitId unit, std::function<void(SceneUnitId)> renderCallback, XMVECTORF32 clearColor)
	{
		BeginRenderPass(unit, clearColor);
		renderCallback(unit);
		EndRenderPass(unit);
	}

	void RenderToTexturePass::BeginRenderPass(SceneUnitId unit, XMVECTORF32 clearColor)
	{
		using namespace Scene;
		auto& commandList = GetSceneUnit(unit)->GetCommandList();

#if defined(_DEVELOPMENT)
		PIXBeginEvent(commandList.p, 0, name.c_str());
#endif

		//transition the texture resources from pixel shader resource to render target
		std::vector<CD3DX12_RESOURCE_BARRIER> barriers;
		for (auto rtt : renderToTexture)
		{
			barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(rtt->renderToTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		}
		if (barriers.size())
		{
			commandList->ResourceBarrier(static_cast<unsigned int>(barriers.size()), barriers.data());
		}

		commandList->RSSetViewports(1, &screenViewport);
		commandList->RSSetScissorRects(1, &scissorRect);

		std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
		for (auto rtt : renderToTexture)
		{
			commandList->ClearRenderTargetView(rtt->cpuRenderTargetViewHandle, clearColor, 0, nullptr);
			rtvHandles.push_back(rtt->cpuRenderTargetViewHandle);
		}

		if (depthStencilViewDescriptorHeap)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(depthStencilViewDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
			commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
			commandList->OMSetRenderTargets(static_cast<unsigned int>(rtvHandles.size()), rtvHandles.data(), false, &dsv);
		}
		else
		{
			commandList->OMSetRenderTargets(static_cast<unsigned int>(rtvHandles.size()), rtvHandles.data(), false, nullptr);
		}
	}

	void RenderToTexturePass::EndRenderPass(SceneUnitId unit)
	{
		using namespace Scene;
		auto& commandList = GetSceneUnit(unit)->GetCommandList();

		//transition the texture resources from render target to pixel shader resource
		std::vector<CD3DX12_RESOURCE_BARRIER> barriers;
		for (auto rtt : renderToTexture)
		{
			barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(rtt->renderToTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		}
		if (barriers.size())
		{
			commandList->ResourceBarrier(static_cast<unsigned int>(barriers.size()), barriers.data());
		}
#if defined(_DEVELOPMENT)
		PIXEndEvent(commandList.p);
#endif
	}

	void RenderToTexturePass::Destroy()
	{
		renderToTexture.clear();
		if (depthStencilTexture != nullptr)
		{
			FreeCSUDescriptor(cpuDepthStencilTextureHandle, gpuDepthStencilTextureHandle);
		}
	}

	void RenderToTexturePass::ReleaseResources()
	{
		for (auto rtt : renderToTexture)
		{
			rtt->ReleaseResources();
		}

		if (depthStencilFormat != DXGI_FORMAT_UNKNOWN)
		{
			depthStencilTexture = nullptr;
			FreeCSUDescriptor(cpuDepthStencilTextureHandle, gpuDepthStencilTextureHandle);
		}
	}

	void RenderToTexturePass::Resize(unsigned int width, unsigned int height)
	{
		auto& d3dDevice = renderer->d3dDevice;

		screenViewport = {
			.TopLeftX = 0.0f, .TopLeftY = 0.0f,
			.Width = static_cast<float>(width), .Height = static_cast<float>(height),
			.MinDepth = 0.0f, .MaxDepth = 1.0f
		};
		scissorRect = {
			.left = 0L, .top = 0L,
			.right = static_cast<long>(width),
			.bottom = static_cast<long>(height)
		};

		for (auto rtt : renderToTexture)
		{
			rtt->Resize(width, height);
			D3D12_SHADER_RESOURCE_VIEW_DESC rttSRVDesc = {
				.Format = rtt->format,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = {.MostDetailedMip = 0, .MipLevels = 1U, .ResourceMinLODClamp = 0.0f },
			};
			d3dDevice->CreateShaderResourceView(rtt->renderToTexture, &rttSRVDesc, rtt->cpuTextureHandle);
		}

		if (depthStencilFormat != DXGI_FORMAT_UNKNOWN)
		{
			UpdateDepthStencilView(d3dDevice, depthStencilViewDescriptorHeap, depthStencilTexture, depthStencilFormat, width, height);
			CCNAME_D3D12_OBJECT_N(depthStencilTexture, name);

			AllocCSUDescriptor(cpuDepthStencilTextureHandle, gpuDepthStencilTextureHandle);

			D3D12_SHADER_RESOURCE_VIEW_DESC depthStencilSRVDesc = {
				.Format = Renderer::depthFormatSRVConversion.contains(depthStencilFormat) ? Renderer::depthFormatSRVConversion.at(depthStencilFormat) : depthStencilFormat,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = {.MostDetailedMip = 0, .MipLevels = 1U, .ResourceMinLODClamp = 0.0f },
			};

			d3dDevice->CreateShaderResourceView(depthStencilTexture, &depthStencilSRVDesc, cpuDepthStencilTextureHandle);
		}
	}
}