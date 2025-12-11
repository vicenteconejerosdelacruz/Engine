#include "Controller.h"
// TestGame.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "TestGame.h"
#include <unordered_map>
#include <memory>
#include <functional>
#include <string>
#include "GameControllers/VenomController.h"
#include "GameControllers/SpinYawController.h"
#include <Renderable/Renderable.h>

//#define _EDITOR

std::string gameAppTitle = "Culpeo Test Game";
extern std::unique_ptr<Renderer> renderer;
CameraUUID levelCameraUUID;
#if defined(_EDITOR)
using namespace Editor;
std::string editorPrePlayDump;
CameraUUID editorCameraUUID = JUUID("editor-cam-uuid");
#endif

namespace Scene
{
	void SceneObjectsStep(DX::StepTimer& timer);
}

#if !defined(_EDITOR) && defined(_DEVELOPMENT)
int main()
{
	return EngineConsoleMain();
}
#else
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	return EngineWinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
#endif

GameStatesMachine<GameStates> gsm =
{
	.currentState = GS_None,
	.onEnter = {
		{ GS_Booting, BootScreenCreate },
		{ GS_Loading, LoadingScreenCreate },
		{ GS_Playing, PlayModeCreate },
#if defined(_EDITOR)
		{ GS_Editor, EditorModeCreate },
		{ GS_EditorPlaying, EditorPlayingModeCreate }
#endif
	},
	.onLeave = {
		{ GS_Booting, BootScreenLeave },
		{ GS_Loading, LoadingScreenLeave },
		{ GS_Playing, PlayModeLeave },
#if defined(_EDITOR)
		{ GS_Editor, EditorModeLeave },
		{ GS_EditorPlaying, EditorPlayingModeLeave }
#endif
	},
	.onStep = {
#if !defined(_EDITOR)
		{ GS_None, []() {gsm.ChangeState(GS_Booting); }},
#else
		{ GS_None, []() {gsm.ChangeState(GS_Editor); }},
#endif
		{ GS_Booting, BootScreenStep },
		{ GS_Loading, LoadingScreenStep },
		{ GS_Playing, PlayModeStep },
#if defined(_EDITOR)
		{ GS_Editor, EditorModeStep },
		{ GS_EditorPlaying, EditorPlayingModeStep }
#endif
	},
	.onRender = {
		{ GS_Booting, BootScreenRender },
		{ GS_Loading, LoadingScreenRender },
		{ GS_Playing, PlayModeRender },
#if defined(_EDITOR)
		{ GS_Editor, EditorModeRender },
		{ GS_EditorPlaying, EditorPlayingModeRender },
#endif
	},
	.onPostRender = {
		{ GS_Loading, LoadingScreenPostRender },
#if defined(_EDITOR)
		{ GS_Editor, EditorModePostRender },
		{ GS_EditorPlaying, EditorPlayingModePostRender },
#endif
	}
};

void GameStep()
{
	gsm.Step();
}

void GameDestroy()
{
	gsm.ChangeState(GS_Destroy);
}

void RunRender()
{
	gsm.Render();
}

void PostRender()
{
	gsm.PostRender();
}

void WindowResizeReleaseResources()
{
	/*
#if defined(_EDITOR)
	Editor::ReleasePickingPassResources();
#endif
	if (mainPass) mainPass->ReleaseResources();
	if (resolvePass) resolvePass->ReleaseResources();
	*/
}

void WindowResize(unsigned int width, unsigned int height)
{
	/*
#if defined(_EDITOR)
	Editor::ResizePickingPass(width, height);
#endif
	if (mainPass) mainPass->Resize(width, height);
	if (resolvePass) resolvePass->Resize(width, height);

	std::shared_ptr<MaterialInstance>& toneMapMaterial = toneMapQuad->meshMaterials.begin()->second;
	toneMapMaterial->textures.insert_or_assign(TextureShaderUsage_Base, GetTextureFromGPUHandle("toneMap", mainPass->renderToTexture[0]->gpuTextureHandle));
	*/
}

void RunPreRenderComputeShaders()
{
	using namespace Scene;
	RunBoundingBoxComputeShaders();
}

void RunPostRenderComputeShaders()
{
	using namespace Scene;
	RunBoundingBoxComputeShadersSolution();
}

void GetAudioListenerVectors(std::function<void(XMFLOAT3, XMVECTOR)> audioListenerCallback)
{
}

//Booting
float bootScreenAlpha = 0.0f;
float loadingProgress = 0.0f;
std::unique_ptr<tween> bootAlphaTween;
std::unique_ptr<tween> loadingProgressTween;
RenderableUUID bootScreen;
RenderableUUID loadingBar;
bool reloadingFromDump = false;

