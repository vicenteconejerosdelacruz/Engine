#pragma once
#include <memory>
#include <string>
#include <vector>
#include <UUID.h>

namespace Game
{
	struct Controller
	{
		JUUID sceneObject;
		virtual void Step(float delta) {};
		virtual void Map(JUUID so) { sceneObject = so; }
		virtual void Unmap() { sceneObject.clear(); }
		virtual void BindToV8Context(v8pp::context& context) {}
	};

	JUUID RegisterController(std::unique_ptr<Controller>& controller, std::string controllerName, JUUID sceneObject);
	void UnregisterController(JUUID controllerUUID);
	void DestroyControllers();
	void StepControllers(float delta);
	std::unique_ptr<Game::Controller>& GetControllerByName(std::string name);
	std::string GetControllerNameByUUID(JUUID uuid);
	void BindToV8Context(v8pp::context& context, JUUID uuid);

	extern std::vector<std::string> GetGameControllers();
	extern std::unique_ptr<Game::Controller> GetGameController(std::string name);
};
