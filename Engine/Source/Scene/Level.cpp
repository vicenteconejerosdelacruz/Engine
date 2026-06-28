#include "pch.h"
#include <Level.h>
//scene objects
#include <Scene.h>
#include <Application.h>
#include <fstream>
#include <SceneObject.h>
#include <Physics.h>
#if defined(_EDITOR)
#include <DefaultLevel.h>
#include <BootLevel/BootLevel.h>

namespace Editor {
	void MarkScenePanelAssetsAsDirty();
	extern void CreateSceneUnitBoundingBox(SceneUnitId id);
	extern void CreateSceneUnitBillboards(SceneUnitId id);
	extern void CreateRegisteredBillboards(SceneUnitId id);
	extern void CreateSceneUnitEditorIndependentCamera(SceneUnitId id);
	extern void CopySceneUnitEditorCameraRenderPasses(SceneUnitId id);
	extern void CreatePickingPass(SceneUnitId id);
	extern void CreateSceneUnitPhysicsController(SceneUnitId id);
	extern void SwitchToSceneUnitEditorCamera(SceneUnitId id);
}
#endif

namespace Scene
{
	extern std::set<SceneUnitId> pendingPhysicsCreation;
};

namespace Scene::Level
{
	using namespace Scene;
	using namespace Physics;

	std::filesystem::path levelToLoad;
#if defined(_EDITOR)
	bool loadDefaultLevel = false;
#endif

	void SetLevelToLoad(std::string levelName)
	{
		levelToLoad = levelName;
	}

#if defined(_EDITOR)
	void SetDefaultLevelToLoad()
	{
		loadDefaultLevel = true;
	}
#endif

	bool PendingLevelToLoad()
	{
#if defined(_EDITOR)
		return !levelToLoad.empty() || loadDefaultLevel;
#else
		return !levelToLoad.empty();
#endif
	}

	nlohmann::json GetDefaultLevel()
	{
		using namespace Editor::DefaultLevel;

		OutputDebugStringA(std::string("Loading default level\n").c_str());

		nlohmann::json level = nlohmann::json::object({});
		level.merge_patch(GetDefaultLevelRenderables());
		level.merge_patch(GetDefaultLevelCameras());
		level.merge_patch(GetDefaultLevelLights());
		level.merge_patch(GetDefaultLevelSounds());
		level.merge_patch(GetDefaultLevelPhysicsScenes());

		return level;
	}

	nlohmann::json GetBootLevel()
	{
		using namespace Game::BootLevel;

		OutputDebugStringA(std::string("Loading boot level\n").c_str());

		nlohmann::json level = nlohmann::json::object({});
		level.merge_patch(GetBootLevelRenderables());
		level.merge_patch(GetBootLevelCameras());
		level.merge_patch(GetBootLevelLights());
		level.merge_patch(GetBootLevelSounds());

		return level;
	}

	nlohmann::json GetLevelFromFile(std::filesystem::path filename)
	{
		std::string pathStr = "" + (std::filesystem::exists(filename) ? filename.generic_string() : (defaultLevelsFolder + filename.generic_string() + (filename.has_extension() ? "" : ".json")));
		OutputDebugStringA(std::string("Loading level: " + pathStr + "\n").c_str());

		std::ifstream file(pathStr);
		bool isOpen = file.is_open();
		nlohmann::json level = nlohmann::json::parse(file);

		return level;
	}

	void LoadSceneObjects(std::unique_ptr<SceneUnit>& scene, nlohmann::json& j, std::string type, std::function<void(nlohmann::json&)> loader)
	{
		if (j.contains(type))
		{
			std::for_each(j.at(type).begin(), j.at(type).end(), [&scene, loader](nlohmann::json& json)
				{
					loader(json);
					scene->AddSceneObjectToUnboundPool(json.at("uuid"));
				}
			);
		}
	}

