#include "pch.h"
#include <Level.h>
//scene objects
#include <Scene.h>
#include <Application.h>
#include <fstream>
#include <SceneObject.h>
//#include <Camera/Camera.h>
//#include <Renderable/Renderable.h>
//#include <Light/Light.h>
//#include <Sound/SoundFX.h>
//#include <Animated.h>
//#include <Controller.h>
#if defined(_EDITOR)
//#include <Editor.h>
#include <DefaultLevel.h>
#include <BootLevel/BootLevel.h>

//using namespace Animation;
//using namespace Editor;
namespace Editor {
	//extern std::string currentLevelName;
	//extern bool defaultLevel;
	//extern bool levelModified;
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

	/*void LoadPendingLevel()
	{
		using namespace Game;

		DestroyControllers();

#if defined(_EDITOR)
		DestroyBillboards();
		DestroyEditorSceneObjectsReferences();
#endif
		DestroySceneObjects();
#if defined(_EDITOR)
		if (!loadDefaultLevel)
			LoadLevel(levelToLoad);
		else
			LoadDefaultLevel();
		loadDefaultLevel = false;
#else
		LoadLevel(levelToLoad);
#endif
		levelToLoad = "";
	}*/

	void LoadSceneObjects(std::unique_ptr<SceneUnit>& scene, nlohmann::json& j, std::string type, std::function<void(nlohmann::json&)> loader)
	{
		if (j.contains(type))
		{
			std::for_each(j.at(type).begin(), j.at(type).end(), [&scene, loader](nlohmann::json& json)
				{
					//if (scene->abortLoading->load())
					//	return;

					loader(json);
					scene->AddSceneObjectToUnboundPool(json.at("uuid"));
					//scene->unboundedSceneObjects.insert(json.at("uuid"));
				}
			);
		}
	}

	void LoadLevel(std::unique_ptr<SceneUnit>& scene, std::string filename, nlohmann::json data, std::function<void(std::string, unsigned int, unsigned int)> progress)
	{
		using namespace Scene;
#if defined(_EDITOR)
		using namespace Editor;

		//if (!scene->IsAttached() && !scene->IsIsolated())
		//{
		CreateSceneUnitBillboards(scene->Id());
		//}
#endif
		scene->ResetLoadingCommandList();
		scene->SetLoading(true);
		scene->SetCanSubmitLoading(false);

		CreateRenderableSUSceneObjects(scene->Id());
		CreateCameraSUSceneObjects(scene->Id());
		CreateLightSUSceneObjects(scene->Id());
		CreateSoundFXSUSceneObjects(scene->Id());

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
				CreateSURenderable(scene->Id(), json);
				scene->InsertRenderableIntoLoadingPool(json.at("uuid"));
				count++;
				progress(json.at("name"), count, total);
			}
		);
		LoadSceneObjects(scene, data, SceneObjectTypeJsonContainer.at(SO_Cameras), [&](nlohmann::json& json)
			{
				progress(json.at("name"), count, total);
				CreateSUCamera(scene->Id(), json);
				scene->InsertCameraIntoLoadingPool(json.at("uuid"));
				count++;
				progress(json.at("name"), count, total);
			}
		);
		LoadSceneObjects(scene, data, SceneObjectTypeJsonContainer.at(SO_Lights), [&](nlohmann::json& json)
			{
				progress(json.at("name"), count, total);
				CreateSULight(scene->Id(), json);
				scene->InsertLightIntoLoadingPool(json.at("uuid"));
				count++;
				progress(json.at("name"), count, total);
			}
		);
		LoadSceneObjects(scene, data, SceneObjectTypeJsonContainer.at(SO_SoundEffects), [&](nlohmann::json& json)
			{
				progress(json.at("name"), count, total);
				CreateSUSoundFX(scene->Id(), json);
				count++;
				progress(json.at("name"), count, total);
			}
		);

#if defined(_EDITOR)
		//if (!scene->IsAttached() && !scene->IsIsolated())
		{
			CreatePickingPass(scene->Id());
			CreateSceneUnitBoundingBox(scene->Id());
			CreateSceneUnitEditorIndependentCamera(scene->Id());
			CreateRegisteredBillboards(scene->Id());
		}
#endif

		//if (scene->abortLoading->load())
		//	return;

		//if (!scene->IsAttached())
		//{
		MapControllers(scene->Id());
		BindSceneObjects(scene->Id());
		//}
#if defined(_EDITOR)
		//if (!scene->IsAttached() && !scene->IsIsolated())
		//{
		CopySceneUnitEditorCameraRenderPasses(scene->Id());
		//}
		//Editor::currentLevelName = filename;
		//Editor::MarkScenePanelAssetsAsDirty();
#endif
	}

	void DestroySceneObjects(SceneUnitId unit)
	{
		using namespace Scene;
		using namespace Animation;

		//Destroy the lights(this will destroy the lights and it's cbvs)
		DestroyLights(unit);

		//Destroy the cameras(this will destroy the cameras and the render passes)
		DestroyCameras(unit);

		//Destroy sound instances
		DestroySoundEffects(unit);

		//Destroy animated(this will destroy constants buffers)
		DestroyAnimated(unit);

		//Destroy the renderables(this will detach the renderables from the cameras and destroy the renderables, materials, cbv, meshes, etc)
		DestroyRenderables(unit);
	}
}