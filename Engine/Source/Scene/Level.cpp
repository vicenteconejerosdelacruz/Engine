#include "pch.h"
#include <Level.h>
//scene objects
#include <Scene.h>
#include <Application.h>
#include <fstream>
#include <SceneObject.h>
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
	extern void SwitchToSceneUnitEditorCamera(SceneUnitId id);
}
#endif

namespace Scene::Level {

	using namespace Scene;

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

	void LoadLevel(std::unique_ptr<SceneUnit>& scene, std::string filename, nlohmann::json data, std::function<void(std::string, unsigned int, unsigned int)> progress)
	{
		using namespace Scene;
#if defined(_EDITOR)
		using namespace Editor;

		if (!scene->IsIsolated())
		{
			CreateSceneUnitBillboards(scene->Id());
		}
#endif
		scene->ResetLoadingCommandList();
		scene->SetLoading(true);
		scene->SetCanSubmitLoading(false);

		CreateRenderableSceneObjects(scene->Id());
		CreateCameraSceneObjects(scene->Id());
		CreateLightSceneObjects(scene->Id());
		CreateSoundFXSceneObjects(scene->Id());

		std::vector<std::string> types =
		{
			SceneObjectTypeJsonContainer.at(SO_Renderables),
			SceneObjectTypeJsonContainer.at(SO_Cameras),
			SceneObjectTypeJsonContainer.at(SO_Lights),
			SceneObjectTypeJsonContainer.at(SO_SoundEffects),
		};

		unsigned int total = std::accumulate(types.begin(), types.end(), 0U, [&](unsigned int sum, std::string type)
			{
				return sum + (data.contains(type) ? static_cast<unsigned int>(data.at(type).size()) : 0U);
			}
		);
		unsigned int count = 0U;

		LoadSceneObjects(scene, data, SceneObjectTypeJsonContainer.at(SO_Renderables), [&](nlohmann::json& json)
			{
				progress(json.at("name"), count, total);
				CreateRenderable(scene->Id(), json);
				scene->InsertRenderableIntoLoadingPool(MAKESUUUID(scene->Id(), JUUID(json.at("uuid"))));
				count++;
				progress(json.at("name"), count, total);
			}
		);
		LoadSceneObjects(scene, data, SceneObjectTypeJsonContainer.at(SO_Cameras), [&](nlohmann::json& json)
			{
				progress(json.at("name"), count, total);
				CreateCamera(scene->Id(), json);
				scene->InsertCameraIntoLoadingPool(MAKESUUUID(scene->Id(), JUUID(json.at("uuid"))));
				count++;
				progress(json.at("name"), count, total);
			}
		);
		LoadSceneObjects(scene, data, SceneObjectTypeJsonContainer.at(SO_Lights), [&](nlohmann::json& json)
			{
				progress(json.at("name"), count, total);
				CreateLight(scene->Id(), json);
				scene->InsertLightIntoLoadingPool(MAKESUUUID(scene->Id(), JUUID(json.at("uuid"))));
				count++;
				progress(json.at("name"), count, total);
			}
		);
		LoadSceneObjects(scene, data, SceneObjectTypeJsonContainer.at(SO_SoundEffects), [&](nlohmann::json& json)
			{
				progress(json.at("name"), count, total);
				CreateSoundFX(scene->Id(), json);
				count++;
				progress(json.at("name"), count, total);
			}
		);

#if defined(_EDITOR)
		if (!scene->IsIsolated())
		{
			CreatePickingPass(scene->Id());
			CreateSceneUnitBoundingBox(scene->Id());
			CreateSceneUnitEditorIndependentCamera(scene->Id());
			CreateRegisteredBillboards(scene->Id());
		}
#endif

		MapControllers(scene->Id());
		BindSceneObjects(scene->Id());
#if defined(_EDITOR)
		if (!scene->IsIsolated())
		{
			CopySceneUnitEditorCameraRenderPasses(scene->Id());
		}
#endif
	}
}