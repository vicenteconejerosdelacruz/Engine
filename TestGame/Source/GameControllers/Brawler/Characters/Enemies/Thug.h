#pragma once
#include "../BrawlerCharacter.h"
#include <GameStateMachine.h>
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>

namespace Game
{
	namespace Brawler
	{
		enum ThugStates
		{
			TS_None,
			TS_Idle,
			TS_CombatIdle,
			TS_CombatFollow,
			//TS_CombatFollowBrawler,
			TS_CombatPunch,
			TS_Death,
		};

		inline static std::unordered_map<std::string, ThugStates> StringToTugStates =
		{
			{ "None", TS_None },
			{ "Idle", TS_Idle },
			{ "CombatIdle", TS_CombatIdle },
			{ "CombatFollow", TS_CombatFollow },
			//{ "CombatFollowBrawler", TS_CombatFollowBrawler },
			{ "CombatPunch", TS_CombatPunch },
			{ "Death", TS_Death }
		};

#if defined(_EDITOR)
#include <Attributes/JOrder.h>
#include <Brawler/ThugAtt.h>
#include <JEnd.h>
#include <Editor/JDrawersDecl.h>
#include <Brawler/ThugAtt.h>
#include <JEnd.h>
#endif

		//struct Venom;
		struct BrawlerScene;

		struct Thug : BrawlerCharacter
		{
#include <Attributes/JFlags.h>
#include <Brawler/ThugAtt.h>
#include <JEnd.h>
#include <Attributes/JDecl.h>
#include <Brawler/ThugAtt.h>
#include <JEnd.h>

			BrawlerScene* GetBrawlerSceneController();

			//Constructor and Binding
			Thug(nlohmann::json& json);
			void SetInitialConditions() override;
#if defined(_EDITOR)
			void WriteJson(nlohmann::json& j) override;
			DECL_CONTROLLER_DRAWER(Thug, BrawlerCharacter);
#endif
			void Map(SUUUID so) override;
			void Unmap() override;

			//Step
			virtual void Step(float delta);

			//JS binding
			v8_templates_creators GetV8TemplatesCreators() override;
			v8_context_creators GetV8ContextCreators() override;
			v8_functions_creators GetV8FunctionsCreators() override;

			//States
			void TakeHit(int damage);
			//Movement
			void CharacterMoveXZPlane(XMVECTOR displacement, float dt, float sideSpeed, XMFLOAT3 gravity);

			//States
			void UpdateLookTo();
			bool IsInAttackRange();

			//Idle
			bool ShouldIdle();
			void EnterIdle();
			void LeaveIdle();
			void Idle();

			//CombatIdle
			bool ShouldCombatIdle();
			void EnterCombatIdle();
			void CombatIdle();
			void CombatIdleNextState();

			//CombatFollow
			bool ShouldCombatFollow();
			void EnterCombatFollow();
			void CombatFollow();
			void MoveTowardHero(float speed);
			void EvaluateNextFollowMovement();

			//CombatPunch
			void EnterCombatPunch();
			void CombatPunch();
			void OnCombatPunchAnimationEnd();

			//Death
			bool ShouldDie();
			void EnterDeath();
			void OnDeathAnimationEnd();

			//State machine
			GameStatesMachine<ThugStates> tsm;

			//Initial States
			XMFLOAT3 thugScale;
			CharacterLookingTo thugInitialLookTo;
			int initialHealth;

			//SceneObjects
			RenderableID thug;
			PhysicSceneID physicScene;
			PhysicObjectID physicObject;

			//Picked hero
			JUUID heroController;
			RenderableID heroRenderable;
			XMFLOAT3 heroAttackOffset;
		};
	};
};