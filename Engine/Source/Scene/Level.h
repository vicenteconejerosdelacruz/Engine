#pragma once

namespace Scene
{
	class SceneUnit;
};

namespace Scene::Level
{
	void SetLevelToLoad(std::string levelName);
#if defined(_EDITOR)
	void SetDefaultLevelToLoad();
#endif
	bool PendingLevelToLoad();

	//level handling
	nlohmann::json GetDefaultLevel();
	nlohmann::json GetBootLevel();

	nlohmann::json GetLevelFromFile(std::filesystem::path level);

	void LoadSceneObjects(std::unique_ptr<SceneUnit>& scene, nlohmann::json& j, std::string type, std::function<void(nlohmann::json&)> loader);
	void LoadLevel(std::unique_ptr<SceneUnit>& scene, std::string filename, nlohmann::json data, std::function<void(std::string, unsigned int, unsigned int)> progress = [](std::string, unsigned int, unsigned int) {}, bool initPhysX = true);
};

