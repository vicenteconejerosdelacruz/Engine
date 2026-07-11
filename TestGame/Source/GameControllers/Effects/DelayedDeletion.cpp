#include "pch.h"
#include "DelayedDeletion.h"
#include <Scene.h>
#if defined(_EDITOR)
#include <Editor.h>
#endif

extern DX::StepTimer timer;

namespace Game::Effects
{
#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include <Effects/DelayedDeletionAtt.h>
#include <JEnd.h>
#endif

	DelayedDeletion::DelayedDeletion(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include <Effects/DelayedDeletionAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <Effects/DelayedDeletionAtt.h>
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include <Effects/DelayedDeletionAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void DelayedDeletion::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <Effects/DelayedDeletionAtt.h>
#include <JEnd.h>
		Controller::WriteJson(j);
	}
#endif

	void DelayedDeletion::Map(SUUUID so)
	{
		Controller::Map(so);

		type = GetSceneObjectType(so);
		if (type == SO_Renderables)
		{
			renderable = so;
		}
		else if (type == SO_Triggers)
		{
			trigger = so;
		}
	}

	void DelayedDeletion::Unmap()
	{
		Controller::Unmap();
	}

	void DelayedDeletion::Step(float delta)
	{
#if defined(_EDITOR)
		if (!Editor::IsPlaying(unit) || Editor::IsPaused(unit))
			return;
#endif

		if (currentTime >= timeToDelete())
			return;

		currentTime += static_cast<float>(timer.GetElapsedSeconds());
		if (currentTime >= timeToDelete())
		{
			switch (type)
			{
			case SO_Renderables:
			{
				renderable->markedForDelete = true;
			}
			break;
			case SO_Triggers:
			{
				trigger->markedForDelete = true;
			}
			break;
			}
		}
	}
}
