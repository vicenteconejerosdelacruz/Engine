#include "pch.h"
#include "LoadingProcessor.h"
#include <map>
#include <Renderer.h>

extern std::unique_ptr<JRenderer> renderer;

namespace
{
	// Estructura para gestionar el retiro diferido por latencia de GPU
	struct RetiredProcessor
	{
		std::unique_ptr<CommandsProcessor> processor;
		unsigned int framesRemaining{ 0U };
	};

	std::map<size_t, std::unique_ptr<CommandsProcessor>> commandsProcessors;
	std::map<size_t, std::unique_ptr<std::atomic_uint>> commandsProcessorDepth;

	// Procesadores diferidos que están finalizando su ejecución en la GPU
	std::vector<RetiredProcessor> retiredProcessors;
	std::mutex processorMutex;
}

LoadingProcessor::LoadingProcessor(CommandsProcessor& p_cmd, std::unique_ptr<std::atomic_uint>& p_depth) :cmd(p_cmd), depth(p_depth)
{
	threadId = nostd::threadIdHash();
	unsigned int prev = depth->fetch_add(1U, std::memory_order_acq_rel);
	if (prev == 0U)
	{
		cmd.ResetCommandList();
	}
}

LoadingProcessor::~LoadingProcessor()
{
	size_t id = threadId;
	unsigned int prev = depth->fetch_sub(1U, std::memory_order_acq_rel);

	if (prev == 1U)
	{
		cmd.CloseCommandList();
		cmd.ExecuteCommandList();

		std::lock_guard<std::mutex> lock(processorMutex);

		// 1. Desvincular el procesador activo del mapa para liberar la clave 'threadId' en el acto
		auto itCmd = commandsProcessors.find(id);
		if (itCmd != commandsProcessors.end())
		{
			std::unique_ptr<DeviceUtils::CommandsProcessor> orphanCmd = std::move(itCmd->second);
			commandsProcessors.erase(itCmd);
			commandsProcessorDepth.erase(id);

			// 2. Transferir la propiedad al vector de retiro con la latencia de numFrames
			retiredProcessors.push_back(RetiredProcessor{
				.processor = std::move(orphanCmd),
				.framesRemaining = JRenderer::numFrames
				}
			);
		}
	}
}

LoadingProcessor CreateLoadingProcessor()
{
	size_t id = nostd::threadIdHash();
	std::lock_guard<std::mutex> lock(processorMutex);

	if (!commandsProcessors.contains(id))
	{
		commandsProcessors.insert_or_assign(
			id,
			std::make_unique<DeviceUtils::CommandsProcessor>(renderer->d3dDevice, 1)
		);
		commandsProcessorDepth.insert_or_assign(
			id,
			std::make_unique<std::atomic_uint>(0U)
		);
	}

	return LoadingProcessor(*commandsProcessors.at(id), commandsProcessorDepth.at(id));
}

void LoadingProcessorsStep()
{
	std::lock_guard<std::mutex> lock(processorMutex);

	if (retiredProcessors.empty())
		return;

	// Limpieza en O(1) recorriendo en sentido inverso usando swap + pop_back
	for (size_t i = retiredProcessors.size(); i-- > 0;)
	{
		auto& item = retiredProcessors[i];
		if (item.framesRemaining > 0)
		{
			item.framesRemaining--;
		}
		else
		{
			// Transcurridos los numFrames, la GPU liberó el allocator/list.
			// Al aplicar swap y pop_back, se destruye el unique_ptr de forma segura.
			std::swap(retiredProcessors[i], retiredProcessors.back());
			retiredProcessors.pop_back();
		}
	}
}