// TestGame.cpp : Defines the entry point for the application.
#include "pch.h"
#include "TestGame.h"
#include <unordered_map>
#include <memory>
#include <functional>
#include <string>

std::string gameAppTitle = "Culpeo Test Game";
float gameUpdateFrequency = (1.0f / 60.0f);

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

/*
GameStatesMachine<GameStates> gsm =
{
	.currentState = GS_None,
#if defined(_EDITOR)
	.onEnter = {
		{ GS_EditorBooting, EditorBootingCreate },
		{ GS_EditorMode, EditorModeCreate },
		{ GS_EditorPlaying, EditorPlayingModeCreate }
	},
	.onLeave = {
		{ GS_EditorBooting, EditorBootingLeave },
		{ GS_EditorMode, EditorModeLeave },
		{ GS_EditorPlaying, EditorPlayingModeLeave }
	},
	.onStep = {
		{ GS_None, []() {gsm.ChangeState(GS_EditorBooting); }},
		{ GS_EditorBooting, EditorBootingStep },
		{ GS_EditorMode, EditorModeStep },
		{ GS_EditorPlaying, EditorPlayingModeStep }
	},
	.onRender = {
		{ GS_EditorBooting, EditorBootingRender },
		{ GS_EditorMode, EditorModeRender },
		{ GS_EditorPlaying, EditorPlayingModeRender },
	},
	.onPostRender = {
		{ GS_EditorBooting, EditorBootingPostRender },
		{ GS_EditorMode, EditorModePostRender },
		{ GS_EditorPlaying, EditorPlayingModePostRender },
	}
#else
	.onEnter = {
		{ GS_Booting, BootScreenCreate },
		{ GS_Loading, LoadingScreenCreate },
		{ GS_Playing, PlayModeCreate },
	},
	.onLeave = {
		{ GS_Booting, BootScreenLeave },
		{ GS_Loading, LoadingScreenLeave },
		{ GS_Playing, PlayModeLeave },
	},
	.onStep = {
		{ GS_None, []() {gsm.ChangeState(GS_Booting); }},
		{ GS_Booting, BootScreenStep },
		{ GS_Loading, LoadingScreenStep },
		{ GS_Playing, PlayModeStep },
	},
	.onRender = {
		{ GS_Booting, BootScreenRender },
		{ GS_Loading, LoadingScreenRender },
		{ GS_Playing, PlayModeRender },
	},
	.onPostRender = {
		{ GS_Loading, LoadingScreenPostRender },
	}
#endif
};
*/

/*
void GameStep()
{
	gsm.Step();
}
*/

/*
void GameDestroy()
{
#if defined(_EDITOR)
	gsm.ChangeState(GS_EditorDestroy);
#else
	gsm.ChangeState(GS_Destroy);
#endif
}
*/

/*
void GameRender()
{
	gsm.Render();
}
*/

/*
void GamePostRender()
{
	gsm.PostRender();
}
*/

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

void GetAudioListenerVectors(std::function<void(XMFLOAT3, XMVECTOR)> audioListenerCallback)
{
}

//Editor
#if defined(_EDITOR)

//EditorMode
void DestroyEditorModeBindings()
{
	/*
	DestroyPickingPass();
	DestroyRenderableBoundingBox();
	DestroyBillboards();
	ResetGizmoVariableWorkers();
	DestroyEditorSceneObjectsReferences();
	MarkScenePanelAssetsAsDirty();
	MarkTemplatesPanelAssetsAsDirty();
	*/
}

static const float cameraEditorDistance = -10.0f;
void CreateEditorIndependentCamera()
{
	/*
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
	*/
}

void SwitchToEditorCamera()
{
	/*
	editorCameraUUID->renderables = levelCameraUUID->renderables;
	EraseCameraFromMouseCameras(levelCameraUUID());
	EraseCameraFromSwapChainCameras(levelCameraUUID());
	InsertCameraIntoMouseCameras(editorCameraUUID());
	InsertCameraIntoSwapChainCameras(editorCameraUUID());
	*/
}

void SwitchToEditorPlayCamera()
{
	/*
	EraseCameraFromMouseCameras(editorCameraUUID());
	EraseCameraFromSwapChainCameras(editorCameraUUID());
	InsertCameraIntoMouseCameras(levelCameraUUID());
	InsertCameraIntoSwapChainCameras(levelCameraUUID());
	*/
}

void DestroyEditorCameras()
{
	/*
	DeleteCamera(editorCameraUUID());
	editorCameraUUID.clear();
	*/
}

void ReloadSceneFromPrePlay()
{
	/*
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
	*/
}

#endif
