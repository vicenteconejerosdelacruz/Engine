#include "pch.h"
#include "EditorBooting.h"
#include <Scene.h>
#include <SceneObject.h>
#include <Level.h>
#include <Renderer.h>
#include <Editor.h>

extern std::unique_ptr<Renderer> renderer;
extern GameStatesMachine<GameStates> gsm;

SceneUnitId bootScreenId;
extern SceneUnitId editorModeId;
CameraUUID bootCamera;
RenderableUUID bootScreen;
RenderableUUID loadingBar;
std::unique_ptr<tween> bootAlphaTween;
//std::unique_ptr<tween> loadingProgressTween;
//bool loadBar = false;
bool bootEnded = false;

void SetLoadingProgress(std::vector<unsigned int> frames, float progress)
{
	using namespace Scene;

	XMFLOAT2 pos(0.0f, -0.8f);
	XMFLOAT2 scale(0.8f, 0.02f);
	auto red = DirectX::Colors::Red;
	auto blue = DirectX::Colors::Blue;

	for (unsigned int frame : frames)
	{
		loadingBar->WriteConstantsBuffer<XMFLOAT2>("pos", pos, frame);
		loadingBar->WriteConstantsBuffer<XMFLOAT2>("scale", scale, frame);
		loadingBar->WriteConstantsBuffer<XMVECTORF32>("color1", red, frame);
		loadingBar->WriteConstantsBuffer<XMVECTORF32>("color2", blue, frame);
		loadingBar->WriteConstantsBuffer<float>("progress", progress, frame);
	}
}

void EditorBootingCreate(GameStates prevState)
{
	using namespace Scene;
	using namespace Scene::Level;
	using namespace Editor;

	CreateSceneLevelAsync("bootscreen", GetLevelFromFile("bootscreen"), [&](SceneUnitId unit)
		{
			bootScreenId = unit;
			bootScreen = GetRenderableUUIDByName("logo");
			loadingBar = GetRenderableUUIDByName("loadingBar");
			bootCamera = GetCameraUUIDByName("cam.0");
			bootAlphaTween = std::make_unique<tween>(tween(0.0f, 1.0f, 1000, tween::easing::linear));
			SetLoadingProgress({ 0,1,2 }, 0.0f);
			SetEditorCamera(bootCamera);
			SetCurrentSceneUnit(unit);
			ShowEditorController(true);
			ShowEditorPanel(true);
			MarkScenePanelAssetsAsDirty();

		}
	);
}

void EditorBootingStep()
{
	using namespace Scene;
	using namespace Scene::Level;

	if (bootScreen() == "") return;

	auto& scene = GetSceneUnit(bootScreenId);
	float screenAlpha = bootAlphaTween->step();
	bootScreen->WriteConstantsBuffer("alpha", screenAlpha, scene->Frame());

	return;

	if (screenAlpha != 1.0f || bootEnded)
		return;

	bootEnded = true;
	//CreateSceneLevelAsync("defaultLevel", GetDefaultLevel(), [&](SceneUnitId unit)
	CreateSceneLevelAsync("venom", GetLevelFromFile("venom"), [&](SceneUnitId unit)
		{
			editorModeId = unit;
			gsm.ChangeState(GS_EditorMode);
		},
		[&](std::string asset, unsigned int count, unsigned int total)
		{
			auto& scene = GetSceneUnit(bootScreenId);
			float progress = static_cast<float>(count) / static_cast<float>(total);
			SetLoadingProgress({ 0,1,2 }, progress);
			OutputDebugStringA(std::string(asset + " count:" + std::to_string(count) + ", total:" + std::to_string(total) + "\n").c_str());
		}
	);

	/*
	if (screenAlpha == 1.0f && !loadBar)
	{
		loadBar = true;
		AttachLevelIntoScene(bootScreenId, "loadingbar", [&](SceneUnitId lbarId)
			{
				loadingBar = GetRenderableUUIDByName("loadingBar");
				loadingProgressTween = std::make_unique<tween>(tween(0.0f, 1.0f, 4000, tween::easing::linear));
			}
		);
		CreateSceneLevelAsync("venom", [&](SceneUnitId unit)
			{
			}
		);
	}
	if (loadingProgressTween != nullptr)
	{
		XMFLOAT2 pos(0.0f, -0.8f);
		XMFLOAT2 scale(0.8f, 0.02f);
		auto red = DirectX::Colors::Red;
		auto blue = DirectX::Colors::Blue;

		float loadingProgress = loadingProgressTween->step();

		loadingBar->WriteConstantsBuffer<XMFLOAT2>("pos", pos, scene.Frame());
		loadingBar->WriteConstantsBuffer<XMFLOAT2>("scale", scale, scene.Frame());
		loadingBar->WriteConstantsBuffer<XMVECTORF32>("color1", red, scene.Frame());
		loadingBar->WriteConstantsBuffer<XMVECTORF32>("color2", blue, scene.Frame());
		loadingBar->WriteConstantsBuffer<float>("progress", loadingProgress, scene.Frame());
	}
	*/
}

void EditorBootingRender()
{
	/*
	using namespace Editor;

	if (bootCamera() == "") return;

	DrawEditor(bootCamera);
	*/
}

void EditorBootingPostRender()
{
}

void EditorBootingLeave(GameStates nextState)
{
	DestroyScene(bootScreenId);
	bootScreenId = 0;
}
