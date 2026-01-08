#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <nlohmann/json.hpp>
#include <Renderer.h>
#include <Templates.h>
#include <SceneUnit.h>
#include <SceneObject.h>

namespace DX { class StepTimer; }

using namespace DeviceUtils;

namespace Scene
{
	void CreateSceneLevelAsync(std::string filename, nlohmann::json level, std::function<void(SceneUnitId)> levelLoaded, std::function<void(std::string, unsigned int, unsigned int)> progress = [](std::string, unsigned int, unsigned int) {});
	void AttachLevelIntoScene(SceneUnitId parentUnit, std::string filename, nlohmann::json level, std::function<void(SceneUnitId)> levelLoaded, std::function<void(std::string, unsigned int, unsigned int)> progress = [](std::string, unsigned int, unsigned int) {});

	std::unique_ptr<SceneUnit>& CreateScene(SceneUnitId unit = nostd::threadIdHash(), std::string unitName = std::to_string(nostd::threadIdHash()), unsigned int numProcessors = Renderer::numFrames);
	std::unique_ptr<SceneUnit>& CreateAttachableScene(SceneUnitId parentUnit, SceneUnitId unit = nostd::threadIdHash(), std::string unitName = std::to_string(nostd::threadIdHash()));
	void DestroyScene(SceneUnitId unit);
	void DestroyScenes(bool inmediate = false);
	bool SceneUnitExits(SceneUnitId unit);
	std::unique_ptr<SceneUnit>& GetSceneUnit(SceneUnitId unit = nostd::threadIdHash());
	size_t GetSceneUnitsCount();
	void MergeAttachedSceneUnits();

	void ResizeReleaseScenePasses();
	void ResizeScenePasses(unsigned int width, unsigned int height);

	std::set<JUUID>& GetSceneObjects(SceneUnitId unit, SceneObjectType type);
	//std::set<JUUID> GetSceneObjects(SceneObjectType type);

	std::unordered_map<JUUID, SceneObjectType>& GetSceneObjectsTypes(SceneUnitId id);

	std::set<JUUID>& GetUnboundedSceneObjects(SceneUnitId id);

	SceneObjectType GetSceneObjectType(SceneUnitId id, JUUID uuid);

	void CreateSceneObject(SceneUnitId id, SceneObjectType so, nlohmann::json json);

	bool SceneObjectExists(SceneUnitId unit, JUUID uuid);

	//SceneUnitId GetSceneObjectSceneUnitId(JUUID uuid);

	//template<SceneObjectType T, typename J>
	//inline void CreateJsonSceneObject(nlohmann::json& json, auto getTypesSceneObjects, SceneUnitId unit = nostd::threadIdHash())
	//{
	//	JUUID uuid = json.at("uuid");
	//	JNAME name = json.at("name");

	//	//these are trackers
	//	auto& uuidSet = GetSceneObjects(unit, T);
	//	auto& unitTypesMap = GetSceneObjectsTypes(unit);
	//	auto& typesMap = GetGlobalSceneObjectsTypes();

	//	//this one is where the object will exits
	//	auto& sceneObjects = getTypesSceneObjects();

	//	//collide
	//	if (sceneObjects.contains(uuid) || uuidSet.contains(uuid) || unitTypesMap.contains(uuid) || typesMap.contains(uuid))
	//	{
	//		assert(!!!"creation collision");
	//	}

	//	//build
	//	std::unique_ptr<J> jT = std::make_unique<J>(json);
	//	jT->unit = unit;
	//	//track
	//	uuidSet.insert(uuid);
	//	unitTypesMap.insert_or_assign(uuid, T);
	//	typesMap.insert_or_assign(uuid, T);

	//	//store and initialize
	//	sceneObjects.insert_or_assign(uuid, std::make_tuple(name, std::move(jT)));
	//	std::get<1>(sceneObjects.at(uuid))->Initialize();
	//}

