#include "pch.h"
#include "BrawlerMenu.h"
#include <Game/Game.h>
#include <Level.h>
#if defined(_EDITOR)
#include <Editor.h>
#include <Builder/ReleaseBuilder.h>
#endif
extern std::unique_ptr<DirectX::GamePad> gamePad;
extern DirectX::GamePad::ButtonStateTracker buttons;
extern std::unique_ptr<DirectX::Keyboard> keyboard;
extern GameInteractionMode gameInteractionMode;
extern std::map<JUUID, std::function<void(JUUID)>> onKeyboardMouseInputDetected;
extern std::map<JUUID, std::function<void(JUUID)>> onGamepadInputDetected;

namespace Game::Brawler
{

#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include "BrawlerMenuAtt.h"
#include <JEnd.h>
#endif

	//Constructor and Binding
	BrawlerMenu::BrawlerMenu(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include "BrawlerMenuAtt.h"
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include "BrawlerMenuAtt.h"
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include "BrawlerMenuAtt.h"
#include <JEnd.h>

		SetInitialConditions();
	}

	void BrawlerMenu::RegisterScript(Isolate* isolate, Local<ObjectTemplate> tpl, SceneUnitScripting* script)
	{}

	void BrawlerMenu::SetInitialConditions()
	{
		if (!menuUIInstance().empty())
		{
			DeleteHtmlUIInstance(menuUIInstance());
			menuUIInstance("");
		}
		loadingLevel = false;
		inGame = false;
	}

#if defined(_EDITOR)
	void BrawlerMenu::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include "BrawlerMenuAtt.h"
#include <JEnd.h>
		Controller::WriteJson(j);
		j.at("menuUIInstance") = "";
	}
	void BrawlerMenu::GatherFiles(nlohmann::json& json, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
#include <Editor/JReleaseBuilder.h>
#include "BrawlerMenuAtt.h"
#include <JEnd.h>
		Controller::GatherFiles(json, filesToCopy, templates, logStream);
	}
#endif

	void BrawlerMenu::Map(SUUUID so)
	{
		using namespace Scene;
		Controller::Map(so);

		SetInitialConditions();

		gameStateSet(false);
		onKeyboardMouseInputDetected.insert_or_assign(uuid(), [&](JUUID)
			{
				gamepadStatusSet(false);
			}
		);
		onGamepadInputDetected.insert_or_assign(uuid(), [&](JUUID)
			{
				gamepadStatusSet(false);
			}
		);
	}

	void BrawlerMenu::Unmap()
	{
		DestroyMenuUI();
		Controller::Unmap();
		if (onKeyboardMouseInputDetected.contains(uuid())) onKeyboardMouseInputDetected.erase(uuid());
		if (onGamepadInputDetected.contains(uuid())) onGamepadInputDetected.erase(uuid());
	}

	//Step
	void BrawlerMenu::Step(float delta)
	{
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
		else if (loadingProgressValue() == 100 && !inGame)
		{
			if (gameInteractionMode == GIM_Gamepad)
			{
				auto state = gamePad->GetState(0);
				if (!state.IsConnected())
					return;

				buttons.Update(state);
				if (buttons.a == GamePad::ButtonStateTracker::PRESSED)
				{
					SwitchToGameLevel();
				}
			}
			else
			{
				auto keys = keyboard->GetState();
				if (keys.IsKeyDown(Keyboard::Keys::Enter))
				{
					SwitchToGameLevel();
				}
			}
		}
	}

	//Rendering
	void BrawlerMenu::Render(SceneUnitId id)
	{
#if defined(_EDITOR)
		if (!Editor::IsPlaying(id) || Editor::IsPaused(id))
#else
		if (GetSceneUnit(unit)->IsPaused())
#endif
			return;

		menuUIInstance().empty() ? CreateMenuUI(id) : UpdateMenuUI(id);
	}

	//UI
	std::string BrawlerMenu::BuildEvalScript(std::string type, nlohmann::json data)
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

	void BrawlerMenu::CreateMenuUI(SceneUnitId id)
	{
		menuUIInstance(menuUI() + "-" + getUUID());
		CreateHtmlUIInstance(menuUIInstance(), [&]()
			{
				return std::make_unique<HtmlUIInstance>(id, menuUIInstance(), menuUI());
			}
		);
		HtmlUIInstanceID instance = menuUIInstance();
		instance->MapBridgeCallback("REACT_READY", [&]
			{
				gamepadStatusSet(false);
				gameStateSet(false);
				loadingStateSet(true);
				loadingProgressSet(true);
				loadingProgressValue(0);
			}
		);
	}

	void BrawlerMenu::DestroyMenuUI()
	{
		HtmlUIInstanceID instance = menuUIInstance();
		if (!instance.empty())
			instance->Destroy();
	}

	void BrawlerMenu::UpdateMenuUI(SceneUnitId id)
	{
		UpdateGameState();
		UpdateGamepad();
		UpdateLoading();
		HtmlUIInstanceID instance = menuUIInstance();
		instance->UpdateTexture(id);
		instance->Resolve(id);
	}

	void BrawlerMenu::UpdateGameState()
	{
		if (!gameStateSet())
		{
			std::string js = BuildEvalScript("MAIN_MENU", { });
			HtmlUIInstanceID instance = menuUIInstance();
			instance->EvaluateScript(js);
			gameStateSet(true);
		}
	}

	void BrawlerMenu::UpdateGamepad()
	{
		if (!gamepadStatusSet())
		{
			std::string js = BuildEvalScript("GAMEPAD_STATUS",
				{
					{ "value", gameInteractionMode == GIM_Gamepad }
				}
			);
			HtmlUIInstanceID instance = menuUIInstance();
			instance->EvaluateScript(js);
			gamepadStatusSet(true);
		}
	}

	void BrawlerMenu::UpdateLoading()
	{
		if (!loadingStateSet())
		{
			std::string js = BuildEvalScript("LOADING_SCREEN", { });
			HtmlUIInstanceID instance = menuUIInstance();
			instance->EvaluateScript(js);
			loadingStateSet(true);
		}
		if (!loadingProgressSet())
		{
			std::string js = BuildEvalScript("LOADING_PROGRESS",
				{
					{ "value", loadingProgressValue() }
				}
			);
			HtmlUIInstanceID instance = menuUIInstance();
			instance->EvaluateScript(js);
			loadingProgressSet(true);
		}
	}

	void BrawlerMenu::LoadVenomLevel()
	{
		loadingLevel = true;
		loadingStateSet(false);

#if defined(_EDITOR)
		Editor::LoadGameLevel("venom.yaml",
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
				LevelLoadingProgress(asset, count, total);
			}
		);
#else
		using namespace Scene::Level;

		LoadLevelIntoSceneUnit("venom.yaml", []() { return GetLevelFromFile("venom.yaml"); },
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
				LevelLoadingProgress(asset, count, total);
			}
		);
#endif
	}

	void BrawlerMenu::LevelLoadingProgress(std::string asset, unsigned int count, unsigned int total)
	{
		int progress = static_cast<int>(100.0f * static_cast<float>(count) / static_cast<float>(std::max(count, total)));
		//OutputDebugStringA(std::string("count:" + std::to_string(count) + " total:" + std::to_string(progress) + " progress:" + std::to_string(progress) + "\n").c_str());
		loadingProgressValue(std::min(progress, 99));
		loadingProgressSet(false);
	}

	void BrawlerMenu::SwitchToGameLevel()
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