#include "pch.h"
#include <Level.h>
//scene objects
#include <Scene.h>
#include <Camera/Camera.h>
#include <Renderable/Renderable.h>
#include <Light/Light.h>
#include <Sound/SoundFX.h>
#include <Animated.h>
#include <Controller.h>
#if defined(_EDITOR)
#include <Editor.h>
#include <DefaultLevel.h>

using namespace Animation;
using namespace Editor;
namespace Editor {
	extern std::string currentLevelName;
	extern bool defaultLevel;
	extern bool levelModified;
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

	void LoadPendingLevel()
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
	}

	void LoadSceneObjects(nlohmann::json& j, std::string type, std::function<void(nlohmann::json&)> loader)
	{
		if (j.contains(type))
		{
			std::for_each(j.at(type).begin(), j.at(type).end(), [loader](nlohmann::json& json)
				{
					loader(json);
				}
			);
		}
	}

#if defined(_EDITOR)

	void LoadDefaultLevel()
	{
		using namespace Editor::DefaultLevel;
		using namespace Scene;
		Editor::levelModified = false;
		Editor::defaultLevel = true;
		LoadSceneObjects(GetDefaultLevelRenderables(), SceneObjectTypeJsonContainer.at(SO_Renderables), Scene::CreateRenderable);
		LoadSceneObjects(GetDefaultLevelCameras(), SceneObjectTypeJsonContainer.at(SO_Cameras), Scene::CreateCamera);
		LoadSceneObjects(GetDefaultLevelLights(), SceneObjectTypeJsonContainer.at(SO_Lights), Scene::CreateLight);
		LoadSceneObjects(GetDefaultLevelSounds(), SceneObjectTypeJsonContainer.at(SO_SoundEffects), Scene::CreateSoundFX);

		MapControllers();
	}
#endif

	void LoadLevel(std::filesystem::path level)
	{
#if defined(_EDITOR)
		Editor::levelModified = false;
		Editor::defaultLevel = false;
#endif
		std::string pathStr = "" + (std::filesystem::exists(level) ? level.generic_string() : (defaultLevelsFolder + level.generic_string() + ".json"));
		OutputDebugStringA(std::string("Loading level: " + pathStr + "\n").c_str());

		std::ifstream file(pathStr);
		bool isOpen = file.is_open();
		nlohmann::json data = nlohmann::json::parse(file);

		DestroySceneObjects();

		LoadSceneObjects(data, SceneObjectTypeJsonContainer.at(SO_Renderables), Scene::CreateRenderable);
		LoadSceneObjects(data, SceneObjectTypeJsonContainer.at(SO_Cameras), Scene::CreateCamera);
		LoadSceneObjects(data, SceneObjectTypeJsonContainer.at(SO_Lights), Scene::CreateLight);
		LoadSceneObjects(data, SceneObjectTypeJsonContainer.at(SO_SoundEffects), Scene::CreateSoundFX);

		MapControllers();

		file.close();
#if defined(_EDITOR)
		Editor::currentLevelName = level.filename().string();
		Editor::MarkScenePanelAssetsAsDirty();
#endif
	}

	void DestroySceneObjects()
	{
		using namespace Scene;
		using namespace Animation;

		//Destroy the lights(this will destroy the lights and it's cbvs)
		DestroyLights();

		//Destroy the cameras(this will destroy the cameras and the render passes)
		DestroyCameras();

		//Destroy sound instances
		DestroySoundEffects();

		//Destroy animated(this will destroy constants buffers)
		DestroyAnimated();

		//Destroy the renderables(this will detach the renderables from the cameras and destroy the renderables, materials, cbv, meshes, etc)
		DestroyRenderables();
	}
}