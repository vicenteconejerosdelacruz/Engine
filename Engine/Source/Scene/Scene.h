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
	void CreateIsolatedSceneLevelAsync(std::string filename, nlohmann::json level, std::function<void(SceneUnitId)> levelLoaded, std::function<void(std::string, unsigned int, unsigned int)> progress = [](std::string, unsigned int, unsigned int) {});
	void AttachLevelIntoScene(SceneUnitId parentUnit, std::string filename, nlohmann::json level, std::function<void(SceneUnitId)> levelLoaded, std::function<void(std::string, unsigned int, unsigned int)> progress = [](std::string, unsigned int, unsigned int) {});

	std::unique_ptr<SceneUnit>& CreateScene(SceneUnitId unit, std::string unitName = std::to_string(nostd::threadIdHash()), unsigned int numProcessors = Renderer::numFrames);
	void DestroyScene(SceneUnitId unit);
	void DestroyScenes(bool inmediate = false);
	bool SceneUnitExits(SceneUnitId unit);
	void SceneUnitsStep();
	std::unique_ptr<SceneUnit>& GetSceneUnit(SceneUnitId unit);
	size_t GetSceneUnitsCount();
	std::set<SceneUnitId> GetSceneUnitIds();
	SceneUnitId GetNextSceneUnitId(SceneUnitId id);

	bool SceneIsIsolated(SceneUnitId id);

	void ResizeReleaseScenePasses();
	void ResizeScenePasses(unsigned int width, unsigned int height);

	std::set<JUUID>& GetSceneObjects(SceneUnitId unit, SceneObjectType type);

	std::unordered_map<JUUID, SceneObjectType>& GetSceneObjectsTypes(SceneUnitId id);

	std::set<JUUID>& GetUnboundedSceneObjects(SceneUnitId id);

	SceneObjectType GetSceneObjectType(SceneUnitId id, JUUID uuid);

	void CreateSceneObject(SceneUnitId id, SceneObjectType so, nlohmann::json json);

	bool SceneObjectExists(SceneUnitId unit, JUUID uuid);

	void MoveSceneObjectUnit(JUUID uuid, SceneUnitId fromId, SceneUnitId toId);

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