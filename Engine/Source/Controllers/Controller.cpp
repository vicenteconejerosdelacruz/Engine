#include "pch.h"
#include "Controller.h"
#include <map>
#include <NoStd.h>

namespace Editor
{
	extern bool IsPlaying(SceneUnitId id);
};

namespace Game
{
	std::unordered_map<JUUID, std::unique_ptr<Controller>> controllersUUIDs;
	//std::unordered_map<std::string, JUUID> controllerUUIDsByName;
	std::unordered_map<SUUUID, std::set<JUUID>> controllerUUIDBySUUUID;

	/*
	std::unordered_map<SUUUID, std::unique_ptr<Controller>> controllersUUIDs;
	std::unordered_map<std::string, JUUID> controllerUUIDsByName;
	std::unordered_map<SUUUID, JUUID> sceneObjectUUIDToControllerUUID;
	*/

	Controller::Controller(nlohmann::json& json) :JObject(json) { (*this)["uuid"] = getUUID(); }

	void Controller::Map(SUUUID so) { unit = std::get<0>(so); sceneObject = so; }

	void Controller::Unmap() { std::get<0>(sceneObject) = 0; std::get<1>(sceneObject).clear(); }

	JUUID RegisterController(std::string controllerName, SUUUID sceneObject, std::unique_ptr<Controller>& controller)
	{
		JUUID uuid = getUUID();
		controller->controller = uuid;
		controllersUUIDs.insert_or_assign(uuid, std::move(controller));
		//controllerUUIDsByName.insert_or_assign(controllerName, uuid);
		//controllerUUIDBySUUUID.insert_or_assign(sceneObject, uuid);
		if (!controllerUUIDBySUUUID.contains(sceneObject))
		{
			controllerUUIDBySUUUID.insert_or_assign(sceneObject, std::set<JUUID>());
		}
		controllerUUIDBySUUUID.at(sceneObject).insert(uuid);
		return uuid;
	}

	/*
	void RegisterController(std::string controllerName, std::unique_ptr<Controller>& controller, SUUUID sceneObject)
	{
		controllerUUIDsByName.insert_or_assign(controllerName, controller->at("uuid"));
		sceneObjectUUIDToControllerUUID.insert_or_assign(sceneObject, controller->at("uuid"));
		controllersUUIDs.insert_or_assign(controller->at("uuid"), std::move(controller));
	}
	*/
	void MapControllers(SceneUnitId id)
	{
		for (auto& [suuuid, uuidset] : controllerUUIDBySUUUID)
		{
			SceneUnitId unit = std::get<0>(suuuid);
			if (unit != id) continue;

			for (auto& uuid : uuidset)
			{
				controllersUUIDs.at(uuid)->Map(suuuid);
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
		//return controllersUUIDs.at(controllerUUIDBySUUUID.at(sceneObject));
	}

	/*std::unique_ptr<Controller>& GetControllerByName(std::string name)
	{
		return controllersUUIDs.at(controllerUUIDsByName.at(name));
	}*/
	/*
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
	*/
	void DestroyController(JUUID uuid)
	{
		if (!controllersUUIDs.contains(uuid)) return;
		controllersUUIDs.at(uuid)->Unmap();
		controllersUUIDs.erase(uuid);
		//for (auto it = controllerUUIDsByName.begin(); it != controllerUUIDsByName.end();)
		//{
		//	if (it->second == uuid)
		//		it = controllerUUIDsByName.erase(it);
		//	else
		//		it++;
		//}
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
		for (auto& [suuuid, uuidset] : controllerUUIDBySUUUID)
		{
#if defined(_EDITOR)
			if (!Editor::IsPlaying(std::get<0>(suuuid))) continue;
			for (auto& uuid : uuidset)
			{
				controllersUUIDs.at(uuid)->Step(dt);
			}
#endif
		}
		/*
		for (auto& [_, c] : controllersUUIDs)
		{
#if defined(_EDITOR)
			if(Editor::IsPlaying())
#endif
			c->Step(dt);
		}
		*/
	}

	void BindToV8Context(v8pp::context& context, SUUUID uuid)
	{
		for (auto& cuuid : GetControllersBySceneObjectUUID(uuid))
		{
			GetController(cuuid)->BindToV8Context(context);
		}
		//using namespace Scripting;
		//JUUID controllerUUID = controllerUUIDBySUUUID.at(uuid);
		//std::unique_ptr<Controller>& controller = controllersUUIDs.at(controllerUUID);
		//controller->BindToV8Context(context);
	}
}
