#include "pch.h"
#include "Test/SpinYaw.h"
#include "ThirdPerson/ThirdPersonCharacter.h"
#include "Brawler/Scene/BrawlerScene.h"
#include "Brawler/Camera/BrawlerCamera.h"
#include "Brawler/Characters/Heroes/Venom.h"
#include "Brawler/Characters/Enemies/Thug.h"

namespace Game
{
	using namespace Test;
	using namespace Brawler;
	using namespace ThirdPerson;

	std::unordered_map<std::string, std::function<std::unique_ptr<Game::Controller>(nlohmann::json&)>> controllers =
	{
		{ "spinyaw", [](nlohmann::json& json) { return std::make_unique<SpinYaw>(json); }},
		{ "thirdperson-character", [](nlohmann::json& json) { return std::make_unique<ThirdPersonCharacter>(json); }},
		{ "brawler-scene", [](nlohmann::json& json) { return std::make_unique<BrawlerScene>(json); }},
		{ "brawler-cam", [](nlohmann::json& json) { return std::make_unique<BrawlerCamera>(json); }},
		{ "venom", [](nlohmann::json& json) { return std::make_unique<Venom>(json); }},
		{ "thug", [](nlohmann::json& json) { return std::make_unique<Thug>(json); }},
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
