#include "pch.h"
#include "RenderToTexture.h"
#include <DeviceUtils/D3D12Device/Builder.h>
#include <Renderer.h>
#include <DirectXHelper.h>

extern std::unique_ptr<JRenderer> renderer;

namespace DeviceUtils {

	static std::unique_ptr<DeviceUtils::DescriptorHeap> renderToTextureDescriptorHeap;

	void CreateRenderToTextureDescriptorHeap()
	{
		auto& d3dDevice = renderer->d3dDevice;
		renderToTextureDescriptorHeap = std::make_unique<DeviceUtils::DescriptorHeap>();
		renderToTextureDescriptorHeap->CreateDescriptorHeap(d3dDevice, maxRenderToTexturesRenderTargets);
	}

	void DestroyRenderToTextureDescriptorHeap()
	{
		renderToTextureDescriptorHeap->DestroyDescriptorHeap();
		renderToTextureDescriptorHeap = nullptr;
	}

	static std::unordered_map<RenderToTextureID, std::unique_ptr<RenderToTexture>> renderToTextures;

	RenderToTextureID CreateRenderToTexture()
	{
		RenderToTextureID rtt = getUUID();
		renderToTextures.insert_or_assign(rtt, std::make_unique<RenderToTexture>());
		return rtt;
	}

	std::unique_ptr<RenderToTexture>& GetRenderToTexture(JUUID rttUUID)
	{
		return renderToTextures.at(rttUUID);
	}

	void DeleteRenderToTexture(RenderToTextureID rttUUID)
	{
		renderToTextures.erase(rttUUID);
	}

	void RenderToTexture::Create()
	{
		auto& d3dDevice = renderer->d3dDevice;

		const CD3DX12_HEAP_PROPERTIES renderToTextureHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
		D3D12_CLEAR_VALUE clearValue =
		{
			.Format = format,
			.Color = {0.0f, 0.0f, 0.0f, 1.0f }
		};
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&renderToTextureHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			useClearColor ? &clearValue : nullptr,
			IID_PPV_ARGS(&renderToTexture)
		));

		CCNAME_D3D12_OBJECT_N(renderToTexture, name);
		LogCComPtrAddress(name, renderToTexture);

		D3D12_RENDER_TARGET_VIEW_DESC rttdesc;
		rttdesc.Format = format;
		rttdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rttdesc.Texture2D.MipSlice = 0;
		rttdesc.Texture2D.PlaneSlice = 0;

		renderToTextureDescriptorHeap->AllocCPUDescriptor(cpuRenderTargetViewHandle);

		d3dDevice->CreateRenderTargetView(renderToTexture, &rttdesc, cpuRenderTargetViewHandle);
	}

	void RenderToTexture::ReleaseResources()
	{
		renderToTexture = nullptr;
		Destroy();
	}

	void RenderToTexture::Resize(unsigned int width, unsigned int height)
	{
		this->width = width;
		this->height = height;
		Create();
	}

	void RenderToTexture::Destroy()
	{
		if (!renderToTextureDescriptorHeap) return;

		renderToTextureDescriptorHeap->FreeCPUDescriptor(cpuRenderTargetViewHandle);
	}
}