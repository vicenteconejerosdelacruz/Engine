#include "pch.h"
#include "Test/SpinYawController.h"
#include "ThirdPerson/ThirdPersonController.h"
#include "Brawler/Scene/BrawlerSceneController.h"
#include "Brawler/Heroes/VenomController.h"
#include "Brawler/Enemies/ThugController.h"
#include "Brawler/Camera/BrawlerCameraController.h"

namespace Game
{
	std::unordered_map<std::string, std::function<std::unique_ptr<Game::Controller>(nlohmann::json&)>> controllers =
	{
		{ "venom", [](nlohmann::json& json) { return std::make_unique<Game::VenomController>(json); }},
		{ "spinyaw", [](nlohmann::json& json) { return std::make_unique<Game::SpinYawController>(json); }},
		{ "thirdperson", [](nlohmann::json& json) { return std::make_unique<Game::ThirdPersonController>(json); }},
		{ "brawler-scene", [](nlohmann::json& json) { return std::make_unique<Game::BrawlerSceneController>(json); }},
		{ "brawler-cam", [](nlohmann::json& json) { return std::make_unique<Game::BrawlerCameraController>(json); }},
		{ "thug", [](nlohmann::json& json) { return std::make_unique<Game::ThugController>(json); }},
	};

	std::vector<std::string> GetControllers()
	{
		return nostd::GetKeysFromMap(controllers);
	}

	JUUID CreateController(std::string name, SUUUID sceneObject, nlohmann::json& json)
	{
		if (!controllers.contains(name)) return "";
		std::unique_ptr<Game::Controller> controller = controllers.at(name)(json);
		JUUID uuid = RegisterController(name, sceneObject, controller);
		return uuid;
	}
};