	template<SceneObjectType T, typename J>
	inline void CreateJsonSUSceneObject(SceneUnitId id, nlohmann::json& json, auto getTypesSceneObjects)
	{
		JUUID uuid = json.at("uuid");
		JNAME name = json.at("name");

		//these are trackers
		auto& uuidSet = GetSceneObjects(id, T);
		auto& unitTypesMap = GetSceneObjectsTypes(id);

		//this one is where the object will exits
		auto& sceneObjects = getTypesSceneObjects(id);

		//collide
		if (sceneObjects.contains(uuid) || uuidSet.contains(uuid) || unitTypesMap.contains(uuid))
		{
			assert(!!!"creation collision");
		}

		//build
		std::unique_ptr<J> jT = std::make_unique<J>(id, json);

		//track
		uuidSet.insert(uuid);
		unitTypesMap.insert_or_assign(uuid, T);

		//store and initialize
		sceneObjects.insert_or_assign(uuid, std::make_tuple(name, std::move(jT)));
		std::get<1>(sceneObjects.at(uuid))->Initialize();
	}

	//template<SceneObjectType T, typename J>
	//inline void DeleteJsonSceneObject(JUUID uuid, auto getTypesSceneObjects)
	//{
	//	SceneUnitId unit = GetSceneObjectSceneUnitId(uuid);
	//	auto& uuidSet = GetSceneObjects(unit, T);
	//	auto& typesMap = GetSceneObjectsTypes(unit);
	//	auto& sceneObjects = getTypesSceneObjects();

	//	if (!sceneObjects.contains(uuid) || !uuidSet.contains(uuid) || !typesMap.contains(uuid))
	//	{
	//		assert(!!!"uuid is not present in a record set");
	//	}

	//	auto& so = std::get<1>(sceneObjects.at(uuid));
	//	so->UnbindFromScene();
	//	uuidSet.erase(uuid);
	//	typesMap.erase(uuid);
	//	sceneObjects.erase(uuid);
	//}

	template<SceneObjectType T, typename J>
	inline void DeleteJsonSUSceneObject(SceneUnitId id, JUUID uuid, auto getTypesSceneObjects)
	{
		auto& uuidSet = GetSceneObjects(id, T);
		auto& typesMap = GetSceneObjectsTypes(id);
		auto& sceneObjects = getTypesSceneObjects(id);

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

	void ResetRenderableScenes();
	void EnableSceneUnitRendering(SceneUnitId id);
	void RemoveSceneUnitRendering(SceneUnitId id);

	void BindSceneObjects(SceneUnitId id);
	JUUID CloneSceneObject(SceneUnitId id, JUUID, nlohmann::json parameters = {});
	void BindToScene(SceneUnitId id, JUUID uuidA, JUUID uuidB);
	void UnbindFromScene(SceneUnitId id, JUUID uuidA);
	void UnbindFromScene(SceneUnitId id, JUUID uuidA, JUUID uuidB);
	void SceneObjectsStep(DX::StepTimer& timer);
	void WriteConstantsBuffers(SceneUnitId id);
	void RenderSceneShadowMaps(SceneUnitId id);
	void RenderSceneCameras(SceneUnitId id);

	void AnimableStep(SceneUnitId id, double elapsedSeconds);
	void SceneRender();
	void ScenePostRender();
	void RunComputeShaders();
	void SolveComputeShaders();
	void DeletedScenes();

	SceneObject* GetSceneObjectPointer(SceneUnitId id, JUUID uuid);

#if defined(_EDITOR)
	//for drawing the panel
	std::function<std::vector<JUUIDName>()> GetSceneObjectsByType(SceneUnitId id, SceneObjectType typeToGet);
	std::vector<JUUIDName> GetSUSceneObjectsByType(SceneUnitId id, SceneObjectType typeToGet);
	std::vector<JUUIDName> GetSceneObjectsTypesList(SceneUnitId id);
	std::vector<std::pair<std::string, JsonToEditorValueType>> GetSceneObjectAttributes(SceneObjectType so);
	std::map<std::string, JEdvEditorDrawerFunction> GetSceneObjectDrawers(SceneObjectType so);
	std::map<std::string, JEdvEditorDrawerFunction> GetSceneObjectPreviewers(SceneObjectType so);
	//for creating the scene objects
	nlohmann::json GetSceneObjectJson(SceneObjectType so);
	std::vector<std::string> GetSceneObjectRequiredAttributes(SceneObjectType so);
	std::map<std::string, JEdvCreatorDrawerFunction> GetSceneObjectCreatorDrawers(SceneObjectType so);
	std::map<std::string, JEdvCreatorValidatorFunction> GetSceneObjectValidators(SceneObjectType so);
	void DeleteSceneObjectFromEditor(SceneUnitId id, JUUID uuid);
#endif
}