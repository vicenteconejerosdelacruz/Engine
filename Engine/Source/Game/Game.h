#pragma once
#include <functional>
#include <string>
#include <nlohmann/json.hpp>

namespace Game
{
	void LoadLevelIntoSceneUnit(
		std::string name,
		std::function<nlohmann::json()> getLevel,
		std::function<void(SceneUnitId)> levelLoaded,
		std::function<void(std::string asset, unsigned int count, unsigned int total)> setProgress
	);
	void CreateSceneUnitGame(SceneUnitId id);
	void GameStep();
	void GameRender();
	void GamePostRender();
};
