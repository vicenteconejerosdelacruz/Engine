#include "pch.h"
#include "Loading.h"

//Loading
void LoadingScreenCreate(GameStates prevState)
{
	//loadingProgressTween = std::make_unique<tween>(tween(0.0f, 1.0f, 4000, tween::easing::linear));
}

void LoadingScreenLeave(GameStates nextState)
{
	/*
	using namespace Scene;

	DeleteRenderable(loadingBar());
	loadingBar.clear();

	loadingProgressTween = nullptr;
	*/
}

void LoadingScreenStep()
{
	//loadingProgress = loadingProgressTween->step();
}

void LoadingScreenRender()
{
	/*
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
	*/
}

//unsigned int nFrames = Renderer::numFrames;
void LoadingScreenPostRender()
{
	/*
	if (loadingProgress == 1.0f)
	{
		nFrames--;
		if (nFrames <= 0)
			gsm.ChangeState(GS_Playing);
	}
	*/
}