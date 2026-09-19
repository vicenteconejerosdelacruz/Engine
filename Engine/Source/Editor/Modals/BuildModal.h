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

	static void GatherFilesFromLevel(nlohmann::json& level, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream);
	static void GatherRecursiveFilesFromFolder(const std::filesystem::path& folderPath, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream);
	static void GatherExecutablesFromFolder(const std::filesystem::path& folderPath, std::set<std::filesystem::path>& filesToCopy, ThreadSafeStream& logStream);
	static void CopyFilesFromTargetFolderToBuildFolder(const std::set<std::filesystem::path>& filesToCopy, ThreadSafeStream& logStream);
	static void CopyFilesIntoBuildFolder(const std::set<std::filesystem::path>& filesToCopy, ThreadSafeStream& logStream);
	static void WriteBootLevelIntoBuildFolder(const std::string& bootLevelName, ThreadSafeStream& logStream);

	std::vector<std::string> files;
	std::vector<bool> includeFlags;
	int bootLevelSelected;
	bool building;
	bool completed;
	ThreadSafeStream logStream;
};