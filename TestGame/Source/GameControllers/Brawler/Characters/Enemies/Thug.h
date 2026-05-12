#pragma once
#include "../BrawlerCharacter.h"
#include <GameStateMachine.h>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <Brawler/Scene/BrawlerScene.h>

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
#include "ThugAtt.h"
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include "ThugAtt.h"
#include <JEnd.h>
#endif

		//struct Venom;
		struct BrawlerScene;

		struct Thug : BrawlerCharacter
		{
#include <Attributes/JFlags.h>
#include "ThugAtt.h"
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include "ThugAtt.h"
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include "ThugAtt.h"
#include <JEnd.h>


			//BrawlerScene* GetBrawlerSceneController();

			//Constructor and Binding
			Thug(nlohmann::json& json);
			static void RegisterScript(Isolate* isolate, Local<ObjectTemplate> tpl, SceneUnitScripting* script);
			void SetInitialConditions() override;
#if defined(_EDITOR)
			void WriteJson(nlohmann::json& j) override;
			DECL_CONTROLLER_DRAWER(Thug, BrawlerCharacter);
#endif
			void Map(SUUUID so) override;
			void Unmap() override;

			//Step
			virtual void Step(float delta);

			//States
			void TakeHit(int damage);
			void PickHeroToFight();

			//Movement
			void CharacterMoveXZPlane(XMVECTOR displacement, float dt, float sideSpeed, XMFLOAT3 gravity);

			//States
			void UpdateLookTo();
			bool IsInAttackRange();

			//Idle
			//bool ShouldIdle();
			void EnterIdle();
			void LeaveIdle();
			void Idle();

			//CombatIdle
			void EnterCombatIdle();
			void CombatIdle();
			void CombatIdleNextState();

			//CombatFollow
			void EnterCombatFollow();
			void CombatFollow();
			XMVECTOR CalculateSteeringDirection();
			void MoveTowardHero(float speed);
			float EvaluateNextFollowMovement(XMVECTOR actualMovementDir);

			//CombatPunch
			void EnterCombatPunch();
			void CombatPunch();
			void OnCombatPunchAnimationEnd();
			void PlayPunchSound();

			//Death
			bool ShouldDie();
			void EnterDeath();
			void OnDeathAnimationEnd();

			//State machine
			GameStatesMachine<ThugStates> tsm;

			//Initial States
			XMFLOAT3 thugScale;
			int initialHealth;

			//SceneObjects
			//RenderableID thug;
			PhysicSceneID physicScene;
			PhysicObjectID physicObject;

			//Picked hero
			EnemyAttackOption pickedHero;
		};
	};
};