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
	std::unordered_map<SUUUID, std::set<JUUID>> controllerUUIDBySUUUID;

	Controller::Controller(nlohmann::json& json) :JObject(json) { (*this)["uuid"] = getUUID(); }

	void Controller::Map(SUUUID so) { unit = std::get<0>(so); sceneObject = so; }

	void Controller::Unmap() { std::get<0>(sceneObject) = 0; std::get<1>(sceneObject).clear(); }

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
				if (!controllersUUIDs.contains(uuid)) continue;
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
	}

	void DestroyControllers()
	{
		controllersUUIDs.clear();
		controllerUUIDBySUUUID.clear();
	}

	void DestroyController(JUUID uuid)
	{
		controllersUUIDs.at(uuid)->Unmap();
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
	}

	void BindToV8Context(v8pp::context& context, SUUUID uuid)
	{
		for (auto& cuuid : GetControllersBySceneObjectUUID(uuid))
		{
			GetController(cuuid)->BindToV8Context(context);
		}
	}
}
