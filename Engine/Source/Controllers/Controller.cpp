#include "pch.h"
#include "Controller.h"
#include <map>
#include <NoStd.h>

namespace Game
{
	std::unordered_map<JUUID, std::unique_ptr<Controller>> controllersUUIDs;
	std::unordered_map<std::string, JUUID> controllerUUIDsByName;
	std::unordered_map<JUUID, JUUID> sceneObjectUUIDToControllerUUID;

	void RegisterController(std::string controllerName, std::unique_ptr<Controller>& controller, JUUID sceneObject)
	{
		controllerUUIDsByName.insert_or_assign(controllerName, controller->at("uuid"));
		sceneObjectUUIDToControllerUUID.insert_or_assign(sceneObject, controller->at("uuid"));
		controllersUUIDs.insert_or_assign(controller->at("uuid"), std::move(controller));
	}

	void MapControllers()
	{
		for (auto& [so, uuid] : sceneObjectUUIDToControllerUUID)
		{
			controllersUUIDs.at(uuid)->Map(so);
		}
	}

	std::unique_ptr<Controller>& GetController(JUUID uuid)
	{
		return controllersUUIDs.at(uuid);
	}

	std::unique_ptr<Controller>& GetControllerBySceneObjectUUID(JUUID sceneObject)
	{
		return controllersUUIDs.at(sceneObjectUUIDToControllerUUID.at(sceneObject));
	}

	std::unique_ptr<Controller>& GetControllerByName(std::string name)
	{
		return controllersUUIDs.at(controllerUUIDsByName.at(name));
	}

	void DestroyControllers()
	{
		for (auto it = controllersUUIDs.begin(); it != controllersUUIDs.end();)
		{
			it->second->Unmap();
			it = controllersUUIDs.erase(it);
		}
		controllerUUIDsByName.clear();
		sceneObjectUUIDToControllerUUID.clear();
	}

	void DestroyController(JUUID uuid)
	{
		controllersUUIDs.at(uuid)->Unmap();
		controllersUUIDs.erase(uuid);
		for (auto it = controllerUUIDsByName.begin(); it != controllerUUIDsByName.end();)
		{
			if (it->second == uuid)
				it = controllerUUIDsByName.erase(it);
			else
				it++;
		}
		for (auto it = sceneObjectUUIDToControllerUUID.begin(); it != sceneObjectUUIDToControllerUUID.end();)
		{
			if (it->second == uuid)
				it = sceneObjectUUIDToControllerUUID.erase(it);
			else
				it++;
		}
	}

	void StepControllers(float delta)
	{
		for (auto& [_, c] : controllersUUIDs)
		{
			c->Step(delta);
		}
	}

	void BindToV8Context(v8pp::context& context, JUUID uuid)
	{
		using namespace Scripting;
		JUUID controllerUUID = sceneObjectUUIDToControllerUUID.at(uuid);
		std::unique_ptr<Controller>& controller = controllersUUIDs.at(controllerUUID);
		controller->BindToV8Context(context);
	}
}
