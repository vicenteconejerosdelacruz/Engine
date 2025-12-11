#pragma once
#include <memory>
#include <string>
#include <vector>
#include <UUID.h>
#include <JObject.h>
#include <nlohmann/json.hpp>

namespace Game
{
	struct Controller : JObject
	{
		virtual ~Controller() = default;
		Controller(nlohmann::json& json) :JObject(json) { (*this)["uuid"] = getUUID(); }
		JUUID sceneObject;
		virtual void Map(JUUID so) { sceneObject = so; }
		virtual void Unmap() { sceneObject.clear(); }
		virtual void Step(float delta) {};
		virtual void BindToV8Context(v8pp::context& context) {}
		virtual std::map<std::string, JEdvEditorDrawerFunction> GetControllerDrawers() { return {}; }
		virtual std::vector<std::pair<std::string, JsonToEditorValueType>> GetControllerAttributes() { return {}; }
	};

	void RegisterController(std::string controllerName, std::unique_ptr<Controller>& controller, JUUID sceneObject);
	void MapControllers();
	std::unique_ptr<Controller>& GetController(JUUID uuid);
	std::unique_ptr<Controller>& GetControllerBySceneObjectUUID(JUUID uuid);
	std::unique_ptr<Controller>& GetControllerByName(std::string name);
	void DestroyControllers();
	void DestroyController(JUUID uuid);
	void StepControllers(float delta);
	void BindToV8Context(v8pp::context& context, JUUID uuid);

	extern std::vector<std::string> GetControllers();
	extern JUUID CreateController(std::string name, JUUID sceneObject, nlohmann::json& json);
};
