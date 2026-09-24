#pragma once
#include "SimpleModal.h"
#include <filesystem>
#include "ThreadSafeStream.h"

struct BuildModal : SimpleModal
{
	void Init();
	void Draw();
	std::vector<std::string> GetYamlFiles(const std::string& rutaDirectorio);
	void StartBuildingProcess();
	std::string GetBootLevelName() const;

	std::vector<std::string> files;
	std::vector<bool> includeFlags;
	int bootLevelSelected;
	bool building;
	bool completed;
	ThreadSafeStream logStream;
};