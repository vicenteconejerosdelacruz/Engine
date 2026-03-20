#include "pch.h"
#include "Controller.h"
#include <map>
#include <NoStd.h>

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

	std::unordered_map<JUUID, std::unique_ptr<Controller>> controllersUUIDs;
	std::unordered_map<SUUUID, std::set<JUUID>> controllerUUIDBySUUUID;
	std::set<JUUID> mappedController;

	Controller::Controller(nlohmann::json& json) :JObject(json)
	{
#include <Attributes/JInit.h>
#include <ControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <ControllerAtt.h>
#include <JEnd.h>

		(*this)["uuid"] = getUUID();
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
	}
#endif

	void Controller::Map(SUUUID so) { unit = std::get<0>(so); sceneObject = so; }

	void Controller::Unmap() { std::get<0>(sceneObject) = 0; std::get<1>(sceneObject).clear(); }

	v8_templates_creators Controller::GetV8TemplatesCreators()
	{
		v8_templates_creators creators;
#include <Attributes/JV8Templates.h>
#include <ControllerAtt.h>
#include <JEnd.h>
		return creators;
	}

	v8_context_creators Controller::GetV8ContextCreators()
	{
		v8_context_creators creators;
#include <Attributes/JV8Context.h>
#include <ControllerAtt.h>
#include <JEnd.h>
		return creators;
	}

	JUUID RegisterController(std::string controllerName, SUUUID sceneObject, std::unique_ptr<Controller>& controller)
	{
		JUUID uuid = getUUID();
		controller->controller = uuid;
		controllersUUIDs.insert_or_assign(uuid, std::move(controller));
		if (!controllerUUIDBySUUUID.contains(sceneObject))
		{
			controllerUUIDBySUUUID.insert_or_assign(sceneObject, std::set<JUUID>());
		}
		controllerUUIDBySUUUID.at(sceneObject).insert(uuid);
		return uuid;
	}

	void MapControllers(SceneUnitId id)
	{
		for (auto& [suuuid, uuidset] : controllerUUIDBySUUUID)
		{
			SceneUnitId unit = std::get<0>(suuuid);
			if (unit != id) continue;

			for (auto& uuid : uuidset)
			{
				if (!controllersUUIDs.contains(uuid) || mappedController.contains(uuid)) continue;
				{
					controllersUUIDs.at(uuid)->Map(suuuid);
					mappedController.insert(uuid);
				}
			}
		}
	}

	std::unique_ptr<Controller>& GetController(JUUID uuid)
	{
		return controllersUUIDs.at(uuid);
	}

	std::set<JUUID> GetControllersBySceneObjectUUID(SUUUID uuid)
	{
		return(controllerUUIDBySUUUID.contains(uuid)) ? controllerUUIDBySUUUID.at(uuid) : std::set<JUUID>();
	}

	void DestroyControllers()
	{
		controllersUUIDs.clear();
		controllerUUIDBySUUUID.clear();
		mappedController.clear();
	}

	void DestroyController(JUUID uuid)
	{
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
	}

	void StepControllers(DX::StepTimer& timer)
	{
		float dt = static_cast<FLOAT>(timer.GetElapsedSeconds());
		std::map<unsigned int, std::set<JUUID>> prioritySet;

		for (auto& [suuuid, uuidset] : controllerUUIDBySUUUID)
		{
#if defined(_EDITOR)
			if (!Editor::IsPlaying(std::get<0>(suuuid))) continue;
#endif
			for (auto& uuid : uuidset)
			{
				prioritySet[controllersUUIDs.at(uuid)->priority()].insert(uuid);
			}
		}

		for (auto& [_, uuidset] : prioritySet)
		{
			for (auto& uuid : uuidset)
			{
				controllersUUIDs.at(uuid)->Step(dt);
			}
		}
	}
}
