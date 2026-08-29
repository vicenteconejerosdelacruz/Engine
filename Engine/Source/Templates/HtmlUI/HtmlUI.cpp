#include "pch.h"
#include "HtmlUI.h"

extern RefPtr<ultralight::Renderer> ultraLightRenderer;
extern std::unique_ptr<JRenderer> renderer;

namespace Templates
{
#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <HtmlUIAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <HtmlUIAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <HtmlUIAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <HtmlUIAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <HtmlUIAtt.h>
#include <JEnd.h>

#endif

	HtmlUIJson::HtmlUIJson(nlohmann::json& json) : JTemplate(json)
	{
#include <Attributes/JInit.h>
#include <HtmlUIAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <HtmlUIAtt.h>
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include <HtmlUIAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void HtmlUIJson::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <HtmlUIAtt.h>
#include <JEnd.h>
	}
#endif

	TEMPDEF_FULL(HtmlUI);
	TEMPDEF_REFTRACKER(HtmlUI);

	HtmlUIInstance::HtmlUIInstance(SceneUnitId id, JUUID instance_uuid, JUUID template_uuid)
	{
		instanceUUID = instance_uuid;
		HtmlUIJsonID tmpl = template_uuid;

		//figure out the width&height
		unsigned int width = renderer->scissorRect.right;
		unsigned int height = renderer->scissorRect.bottom;

		//create the rtt
		rt_texture = CreateRenderToTexture();
		rt_texture->name = tmpl->name();
		rt_texture->format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rt_texture->width = width;
		rt_texture->height = height;
		rt_texture->useClearColor = false;
		rt_texture->Create();
		AllocCSUDescriptor(rt_texture->cpuTextureHandle, rt_texture->gpuTextureHandle);

		//create a SRV for the rtt
		D3D12_SHADER_RESOURCE_VIEW_DESC rttSRVDesc = {
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			.Texture2D = {.MostDetailedMip = 0, .MipLevels = 1U, .ResourceMinLODClamp = 0.0f },
		};
		renderer->d3dDevice->CreateShaderResourceView(rt_texture->renderToTexture, &rttSRVDesc, rt_texture->cpuTextureHandle);

		//create a resolve pass
		resolvePass = CreateRenderPassInstance(CameraID(), GetRenderPassUUIDByName("resolveUIPass"), 1, width, height);
		auto* overrideResolvePass = static_cast<ResolvePass*>(resolvePass->overridePass.get());
		overrideResolvePass->rt_texture = rt_texture;
		overrideResolvePass->clearRTV = false;

		//calculate the uploadSize and rowPitch
		unsigned int bytesPerPixel = 4;
		rowPitch = (width * bytesPerPixel + 255) & ~255;
		size_t uploadSize = (size_t)rowPitch * height;

		//create an UploadBuffer in the Default Heap
		D3D12_RESOURCE_DESC bufDesc = {};
		bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufDesc.Width = uploadSize;
		bufDesc.Height = 1;
		bufDesc.DepthOrArraySize = 1;
		bufDesc.MipLevels = 1;
		bufDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufDesc.SampleDesc.Count = 1;
		bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		const CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);
		HRESULT hr = renderer->d3dDevice->CreateCommittedResource(
			&uploadHeap, D3D12_HEAP_FLAG_NONE, &bufDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&uploadBuffer));

		//create the ultralight view
		ViewConfig view_config;
		view_config.is_transparent = true;
		view_config.initial_device_scale = 1.0;
		view = ultraLightRenderer->CreateView(width, height, view_config, nullptr);

		//set the loadlistener
		view->set_load_listener(this);

		//load the webpage view
		std::string path = std::string("file:///" + defaultUIFolder + tmpl->path());
		view->LoadURL(path.c_str());
		view->Focus();
	}

	void HtmlUIInstance::Destroy()
	{
		RefPtr<View> view = nullptr;
		DeleteRenderToTexture(rt_texture);
		if (resolvePass)
		{
			resolvePass->MarkForDelete();
		}
		uploadBuffer = nullptr;
	}

	void HtmlUIInstance::UpdateTexture(SceneUnitId id)
	{
		BitmapSurface* surface = (BitmapSurface*)(view->surface());
		if (surface->dirty_bounds().IsEmpty()) return;

		//unlock and mapping into the uploadBuffer
		auto bitmap = surface->bitmap();
		void* pixels = bitmap->LockPixels();
		void* pMappedData = nullptr;
		uploadBuffer->Map(0, nullptr, &pMappedData);

		//casting
		uint8_t* pSrc = (uint8_t*)pixels;
		uint8_t* pDest = (uint8_t*)pMappedData;
		uint32_t srcPitch = (uint32_t)bitmap->row_bytes();

		//copy line by line(pitch offset based)
		for (uint32_t y = 0; y < rt_texture->height; ++y) {
			memcpy(pDest + (y * rowPitch), pSrc + (y * srcPitch), srcPitch);
		}

		//unmapping and unlocking
		uploadBuffer->Unmap(0, nullptr);
		bitmap->UnlockPixels();

		//dirty bounds are clear now
		surface->ClearDirtyBounds();

		auto& commandList = GetSceneUnit(id)->GetCommandList();

		//define the destiny location
		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = rt_texture->renderToTexture;
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		//define the src location using placedfootprint
		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = uploadBuffer.p;
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		src.PlacedFootprint.Footprint.Width = rt_texture->width;
		src.PlacedFootprint.Footprint.Height = rt_texture->height;
		src.PlacedFootprint.Footprint.Depth = 1;
		src.PlacedFootprint.Footprint.RowPitch = rowPitch; // 256 bytes aligned!

		//place a barrier to allow the copy on the destination resource
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = rt_texture->renderToTexture;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &barrier);

		//make the copy
		commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

		//restore to allow reading as a pixel shader resource
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		commandList->ResourceBarrier(1, &barrier);
	}
	void HtmlUIInstance::Resolve(SceneUnitId id)
	{
		resolvePass->Pass(id);
	}

	void HtmlUIInstance::OnDOMReady(View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url)
	{
		RefPtr<JSContext> context = caller->LockJSContext();
		SetJSContext(context->ctx());
		JSObject global = JSGlobalObject();

		global["JSBridge"] = (JSCallback)[caller, this](const JSObject& thisObject, const JSArgs& args)
		{
			if (args.size() > 0 && args[0].IsString()) {
				ultralight::String mensaje = args[0].ToString();
				std::string msg(mensaje.utf8().data());
				if (bridgeCallbacks.contains(msg))
				{
					bridgeCallbacks.at(msg)();
				}
			}
		};
	}

	void HtmlUIInstance::MapBridgeCallback(std::string event, std::function<void()> callback)
	{
		bridgeCallbacks.insert_or_assign(event, callback);
	}

	void HtmlUIInstance::EvaluateScript(std::string js)
	{
		view->EvaluateScript(js.c_str());
	}
};