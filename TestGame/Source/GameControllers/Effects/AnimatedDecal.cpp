#include "pch.h"
#include "AnimatedDecal.h"
#include <Scene.h>
#if defined(_EDITOR)
#include <Editor.h>
#endif

extern DX::StepTimer timer;

namespace Game::Effects
{
#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include <Effects/AnimatedDecalAtt.h>
#include <JEnd.h>
#endif

	AnimatedDecal::AnimatedDecal(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include <Effects/AnimatedDecalAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <Effects/AnimatedDecalAtt.h>
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include <Effects/AnimatedDecalAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void AnimatedDecal::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <Effects/AnimatedDecalAtt.h>
#include <JEnd.h>
		Controller::WriteJson(j);
	}

	void AnimatedDecal::DrawPlayer()
	{
		if (!renderable) return;

		bool update = false;

		ImGui::PushID(std::string(renderable->uuid() + "-animated-decal-player").c_str());
		ImGui::DrawAnimationController(
			[&] { return playing; },
			[&](auto play) { playing = play; },
			[&](float time)
			{
				currentTime = time;
				update = true;
			},
			[&] { return 1.0f; },
			[&](float timeFactor) {},
			[&] {
				currentTime = 0.0f;
				looping = false;
				currentFrame = 0;
				update = true;
			},
			[&] {
				currentTime = 1.0f;
				looping = false;
				currentFrame = totalFrames() - 1;
				update = true;
			},
			[&] { return looping; },
			[&](bool looping) {
				this->looping = looping;
				update = true;
			}
		);
		ImGui::PopID();

		if (playing || update)
		{
			currentTime += static_cast<float>(timer.GetElapsedSeconds());
			currentFrame = static_cast<unsigned int>(totalFrames() * timeBetweenFrames() * currentTime);
			for (unsigned int i = 0; i < JRenderer::numFrames; i++)
			{
				renderable->WriteConstantsBuffer("frameIndex", &currentFrame, i);
			}
			if (looping && currentFrame >= totalFrames())
			{
				currentFrame = 0;
				currentTime = 0.0f;
			}
		}
	}
#endif

	void AnimatedDecal::Map(SUUUID so)
	{
		Controller::Map(so);

		SceneObjectType type = GetSceneObjectType(so);
		if (type == SO_Renderables)
		{
			renderable = so;
		}
	}

	void AnimatedDecal::Unmap()
	{
		Controller::Unmap();
	}

	void AnimatedDecal::Step(float delta)
	{
#if defined(_EDITOR)
		if (!Editor::IsPlaying(unit) || Editor::IsPaused(unit))
			return;
#endif

		if (animationEnded)
			return;

		currentTime += static_cast<float>(timer.GetElapsedSeconds());
		currentFrame = static_cast<unsigned int>(totalFrames() * timeBetweenFrames() * currentTime);
		for (unsigned int i = 0; i < JRenderer::numFrames; i++)
		{
			renderable->WriteConstantsBuffer("frameIndex", &currentFrame, i);
		}
		if (currentFrame >= totalFrames())
		{
			animationEnded = true;
			if (deleteAtFinish())
			{
				renderable->markedForDelete = true;
			}
		}
	}
}
