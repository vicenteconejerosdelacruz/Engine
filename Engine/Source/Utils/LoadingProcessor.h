#pragma once
#include <DeviceUtils/CommandsProcessor/CommandsProcessor.h>

struct LoadingProcessor
{
	size_t threadId;
	CommandsProcessor& cmd;
	std::unique_ptr<std::atomic_uint>& depth;

	LoadingProcessor(CommandsProcessor& p_cmd, std::unique_ptr<std::atomic_uint>& p_depth);
	~LoadingProcessor();
};

LoadingProcessor CreateLoadingProcessor();
void LoadingProcessorsStep();