#pragma once
#include <Controller.h>
#include <GameStateMachine.h>
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>

enum TugStates
{
	TS_None,
	TS_Idle
};

inline static std::unordered_map<std::string, TugStates> StringToTugStates =
{
	{ "None", TS_None},
	{ "Idle", TS_Idle},
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

	struct TugController : Controller
	{
#include <Attributes/JFlags.h>
#include <TugControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <TugControllerAtt.h>
#include <JEnd.h>

		enum LookingTo
		{
			LT_Right,
			LT_Left,
		};

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
		void ShouldIdle();
		void EnterIdle();
		void LeaveIdle();
		void Idle();

		//State machine
		GameStatesMachine<TugStates> tsm;

		//Initial States
		XMFLOAT3 tugScale;
		LookingTo lookingTo = LT_Right;

		//SceneObjects
		RenderableID tug;
	};
};