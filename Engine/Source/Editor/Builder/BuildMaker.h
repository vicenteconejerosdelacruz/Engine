#pragma once

void GenerateBuild(const std::vector<std::string>& buildParams, const std::string& bootParam);
std::thread CreateBuilderThread(std::function<std::set<std::string>()> getFileList, std::function<std::string()> getBootLevel,
	std::function<void()> onStart, std::function<void()> onComplete, ThreadSafeStream* logStream);
void GatherFilesFromLevel(nlohmann::json& level, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream);
void GatherRecursiveFilesFromFolder(const std::filesystem::path& folderPath, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream);
void GatherExecutablesFromFolder(const std::filesystem::path& folderPath, std::set<std::filesystem::path>& filesToCopy, ThreadSafeStream& logStream);
void CopyFilesFromTargetFolderToBuildFolder(const std::set<std::filesystem::path>& filesToCopy, ThreadSafeStream& logStream);
void CopyFilesIntoBuildFolder(const std::set<std::filesystem::path>& filesToCopy, ThreadSafeStream& logStream);
void WriteBootLevelIntoBuildFolder(const std::string& bootLevelName, ThreadSafeStream& logStream);