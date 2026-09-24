#include "pch.h"
#include "BootScreen.h"
#include <Game/Game.h>
#include <Level.h>
#if defined(_EDITOR)
#include <Editor.h>
#include <Builder/ReleaseBuilder.h>
#endif

namespace Game
{

#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include "BootScreenAtt.h"
#include <JEnd.h>
#endif

	//Constructor and Binding
	BootScreen::BootScreen(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include "BootScreenAtt.h"
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include "BootScreenAtt.h"
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include "BootScreenAtt.h"
#include <JEnd.h>

		SetInitialConditions();
	}

	void BootScreen::RegisterScript(Isolate* isolate, Local<ObjectTemplate> tpl, SceneUnitScripting* script)
	{}

	void BootScreen::SetInitialConditions()
	{
		if (!screenUIInstance().empty())
		{
			DeleteHtmlUIInstance(screenUIInstance());
			screenUIInstance("");
		}
		loadingLevel = false;
		inGame = false;
		loadingProgressValue(0);
	}

#if defined(_EDITOR)
	void BootScreen::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include "BootScreenAtt.h"
#include <JEnd.h>
		Controller::WriteJson(j);
		j.at("screenUIInstance") = "";
	}
	void BootScreen::GatherFiles(nlohmann::json& json, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
#include <Editor/JReleaseBuilder.h>
#include "BootScreenAtt.h"
#include <JEnd.h>
		Controller::GatherFiles(json, filesToCopy, templates, logStream);
	}
#endif

	void BootScreen::Map(SUUUID so)
	{
		using namespace Scene;
		Controller::Map(so);

		SetInitialConditions();

		gameStateSet(false);
	}

	void BootScreen::Unmap()
	{
		DestroyBootScreenUI();
		Controller::Unmap();
	}

	//Step
	void BootScreen::Step(float delta)
	{
		/*
		if (!loadingLevel)
		{
			if (gameInteractionMode == GIM_Gamepad)
			{
				auto state = gamePad->GetState(0);
				if (!state.IsConnected())
					return;

				buttons.Update(state);
				if (buttons.a == GamePad::ButtonStateTracker::PRESSED)
				{
					LoadVenomLevel();
				}
			}
			else
			{
				auto keys = keyboard->GetState();
				if (keys.IsKeyDown(Keyboard::Keys::Enter))
				{
					LoadVenomLevel();
				}
			}
		}
		else */if (loadingProgressValue() == 100 && !inGame)
		{
			SwitchToMainMenu();
			/*
			if (gameInteractionMode == GIM_Gamepad)
			{
				auto state = gamePad->GetState(0);
				if (!state.IsConnected())
					return;

				buttons.Update(state);
				if (buttons.a == GamePad::ButtonStateTracker::PRESSED)
				{
					SwitchToMainMenu();
				}
			}
			else
			{
				auto keys = keyboard->GetState();
				if (keys.IsKeyDown(Keyboard::Keys::Enter))
				{
					SwitchToMainMenu();
				}
			}
			*/
		}
	}

	//Rendering
	void BootScreen::Render(SceneUnitId id)
	{
#if defined(_EDITOR)
		if (!Editor::IsPlaying(id) || Editor::IsPaused(id))
#else
		if (GetSceneUnit(unit)->IsPaused())
#endif
			return;

		screenUIInstance().empty() ? CreateBootScreen(id) : UpdateBootScreenUI(id);
	}

	//UI
	std::string BootScreen::BuildEvalScript(std::string type, nlohmann::json data)
	{
		nlohmann::json event =
		{
			{ "detail", {{ "type", type }}}
		};
		for (auto it = data.begin(); it != data.end(); ++it) {
			event["detail"][it.key()] = it.value();
		}
		std::string eventStr = event.dump();
		std::string js = "window.dispatchEvent(new CustomEvent('engineUpdate', " + eventStr + "));";
		return js;
	}

	void BootScreen::CreateBootScreen(SceneUnitId id)
	{
		screenUIInstance(screenUI() + "-" + getUUID());
		CreateHtmlUIInstance(screenUIInstance(), [&]()
			{
				return std::make_unique<HtmlUIInstance>(id, screenUIInstance(), screenUI());
			}
		);
		HtmlUIInstanceID instance = screenUIInstance();
		instance->MapBridgeCallback("REACT_READY", [&]
			{
				gameStateSet(false);
			}
		);
		instance->MapBridgeCallback("BOOT_COMPLETE", [&]
			{

				//loadingStateSet(true);
				//loadingProgressSet(true);
				loadingProgressValue(0);
				LoadMainMenu();
			}
		);
	}

	void BootScreen::DestroyBootScreenUI()
	{
		HtmlUIInstanceID instance = screenUIInstance();
		if (!instance.empty())
			instance->Destroy();
	}

	void BootScreen::UpdateBootScreenUI(SceneUnitId id)
	{
		UpdateGameState();
		HtmlUIInstanceID instance = screenUIInstance();
		instance->UpdateTexture(id);
		instance->Resolve(id);
	}

	void BootScreen::UpdateGameState()
	{
		if (!gameStateSet())
		{
			std::string js = BuildEvalScript("BOOT_SCREEN", { });
			HtmlUIInstanceID instance = screenUIInstance();
			instance->EvaluateScript(js);
			gameStateSet(true);
		}
	}

	void BootScreen::LoadMainMenu()
	{
		loadingLevel = true;
		loadingStateSet(false);

#if defined(_EDITOR)
		Editor::LoadGameLevel("mainmenu.yaml",
			[&](SceneUnitId id)
			{
				gameUnit = id;
				loadingProgressValue(100);
				loadingProgressSet(false);
				EnableSceneUnitRendering(unit);
				RemoveSceneUnitRendering(gameUnit);
			},
			[&](std::string asset, unsigned int count, unsigned int total)
			{
			}
		);
#else
		using namespace Scene::Level;

		LoadLevelIntoSceneUnit("mainmenu.yaml", []() { return GetLevelFromFile("mainmenu.yaml"); },
			[&](SceneUnitId id)
			{
				gameUnit = id;
				loadingProgressValue(100);
				loadingProgressSet(false);
				EnableSceneUnitRendering(unit);
				RemoveSceneUnitRendering(gameUnit);
				GetSceneUnit(id)->SetPaused(true);
			},
			[&](std::string asset, unsigned int count, unsigned int total)
			{
			}
		);
#endif
	}

	void BootScreen::SwitchToMainMenu()
	{
		inGame = true;
		EnableSceneUnitRendering(gameUnit);
		RemoveSceneUnitRendering(unit);
#if defined(_EDITOR)
		Editor::SwitchToPlayMode(gameUnit);
		Editor::SwitchToUnPausedMode(gameUnit);
#else
		GetSceneUnit(gameUnit)->SetPaused(false);
#endif
		auto& scene = GetSceneUnit(unit);
		scene->MarkForDelete();
	}
}