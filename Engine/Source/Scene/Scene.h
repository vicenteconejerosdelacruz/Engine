#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <nlohmann/json.hpp>
#include <SceneObject.h>

namespace DX { class StepTimer; }

namespace Scene
{
	std::set<JUUID>& GetSceneObjects(SceneObjectType type);
	std::unordered_map<JUUID, SceneObjectType>& GetSceneObjectsTypes();
	SceneObjectType GetSceneObjectType(JUUID uuid);
	bool SceneObjectExists(JUUID uuid);

	template<SceneObjectType T, typename J>
	inline void CreateJsonSceneObject(nlohmann::json& json, auto getTypesSceneObjects)
	{
		JUUID uuid = json.at("uuid");
		JNAME name = json.at("name");

		auto& uuidSet = GetSceneObjects(T);
		auto& typesMap = GetSceneObjectsTypes();
		auto& sceneObjects = getTypesSceneObjects();

		if (sceneObjects.contains(uuid) || uuidSet.contains(uuid) || typesMap.contains(uuid))
		{
			assert(!!!"creation collision");
		}

		std::unique_ptr<J> jT = std::make_unique<J>(json);
		sceneObjects.insert_or_assign(uuid, std::make_tuple(name, std::move(jT)));
		uuidSet.insert(uuid);
		typesMap.insert_or_assign(uuid, T);
		std::get<1>(sceneObjects.at(uuid))->Initialize();
	}

	template<SceneObjectType T, typename J>
	inline void DeleteJsonSceneObject(JUUID uuid, auto getTypesSceneObjects)
	{
		auto& uuidSet = GetSceneObjects(T);
		auto& typesMap = GetSceneObjectsTypes();
		auto& sceneObjects = getTypesSceneObjects();

		if (!sceneObjects.contains(uuid) || !uuidSet.contains(uuid) || !typesMap.contains(uuid))
		{
			assert(!!!"uuid is not present in a record set");
		}

		auto& so = std::get<1>(sceneObjects.at(uuid));
		so->UnbindFromScene();
		uuidSet.erase(uuid);
		typesMap.erase(uuid);
		sceneObjects.erase(uuid);
	}

	inline void WriteSceneObjectsJson(nlohmann::json& json, auto& sceneObjects)
	{

	}

	void BindSceneObjects();
	JUUID CloneSceneObject(JUUID, nlohmann::json parameters = {});

	void BindToScene(JUUID uuidA, JUUID uuidB);
	void UnbindFromScene(JUUID uuidA);
	void UnbindFromScene(JUUID uuidA, JUUID uuidB);
	void SceneObjectsStep(DX::StepTimer& timer);
	void WriteConstantsBuffers();
	void RenderSceneShadowMaps();
	void RenderSceneCameras();

	SceneObject* GetSceneObjectPointer(JUUID uuid);

#if defined(_EDITOR)
	std::function<std::vector<JUUIDName>()> GetSceneObjectsByType(SceneObjectType typeToGet);
	std::vector<JUUIDName> GetSceneObjectsTypesList();
	std::vector<std::pair<std::string, JsonToEditorValueType>> GetSceneObjectAttributes(SceneObjectType so);
	std::map<std::string, JEdvEditorDrawerFunction> GetSceneObjectDrawers(SceneObjectType so);
	std::map<std::string, JEdvEditorDrawerFunction> GetSceneObjectPreviewers(SceneObjectType so);
	std::vector<std::string> GetSceneObjectRequiredAttributes(SceneObjectType so);
	nlohmann::json GetSceneObjectJson(SceneObjectType so);
	std::map<std::string, JEdvCreatorDrawerFunction> GetSceneObjectCreatorDrawers(SceneObjectType so);
	std::map<std::string, JEdvCreatorValidatorFunction> GetSceneObjectValidators(SceneObjectType so);

	void CreateSceneObject(SceneObjectType so, nlohmann::json json);
	void DeleteSceneObjectFromEditor(JUUID uuid);
#endif
}