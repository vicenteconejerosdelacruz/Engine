#include "pch.h"
#include "TugController.h"
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
			},
			.onLeave = {
				{ TS_Idle,[&](auto* sm, TugStates prevState) { LeaveIdle(); }},
			},
			.onStep = {
				{ TS_Idle, [&](auto* sm) { Idle(); }},
			}
		};
		SetInitialConditions();
	}

	void TugController::SetInitialConditions()
	{
		tsm.currentState = TS_None;
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
		/*
		physicScene = MAKESUUUID(unit, *GetPhysicScenes(unit).begin());
		physicObject = venom->at("physicObject").at(0);
		RegisterContactCallback(PB_Static, physicObject(), [&](JUUID uuid, unsigned int event)
			{
				OnStaticContactEvent(uuid, event);
			}
		);
		RegisterCharacterHitCallback(physicObject(), [&](PxFilterData fd)
			{
				OnCharacterHitEvent(fd);
			}
		);
		*/
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

		UpdateLookTo();
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
	}

	//Idle
	void TugController::ShouldIdle()
	{
	}
	void TugController::EnterIdle()
	{
	}
	void TugController::LeaveIdle()
	{
	}
	void TugController::Idle()
	{
	}
};