#pragma once
#include <Controller.h>

namespace Game
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include "BootScreenAtt.h"
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include "BootScreenAtt.h"
#include <JEnd.h>

#endif
	struct BootScreen : Controller
	{
#include <Attributes/JFlags.h>
#include "BootScreenAtt.h"
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include "BootScreenAtt.h"
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include "BootScreenAtt.h"
#include <JEnd.h>

		DEF_STRING2FLAGS_FUNC(BootScreen, Controller);

		//Constructor and Binding
		BootScreen(nlohmann::json& json);
		static void RegisterScript(Isolate* isolate, Local<ObjectTemplate> tpl, SceneUnitScripting* script);
		void RegisterScriptInstance(Isolate* isolate, Local<ObjectTemplate> proto, SceneUnitScripting* script) override { BootScreen::RegisterScript(isolate, proto, script); }
		std::set<std::string> GetControllerAliases() override { return { "bootscreen" }; }
		void SetInitialConditions() override;
#if defined(_EDITOR)
		void WriteJson(nlohmann::json& j) override;
		static void GatherFiles(nlohmann::json& json, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream);
		DECL_CONTROLLER_DRAWER(BootScreen, Controller);
#endif
		void Map(SUUUID so) override;
		void Unmap() override;

		//Step
		void Step(float delta) override;

		//Rendering
		void Render(SceneUnitId id) override;

		//UI
		std::string BuildEvalScript(std::string type, nlohmann::json data);
		void CreateBootScreen(SceneUnitId id);
		void DestroyBootScreenUI();
		void UpdateBootScreenUI(SceneUnitId id);
		void UpdateGameState();
		void LoadMainMenu();
		void SwitchToMainMenu();

		bool loadingLevel;
		bool inGame;
		SceneUnitId gameUnit;
	};
};