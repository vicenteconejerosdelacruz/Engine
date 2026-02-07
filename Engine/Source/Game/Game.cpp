#include "pch.h"
#include "Game.h"
#include <Scene.h>

namespace Game
{
	std::unordered_map<SceneUnitId, GEngineSM> gameInstances;

	GEngineSM CreateEngineStateMachine(SceneUnitId id)
	{
		GEngineSM ges{ .unit = id };
		ges.currentState = GES_None;
		ges.onStep.insert_or_assign(GES_None, [](auto* sm) { sm->ChangeState(GES_Boot); });

		return ges;
	}

	void LoadLevelIntoSceneUnit(
		std::string name,
		std::function<nlohmann::json()> getLevel,
		std::function<void(SceneUnitId)> levelLoaded,
		std::function<void(std::string asset, unsigned int count, unsigned int total)> setProgress
	)
	{
		using namespace Scene;

		CreateSceneLevelAsync(
			name, getLevel(), [levelLoaded](SceneUnitId id)
			{
				CreateSceneUnitGame(id);
				levelLoaded(id);
			},
			setProgress
		);
	}

	void CreateSceneUnitGame(SceneUnitId id)
	{
		GEngineSM gesm = CreateEngineStateMachine(id);
		RegisterSceneUnitGame(id, gesm);
	}

	void RegisterSceneUnitGame(SceneUnitId id, GEngineSM& gesm)
	{
		gameInstances.insert_or_assign(id, gesm);
	}

	void DestroySeneUnitGame(SceneUnitId id)
	{
		if (gameInstances.contains(id))
			gameInstances.erase(id);
	}

	void GameStep()
	{
		for (auto& [_, gesm] : gameInstances)
		{
			gesm.Step();
		}
	}

	void GameRender()
	{
		//ges.Render();
	}

	void GamePostRender()
	{
		//ges.PostRender();
	}
};