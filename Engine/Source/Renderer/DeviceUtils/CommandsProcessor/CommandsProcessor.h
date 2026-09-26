#pragma once
#include <map>
#include <vector>
#include <memory>
#include <atomic>
#include <functional>
#include <set>
#include <atlbase.h>
#include <d3dx12.h>
#include <Renderer.h>

namespace DeviceUtils
{
	struct CommandsProcessor
	{
		CommandsProcessor(CComPtr<ID3D12Device2> d3dDevice, size_t capacity, size_t id = nostd::threadIdHash());
		~CommandsProcessor();

		// Implementación manual de Move Constructor y Assignment para std::atomic
		CommandsProcessor(CommandsProcessor&& other) noexcept;
		CommandsProcessor& operator=(CommandsProcessor&& other) noexcept;

		// Bloqueo explícito de copia
		CommandsProcessor(const CommandsProcessor&) = delete;
		CommandsProcessor& operator=(const CommandsProcessor&) = delete;

		// Consulta segura sin 'volatile'
		bool IsOpen() const { return openedFrames.at(frame).load(std::memory_order_acquire); }

		CComPtr<ID3D12GraphicsCommandList2>& GetCommandList();
		void ResetCommandList();
		void CloseCommandList();
		void ExecuteCommandList();
		void Next();
		void LoadingPoolInsert(SceneObjectType type, SUUUID uuid);
		void RunPostExecution(std::function<void()> cb);
		void RunPreDeletion(std::function<void()> cb);

		size_t id;
		std::vector<CComPtr<ID3D12CommandAllocator>> commandAllocators;
		std::vector<CComPtr<ID3D12GraphicsCommandList2>> commandLists;

		// Vector directo de atómicos en contigüidad de memoria
		std::vector<std::atomic<bool>> openedFrames;

		unsigned int frame{ 0U };
		std::map<SceneObjectType, std::set<SUUUID>> loadingPool;
		std::vector<std::function<void()>> postExecutionCallbacks;
		std::vector<std::function<void()>> preDeletionCallbacks;
	};
}
