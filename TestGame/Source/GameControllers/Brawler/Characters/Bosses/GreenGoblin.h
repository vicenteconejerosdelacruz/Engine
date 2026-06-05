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
		enum GreenGoblinStates
		{
			GGS_None,
			GGS_Idle,
		};

		inline static std::unordered_map<std::string, GreenGoblinStates> StringToGreenGoblinStates =
		{
			{ "None", GGS_None },
			{ "Idle", GGS_Idle },
		};

#if defined(_EDITOR)
#include <Attributes/JOrder.h>
#include "GreenGoblinAtt.h"
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include "GreenGoblinAtt.h"
#include <JEnd.h>
#endif

		//struct Venom;
		struct BrawlerScene;

		struct GreenGoblin : BrawlerCharacter
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

			//Constructor and Binding
			GreenGoblin(nlohmann::json& json);
			static void RegisterScript(Isolate* isolate, Local<ObjectTemplate> tpl, SceneUnitScripting* script);
			void SetInitialConditions() override;
#if defined(_EDITOR)
			void WriteJson(nlohmann::json& j) override;
			DECL_CONTROLLER_DRAWER(GreenGoblin, BrawlerCharacter);
#endif
			void Map(SUUUID so) override;
			void Unmap() override;

			//Step
			virtual void Step(float delta);

			/*
			//States
			void TakeHit(int damage);
			void PickHeroToFight();
			void UnregisterFromCombat();

			//Movement
			void CharacterMoveXZPlane(XMVECTOR displacement, float dt, float sideSpeed, XMFLOAT3 gravity);

			//States
			void UpdateLookTo();
			bool IsInAttackRange();

			//Idle
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
			*/
			//State machine
			GameStatesMachine<GreenGoblinStates> ggsm;

			//Initial States
			XMFLOAT3 greenGoblinScale;
			int initialHealth;
			//SceneObjects
			PhysicSceneID physicScene;
			PhysicObjectID physicObject;

			//Picked hero
			/*
			JUUID pickedHeroID;
			*/
		};
	};
};