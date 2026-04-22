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
	{
		using namespace nostd;
		this->id = id;

		for (int i = 0; i < capacity; ++i)
		{
			std::string allocatorName = "commandAllocator:" + std::to_string(id) + "/[" + std::to_string(i) + "]";
			std::string commandListName = "commandList:" + std::to_string(id) + "/[" + std::to_string(i) + "]";

			auto allocator = CreateCommandAllocator(d3dDevice);
			auto commandList = CreateCommandList(d3dDevice, allocator);

			allocator->SetName(StringToWString(allocatorName).c_str());
			commandList->SetName(StringToWString(commandListName).c_str());

			commandAllocators.push_back(allocator);
			commandLists.push_back(commandList);
			openedFrames.push_back(std::make_unique<std::atomic_bool>(false));
		}

		frame = 0U;
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
		if (openedFrames.at(frame)->load() == true)
		{
			openedFrames.at(frame)->wait(true);
		}
		auto& commandAllocator = commandAllocators[frame];
		commandAllocator->Reset();
		auto& commandList = commandLists[frame];
		ID3D12DescriptorHeap* ppHeaps[] = { GetCSUDescriptorHeap() };
		commandList->Reset(commandAllocator, nullptr);
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		openedFrames.at(frame)->store(true);
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
				openedFrames.at(frameIndex)->store(false);
				openedFrames.at(frameIndex)->notify_one();
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
