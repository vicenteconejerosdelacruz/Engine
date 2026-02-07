#include "pch.h"
#include "EditorMode.h"
#include <Editor.h>

SceneUnitId editorModeId;
extern SceneUnitId bootScreenId;
CameraUUID editorModeCamera;

void EditorModeCreate(GameStates prevState)
{
	using namespace Editor;

	editorModeCamera = (*GetSwapChainCameras(editorModeId).begin());
	//editorModeCamera = GetCameraUUIDByName("default.cam.0");
	SetCurrentSceneUnit(editorModeId);
	SetEditorCamera(editorModeCamera);
	ShowEditorController(true);
	ShowEditorPanel(true);
	MarkScenePanelAssetsAsDirty();
	/*
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
	*/
}

void EditorModeLeave(GameStates nextState)
{
	/*
	editorCameraUUID->renderables.clear();

	renderer->Flush();
	renderer->RenderCriticalFrame([]
		{
			DestroyEditorModeBindings();
		}
	);
	*/
}

void EditorModeStep()
{
	/*
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
	*/
}

void EditorModeRender()
{
	/*
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
	*/
}

void EditorModePostRender()
{
	/*
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
	*/
}