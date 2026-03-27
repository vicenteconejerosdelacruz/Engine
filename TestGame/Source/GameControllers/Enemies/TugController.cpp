#include "pch.h"
#include "TugController.h"
#include "../VenomController.h"
#include <StepTimer.h>
#include <NoStd.h>
#if defined(_EDITOR)
#include <Editor.h>
#endif

//Timer
extern DX::StepTimer timer;
extern float gameUpdateFrequency;
namespace Game
{
#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <TugControllerAtt.h>
#include <JEnd.h>

#endif

	VenomStates TugController::venomState()
	{
		return venomController()->vsm.currentState;
	}

	VenomController* TugController::venomController()
	{
		return GetController<VenomController>(venomR->at("controllers").at("venom"));
	}

	//Constructor and Binding
	TugController::TugController(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include <TugControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <TugControllerAtt.h>
#include <JEnd.h>

		tsm = {
			.currentState = TS_None,
			.onEnter = {
				{ TS_Idle, [&](auto* sm, TugStates prevState) { EnterIdle(); }},
				{ TS_CombatIdle, [&](auto* sm, TugStates prevState) { EnterCombatIdle(); }},
			},
			.onLeave = {
				{ TS_Idle,[&](auto* sm, TugStates prevState) { LeaveIdle(); }},
			},
			.onStep = {
				{ TS_None, [&](auto* sm) { tugScale = tug->scale(); tugInitialLookTo = lookingTo(); tsm.ChangeState(TS_Idle); }},
				{ TS_Idle, [&](auto* sm) { Idle(); }},
				{ TS_CombatIdle, [&](auto* sm) { CombatIdle(); }},
			}
		};
		SetInitialConditions();
	}

	void TugController::SetInitialConditions()
	{
		tsm.currentState = TS_None;
		venomR = MAKESUUUID(tug.unit(), venom());
	}

#if defined(_EDITOR)
	void TugController::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <TugControllerAtt.h>
#include <JEnd.h>
		j.erase("uuid");
	}
#endif

	void TugController::Map(SUUUID so)
	{
		using namespace Scene;
		Controller::Map(so);
		SceneObjectType type = GetSceneObjectType(FROMSUUUID(so));

		if (type == SO_Renderables)
		{
			tug = so;
		}
		SetInitialConditions();
	}

	void TugController::Unmap()
	{
		Controller::Unmap();
		//UnregisterContactCallback(PB_Static, physicObject());
		//UnregisterCharacterHitCallback(physicObject());
		tug.clear();
	}

	//Step
	void TugController::Step(float delta)
	{
#if defined(_EDITOR)
		if (!Editor::IsPlaying(unit) || Editor::IsPaused(unit))
			return;
#endif

		float dt = static_cast<float>(timer.GetElapsedSeconds());

		tsm.Step();
	}

	//JS binding
	v8_templates_creators TugController::GetV8TemplatesCreators()
	{
		v8_templates_creators creators = Controller::GetV8TemplatesCreators();
#include <Attributes/JV8Templates.h>
#include <TugControllerAtt.h>
#include <JEnd.h>
		return creators;
	}

	v8_context_creators TugController::GetV8ContextCreators()
	{
		v8_context_creators creators = Controller::GetV8ContextCreators();
#include <Attributes/JV8Context.h>
#include <TugControllerAtt.h>
#include <JEnd.h>
		return creators;
	}

	v8_functions_creators TugController::GetV8FunctionsCreators()
	{
		return {
		};
	}

	void TugController::UpdateLookTo()
	{
		float dx = venomR->position().x - tug->position().x;
		if (dx > 0.0f)
		{
			if (lookingTo() == CLT_Left)
			{
				lookingTo(CLT_Right);
				XMFLOAT3 scl = tug->scale() * lookToSwapVector();
				tug->scale(scl);
			}
		}
		else if (dx < 0.0f)
		{
			if (lookingTo() == CLT_Right)
			{
				lookingTo(CLT_Left);
				XMFLOAT3 scl = tug->scale() * lookToSwapVector();
				tug->scale(scl);
			}
		}
	}

	//Idle
	bool TugController::ShouldIdle()
	{
		return false;
	}

	void TugController::EnterIdle()
	{
		tug->SetCurrentAnimation("TugIdle", 0.0f, 1.0f, true, true);
	}

	void TugController::LeaveIdle()
	{
	}

	void TugController::Idle()
	{
		if (ShouldCombatIdle())
		{
			tsm.ChangeState(TS_CombatIdle);
		}
	}

	//CombatIdle
	bool TugController::ShouldCombatIdle()
	{
		return (venomState() == VS_Idle);
	}

	void TugController::EnterCombatIdle()
	{
		tug->SetCurrentAnimation("TugCombatIdle", 0.0f, combatIdleTimeFactor(), true, true);
	}

	void TugController::CombatIdle()
	{
		UpdateLookTo();
	}
};