#include "pch.h"
#include "Renderer.h"
#include <DirectXHelper.h>
#include <DeviceUtils/D3D12Device/Builder.h>
#include <DeviceUtils/D3D12Device/Interop.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <DeviceUtils/RenderToTexture/RenderToTexture.h>
#include <RenderPass/RenderPass.h>

using namespace Templates::RenderPass;
using namespace DeviceUtils;

#if defined(_DEBUG)
CComPtr<ID3D12Debug1> debugController;
CComPtr<ID3D12DebugDevice1> debugDevice;
#endif

unsigned int backBufferIndex;

//CREATE
void Renderer::Initialize(HWND coreHwnd) {
	hwnd = coreHwnd;

#if defined(_DEBUG)
	ComPtr<ID3D12Debug1> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();

		ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
		{
			//m_dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
		}
	}
#endif

	CComPtr<IDXGIAdapter4> dxgiAdapter4 = GetAdapter();
	d3dDevice = CreateDevice(dxgiAdapter4);

	d32FSupported = D32FSupported(d3dDevice);

#if defined(_DEBUG)
	d3dDevice->QueryInterface(IID_PPV_ARGS(&debugDevice));
#endif

	commandQueue = CreateCommandQueue(d3dDevice);
	swapChain = CreateSwapChain(hwnd, commandQueue, numFrames);
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	swapChainFormat = GetSwapChainFormat(swapChain);

	CCNAME_D3D12_OBJECT_N(d3dDevice, std::string("Renderer"));
	CCNAME_D3D12_OBJECT_N(commandQueue, std::string("Renderer"));

	CreateCSUDescriptorHeap(numFrames);
	CreateRenderToTextureDescriptorHeap();
	CreateRenderPassMainHeap();
	UpdateViewportPerspective();
	CreateSwapChainPass();

	fence = CreateFence(d3dDevice, "Renderer");
	fenceEvent = CreateEventHandle();
}

void Renderer::CreateSwapChainPass()
{
	using namespace Templates;
	swapChainPass = CreateRenderPassInstance(0, "", GetRenderPassUUIDByName("simplePass"), 0);
}

unsigned int Renderer::GetBackBufferIndex()
{
	return backBufferIndex;
}

void Renderer::UpdateViewportPerspective() {
	RECT rect;
	GetWindowRect(hwnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	scissorRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
	screenViewport = { 0.0f, 0.0f, static_cast<FLOAT>(width), static_cast<FLOAT>(height), 0.0f, 1.0f };
}

void Renderer::Resize(unsigned int width, unsigned int height) {
	using namespace DeviceUtils;

	Flush();

	for (unsigned int i = 0; i < numFrames; ++i)
	{
		frameFenceValues[i] = frameFenceValues[backBufferIndex];
	}

	scissorRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
	screenViewport = { 0.0f, 0.0f, static_cast<FLOAT>(width), static_cast<FLOAT>(height), 0.0f, 1.0f };

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	DX::ThrowIfFailed(swapChain->GetDesc(&swapChainDesc));
	DX::ThrowIfFailed(swapChain->ResizeBuffers(numFrames, width, height,
		swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
}

void Renderer::ExecuteCommands(CComPtr<ID3D12GraphicsCommandList2>& commandList, std::function<void()> callback)
{
	ID3D12CommandList* const commandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
	if (callback) { executionCallback.push_back(callback); }
}

void Renderer::Present() {
	using namespace DeviceUtils;

	//present
	DX::ThrowIfFailed(swapChain->Present(1, 0));
	frameFenceValues[backBufferIndex] = Signal(commandQueue, fence, fenceValue);
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	//make the CPU to wait for the GPU to finish the current processing
	WaitForFenceValue(fence, frameFenceValues[backBufferIndex], fenceEvent);
	std::for_each(executionCallback.begin(), executionCallback.end(), [](auto x) {x(); });
	executionCallback.clear();
}

void Renderer::Flush()
{
	DeviceUtils::Flush(commandQueue, fence, fenceValue, fenceEvent);
}
