#pragma once
#include <Controller.h>

namespace Game
{
	namespace Brawler
	{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include "BrawlerMenuAtt.h"
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include "BrawlerMenuAtt.h"
#include <JEnd.h>

#endif
		struct BrawlerMenu : Controller
		{
#include <Attributes/JFlags.h>
#include "BrawlerMenuAtt.h"
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include "BrawlerMenuAtt.h"
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include "BrawlerMenuAtt.h"
#include <JEnd.h>

			DEF_STRING2FLAGS_FUNC(BrawlerMenu, Controller);

			//Constructor and Binding
			BrawlerMenu(nlohmann::json& json);
			static void RegisterScript(Isolate* isolate, Local<ObjectTemplate> tpl, SceneUnitScripting* script);
			void RegisterScriptInstance(Isolate* isolate, Local<ObjectTemplate> proto, SceneUnitScripting* script) override { BrawlerMenu::RegisterScript(isolate, proto, script); }
			std::set<std::string> GetControllerAliases() override { return { "menu" }; }
			void SetInitialConditions() override;
#if defined(_EDITOR)
			void WriteJson(nlohmann::json& j) override;
			static void GatherFiles(nlohmann::json& json, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream);
			DECL_CONTROLLER_DRAWER(BrawlerMenu, Controller);
#endif
			void Map(SUUUID so) override;
			void Unmap() override;

			//Step
			void Step(float delta) override;

			//Rendering
			void Render(SceneUnitId id) override;

			//UI
			std::string BuildEvalScript(std::string type, nlohmann::json data);
			void CreateMenuUI(SceneUnitId id);
			void DestroyMenuUI();
			void UpdateMenuUI(SceneUnitId id);
			void UpdateGameState();
			void UpdateGamepad();
			void UpdateLoading();
			void LoadVenomLevel();
			void LevelLoadingProgress(std::string asset, unsigned int count, unsigned int total);
			void SwitchToGameLevel();

			bool loadingLevel;
			bool inGame;
			SceneUnitId gameUnit;
		};
	};
};