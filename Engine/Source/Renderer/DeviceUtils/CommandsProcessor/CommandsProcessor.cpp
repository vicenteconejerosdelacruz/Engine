#include "pch.h"
#include "CommandsProcessor.h"
#include <DeviceUtils/D3D12Device/Builder.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <DirectXHelper.h>

namespace DeviceUtils
{
	void CommandsProcessor::Init(CComPtr<ID3D12Device2> d3dDevice, size_t id, size_t capacity)
	{
		using namespace nostd;

		for (int i = 0; i < capacity; ++i)
		{
			std::string allocatorName = "commandAllocator:" + std::to_string(id) + "[" + std::to_string(i) + "]";
			std::string commandListName = "commandList:" + std::to_string(id) + "[" + std::to_string(i) + "]";

			auto allocator = CreateCommandAllocator(d3dDevice);
			auto commandList = CreateCommandList(d3dDevice, allocator);

			allocator->SetName(StringToWString(allocatorName).c_str());
			commandList->SetName(StringToWString(commandListName).c_str());

			commandAllocators.push_back(allocator);
			commandLists.push_back(commandList);
			open.push_back(false);
		}

		frame = 0U;
	}

	CComPtr<ID3D12GraphicsCommandList2>& CommandsProcessor::GetCommandList(bool OpenIfClosed)
	{
		if (!IsOpen() && OpenIfClosed)
		{
			ResetCommandList();
		}
		return commandLists.at(frame);
	}

	void CommandsProcessor::ResetCommandList()
	{
		using namespace DeviceUtils;
		auto& commandAllocator = commandAllocators[frame];
		commandAllocator->Reset();
		auto& commandList = commandLists[frame];
		ID3D12DescriptorHeap* ppHeaps[] = { GetCSUDescriptorHeap() };
		commandList->Reset(commandAllocator, nullptr);
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		open.at(frame) = true;
	}

	void CommandsProcessor::CloseCommandList()
	{
		if (IsOpen())
		{
			auto& commandList = commandLists[frame];
			DX::ThrowIfFailed(commandList->Close());
		}
		open.at(frame) = false;
	}
}
