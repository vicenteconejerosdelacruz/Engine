#include "pch.h"
#include "EditorPlaying.h"

//EditorPlayingMode
void EditorPlayingModeCreate(GameStates prevState)
{
	/*
	using namespace Editor;
	SwitchToEditorPlayCamera();
	editorPrePlayDump = GetLevelString();
	//kick a step for first frame rendering consistency
	EditorPlayingModeStep();
	Scene::SceneObjectsStep(timer);
	*/
}

void EditorPlayingModeLeave(GameStates nextState)
{

}

void EditorPlayingModeStep()
{
	/*
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
	*/
}

void EditorPlayingModeRender()
{
	/*
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
	*/
}

void EditorPlayingModePostRender()
{

}