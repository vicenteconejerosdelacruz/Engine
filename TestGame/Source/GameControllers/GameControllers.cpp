#include "pch.h"
//#include "VenomController.h"
//#include "SpinYawController.h"

namespace Game
{
#if defined(_EDITOR)
	std::unordered_map<std::string, std::function<std::map<std::string, JEdvEditorDrawerFunction>()>> controllerDrawers =
	{
		/*
		{ "venom", [] { return Game::GetVenomControllerDrawers(); }},
		{ "spinyaw", [] { return Game::GetSpinYawControllerDrawers(); }}
		*/
	};
#endif

	std::unordered_map<std::string, std::function<std::unique_ptr<Game::Controller>(nlohmann::json&)>> controllers =
	{
		/*
		{ "venom", [](nlohmann::json& json) { return std::make_unique<Game::VenomController>(json); }},
		{ "spinyaw", [](nlohmann::json& json) { return std::make_unique<Game::SpinYawController>(json); }},
		*/
	};

	std::vector<std::string> GetControllers()
	{
		return nostd::GetKeysFromMap(controllers);
	}

	JUUID CreateController(std::string name, JUUID sceneObject, nlohmann::json& json)
	{
		if (!controllers.contains(name)) return "";
		std::unique_ptr<Game::Controller> controller = controllers.at(name)(json);
		JUUID uuid = controller->at("uuid");
		RegisterController(name, controller, sceneObject);
		return uuid;
	}
};
