#include "pch.h"
#include "Test/SpinYaw.h"
#include "ThirdPerson/ThirdPersonCharacter.h"
#include "Brawler/Scene/BrawlerScene.h"
#include "Brawler/Camera/BrawlerCamera.h"
#include "Brawler/Characters/Heroes/Venom.h"
#include "Brawler/Characters/Enemies/Thug.h"
#include "Brawler/Characters/Bosses/GreenGoblin.h"
#include "Brawler/Characters/Bosses/PumpkinBomb.h"
#include "Brawler/Menus/BrawlerMenu.h"
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
		{ "brawler-menu", [](nlohmann::json& json) { return std::make_unique<BrawlerMenu>(json); }},
		{ "venom", [](nlohmann::json& json) { return std::make_unique<Venom>(json); }},
		{ "thug", [](nlohmann::json& json) { return std::make_unique<Thug>(json); }},
		{ "greengoblin", [](nlohmann::json& json) { return std::make_unique<GreenGoblin>(json); }},
		{ "pumpkin-bomb", [](nlohmann::json& json) { return std::make_unique<PumpkinBomb>(json); }},
		{ "animated-decal", [](nlohmann::json& json) { return std::make_unique<AnimatedDecal>(json); }},
		{ "delayed-deletion", [](nlohmann::json& json) { return std::make_unique<DelayedDeletion>(json); }},
	};

#if defined(_EDITOR)
	std::unordered_map<std::string, std::function<void(nlohmann::json& controller, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)>> controllerReleaseBuilders =
	{
		{ "spinyaw", [](nlohmann::json& controller, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
		{
			SpinYaw::GatherFiles(controller, filesToCopy, templates, logStream);
		}
		},
		{ "thirdperson-character", [](nlohmann::json& controller, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
		{
			ThirdPersonCharacter::GatherFiles(controller, filesToCopy, templates, logStream);
		}
		},
		{ "brawler-scene", [](nlohmann::json& controller, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
		{
			BrawlerScene::GatherFiles(controller, filesToCopy, templates, logStream);
		}
		},
		{ "brawler-cam", [](nlohmann::json& controller, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
		{
			BrawlerCamera::GatherFiles(controller, filesToCopy, templates, logStream);
		}
		},
		{ "brawler-menu", [](nlohmann::json& controller, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
		{
			BrawlerMenu::GatherFiles(controller, filesToCopy, templates, logStream);
		}
		},
		{ "venom", [](nlohmann::json& controller, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
		{
			Venom::GatherFiles(controller, filesToCopy, templates, logStream);
		}
		},
		{ "thug", [](nlohmann::json& controller, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
		{
			Thug::GatherFiles(controller, filesToCopy, templates, logStream);
		}
		},
		{ "greengoblin", [](nlohmann::json& controller, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
		{
			GreenGoblin::GatherFiles(controller, filesToCopy, templates, logStream);
		}
		},
		{ "pumpkin-bomb", [](nlohmann::json& controller, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
		{
			PumpkinBomb::GatherFiles(controller, filesToCopy, templates, logStream);
		}
		},
		{ "animated-decal", [](nlohmann::json& controller, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
		{
			AnimatedDecal::GatherFiles(controller, filesToCopy, templates, logStream);
		}
		},
		{ "delayed-deletion", [](nlohmann::json& controller, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
		{
			DelayedDeletion::GatherFiles(controller, filesToCopy, templates, logStream);
		}
		},

	};
#endif

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
		SceneUnitScripting::GetOrCreateTemplate(isolate, id, BrawlerMenu::GetClassName(), BrawlerMenu::RegisterScript);
		SceneUnitScripting::GetOrCreateTemplate(isolate, id, Venom::GetClassName(), Venom::RegisterScript);
		SceneUnitScripting::GetOrCreateTemplate(isolate, id, Thug::GetClassName(), Thug::RegisterScript);
		SceneUnitScripting::GetOrCreateTemplate(isolate, id, GreenGoblin::GetClassName(), GreenGoblin::RegisterScript);
		SceneUnitScripting::GetOrCreateTemplate(isolate, id, PumpkinBomb::GetClassName(), PumpkinBomb::RegisterScript);
		SceneUnitScripting::GetOrCreateTemplate(isolate, id, AnimatedDecal::GetClassName(), AnimatedDecal::RegisterScript);
		SceneUnitScripting::GetOrCreateTemplate(isolate, id, DelayedDeletion::GetClassName(), DelayedDeletion::RegisterScript);
	}
};