	static std::mutex loadLevelMutex;
	void LoadLevel(std::unique_ptr<SceneUnit>& scene, std::string filename, nlohmann::json data, std::function<void(std::string, unsigned int, unsigned int)> progress, bool initPhysX)
	{
		std::lock_guard<std::mutex> lock(loadLevelMutex);
		using namespace Scene;
#if defined(_EDITOR)
		using namespace Editor;
		scene->SetCanBuildAssetsTree(false);

		SceneUnitId id = scene->Id();

		if (!scene->IsIsolated())
		{
			CreateSceneUnitBillboards(id);
		}
#endif

		auto loading = CreateLoadingProcessor();

		CreateSceneUnitSceneObjects(id);

		unsigned int total = std::accumulate(SceneObjectTypeJsonContainer.begin(), SceneObjectTypeJsonContainer.end(), 0U, [&](unsigned int sum, auto& pair)
			{
				return sum + (data.contains(pair.second) ? static_cast<unsigned int>(data.at(pair.second).size()) : 0U);
			}
		);

		unsigned int count = 0U;

		std::vector<std::tuple<SceneObjectType, SUUUID>> objectsToLoad;

		LoadSceneObjects(scene, data, SceneObjectTypeJsonContainer.at(SO_Renderables), [&](nlohmann::json& json)
			{
				progress(json.at("name"), count, total);
				CreateRenderable(id, json);
				objectsToLoad.push_back(std::make_tuple(SO_Renderables, MAKESUUUID(id, JUUID(json.at("uuid")))));
				count++;
				progress(json.at("name"), count, total);
			}
		);
		LoadSceneObjects(scene, data, SceneObjectTypeJsonContainer.at(SO_Cameras), [&](nlohmann::json& json)
			{
				progress(json.at("name"), count, total);
				CreateCamera(id, json);
				objectsToLoad.push_back(std::make_tuple(SO_Cameras, MAKESUUUID(id, JUUID(json.at("uuid")))));
				count++;
				progress(json.at("name"), count, total);
			}
		);
		LoadSceneObjects(scene, data, SceneObjectTypeJsonContainer.at(SO_Lights), [&](nlohmann::json& json)
			{
				progress(json.at("name"), count, total);
				CreateLight(id, json);
				objectsToLoad.push_back(std::make_tuple(SO_Lights, MAKESUUUID(id, JUUID(json.at("uuid")))));
				count++;
				progress(json.at("name"), count, total);
			}
		);
		LoadSceneObjects(scene, data, SceneObjectTypeJsonContainer.at(SO_SoundEffects), [&](nlohmann::json& json)
			{
				progress(json.at("name"), count, total);
				CreateSoundFX(id, json);
				count++;
				progress(json.at("name"), count, total);
			}
		);
		LoadSceneObjects(scene, data, SceneObjectTypeJsonContainer.at(SO_PhysicScenes), [&](nlohmann::json& json)
			{
				progress(json.at("name"), count, total);
				CreatePhysicScene(id, json);
				count++;
				progress(json.at("name"), count, total);
			}
		);
		LoadSceneObjects(scene, data, SceneObjectTypeJsonContainer.at(SO_Triggers), [&](nlohmann::json& json)
			{
				progress(json.at("name"), count, total);
				CreateTrigger(id, json);
				count++;
				progress(json.at("name"), count, total);
			}
		);
		LoadSceneObjects(scene, data, SceneObjectTypeJsonContainer.at(SO_Boundaries), [&](nlohmann::json& json)
			{
				progress(json.at("name"), count, total);
				CreateBoundary(id, json);
				count++;
				progress(json.at("name"), count, total);
			}
		);
		LoadSceneObjects(scene, data, SceneObjectTypeJsonContainer.at(SO_SceneControllers), [&](nlohmann::json& json)
			{
				progress(json.at("name"), count, total);
				CreateSceneController(id, json);
				count++;
				progress(json.at("name"), count, total);
			}
		);

#if defined(_EDITOR)
		if (!scene->IsIsolated())
		{
			CreatePickingPass(id);
			CreateSceneUnitEditorIndependentCamera(id);
			CreateSceneUnitPhysicsController(id);
		}
#endif

		BindSceneObjects(id);
#if defined(_EDITOR)
		if (!scene->IsIsolated())
		{
			CreateSceneUnitBoundingBox(id);
			CreateRegisteredBillboards(id);
		}
#endif
		if (initPhysX)
		{
		CreatePhysicsObjectsBehaviors(id);
		}
		else
		{
			pendingPhysicsCreation.insert(id);
		}
		//leave this to last
		MapControllers(id);
#if defined(_EDITOR)
		if (!scene->IsIsolated())
		{
			CopySceneUnitEditorCameraRenderPasses(id);
		}
#endif

		for (auto& intoThePool : objectsToLoad)
		{
			loading.cmd.LoadingPoolInsert(
				std::get<0>(intoThePool),
				std::get<1>(intoThePool)
			);
		}

#if defined(_EDITOR)
		scene->SetCanBuildAssetsTree(true);
#endif
	}
}