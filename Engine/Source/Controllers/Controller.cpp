#include "pch.h"
#include "Controller.h"
#include <map>
#include <NoStd.h>
#include <mutex>

namespace Editor
{
	extern bool IsPlaying(SceneUnitId id);
	extern void MarkSceneUnitAsModified(SceneUnitId id);
};

namespace Game
{
#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <ControllerAtt.h>
#include <JEnd.h>

#endif

	std::map<JUUID, std::unique_ptr<Controller>> controllersUUIDs;
	std::map<SUUUID, std::set<JUUID>> controllerUUIDBySUUUID;
	std::map<SUUUID, std::set<JUUIDName>> controllerUUIDNameBySUUUID;
	std::map<JUUID, SUUUID> controllersSUUUID;
	std::set<JUUID> mappedController;
	static std::mutex controllerMutex;

	Controller::Controller(nlohmann::json& json) :JObject(json)
	{
#include <Attributes/JInit.h>
#include <ControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <ControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include <ControllerAtt.h>
#include <JEnd.h>

		uuid(getUUID());
	}

	void Controller::JUpdate(nlohmann::json p)
	{
#if defined(_EDITOR)
		Editor::MarkSceneUnitAsModified(unit);
#endif
		JObject::JUpdate(p);
	}

	void Controller::JPatch(nlohmann::json p)
	{
#if defined(_EDITOR)
		Editor::MarkSceneUnitAsModified(unit);
#endif
		JObject::JPatch(p);
	}

#if defined(_EDITOR)
	void Controller::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <ControllerAtt.h>
#include <JEnd.h>
		if (j.contains("uuid"))
			j.erase("uuid");
	}
#endif

	void Controller::Map(SUUUID so) { unit = std::get<0>(so); sceneObject = so; }

	void Controller::Unmap() { std::get<0>(sceneObject) = 0; std::get<1>(sceneObject).clear(); }

	std::map<JUUID, std::unique_ptr<Controller>>& GetControllersUUIDs()
	{
		return controllersUUIDs;
	}

	std::map<SUUUID, std::set<JUUIDName>>& GetControllerUUIDNameBySUUUID()
	{
		return controllerUUIDNameBySUUUID;
	}

#if defined(_EDITOR)
	std::map<unsigned int, std::set<JUUID>> GetControllersPrioritySet(bool ignoreEditorPlay)
#else
	std::map<unsigned int, std::set<JUUID>> GetControllersPrioritySet()
#endif
	{
		std::map<unsigned int, std::set<JUUID>> prioritySet;

		for (auto& [suuuid, uuidset] : controllerUUIDBySUUUID)
		{
#if defined(_EDITOR)
			if (!Editor::IsPlaying(std::get<0>(suuuid)) && !ignoreEditorPlay) continue;
#endif
			for (auto& uuid : uuidset)
			{
				prioritySet[controllersUUIDs.at(uuid)->priority()].insert(uuid);
			}
		}
		return prioritySet;
	}

#if defined(_EDITOR)
	std::map<unsigned int, std::set<JUUID>> GetControllersPrioritySet(SceneUnitId id, bool ignoreEditorPlay)
#else
	std::map<unsigned int, std::set<JUUID>> GetControllersPrioritySet(SceneUnitId id)
