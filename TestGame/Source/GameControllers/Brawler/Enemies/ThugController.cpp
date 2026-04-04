#include "pch.h"
#include "ThugController.h"
#include "../Scene/BrawlerSceneController.h"
#include <StepTimer.h>
#include <NoStd.h>
#include <SimpleMath.h>
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
#include <Brawler/ThugControllerAtt.h>
#include <JEnd.h>

#endif

	BrawlerSceneController* ThugController::GetBrawlerSceneController()
	{
		return GetController<BrawlerSceneController>(unit, sceneController());
	}

	//Constructor and Binding
	ThugController::ThugController(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include <Brawler/ThugControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <Brawler/ThugControllerAtt.h>
#include <JEnd.h>

		tsm = {
			.currentState = TS_None,
			.onEnter = {
				{ TS_Idle, [&](auto* sm, ThugStates prevState) { EnterIdle(); }},
				{ TS_CombatIdle, [&](auto* sm, ThugStates prevState) { EnterCombatIdle(); }},
				{ TS_CombatFollow, [&](auto* sm, ThugStates prevState) { EnterCombatFollow(); }},
				{ TS_CombatPunch, [&](auto* sm, ThugStates prevState) { EnterCombatPunch(); }},
			},
			.onLeave = {
				{ TS_Idle,[&](auto* sm, ThugStates prevState) { LeaveIdle(); }},
			},
			.onStep = {
				{ TS_None, [&](auto* sm) { thugScale = thug->scale(); tsm.ChangeState(TS_Idle); }},
				{ TS_Idle, [&](auto* sm) { Idle(); }},
				{ TS_CombatIdle, [&](auto* sm) { CombatIdle(); }},
				{ TS_CombatFollow, [&](auto* sm) { CombatFollow(); }},
				{ TS_CombatPunch, [&](auto* sm) { CombatPunch(); }},
			}
		};

		thugInitialLookTo = lookingTo();
		SetInitialConditions();
	}

	void ThugController::SetInitialConditions()
	{
		tsm.currentState = TS_None;
		lookingTo(thugInitialLookTo);
		heroController.clear();
		heroRenderable.clear();
		heroAttackOffset = { 0.0f,0.0f,0.0f };
	}

#if defined(_EDITOR)
	void ThugController::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <Brawler/ThugControllerAtt.h>
#include <JEnd.h>
		j.erase("uuid");
	}
