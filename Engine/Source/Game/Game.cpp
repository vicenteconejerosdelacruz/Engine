#include "pch.h"
#include "Game.h"
#include <Scene.h>
#include <Level.h>

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

	void BootGame()
	{
		using namespace Scene::Level;

		std::string bootLevelName = "mainmenu.yaml"; // Nivel por defecto si falla la lectura
		std::filesystem::path bootIniPath = "boot.ini"; // O "../Build/boot.ini" según dónde se ejecute el binario

		// Intentamos leer el archivo boot.ini
		if (std::filesystem::exists(bootIniPath))
		{
			std::ifstream inFile(bootIniPath);
			if (inFile.is_open())
			{
				std::string line;
				if (std::getline(inFile, line))
				{
					// Limpiamos posibles espacios o saltos de línea al inicio/final
					if (!line.empty())
					{
						bootLevelName = line;
						// Opcional: si guardaste solo el nombre base o con espacios extra, puedes depurarlo aquí
					}
				}
				inFile.close();
			}
		}

		// Cargamos el nivel leído del boot.ini
		LoadLevelIntoSceneUnit(bootLevelName, [bootLevelName]() { return GetLevelFromFile(bootLevelName); },
			[](SceneUnitId id)
			{
				EnableSceneUnitRendering(id);
			},
			[](std::string asset, unsigned int count, unsigned int total) {}
		);
	}

	void GameStep()
	{
#if defined(_DEVELOPMENT)
		std::string event = std::string(__FUNCTION__);
		PIXScopedEvent(0, nostd::StringToWString(event).c_str());
#endif
		for (auto& [_, gesm] : gameInstances)
		{
			gesm.Step();
		}
	}
};