void BootScreenCreate(GameStates prevState)
{
	using namespace Scene;
	renderer->RenderCriticalFrame([]
		{
			using namespace Scene::Level;

			LoadLevel("bootscreen");
			BindSceneObjects();
		}
	);

	bootScreen = GetRenderableUUIDByName("logo");
	loadingBar = GetRenderableUUIDByName("loadingBar");
	bootAlphaTween = std::make_unique<tween>(tween(0.0f, 1.0f, 1000, tween::easing::linear));
}

void BootScreenLeave(GameStates nextState)
{
	using namespace Scene;

	DeleteRenderable(bootScreen());
	bootScreen.clear();
	bootAlphaTween = nullptr;
}

void BootScreenStep()
{
	bootScreenAlpha = bootAlphaTween->step();

	if (bootScreenAlpha == 1.0f)
	{
		gsm.ChangeState(GS_Loading);
	}
}

void BootScreenRender()
{
	using namespace Scene;
	if (GetCountFromSwapChainCameras() > 0ULL)
	{
		bootScreen->WriteConstantsBuffer("alpha", bootScreenAlpha, renderer->backBufferIndex);

		//hide the loading bar
		XMFLOAT2 scale(0.0f, 0.0f);
		loadingBar->WriteConstantsBuffer<XMFLOAT2>("scale", scale, renderer->backBufferIndex);

		WriteConstantsBuffers();
		RenderSceneShadowMaps();
		RenderSceneCameras();
	}
}

//Loading
void LoadingScreenCreate(GameStates prevState)
{
	loadingProgressTween = std::make_unique<tween>(tween(0.0f, 1.0f, 4000, tween::easing::linear));
}

void LoadingScreenLeave(GameStates nextState)
{
	using namespace Scene;

	DeleteRenderable(loadingBar());
	loadingBar.clear();

	loadingProgressTween = nullptr;
}

void LoadingScreenStep()
{
	loadingProgress = loadingProgressTween->step();
}

void LoadingScreenRender()
{
	using namespace Scene;
	if (GetCountFromSwapChainCameras() > 0ULL)
	{
		XMFLOAT2 pos(0.0f, -0.8f);
		XMFLOAT2 scale(0.8f, 0.02f);
		auto red = DirectX::Colors::Red;
		auto blue = DirectX::Colors::Blue;

		loadingBar->WriteConstantsBuffer<XMFLOAT2>("pos", pos, renderer->backBufferIndex);
		loadingBar->WriteConstantsBuffer<XMFLOAT2>("scale", scale, renderer->backBufferIndex);
		loadingBar->WriteConstantsBuffer<XMVECTORF32>("color1", red, renderer->backBufferIndex);
		loadingBar->WriteConstantsBuffer<XMVECTORF32>("color2", blue, renderer->backBufferIndex);
		loadingBar->WriteConstantsBuffer<float>("progress", loadingProgress, renderer->backBufferIndex);
		WriteConstantsBuffers();
		RenderSceneShadowMaps();
		RenderSceneCameras();
	}
}

unsigned int nFrames = Renderer::numFrames;
void LoadingScreenPostRender()
{
	if (loadingProgress == 1.0f)
	{
		nFrames--;
		if (nFrames <= 0)
			gsm.ChangeState(GS_Playing);
	}
}

//Playing
void PlayModeCreate(GameStates prevState)
{
	using namespace Scene;
	using namespace Scene::Level;
	renderer->RenderCriticalFrame([]
		{
			LoadLevel("venom");
			BindSceneObjects();
		}
	);
}

void PlayModeLeave(GameStates nextState)
{
}

void PlayModeStep()
{
	Game::StepControllers(static_cast<float>(timer.GetElapsedSeconds() / 1000.0f));
}

void PlayModeRender()
{
	using namespace Scene;
	if (GetCountFromSwapChainCameras() > 0ULL)
	{
		WriteConstantsBuffers();
		RenderSceneShadowMaps();
		RenderSceneCameras();
	}
}

//Editor
#if defined(_EDITOR)

//EditorMode
void DestroyEditorModeBindings()
{
	DestroyPickingPass();
	DestroyRenderableBoundingBox();
	DestroyBillboards();
	ResetGizmoVariableWorkers();
	DestroyEditorSceneObjectsReferences();
	MarkScenePanelAssetsAsDirty();
	MarkTemplatesPanelAssetsAsDirty();
}