#endif
	{
		std::map<unsigned int, std::set<JUUID>> prioritySet;

		for (auto& [suuuid, uuidset] : controllerUUIDBySUUUID)
		{
#if defined(_EDITOR)
			if (!Editor::IsPlaying(std::get<0>(suuuid)) && !ignoreEditorPlay) continue;
#endif
			if (std::get<0>(suuuid) != id) continue;

			for (auto& uuid : uuidset)
			{
				prioritySet[controllersUUIDs.at(uuid)->priority()].insert(uuid);
			}
		}
		return prioritySet;
	}

	JUUID RegisterController(std::string controllerName, SUUUID sceneObject, std::unique_ptr<Controller>& controller)
	{
		std::lock_guard<std::mutex> lock(controllerMutex);
		JUUID uuid = controller->uuid();
		controllersUUIDs.insert_or_assign(uuid, std::move(controller));
		//relate the suuuid to the controllers
		if (!controllerUUIDBySUUUID.contains(sceneObject))
		{
			controllerUUIDBySUUUID.insert_or_assign(sceneObject, std::set<JUUID>());
		}
		controllerUUIDBySUUUID.at(sceneObject).insert(uuid);
		//relate the suuuid
		if (!controllerUUIDNameBySUUUID.contains(sceneObject))
		{
			controllerUUIDNameBySUUUID.insert_or_assign(sceneObject, std::set<JUUIDName>());
		}
		controllerUUIDNameBySUUUID.at(sceneObject).insert(std::make_tuple(uuid, controllerName));
		controllersSUUUID.insert_or_assign(uuid, sceneObject);

		return uuid;
	}

	void MapControllers(SceneUnitId id)
	{
		std::map<unsigned int, std::set<JUUID>> prioritySet = GetControllersPrioritySet(true);

		for (auto& [_, uuidset] : prioritySet)
		{
			for (auto& uuid : uuidset)
			{
				if (std::get<0>(controllersSUUUID.at(uuid)) != id ||
					!controllersUUIDs.contains(uuid) ||
					mappedController.contains(uuid)) continue;
				{
					controllersUUIDs.at(uuid)->Map(controllersSUUUID.at(uuid));
					mappedController.insert(uuid);
				}
			}
		}
	}

	std::set<JUUID> GetControllersInSceneUnit(SceneUnitId id)
	{
		std::set<JUUID> controllers;
		for (auto& [suuuid, uuidset] : controllerUUIDBySUUUID)
		{
			SceneUnitId unit = std::get<0>(suuuid);
			if (unit != id) continue;

			controllers.insert(uuidset.begin(), uuidset.end());
		}
		return controllers;
	}

	std::vector<JUUIDName> GetControllersInstancesInSceneUnit(SceneUnitId id)
	{
		std::vector<JUUIDName> controllers;
		for (auto& [suuuid, uuidset] : controllerUUIDNameBySUUUID)
		{
			SceneUnitId unit = std::get<0>(suuuid);
			if (unit != id) continue;

			for (auto& uuidName : uuidset)
			{
				controllers.push_back(std::make_tuple(std::get<1>(suuuid), std::get<1>(uuidName)));
			}
		}
		SortUUIDByName(controllers);
		return controllers;
	}

	std::unique_ptr<Controller>& GetController(JUUID uuid)
	{
		return controllersUUIDs.at(uuid);
	}

	std::set<JUUID> GetControllersBySceneObjectUUID(SUUUID uuid)
	{
		return(controllerUUIDBySUUUID.contains(uuid)) ? controllerUUIDBySUUUID.at(uuid) : std::set<JUUID>();
	}

	std::set<JUUIDName> GetControllersUUIDNamesBySceneObjectUUID(SUUUID uuid)
	{
		return (controllerUUIDNameBySUUUID.contains(uuid)) ? controllerUUIDNameBySUUUID.at(uuid) : std::set<JUUIDName>();
	}

	void DestroyControllers()
	{
		std::lock_guard<std::mutex> lock(controllerMutex);
		controllersUUIDs.clear();
		controllerUUIDBySUUUID.clear();
		controllerUUIDNameBySUUUID.clear();
		controllersSUUUID.clear();
		mappedController.clear();
	}

	void DestroyController(JUUID uuid)
	{
		std::lock_guard<std::mutex> lock(controllerMutex);
		controllersUUIDs.at(uuid)->Unmap();
		mappedController.erase(uuid);
		controllersUUIDs.erase(uuid);
		for (auto it = controllerUUIDBySUUUID.begin(); it != controllerUUIDBySUUUID.end();)
		{
			if (it->second.contains(uuid))
				it->second.erase(uuid);

			if (it->second.size() == 0ULL)
				it = controllerUUIDBySUUUID.erase(it);
			else
				it++;
		}
		//delete the controller names
		for (auto it = controllerUUIDNameBySUUUID.begin(); it != controllerUUIDNameBySUUUID.end();)
		{
			for (auto sit = it->second.begin(); sit != it->second.end();)
			{
				if (std::get<0>(*sit) == uuid)
					sit = it->second.erase(sit);
				else
					sit++;
			}

			if (it->second.size() == 0ULL)
				it = controllerUUIDNameBySUUUID.erase(it);
			else
				it++;
		}
		controllersSUUUID.erase(uuid);
	}

	void StepControllers(DX::StepTimer& timer)
	{
		std::lock_guard<std::mutex> lock(controllerMutex);
		float dt = static_cast<float>(timer.GetElapsedSeconds());
		std::map<unsigned int, std::set<JUUID>> prioritySet = GetControllersPrioritySet();

		for (auto& [_, uuidset] : prioritySet)
		{
			for (auto& uuid : uuidset)
			{
#if defined(_DEVELOPMENT)
				SceneUnitId unit = std::get<0>(controllersSUUUID.at(uuid));
				auto& scene = GetSceneUnit(unit);
				auto& commandList = scene->GetCommandList();
				std::string StepControllersEvent = "StepControllers:" + std::to_string(unit) + ":" + uuid;
				PIXScopedEvent(commandList.p, 0, nostd::StringToWString(StepControllersEvent).c_str());
#endif

				controllersUUIDs.at(uuid)->Step(dt);
			}
		}
	}

	SUUUID GetControllerSUUUID(JUUID uuid)
	{
		return controllersSUUUID.at(uuid);
	}

	Controller* GetController(SceneUnitId id, ControllerBinding cb)
	{
		using namespace Scene;
		SceneObject* so = GetSceneObjectPointer(MAKESUUUID(id, cb.uuid));
		return GetController(so->at("controllers").at(cb.name)).get();
	}

	JUUID GetControllerUUID(SceneUnitId id, ControllerBinding cb)
	{
		return GetController(id, cb)->uuid();
	}
}
