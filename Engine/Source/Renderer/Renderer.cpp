#include "pch.h"
#include "Renderer.h"
#include <DirectXHelper.h>
#include <DeviceUtils/D3D12Device/Builder.h>
#include <DeviceUtils/D3D12Device/Interop.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <DeviceUtils/RenderToTexture/RenderToTexture.h>
#include <RenderPass/RenderPass.h>
//#include <RenderPass/RenderPass.h>

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

	/*
	for (int i = 0; i < numFrames; ++i) {
		commandAllocators[i] = CreateCommandAllocator(d3dDevice);
		commandAllocators[i]->SetName((L"commandAllocator[" + std::to_wstring(i) + L"]").c_str());
	}

	commandList = CreateCommandList(d3dDevice, commandAllocators[backBufferIndex]);
	CCNAME_D3D12_OBJECT(commandList);
	*/

}

/*
void Renderer::CreateComputeEngine()
{
}
*/

void Renderer::CreateSwapChainPass()
{
	using namespace Templates;
	swapChainPass = CreateRenderPassInstance("", GetRenderPassUUIDByName("simplePass"), 0);
}

unsigned int Renderer::GetBackBufferIndex()
{
	return backBufferIndex;
}

//DESTROY
/*
void Renderer::DestroySwapChainPass()
{
	using namespace Templates;
	DestroyRenderPassInstance(swapChainPass());
}
*/

/*
void Renderer::Destroy() {
	Flush();

	fence.Release();
	fence = nullptr;

	//release d3d12
	commandList.Release();
	commandList = nullptr;

	for (int i = 0; i < numFrames; i++) {
		commandAllocators[i].Release();
		commandAllocators[i] = nullptr;
	}

	DestroyRenderToTextureDescriptorHeap();

	using namespace DeviceUtils;
	DestroyCSUDescriptorHeap();

	swapChain.Release();
	swapChain = nullptr;

	commandQueue.Release();
	commandQueue = nullptr;

	d3dDevice.Release();
	d3dDevice = nullptr;

#if defined(_DEBUG)
	debugController.Release();
	debugController = nullptr;

	//debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
	//debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_IGNORE_INTERNAL);
	//debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY);

	debugDevice.Release();
	debugDevice = nullptr;
#endif
}
*/

void Renderer::UpdateViewportPerspective() {
	RECT rect;
	GetWindowRect(hwnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	scissorRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
	screenViewport = { 0.0f, 0.0f, static_cast<FLOAT>(width), static_cast<FLOAT>(height), 0.0f, 1.0f };
}

/*
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
*/

/*
void Renderer::ResetCommands() const {
	auto commandAllocator = commandAllocators[backBufferIndex];
	commandAllocator->Reset();
	commandList->Reset(commandAllocator, nullptr);
}
*/

/*
void Renderer::SetCSUDescriptorHeap() const {
	ID3D12DescriptorHeap* ppHeaps[] = { GetCSUDescriptorHeap() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}
*/

/*
void Renderer::ExecuteCommands() const
{
	DX::ThrowIfFailed(commandList->Close());
	ID3D12CommandList* const commandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
}
*/

void Renderer::ExecuteCommands(CComPtr<ID3D12GraphicsCommandList2>& commandList)
{
	ID3D12CommandList* const commandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
}

void Renderer::Present() {
	using namespace DeviceUtils;

	//present
	DX::ThrowIfFailed(swapChain->Present(1, 0));
	frameFenceValues[backBufferIndex] = Signal(commandQueue, fence, fenceValue);
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	//make the CPU to wait for the GPU to finish the current processing
	WaitForFenceValue(fence, frameFenceValues[backBufferIndex], fenceEvent);
}

void Renderer::Flush()
{
	DeviceUtils::Flush(commandQueue, fence, fenceValue, fenceEvent);
}

/*
void Renderer::CloseCommandsAndFlush() {
	commandList->Close();
	ID3D12CommandList* const commandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
	Flush();
}
*/

/*
void Renderer::RenderCriticalFrame(std::function<void()> callback, bool flush)
{
	if (flush)
		Flush();

	ResetCommands();
	SetCSUDescriptorHeap();

	callback();

	CloseCommandsAndFlush();
}
*/