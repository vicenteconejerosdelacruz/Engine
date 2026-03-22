#pragma once
#include <map>
#include <atlbase.h>
#include <d3dx12.h>
#include <Renderer.h>

namespace DeviceUtils
{
	struct CommandsProcessor
	{

		CommandsProcessor(CComPtr<ID3D12Device2> d3dDevice, size_t capacity, size_t id = nostd::threadIdHash());
		bool IsOpen() { return openedFrames.at(frame)->load(); }
		CComPtr<ID3D12GraphicsCommandList2>& GetCommandList();
		void ResetCommandList();
		void CloseCommandList();
		void ExecuteCommandList();
		void Next();
		void LoadingPoolInsert(SceneObjectType type, SUUUID uuid);
		void RunPostExecution(std::function<void()> cb);

		size_t id;
		std::vector<CComPtr<ID3D12CommandAllocator>> commandAllocators;
		std::vector<CComPtr<ID3D12GraphicsCommandList2>> commandLists;
		std::vector<std::unique_ptr<std::atomic_bool>> openedFrames;
		unsigned int frame;
		std::map<SceneObjectType, std::set<SUUUID>> loadingPool;
		std::vector<std::function<void()>> postExecutionCallbacks;
	};
}
