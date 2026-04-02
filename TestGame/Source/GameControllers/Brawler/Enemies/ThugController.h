#pragma once
#include <Controller.h>
#include <GameStateMachine.h>
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>

enum VenomStates;

enum CharacterLookingTo
{
	CLT_Right,
	CLT_Left,
};

inline static std::unordered_map<CharacterLookingTo, std::string> CharacterLookingToToString =
{
	{ CLT_Right, "Right" },
	{ CLT_Left, "Left" },
};

inline static std::unordered_map<std::string, CharacterLookingTo> StringToCharacterLookingTo =
{
	{ "Right", CLT_Right },
	{ "Left", CLT_Left },
};

enum ThugStates
{
	TS_None,
	TS_Idle,
	TS_CombatIdle,
	TS_CombatFollow,
	//TS_CombatFollowBrawler,
	TS_CombatPunch,
};

inline static std::unordered_map<std::string, ThugStates> StringToTugStates =
{
	{ "None", TS_None },
	{ "Idle", TS_Idle },
	{ "CombatIdle", TS_CombatIdle },
	{ "CombatFollow", TS_CombatFollow },
	//{ "CombatFollowBrawler", TS_CombatFollowBrawler },
	{ "CombatPunch", TS_CombatPunch },
};

namespace Game
{
	struct BrawlerSceneController;

#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <Brawler/ThugControllerAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <Brawler/ThugControllerAtt.h>
#include <JEnd.h>

#endif

	struct VenomController;

	struct ThugController : Controller
	{
#include <Attributes/JFlags.h>
#include <Brawler/ThugControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <Brawler/ThugControllerAtt.h>
#include <JEnd.h>

		BrawlerSceneController* GetBrawlerSceneController();

		//Constructor and Binding
		ThugController(nlohmann::json& json);
		virtual void SetInitialConditions();
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
		DECL_CONTROLLER_DRAWER(ThugController, Controller);
#endif
		virtual void Map(SUUUID so);
		virtual void Unmap();

		//Step
		virtual void Step(float delta);

		//JS binding
		virtual v8_templates_creators GetV8TemplatesCreators();
		virtual v8_context_creators GetV8ContextCreators();
		virtual v8_functions_creators GetV8FunctionsCreators();

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

		//State machine
		GameStatesMachine<ThugStates> tsm;

		//Initial States
		XMFLOAT3 thugScale;
		CharacterLookingTo thugInitialLookTo;

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