static const float cameraEditorDistance = -10.0f;
void CreateEditorIndependentCamera()
{
	if (GetCountFromMouseCameras() > 0ULL)
	{
		//no more than a single swapchain camera or mouse controller is allowed
		//todo handle RTT cameras that does resolving
		levelCameraUUID = *GetMouseCameras().begin();

		//this should be done and reversed later in the same function.
		//the purpose is to allow to create the editor camera
		//switching will be performed by switching functions later so we undo this in a few lines below
		EraseCameraFromMouseCameras(levelCameraUUID());
		EraseCameraFromSwapChainCameras(levelCameraUUID());

		//make a patch for the uuid and clone the camera
		nlohmann::json parameters = {
			{ "uuid", editorCameraUUID()},
			{ "name", "editorCamera" },
			{ "hidden", true },
			{ "systemCreated", true }
		};
		CloneSceneObject(levelCameraUUID(), parameters);

		//step out a little bit of the scene, we can came up with a better number eventually
		editorCameraUUID->MoveForward(cameraEditorDistance);
		editorCameraUUID->WriteConstantsBuffer(renderer->backBufferIndex);

		//restore cameras mapping
		EraseCameraFromMouseCameras(editorCameraUUID());
		EraseCameraFromSwapChainCameras(editorCameraUUID());
		InsertCameraIntoMouseCameras(levelCameraUUID());
		InsertCameraIntoSwapChainCameras(levelCameraUUID());

		Editor::RegisterBillboard(levelCameraUUID());
	}
	else
	{

	}
}

void SwitchToEditorCamera()
{
	editorCameraUUID->renderables = levelCameraUUID->renderables;
	EraseCameraFromMouseCameras(levelCameraUUID());
	EraseCameraFromSwapChainCameras(levelCameraUUID());
	InsertCameraIntoMouseCameras(editorCameraUUID());
	InsertCameraIntoSwapChainCameras(editorCameraUUID());
}

void SwitchToEditorPlayCamera()
{
	EraseCameraFromMouseCameras(editorCameraUUID());
	EraseCameraFromSwapChainCameras(editorCameraUUID());
	InsertCameraIntoMouseCameras(levelCameraUUID());
	InsertCameraIntoSwapChainCameras(levelCameraUUID());
}

void DestroyEditorCameras()
{
	DeleteCamera(editorCameraUUID());
	editorCameraUUID.clear();
}

void ReloadSceneFromPrePlay()
{
	using namespace Editor;
	using namespace Scene::Level;
	using namespace Game;

	DestroySceneObjects();
	ClearBillboardsRegistry();

	nlohmann::json data = nlohmann::json::parse(editorPrePlayDump);

	LoadSceneObjects(data, SceneObjectTypeJsonContainer.at(SO_Renderables), Scene::CreateRenderable);
	LoadSceneObjects(data, SceneObjectTypeJsonContainer.at(SO_Cameras), Scene::CreateCamera);
	LoadSceneObjects(data, SceneObjectTypeJsonContainer.at(SO_Lights), Scene::CreateLight);
	LoadSceneObjects(data, SceneObjectTypeJsonContainer.at(SO_SoundEffects), Scene::CreateSoundFX);

	MapControllers();
}

void EditorModeCreate(GameStates prevState)
{
	if (prevState == GS_None)
	{
		renderer->RenderCriticalFrame([]
			{
				using namespace Scene;
				using namespace Scene::Level;

				//LoadDefaultLevel();
				//LoadLevel("bootscreen");
				LoadLevel("venom");
				BindSceneObjects();
			}
		);
	}
	else if (prevState == GS_EditorPlaying)
	{
		renderer->RenderCriticalFrame([]
			{
				//DestroyEditorCameras();
				ReloadSceneFromPrePlay();
				BindSceneObjects();
			}
		);
	}

	CreateEditorIndependentCamera();
	SwitchToEditorCamera();
	WriteConstantsBuffers();
}

void EditorModeLeave(GameStates nextState)
{
	editorCameraUUID->renderables.clear();

	renderer->Flush();
	renderer->RenderCriticalFrame([]
		{
			DestroyEditorModeBindings();
		}
	);
}

void EditorModeStep()
{
	using namespace Scene::Level;

	//if there is a level pending to load
	if (PendingLevelToLoad())
	{
		//then we can load the scene in a new critical frame
		renderer->RenderCriticalFrame([]
			{
				DestroyEditorModeBindings();
				LoadPendingLevel();
				BindSceneObjects();
			}
		);
		CreateEditorIndependentCamera();
		SwitchToEditorCamera();
	}

	BuildAssetsTree();

	if (RenderableBoundingBoxExists())
	{
		UpdateBoundingBox();
	}

	if (GetCountFromMouseCameras() > 0ULL)
	{
		GameAreaMouseProcessing(mouse, *GetMouseCameras().begin());
	}

	UpdateBillboards();

	StepAnimationSequencer();

	if (Editor::IsPlaying())
	{
		Editor::DestroyBillboard(levelCameraUUID());
		gsm.ChangeState(GS_EditorPlaying);
	}
}

