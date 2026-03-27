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

enum TugStates
{
	TS_None,
	TS_Idle,
	TS_CombatIdle,
	TS_CombatFollow,
	TS_CombatFollowBrawler,
	TS_CombatPunch,
};

inline static std::unordered_map<std::string, TugStates> StringToTugStates =
{
	{ "None", TS_None },
	{ "Idle", TS_Idle },
	{ "CombatIdle", TS_CombatIdle },
	{ "CombatFollowBrawler", TS_CombatFollowBrawler },
	{ "CombatPunch", TS_CombatPunch },
};

namespace Game
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <TugControllerAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <TugControllerAtt.h>
#include <JEnd.h>

#endif

	struct VenomController;

	struct TugController : Controller
	{
#include <Attributes/JFlags.h>
#include <TugControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <TugControllerAtt.h>
#include <JEnd.h>

		VenomStates venomState();
		VenomController* venomController();

		//Constructor and Binding
		TugController(nlohmann::json& json);
		virtual void SetInitialConditions();
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
		DECL_CONTROLLER_DRAWER(TugController, Controller);
#endif
		virtual void Map(SUUUID so);
		virtual void Unmap();

		//Step
		virtual void Step(float delta);

		//JS binding
		virtual v8_templates_creators GetV8TemplatesCreators();
		virtual v8_context_creators GetV8ContextCreators();
		virtual v8_functions_creators GetV8FunctionsCreators();

		//States
		void UpdateLookTo();

		//Idle
		bool ShouldIdle();
		void EnterIdle();
		void LeaveIdle();
		void Idle();

		//CombatIdle
		bool ShouldCombatIdle();
		void EnterCombatIdle();
		void CombatIdle();

		//State machine
		GameStatesMachine<TugStates> tsm;

		//Initial States
		XMFLOAT3 tugScale;
		CharacterLookingTo tugInitialLookTo;

		//SceneObjects
		RenderableID tug;
		RenderableID venomR;
	};
};