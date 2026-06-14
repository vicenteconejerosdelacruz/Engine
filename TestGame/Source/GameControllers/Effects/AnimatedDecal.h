#pragma once
#include <Controller.h>
#include "../../JExpose/Editor/JEdvBrawlerDrawer.h"

namespace Game
{
	namespace Effects
	{
#if defined(_EDITOR)
#include <Attributes/JOrder.h>
#include <Effects/AnimatedDecalAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <Effects/AnimatedDecalAtt.h>
#include <JEnd.h>
#endif

		struct AnimatedDecal : Controller
		{
#include <Attributes/JFlags.h>
#include <Effects/AnimatedDecalAtt.h>
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include <Effects/AnimatedDecalAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <Effects/AnimatedDecalAtt.h>
#include <JEnd.h>

			AnimatedDecal(nlohmann::json& json);
#if defined(_EDITOR)
			void WriteJson(nlohmann::json& j) override;
			std::map<std::string, JEdvEditorDrawerFunction> GetControllerDrawers() override
			{
				std::map<std::string, JEdvEditorDrawerFunction> drawers = Controller::GetControllerDrawers();
				drawers.merge(GetAnimatedDecalDrawers());
				drawers.insert_or_assign("player", DrawAnimatedDecal);
				return drawers;
			}
			std::vector<std::pair<std::string, JsonToEditorValueType>> GetControllerAttributes() override
			{
				std::vector<std::pair<std::string, JsonToEditorValueType>> attributes = Controller::GetControllerAttributes();
				auto atts = GetAnimatedDecalAttributes();
				attributes.insert(attributes.end(), atts.begin(), atts.end());
				attributes.push_back(std::make_pair("player", jedv_t_hidden));
				return attributes;
			};
			void DrawPlayer();
#endif
			void Map(SUUUID so) override;
			void Unmap() override;
			void Step(float delta) override;

			RenderableID renderable;
			bool playing = false;
			bool looping = false;
			unsigned int currentFrame = 0;
			float currentTime = 0.0f;
		};
	}
}