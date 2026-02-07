#include "pch.h"
#include "PlayMode.h"

//Playing
void PlayModeCreate(GameStates prevState)
{
	/*
	using namespace Scene;
	using namespace Scene::Level;
	renderer->RenderCriticalFrame([]
		{
			LoadLevel("venom");
			BindSceneObjects();
		}
	);
	*/
}

void PlayModeLeave(GameStates nextState)
{
}

void PlayModeStep()
{
	//Game::StepControllers(static_cast<float>(timer.GetElapsedSeconds() / 1000.0f));
}

void PlayModeRender()
{
	/*
	using namespace Scene;
	if (GetCountFromSwapChainCameras() > 0ULL)
	{
		WriteConstantsBuffers();
		RenderSceneShadowMaps();
		RenderSceneCameras();
	}
	*/
}
