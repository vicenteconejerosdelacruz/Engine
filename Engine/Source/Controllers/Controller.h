#pragma once
#include <memory>
#include <string>
#include <vector>
#include <UUID.h>
#include <JObject.h>
#include <JTypes.h>
#include <nlohmann/json.hpp>
#include <StepTimer.h>

namespace Game
{
	struct Controller : JObject
	{
		virtual ~Controller() = default;
		Controller(nlohmann::json& json) :JObject(json) { (*this)["uuid"] = getUUID(); }
		SUUUID sceneObject;
		virtual void Map(SUUUID so) { sceneObject = so; }
		virtual void Unmap() {
			std::get<0>(sceneObject) = 0;
			std::get<1>(sceneObject).clear();
		}
		virtual void Step(float delta) {};
		virtual void BindToV8Context(v8pp::context& context) {}
		virtual std::map<std::string, JEdvEditorDrawerFunction> GetControllerDrawers() { return {}; }
		virtual std::vector<std::pair<std::string, JsonToEditorValueType>> GetControllerAttributes() { return {}; }
	};

	JUUID RegisterController(std::string controllerName, SUUUID sceneObject, std::unique_ptr<Controller>& controller);
	void MapControllers(SceneUnitId id);
	std::unique_ptr<Controller>& GetController(JUUID uuid);
	std::unique_ptr<Controller>& GetControllerBySceneObjectUUID(SUUUID uuid);
	std::unique_ptr<Controller>& GetControllerByName(std::string name);
	//void DestroyControllers();
	void DestroyController(JUUID uuid);
	void StepControllers(DX::StepTimer& timer);
	void BindToV8Context(v8pp::context& context, SUUUID uuid);

	extern std::vector<std::string> GetControllers();
	extern JUUID CreateController(std::string name, SUUUID sceneObject, nlohmann::json& json);
};
