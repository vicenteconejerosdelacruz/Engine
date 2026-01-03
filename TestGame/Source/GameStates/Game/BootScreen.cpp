#include "pch.h"
#include "BootScreen.h"

//Booting
/*
float bootScreenAlpha = 0.0f;
float loadingProgress = 0.0f;
std::unique_ptr<tween> bootAlphaTween;
std::unique_ptr<tween> loadingProgressTween;
RenderableUUID bootScreen;
RenderableUUID loadingBar;
bool reloadingFromDump = false;
*/

void BootScreenCreate(GameStates prevState)
{
	/*
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
	*/
}

void BootScreenLeave(GameStates nextState)
{
	/*
	using namespace Scene;

	DeleteRenderable(bootScreen());
	bootScreen.clear();
	bootAlphaTween = nullptr;
	*/
}

void BootScreenStep()
{
	/*
	bootScreenAlpha = bootAlphaTween->step();

	if (bootScreenAlpha == 1.0f)
	{
		gsm.ChangeState(GS_Loading);
	}
	*/
}

void BootScreenRender()
{
	/*
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
	*/
}