void EditorModeRender()
{
	using namespace Scene;
	using namespace Templates;
	if (GetCountFromSwapChainCameras() > 0ULL)
	{
		XMFLOAT3 position = levelCameraUUID->position();
		XMFLOAT3 rotation = levelCameraUUID->rotation();
		levelCameraUUID->position(editorCameraUUID->position());
		levelCameraUUID->rotation(editorCameraUUID->rotation());

		SwitchToEditorPlayCamera();

		WriteConstantsBuffers();
		RenderPickingPass(*GetSwapChainCameras().begin());
		RenderSceneShadowMaps();

		RenderShadowMapMinMaxChain();

		RenderSceneCameras();
		editorCameraUUID->position(levelCameraUUID->position());
		editorCameraUUID->rotation(levelCameraUUID->rotation());

		SwitchToEditorCamera();
		levelCameraUUID->position(position);
		levelCameraUUID->rotation(rotation);

		DrawEditor(*GetSwapChainCameras().begin());
	}
	else
	{
#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, L"Fallback Draw");
#endif
		auto& swapChainPass = renderer->swapChainPass;
		swapChainPass->Pass();
		DrawEditor();
#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif
	}
}

void EditorModePostRender()
{
	bool criticalFrame = (
		!PickingPassExists() ||
		(!RenderableBoundingBoxExists() && GetCountFromSwapChainCameras() > 0ULL) ||
		PendingBillboards() ||
		PendingBillboardsDestruction() ||
		PendingAnimationSequencer() ||
		PendingAnimationSequencerDestruction());

	if (criticalFrame)
	{
		renderer->RenderCriticalFrame([]
			{
				if (!PickingPassExists())
				{
					CreatePickingPass();
					if (PickingPassExists())
					{
						BindPickingRenderables();
					}
				}

				if (GetCountFromMouseCameras() > 0ULL && !RenderableBoundingBoxExists())
				{
					CreateRenderableBoundingBox(levelCameraUUID);
				}

				if (PendingBillboards())
					CreateRegisteredBillboards(levelCameraUUID);

				if (PendingBillboardsDestruction())
					DestroyPendingBillboards();

				if (PendingAnimationSequencer())
					LoadAnimationSequencer();

				if (PendingAnimationSequencerDestruction())
					DestroyAnimationSequencer();
			}
		);
	}

	PickFromScene();
}

//EditorPlayingMode
void EditorPlayingModeCreate(GameStates prevState)
{
	using namespace Editor;
	SwitchToEditorPlayCamera();
	editorPrePlayDump = GetLevelString();
	//kick a step for first frame rendering consistency
	EditorPlayingModeStep();
	Scene::SceneObjectsStep(timer);
}

void EditorPlayingModeLeave(GameStates nextState)
{

}

void EditorPlayingModeStep()
{
	using namespace Scene::Level;

	if (PendingLevelToLoad())
	{
		renderer->Flush();
		renderer->RenderCriticalFrame([]
			{
				DestroyEditorModeBindings();
				LoadPendingLevel();
				BindSceneObjects();
			}
		);
	}

	if (!Editor::IsPlaying())
	{
		gsm.ChangeState(GS_Editor);
		return;
	}

	Game::StepControllers(static_cast<float>(timer.GetElapsedSeconds() / 1000.0f));
}

void EditorPlayingModeRender()
{
	using namespace Scene;
	if (GetCountFromSwapChainCameras() > 0ULL)
	{
#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, "EditorPlayingModeRender");
#endif
		WriteConstantsBuffers();


#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, "RenderSceneShadowMaps");
#endif
		RenderSceneShadowMaps();
#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif

#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, "RenderSceneCameras");
#endif
		RenderSceneCameras();
#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif

#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, "DrawEditor");
#endif
		DrawEditor(*GetSwapChainCameras().begin());
#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif

#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif
	}
	else
	{
#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, L"Fallback Draw");
#endif
		renderer->swapChainPass->Pass();
		DrawEditor();
#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif
	}
}

void EditorPlayingModePostRender()
{

}

#endif

namespace Game
{
#if defined(_EDITOR)
	std::unordered_map<std::string, std::function<std::map<std::string, JEdvEditorDrawerFunction>()>> controllerDrawers =
	{
		{ "venom", [] { return Game::GetVenomControllerDrawers(); }},
		{ "spinyaw", [] { return Game::GetSpinYawControllerDrawers(); }}
	};
#endif

	std::unordered_map<std::string, std::function<std::unique_ptr<Game::Controller>(nlohmann::json&)>> controllers =
	{
		{ "venom", [](nlohmann::json& json) { return std::make_unique<Game::VenomController>(json); }},
		{ "spinyaw", [](nlohmann::json& json) { return std::make_unique<Game::SpinYawController>(json); }},
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