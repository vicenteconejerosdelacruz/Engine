#pragma once
#include <map>
#include <atlbase.h>
#include <d3dx12.h>
#include <Renderer.h>

namespace DeviceUtils
{
	struct CommandsProcessor
	{
		std::vector<CComPtr<ID3D12CommandAllocator>> commandAllocators;
		std::vector<CComPtr<ID3D12GraphicsCommandList2>> commandLists;
		unsigned int frame;

		void Init(CComPtr<ID3D12Device2> d3dDevice, size_t id, size_t capacity);
		CComPtr<ID3D12GraphicsCommandList2>& GetCommandList();
		void ResetCommandList();
		void CloseCommandList();
		void Next() { frame = (frame + 1) % static_cast<unsigned int>(commandLists.size()); }
	};
}
