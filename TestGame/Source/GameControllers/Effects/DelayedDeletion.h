#pragma once
#include <Controller.h>
#include "../../JExpose/Editor/JEdvBrawlerDrawer.h"

namespace Game
{
	namespace Effects
	{
#if defined(_EDITOR)
#include <Attributes/JOrder.h>
#include <Effects/DelayedDeletionAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <Effects/DelayedDeletionAtt.h>
#include <JEnd.h>
#endif

		struct DelayedDeletion : Controller
		{
#include <Attributes/JFlags.h>
#include <Effects/DelayedDeletionAtt.h>
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include <Effects/DelayedDeletionAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <Effects/DelayedDeletionAtt.h>
#include <JEnd.h>

			DEF_STRING2FLAGS_FUNC(DelayedDeletion, Controller);

			DelayedDeletion(nlohmann::json& json);
			~DelayedDeletion() {}
#if defined(_EDITOR)
			void WriteJson(nlohmann::json& j) override;
#endif
			void Map(SUUUID so) override;
			void Unmap() override;
			void Step(float delta) override;

			float currentTime = 0.0f;
			SceneObjectType type = SceneObjectType::SO_None;
			RenderableID renderable;
			TriggerID trigger;
		};
	}
}