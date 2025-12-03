#include "pch.h"
#include "Controller.h"
#include <map>
#include <NoStd.h>

namespace Game
{
	std::unordered_map<JUUID, std::unique_ptr<Controller>> controllersUUIDs;
	std::unordered_map<std::string, JUUID> controllerUUIDsByName;
	std::unordered_map<JUUID, JUUID> sceneObjectUUIDToControllerUUID;

	JUUID RegisterController(std::unique_ptr<Controller>& controller, std::string controllerName, JUUID sceneObject)
	{
		JUUID controllerUUID = getUUID();
		controller->Map(sceneObject);
		controllersUUIDs.insert_or_assign(controllerUUID, std::move(controller));
		controllerUUIDsByName.insert_or_assign(controllerName, controllerUUID);
		sceneObjectUUIDToControllerUUID.insert_or_assign(sceneObject, controllerUUID);
		return controllerUUID;
	}

	void UnregisterController(JUUID controllerUUID)
	{
		if (controllersUUIDs.contains(controllerUUID))
		{
			auto& controller = controllersUUIDs.at(controllerUUID);
			controller->Unmap();
			controllersUUIDs.erase(controllerUUID);
			for (auto it = controllerUUIDsByName.begin(); it != controllerUUIDsByName.end();)
			{
				if (it->second == controllerUUID)
				{
					it = controllerUUIDsByName.erase(it);
					break;
				}
				else
				{
					it++;
				}
			}
			for (auto it = sceneObjectUUIDToControllerUUID.begin(); it != sceneObjectUUIDToControllerUUID.end();)
			{
				if (it->second == controllerUUID)
				{
					it = sceneObjectUUIDToControllerUUID.erase(it);
				}
				else
				{
					it++;
				}
			}
		}
	}

	void DestroyControllers()
	{
		for (auto it = controllersUUIDs.begin(); it != controllersUUIDs.end();)
		{
			it->second->Unmap();
			it = controllersUUIDs.erase(it);
		}
		controllerUUIDsByName.clear();
	}

	void StepControllers(float delta)
	{
		for (auto& [_, c] : controllersUUIDs)
		{
			c->Step(delta);
		}
	}

	std::unique_ptr<Game::Controller>& GetControllerByName(std::string name)
	{
		return controllersUUIDs.at(controllerUUIDsByName.at(name));
	}

	std::string GetControllerNameByUUID(JUUID uuid)
	{
		return std::find_if(controllerUUIDsByName.begin(), controllerUUIDsByName.end(), [&](const auto& pair)
			{
				return pair.second == uuid;
			}
		)->first;
	}


	void BindToV8Context(v8pp::context& context, JUUID uuid)
	{
		using namespace Scripting;
		JUUID controllerUUID = sceneObjectUUIDToControllerUUID.at(uuid);
		std::string name = GetControllerNameByUUID(controllerUUID);
		std::unique_ptr<Controller>& controller = controllersUUIDs.at(controllerUUID);
		controller->BindToV8Context(context);
	}
}
