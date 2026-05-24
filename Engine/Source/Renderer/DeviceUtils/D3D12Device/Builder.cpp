#include "pch.h"
#include "Builder.h"
#include <DirectXHelper.h>

namespace DeviceUtils {

	CComPtr<IDXGIAdapter4> GetAdapter() {

		CComPtr<IDXGIFactory4> dxgiFactory;
		UINT createFactoryFlags = 0;
#if defined(_DEBUG)
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

		DX::ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

		CComPtr<IDXGIAdapter1> dxgiAdapter1;
		CComPtr<IDXGIAdapter4> dxgiAdapter4;
		{
			SIZE_T maxDedicatedVideoMemory = 0;
			for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
				dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

				if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
					SUCCEEDED(D3D12CreateDevice(dxgiAdapter1,
						D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), nullptr)) &&
					dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
				{
					maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
					dxgiAdapter4.Release();
					DX::ThrowIfFailed(dxgiAdapter1.QueryInterface(&dxgiAdapter4));
					//DX::ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
				}
				dxgiAdapter1.Release();
			}
		}

		return dxgiAdapter4;
	}

	CComPtr<ID3D12Device2> CreateDevice(CComPtr<IDXGIAdapter4> adapter)
	{
		CComPtr<ID3D12Device2> d3d12Device2;
		DX::ThrowIfFailed(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&d3d12Device2)));

		// Enable debug messages in debug mode.
#if defined(_DEBUG)
		CComPtr<ID3D12InfoQueue> pInfoQueue;
		//if (SUCCEEDED(d3d12Device2.As(&pInfoQueue)))
		if (SUCCEEDED(d3d12Device2.QueryInterface(&pInfoQueue)))
		{
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

			// Suppress whole categories of messages

			// Suppress messages based on their severity level
			D3D12_MESSAGE_SEVERITY Severities[] =
			{
					D3D12_MESSAGE_SEVERITY_INFO
			};

			// Suppress individual messages by their ID
			D3D12_MESSAGE_ID DenyIds[] = {
				D3D12_MESSAGE_ID_CREATEBLENDSTATE_BLENDOPALPHA_WARNING,
				D3D12_MESSAGE_ID_CREATEBLENDSTATE_BLENDOP_WARNING,
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,				// I'm really not sure how to avoid this message.
				D3D12_MESSAGE_ID_DRAW_EMPTY_SCISSOR_RECTANGLE,
				D3D12_MESSAGE_ID_DRAW_POTENTIALLY_OUTSIDE_OF_VALID_RENDER_AREA,
				//D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,										// This warning occurs when using capture frame while graphics debugging.
				//D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,									// This warning occurs when using capture frame while graphics debugging.
				//D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,									// This occurs when there are uninitialized descriptors in a descriptor table, even when a shader does not access the missing descriptors.
				//D3D12_MESSAGE_ID_CREATERESOURCE_STATE_IGNORED,								// D3D11On12
				//D3D12_MESSAGE_ID_CREATEGRAPHICSPIPELINESTATE_RENDERTARGETVIEW_NOT_SET		// Shadow Map of 5.1 sucks
				D3D12_MESSAGE_ID_COPYRESOURCE_INVALIDDSTRESOURCE  //this is because of picking D3D12_RESOURCE_DIMENSION_TEXTURE2D -> D3D12_RESOURCE_DIMENSION_BUFFER
			};

			D3D12_INFO_QUEUE_FILTER NewFilter = {};
			NewFilter.DenyList.NumSeverities = _countof(Severities);
			NewFilter.DenyList.pSeverityList = Severities;
			NewFilter.DenyList.NumIDs = _countof(DenyIds);
			NewFilter.DenyList.pIDList = DenyIds;

			DX::ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
		}
#endif

		return d3d12Device2;
	}

	CComPtr<ID3D12CommandQueue> CreateCommandQueue(CComPtr<ID3D12Device2> device)
	{
		CComPtr<ID3D12CommandQueue> d3d12CommandQueue;

		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		DX::ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)));

		return d3d12CommandQueue;
	}

	CComPtr<IDXGISwapChain4> CreateSwapChain(HWND hwnd, CComPtr<ID3D12CommandQueue> commandQueue, UINT bufferCount)
	{
		CComPtr<IDXGISwapChain4> dxgiSwapChain4;
		CComPtr<IDXGIFactory4> dxgiFactory4;
		UINT createFactoryFlags = 0;
#if defined(_DEBUG)
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

		DX::ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

		RECT rect;
		GetWindowRect(hwnd, &rect);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc = { 1, 0 };
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = bufferCount;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
		fsSwapChainDesc.Windowed = TRUE;

		CComPtr<IDXGISwapChain1> swapChain1;
		DX::ThrowIfFailed(
			dxgiFactory4->CreateSwapChainForHwnd(
				commandQueue, hwnd, &swapChainDesc, &fsSwapChainDesc, nullptr, &swapChain1
			)
		);

		DX::ThrowIfFailed(swapChain1.QueryInterface(&dxgiSwapChain4));

		return dxgiSwapChain4;
	}

	CComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(CComPtr<ID3D12Device2> device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
	{
		CComPtr<ID3D12DescriptorHeap> descriptorHeap;
		D3D12_DESCRIPTOR_HEAP_DESC desc = { .Type = type, .NumDescriptors = numDescriptors, .Flags = flags };
		DX::ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));
		return descriptorHeap;
	}

	CComPtr<ID3D12CommandAllocator> CreateCommandAllocator(CComPtr<ID3D12Device2> device)
	{
		CComPtr<ID3D12CommandAllocator> commandAllocator;
		DX::ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
		return commandAllocator;
	}

	CComPtr<ID3D12GraphicsCommandList2> CreateCommandList(CComPtr<ID3D12Device2> device, CComPtr<ID3D12CommandAllocator> commandAllocator)
	{
		CComPtr<ID3D12GraphicsCommandList2> commandList;
		DX::ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList)));
		DX::ThrowIfFailed(commandList->Close());
		return commandList;
	}

	CComPtr<ID3D12Fence> CreateFence(CComPtr<ID3D12Device2> device, std::string name)
	{
		CComPtr<ID3D12Fence> fence;
		DX::ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
		CCNAME_D3D12_OBJECT_N(fence, name);
		LogCComPtrAddress(name, fence);
		return fence;
	}

	HANDLE CreateEventHandle()
	{
		HANDLE fenceEvent;
		fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		if (fenceEvent == nullptr) {
			DX::ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		return fenceEvent;
	}

	bool D32FSupported(CComPtr<ID3D12Device2> device)
	{
		D3D12_FEATURE_DATA_FORMAT_SUPPORT supported =
		{
			.Format = DXGI_FORMAT_D32_FLOAT
		};
		DX::ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &supported, sizeof(supported)));
		return !!(supported.Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL);
	}

	DXGI_FORMAT GetSwapChainFormat(CComPtr<IDXGISwapChain4> swapChain)
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		DX::ThrowIfFailed(swapChain->GetDesc(&swapChainDesc));
		return swapChainDesc.BufferDesc.Format;
	}
}