#endif

	void ThugController::Map(SUUUID so)
	{
		using namespace Scene;
		Controller::Map(so);
		SceneObjectType type = GetSceneObjectType(FROMSUUUID(so));

		if (type == SO_Renderables)
		{
			thug = so;
		}

		physicScene = MAKESUUUID(unit, *GetPhysicScenes(unit).begin());
		physicObject = thug->at("physicObject").at(0);

		GetBrawlerSceneController()->RegisterEnemy(controller);
		SetInitialConditions();
	}

	void ThugController::Unmap()
	{
		Controller::Unmap();
		thug.clear();
	}

	void ThugController::TakeHit(int damage)
	{
		OutputDebugStringA(std::string("ThugController take hit " + std::to_string(damage) + "\n").c_str());
	}

	//Step
	void ThugController::Step(float delta)
	{
#if defined(_EDITOR)
		if (!Editor::IsPlaying(unit) || Editor::IsPaused(unit))
			return;
#endif

		float dt = static_cast<float>(timer.GetElapsedSeconds());

		tsm.Step();
	}

	//JS binding
	v8_templates_creators ThugController::GetV8TemplatesCreators()
	{
		v8_templates_creators creators = Controller::GetV8TemplatesCreators();
#include <Attributes/JV8Templates.h>
#include <Brawler/ThugControllerAtt.h>
#include <JEnd.h>
		return creators;
	}

	v8_context_creators ThugController::GetV8ContextCreators()
	{
		v8_context_creators creators = Controller::GetV8ContextCreators();
#include <Attributes/JV8Context.h>
#include <Brawler/ThugControllerAtt.h>
#include <JEnd.h>
		return creators;
	}

	v8_functions_creators ThugController::GetV8FunctionsCreators()
	{
		return {
			{ "EvaluateNextFollowMovement", v8_wrap_call([&] { EvaluateNextFollowMovement(); }) },
			{ "CombatIdleNextState", v8_wrap_call([&] { CombatIdleNextState(); }) },
			{ "OnCombatPunchAnimationEnd", v8_wrap_call([&] { OnCombatPunchAnimationEnd(); }) },
			{ "TakeHit", v8_wrap_call([&](int damage) { TakeHit(damage); }) },
		};
	}

	void ThugController::CharacterMoveXZPlane(XMVECTOR displacement, float dt, float sideSpeed, XMFLOAT3 gravity)
	{
		XMVECTOR move = XMVector3Normalize(displacement) * sideSpeed * dt;
		physicObject->MoveCharacter(move, dt);
		/*
		XMVECTOR downDisp = { 0.0f, fixedDownDisplacement() + downSpeed * dt, 0.0f };
		XMVECTOR move = XMVector3Normalize(stickDisplacement) * sideSpeed * dt;
		move += downDisp;
		PxControllerCollisionFlags colFlag = physicObject->MoveCharacter(move, dt);
		touchingDown = !!(colFlag & PxControllerCollisionFlag::Enum::eCOLLISION_DOWN);
		if (!!(colFlag & PxControllerCollisionFlag::Enum::eCOLLISION_UP))
		{
			downSpeed = 0.0f;
		}
		downSpeed = (touchingDown) ? 0.0f : (downSpeed + gravity.y * dt);
		*/
	}

	void ThugController::UpdateLookTo()
	{
		float dx = heroRenderable->position().x - thug->position().x;
		if (dx > 0.0f)
		{
			if (lookingTo() == CLT_Left)
			{
				lookingTo(CLT_Right);
				XMFLOAT3 scl = thug->scale() * lookToSwapVector();
				thug->scale(scl);
			}
		}
		else if (dx < 0.0f)
		{
			if (lookingTo() == CLT_Right)
			{
				lookingTo(CLT_Left);
				XMFLOAT3 scl = thug->scale() * lookToSwapVector();
				thug->scale(scl);
			}
		}
	}

	bool ThugController::IsInAttackRange()
	{
		if (!heroRenderable) return false;

		XMFLOAT3 heroPos = heroRenderable->position() + heroAttackOffset;
		XMFLOAT3 myPos = thug->position();
		XMVECTOR len = XMVector3Length(XMVectorSubtract(XMLoadFloat3(&heroPos), XMLoadFloat3(&myPos)));
		return len.m128_f32[0] <= combatMinDistanceToAttack();
	}

	//Idle
	//bool ThugController::ShouldIdle()
	//{
	//	return false;
	//}

	void ThugController::EnterIdle()
	{
		thug->SetCurrentAnimation("ThugIdle", 0.0f, 1.0f, true, true);
	}

	void ThugController::LeaveIdle()
	{
	}

	void ThugController::Idle()
	{
		if (ShouldCombatIdle())
		{
			tsm.ChangeState(TS_CombatIdle);
		}
		else if (ShouldCombatFollow())
		{
			tsm.ChangeState(TS_CombatFollow);
		}
	}

	//CombatIdle
	bool ThugController::ShouldCombatIdle()
	{
		if (!GetBrawlerSceneController()->HeroesReadyToFight()) return false;
		return IsInAttackRange();
	}

	void ThugController::EnterCombatIdle()
	{
		thug->SetCurrentAnimation("ThugCombatIdle", 0.0f, combatIdleTimeFactor(), true, true);
	}

	void ThugController::CombatIdle()
	{
		UpdateLookTo();
		if (ShouldCombatFollow())
		{
			tsm.ChangeState(TS_CombatFollow);
		}
	}

	void ThugController::CombatIdleNextState()
	{
		if (ShouldCombatFollow())
		{
			tsm.ChangeState(TS_CombatFollow);
		}
		else if (IsInAttackRange())
		{
			tsm.ChangeState(TS_CombatPunch);
		}
	}

	//CombatFollow
	bool ThugController::ShouldCombatFollow()
	{
		if (!GetBrawlerSceneController()->HeroesReadyToFight()) return false;
		return !IsInAttackRange();
	}

	void ThugController::EnterCombatFollow()
	{
		if (heroController.empty())
		{
			std::tuple<JUUID, XMFLOAT3> heroAndOffset = GetBrawlerSceneController()->PickHeroToFight(controller);
			if (!std::get<0>(heroAndOffset).empty())
			{
				heroController = std::get<0>(heroAndOffset);
				heroAttackOffset = std::get<1>(heroAndOffset);
				heroRenderable = GetController(heroController)->sceneObject;
			}
			else
			{

			}
		}
		EvaluateNextFollowMovement();
	}

	void ThugController::CombatFollow()
	{
		if (IsInAttackRange())
		{
			tsm.ChangeState(TS_CombatPunch);
		}
		else
		{
			UpdateLookTo();
			MoveTowardHero(walkSpeed());
		}
	}

	void ThugController::MoveTowardHero(float speed)
	{
		XMFLOAT3 heroPos = heroRenderable->position() + heroAttackOffset;
		XMFLOAT3 myPos = thug->position();
		XMVECTOR diff = XMVectorSubtract(XMLoadFloat3(&heroPos), XMLoadFloat3(&myPos));
		diff.m128_f32[1] = 0.0f; diff.m128_f32[3] = 0.0f;
		XMVECTOR disp = XMVector3Normalize(diff);
		CharacterMoveXZPlane(disp, gameUpdateFrequency, speed, physicScene->gravity());
	}

	void ThugController::EvaluateNextFollowMovement()
	{
		XMFLOAT3 heroPos = heroRenderable->position() + heroAttackOffset;
		XMFLOAT3 myPos = thug->position();
		XMVECTOR diff = XMVectorSubtract(XMLoadFloat3(&heroPos), XMLoadFloat3(&myPos));
		diff.m128_f32[1] = 0.0f; diff.m128_f32[3] = 0.0f;
		XMVECTOR right = { 1.0f,0.0f,0.0f,0.0f };
		XMVECTOR radians = XMVector4AngleBetweenVectors(diff, right);
		float degree = XMConvertToDegrees(radians.m128_f32[0]);

		std::set<std::tuple<float, float>> horizontalRanges =
		{
			std::make_tuple(0.0f, combatMoveNearFarAngle()),
			std::make_tuple(180.0f - combatMoveNearFarAngle(),180.0f),
		};

		if (std::any_of(horizontalRanges.begin(), horizontalRanges.end(), [degree](auto& range)
			{
				return degree >= std::get<0>(range) && degree <= std::get<1>(range);
			}
		))
		{
			thug->SetCurrentAnimation("ThugCombatMoveFw", 0.0f, combatMoveFwTimeFactor(), true, false);
		}
		else if (diff.m128_f32[2] > 0.0f)
		{
			thug->SetCurrentAnimation("ThugCombatMoveFar", 0.0f, combatMoveNearFarTimeFactor(), true, false);
		}
		else
		{
			thug->SetCurrentAnimation("ThugCombatMoveNear", 0.0f, combatMoveNearFarTimeFactor(), true, false);
		}
	}

	//CombatPunch
	void ThugController::EnterCombatPunch()
	{
		thug->SetCurrentAnimation("ThugCombatPunch", 0.0f, combatPunchTimeFactor(), true, false);
	}

	void ThugController::CombatPunch()
	{

	}
	void ThugController::OnCombatPunchAnimationEnd()
	{
		tsm.ChangeState(TS_CombatIdle);
	}
};