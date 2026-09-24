#include "pch.h"
#include "CommandsProcessor.h"
#include <DeviceUtils/D3D12Device/Builder.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <DirectXHelper.h>
#include <Renderer.h>

extern std::unique_ptr<JRenderer> renderer;

namespace DeviceUtils
{
	CommandsProcessor::CommandsProcessor(CComPtr<ID3D12Device2> d3dDevice, size_t capacity, size_t id)
		: id(id), frame(0U)
	{
		using namespace nostd;

		// Inicializamos el vector con el tamaño requerido directamente
		openedFrames = std::vector<std::atomic<bool>>(capacity);

		for (size_t i = 0; i < capacity; ++i)
		{
			std::string allocatorName = "commandAllocator:" + std::to_string(id) + "/[" + std::to_string(i) + "]";
			std::string commandListName = "commandList:" + std::to_string(id) + "/[" + std::to_string(i) + "]";

			auto allocator = CreateCommandAllocator(d3dDevice);
			auto commandList = CreateCommandList(d3dDevice, allocator);

			allocator->SetName(StringToWString(allocatorName).c_str());
			commandList->SetName(StringToWString(commandListName).c_str());

			//close the command list so allocator->Reset doesn't fail
			//DX::ThrowIfFailed(commandList->Close());

			commandAllocators.push_back(allocator);
			commandLists.push_back(commandList);

			// Inicializar en false
			openedFrames[i].store(false, std::memory_order_relaxed);
		}
	}

	// Move Constructor
	CommandsProcessor::CommandsProcessor(CommandsProcessor&& other) noexcept
		: id(other.id),
		commandAllocators(std::move(other.commandAllocators)),
		commandLists(std::move(other.commandLists)),
		frame(other.frame),
		loadingPool(std::move(other.loadingPool)),
		postExecutionCallbacks(std::move(other.postExecutionCallbacks)),
		preDeletionCallbacks(std::move(other.preDeletionCallbacks))
	{
		// Los std::atomic no se pueden mover por defecto, copiamos su valor atómicamente
		openedFrames = std::vector<std::atomic<bool>>(other.openedFrames.size());
		for (size_t i = 0; i < other.openedFrames.size(); ++i)
		{
			openedFrames[i].store(other.openedFrames[i].load(std::memory_order_relaxed), std::memory_order_relaxed);
		}
	}

	// Move Assignment
	CommandsProcessor& CommandsProcessor::operator=(CommandsProcessor&& other) noexcept
	{
		if (this != &other)
		{
			id = other.id;
			commandAllocators = std::move(other.commandAllocators);
			commandLists = std::move(other.commandLists);
			frame = other.frame;
			loadingPool = std::move(other.loadingPool);
			postExecutionCallbacks = std::move(other.postExecutionCallbacks);
			preDeletionCallbacks = std::move(other.preDeletionCallbacks);

			openedFrames = std::vector<std::atomic<bool>>(other.openedFrames.size());
			for (size_t i = 0; i < other.openedFrames.size(); ++i)
			{
				openedFrames[i].store(other.openedFrames[i].load(std::memory_order_relaxed), std::memory_order_relaxed);
			}
		}
		return *this;
	}

	CommandsProcessor::~CommandsProcessor()
	{
		for (auto& cb : preDeletionCallbacks)
		{
			cb();
		}
	}

	CComPtr<ID3D12GraphicsCommandList2>& CommandsProcessor::GetCommandList()
	{
		return commandLists.at(frame);
	}

	void CommandsProcessor::ResetCommandList()
	{
		using namespace DeviceUtils;

		// 1. ESPERA ROBUSTA (While + Acquire):
		// Bloquea MIENTRAS sea true. Evita race conditions y desvelos espontáneos.
		while (openedFrames.at(frame).load(std::memory_order_acquire) == true)
		{
			openedFrames.at(frame).wait(true, std::memory_order_relaxed);
		}

		auto& commandAllocator = commandAllocators[frame];
		auto& commandList = commandLists[frame];

		commandAllocator->Reset();

		ID3D12DescriptorHeap* ppHeaps[] = { GetCSUDescriptorHeap() };
		DX::ThrowIfFailed(commandList->Reset(commandAllocator, nullptr));
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		// 2. Marcar como abierto
		openedFrames.at(frame).store(true, std::memory_order_release);
	}

	void CommandsProcessor::CloseCommandList()
	{
		auto& commandList = commandLists[frame];
		DX::ThrowIfFailed(commandList->Close());
	}

	void CommandsProcessor::ExecuteCommandList()
	{
		unsigned int frameIndex = frame;
		renderer->ExecuteCommands(commandLists[frame], [&, frameIndex]
			{
				auto isBound = [](SUUUID suuuid) { return GetSceneUnit(std::get<0>(suuuid))->IsBound(std::get<1>(suuuid)); };
				auto makeReady = [&](auto& so)
					{
						if (so->RenderReady() || !isBound(so())) return;
						so->RenderReady(true);
					};
				std::map<SceneObjectType, std::function<void(SUUUID)>> loadedMap =
				{
					{ SO_Renderables, [&](RenderableID r) { makeReady(r); }},
					{ SO_Cameras, [&](CameraID c) { makeReady(c); }},
					{ SO_Lights, [&](LightID l) { makeReady(l); }},
				};

				std::vector<std::tuple<SceneObjectType, SUUUID>> toClean;
				for (auto& [type, uuidset] : loadingPool)
				{
					for (auto& uuid : uuidset)
					{
						if (!SceneObjectExists(uuid))
							continue;
						loadedMap.at(type)(uuid);
						toClean.push_back(std::make_tuple(type, uuid));
					}
				}
				for (auto& wipe : toClean)
				{
					loadingPool.at(std::get<0>(wipe)).erase(std::get<1>(wipe));
					if (loadingPool.at(std::get<0>(wipe)).size() == 0ULL)
					{
						loadingPool.erase(std::get<0>(wipe));
					}
				}
				for (auto& cb : postExecutionCallbacks)
				{
					cb();
				}
				postExecutionCallbacks.clear();

				// 3. Notificación de liberación (Release) al hilo principal:
				openedFrames.at(frameIndex).store(false, std::memory_order_release);
				openedFrames.at(frameIndex).notify_all(); // notify_all en lugar de notify_one si múltiples hilos leen la señal
			}
		);
	}

	void CommandsProcessor::Next()
	{
		frame = (frame + 1) % static_cast<unsigned int>(commandLists.size());
	}

	void CommandsProcessor::LoadingPoolInsert(SceneObjectType type, SUUUID uuid)
	{
		loadingPool[type].insert(uuid);
	}

	void CommandsProcessor::RunPostExecution(std::function<void()> cb)
	{
		postExecutionCallbacks.push_back(cb);
	}

	void CommandsProcessor::RunPreDeletion(std::function<void()> cb)
	{
		preDeletionCallbacks.push_back(cb);
	}
}