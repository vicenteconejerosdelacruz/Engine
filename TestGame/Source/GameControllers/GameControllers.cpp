#include "pch.h"
#include "Test/SpinYaw.h"
#include "ThirdPerson/ThirdPersonCharacter.h"
#include "Brawler/Scene/BrawlerScene.h"
#include "Brawler/Camera/BrawlerCamera.h"
#include "Brawler/Characters/Heroes/Venom.h"
#include "Brawler/Characters/Enemies/Thug.h"
#include "Brawler/Characters/Bosses/GreenGoblin.h"
#include "Brawler/Characters/Bosses/PumpkinBomb.h"
#include "Effects/AnimatedDecal.h"
#include "Effects/DelayedDeletion.h"

namespace Game
{
	using namespace Test;
	using namespace Brawler;
	using namespace ThirdPerson;
	using namespace Effects;

	std::unordered_map<std::string, std::function<std::unique_ptr<Game::Controller>(nlohmann::json&)>> controllers =
	{
		{ "spinyaw", [](nlohmann::json& json) { return std::make_unique<SpinYaw>(json); }},
		{ "thirdperson-character", [](nlohmann::json& json) { return std::make_unique<ThirdPersonCharacter>(json); }},
		{ "brawler-scene", [](nlohmann::json& json) { return std::make_unique<BrawlerScene>(json); }},
		{ "brawler-cam", [](nlohmann::json& json) { return std::make_unique<BrawlerCamera>(json); }},
		{ "venom", [](nlohmann::json& json) { return std::make_unique<Venom>(json); }},
		{ "thug", [](nlohmann::json& json) { return std::make_unique<Thug>(json); }},
		{ "greengoblin", [](nlohmann::json& json) { return std::make_unique<GreenGoblin>(json); }},
		{ "pumpkin-bomb", [](nlohmann::json& json) { return std::make_unique<PumpkinBomb>(json); }},
		{ "animated-decal", [](nlohmann::json& json) { return std::make_unique<AnimatedDecal>(json); }},
		{ "delayed-deletion", [](nlohmann::json& json) { return std::make_unique<DelayedDeletion>(json); }},
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

	void CreateControllersMemberFunctionTemplates(Isolate* isolate, SceneUnitId id)
	{
		SceneUnitScripting::GetOrCreateTemplate(isolate, id, SpinYaw::GetClassName(), SpinYaw::RegisterScript);
		SceneUnitScripting::GetOrCreateTemplate(isolate, id, ThirdPersonCharacter::GetClassName(), ThirdPersonCharacter::RegisterScript);
		SceneUnitScripting::GetOrCreateTemplate(isolate, id, BrawlerScene::GetClassName(), BrawlerScene::RegisterScript);
		SceneUnitScripting::GetOrCreateTemplate(isolate, id, BrawlerCamera::GetClassName(), BrawlerCamera::RegisterScript);
		SceneUnitScripting::GetOrCreateTemplate(isolate, id, Venom::GetClassName(), Venom::RegisterScript);
		SceneUnitScripting::GetOrCreateTemplate(isolate, id, Thug::GetClassName(), Thug::RegisterScript);
		SceneUnitScripting::GetOrCreateTemplate(isolate, id, GreenGoblin::GetClassName(), GreenGoblin::RegisterScript);
		SceneUnitScripting::GetOrCreateTemplate(isolate, id, PumpkinBomb::GetClassName(), PumpkinBomb::RegisterScript);
		SceneUnitScripting::GetOrCreateTemplate(isolate, id, AnimatedDecal::GetClassName(), AnimatedDecal::RegisterScript);
		SceneUnitScripting::GetOrCreateTemplate(isolate, id, DelayedDeletion::GetClassName(), DelayedDeletion::RegisterScript);
	}
};
