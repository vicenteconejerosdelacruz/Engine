#pragma once
#include "../Enemies/Thug.h"
#include <GameStateMachine.h>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <Brawler/Scene/BrawlerScene.h>

namespace Game
{
	namespace Brawler
	{
		enum GreenGoblinCombatStates
		{
			GGCS_Glider,
			GGCS_Floor,
		};

		inline static std::unordered_map<std::string, GreenGoblinCombatStates> StringToGreenGoblinCombatStates =
		{
			{ "Glider", GGCS_Glider },
			{ "Floor", GGCS_Floor },
		};

		enum GreenGoblinGliderState
		{
			GGGS_Idle,
			GGGS_GlideLeft,
			GGGS_GlideRight
		};

#if defined(_EDITOR)
#include <Attributes/JOrder.h>
#include "GreenGoblinAtt.h"
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include "GreenGoblinAtt.h"
#include <JEnd.h>
#endif

		struct GreenGoblin : Thug
		{
#include <Attributes/JFlags.h>
#include "GreenGoblinAtt.h"
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include "GreenGoblinAtt.h"
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include "GreenGoblinAtt.h"
#include <JEnd.h>

			DEF_STRING2FLAGS_FUNC(GreenGoblin, Thug);

			//Constructor and Binding
			GreenGoblin(nlohmann::json& json);
			static void RegisterScript(Isolate* isolate, Local<ObjectTemplate> tpl, SceneUnitScripting* script);
			void RegisterScriptInstance(Isolate* isolate, Local<ObjectTemplate> proto, SceneUnitScripting* script) override { GreenGoblin::RegisterScript(isolate, proto, script); }
			std::set<std::string> GetControllerAliases() override {
				std::set<std::string> aliases = Thug::GetControllerAliases();
				aliases.insert("thug");
				return aliases;
			}
			void SetInitialConditions() override;
#if defined(_EDITOR)
			void WriteJson(nlohmann::json& j) override;
			DECL_CONTROLLER_DRAWER(GreenGoblin, Thug);
#endif
			void Map(SUUUID so) override;
			void Unmap() override;

			//Step
			virtual void Step(float delta);
			void TakeHit(int damage) override;

			//GGIdle
			void GGIdle();
			void GotoToFloor();

			//GlideLeft
			void GGEnterGlideLeft();
			void GGGlideLeftStep();

			//GlideRight
			void GGEnterGlideRight();
			void GGGlideRightStep();

			bool GetBombTarget(XMFLOAT3& target);
			void SetThrowBombTarget(XMFLOAT3 target);
			XMVECTOR GetBombInitialVelocity();
			void ThrowBombAtTarget();
			void GetBackGlideIdleAnimation();

			//State machine
			GameStatesMachine<GreenGoblinGliderState> gggsm;

			GreenGoblinCombatStates combatState = GreenGoblinCombatStates::GGCS_Glider;
			std::unique_ptr<tween> leftGlideTween;
			std::unique_ptr<tween> rightGlideTween;
			bool firstGlide = true;
			bool bombThrown = false;
			XMFLOAT3 bombInitialPos{};
			XMFLOAT3 bombTarget{};
		};
	};
};