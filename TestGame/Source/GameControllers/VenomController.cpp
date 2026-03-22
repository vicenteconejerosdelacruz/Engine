#include "pch.h"
#include "VenomController.h"
#include <Scene.h>
#include <SceneObject.h>
#include <Renderable/Renderable.h>
#include <Mouse.h>
#include <Keyboard.h>
#include <GamePad.h>
#include <StepTimer.h>
#include <Camera/Camera.h>
#if defined(_EDITOR)
#include <Editor.h>
#endif

//Mouse
extern std::unique_ptr<DirectX::Mouse> mouse;
//Keyboard
extern std::unique_ptr<DirectX::Keyboard> keyboard;
extern DirectX::Keyboard::KeyboardStateTracker keys;
//GamePad
extern std::unique_ptr<DirectX::GamePad> gamePad;
extern DirectX::GamePad::ButtonStateTracker buttons;
//Timer
extern DX::StepTimer timer;
extern float gameUpdateFrequency;

namespace Game
{
#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <VenomControllerAtt.h>
#include <JEnd.h>

#endif
	//Constructor and Binding
	VenomController::VenomController(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include <VenomControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <VenomControllerAtt.h>
#include <JEnd.h>

		vsm = {
			.currentState = VS_None,
			.onEnter = {
				{ VS_Intro, [&](auto* sm, VenomStates prevState) { EnterIntro(); }},
				{ VS_Idle, [&](auto* sm, VenomStates prevState) { EnterIdle(); }},
				{ VS_Walking, [&](auto* sm, VenomStates prevState) { EnterWalking(); }},
				{ VS_Running, [&](auto* sm, VenomStates prevState) { EnterRunning(); }},
				{ VS_Jumping, [&](auto* sm, VenomStates prevState) { EnterJumping(); }},
				{ VS_RunningJump, [&](auto* sm, VenomStates prevState) { EnterRunningJump(); }},
				{ VS_Attack_1,[&](auto* sm, VenomStates prevState) { EnterAttack1(); }},
				{ VS_JumpKick,[&](auto* sm, VenomStates prevState) { EnterJumpKick(); }},
				{ VS_JumpDash,[&](auto* sm, VenomStates prevState) { EnterJumpDash(); }},
			},
			.onLeave = {
				{ VS_Attack_1,[&](auto* sm, VenomStates prevState) { LeaveAttack1(); }},
			},
			.onStep = {
				{ VS_None, [&](auto* sm) { venomScale = venom->scale(); vsm.ChangeState(VS_Intro); }},
				{ VS_Idle, [&](auto* sm) { Idle(); }},
				{ VS_Walking, [&](auto* sm) { Walking(); }},
				{ VS_Running, [&](auto* sm) { Running(); }},
				{ VS_Jumping, [&](auto* sm) { Jumping(); }},
				{ VS_RunningJump, [&](auto* sm) { RunningJump(); }},
				{ VS_Attack_1, [&](auto* sm) { Attacking1(); }},
				{ VS_JumpKick, [&](auto* sm) { JumpKick(); } },
				{ VS_JumpDash, [&](auto* sm) { JumpDash(); } },
			}
		};
		SetInitialConditions();
	}

	void VenomController::SetInitialConditions()
	{
		vsm.currentState = VS_None;
		venomScale = { 0.0f,0.0f,0.0f };
		leftStick = XMVectorZero();
		runningJumpLeftStick = XMVectorZero();
		downSpeed = 0.0f;
		touchingDown = true;
		canJump = false;
		jumping = false;
		attack1Window = false;
		newAttack1 = false;
		currentAttack1Animation = 0;
	}

#if defined(_EDITOR)
	void VenomController::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <VenomControllerAtt.h>
#include <JEnd.h>
		j.erase("uuid");
	}
#endif

	void VenomController::Map(SUUUID so)
	{
		using namespace Scene;
		Controller::Map(so);
		SceneObjectType type = GetSceneObjectType(FROMSUUUID(so));

		if (type == SO_Renderables)
		{
			venom = so;
		}
		if (GetCountFromMouseCameras(unit) > 0ULL)
		{
			camera = MAKESUUUID(unit, *GetMouseCameras(unit).begin());
		}
		physicScene = MAKESUUUID(unit, *GetPhysicScenes(unit).begin());
		physicObject = venom->at("physicObject").at(0);

		//BindV8Module();
		SetInitialConditions();
	}

	void VenomController::Unmap()
	{
		Controller::Unmap();
		venom.clear();
		camera.clear();
		physicScene.clear();
		physicObject.clear();
	}

	//Step
	void VenomController::Step(float delta)
	{
#if defined(_EDITOR)
		if (!Editor::IsPlaying(unit) || Editor::IsPaused(unit))
			return;
#endif

		float dt = static_cast<float>(timer.GetElapsedSeconds());

		auto state = gamePad->GetState(0);
		if (state.IsConnected())
		{
			buttons.Update(state);
		}
		else
		{
			buttons.Reset();
		}

		XMVECTOR XMpos, XMrot, XMscl;
		XMMatrixDecompose(&XMscl, &XMrot, &XMpos, venom->animationTransformation);

		UpdateLeftStickVector();
		UpdateLookTo();
		vsm.Step();
	}

	//JS binding
	v8_templates_creators VenomController::GetV8TemplatesCreators()
	{
		v8_templates_creators creators = Controller::GetV8TemplatesCreators();
#include <Attributes/JV8Templates.h>
#include <VenomControllerAtt.h>
#include <JEnd.h>
		return creators;
	}

	v8_context_creators VenomController::GetV8ContextCreators()
	{
		v8_context_creators creators = Controller::GetV8ContextCreators();
#include <Attributes/JV8Context.h>
#include <VenomControllerAtt.h>
#include <JEnd.h>
		return creators;
	}

	v8_functions_creators VenomController::GetV8FunctionsCreators()
	{
		return {
			//{ "animationUseTransformation", [&](bool value) venom->animationUseTransformation(value); }) },
			{ "PlayerReady", v8_wrap_call([&] { VenomReady(); }) },
			{ "StartNextPunchWindow", v8_wrap_call([&] { StartVenomNextPunchWindow(); }) },
			{ "EvaluateNextPunch", v8_wrap_call([&] { EvaluateVenomNextPunch(); }) },
			//{ "SwitchToState", v8_wrap_call([&](std::string state) vsm.ChangeState(stringToVenomStates.at(state)); }) },
			{ "BeginJump", v8_wrap_call([&] { VenomBeginJump(); }) },
			{ "EndJumpLanding", v8_wrap_call([&] { VenomEndJumpLanding(); }) },
			{ "BeginRunJump", v8_wrap_call([&] { VenomBeginRunJump(); }) },
			{ "RunJumpLanding", v8_wrap_call([&] { VenomRunJumpLanding(); }) },
		};
	}

	//Joystick
	void VenomController::UpdateLeftStickVector()
	{
		std::set<VenomStates> noUpdateStates = { VS_RunningJump, VS_JumpDash };

		if (noUpdateStates.contains(vsm.currentState)) return; //maybe Vec0?

		auto pad = gamePad->GetState(0);
		if (pad.IsConnected())
		{
			leftStick = { pad.thumbSticks.leftX, 0.0f, pad.thumbSticks.leftY, 0.0f };
		}
		else
		{
			leftStick = XMVectorZero();
		}
	}

	void VenomController::UpdateLookTo()
	{
		if (vsm.currentState == VS_Intro) return;

		float len = leftStick.m128_f32[0];

		if (fabsf(len) < lookToThreshold()) return;

		if (len < 0.0f)
		{
			XMFLOAT3 scale = venom->scale();
			if (scale.z > 0.0f)
			{
				scale.z = -venomScale.z;
				venom->scale(scale);
			}
		}
		else if (len > 0.0f)
		{
			XMFLOAT3 scale = venom->scale();
			if (scale.z < 0.0f)
			{
				scale.z = venomScale.z;
				venom->scale(scale);
			}
		}
	}

	//Movement
	void VenomController::CharacterMove(XMVECTOR stickDisplacement, float dt, float sideSpeed, XMFLOAT3 gravity)
	{
		XMVECTOR downDisp = { 0.0f, fixedDownDisplacement() + downSpeed * dt, 0.0f };
		XMFLOAT3 scale = venom->scale();
		XMVECTOR move = XMVector3Normalize(stickDisplacement) * sideSpeed * dt;
		//move = XMVectorScale(move, delta) + downDisp;
		move += downDisp;
		PxControllerCollisionFlags colFlag = physicObject->MoveCharacter(move, dt);
		touchingDown = !!(colFlag & PxControllerCollisionFlag::Enum::eCOLLISION_DOWN);
		downSpeed = (touchingDown) ? 0.0f : (downSpeed + gravity.y * dt);
	}

	void VenomController::MoveForward(float sideSpeed)
	{
		CharacterMove(leftStick, gameUpdateFrequency, sideSpeed, physicScene->gravity());
	}

	void VenomController::JumpingMoveForward(float sideSpeed)
	{
		CharacterMove(leftStick, gameUpdateFrequency, sideSpeed, physicScene->gravity());
	}

	void VenomController::RunningJumpMoveForward(float sideSpeed)
	{
		CharacterMove(runningJumpLeftStick, gameUpdateFrequency, sideSpeed, physicScene->gravity());
	}

	//Intro
	void VenomController::EnterIntro()
	{
		venom->animationUseTransformation(true);
		venom->SetCurrentAnimation("Intro", 0.0f, 1.0f, true, false);
	}

	void VenomController::VenomReady()
	{
		vsm.ChangeState(VS_Idle);
	}

	//Idle
	bool VenomController::ShouldIdle()
	{
		XMVECTOR len = XMVector3Length(leftStick);
		float l = len.m128_f32[0];
		return l < walkThreshold();
	}

	void VenomController::EnterIdle()
	{
		canJump = true;
		venom->animationUseTransformation(false);
		venom->SetCurrentAnimation("Idle", 0.0f, 1.0f, true, true);
	}

	void VenomController::Idle()
	{
		if (ShouldAttackX())
		{
			vsm.ChangeState(VS_Attack_1);
		}
		else if (ShouldJump())
		{
			vsm.ChangeState(VS_Jumping);
		}
		else if (ShouldRun())
		{
			vsm.ChangeState(VS_Running);
		}
		else if (ShouldWalk())
		{
			vsm.ChangeState(VS_Walking);
		}
	}

	//Walking
	bool VenomController::ShouldWalk()
	{
		XMVECTOR len = XMVector3Length(leftStick);
		float l = len.m128_f32[0];
		return l > walkThreshold() && l < runThreshold();
	}

	void VenomController::EnterWalking()
	{
		venom->SetCurrentAnimation("Walk", 0.0f, 1.0f, true, true);
	}

	void VenomController::Walking()
	{
		if (ShouldAttackX())
		{
			vsm.ChangeState(VS_Attack_1);
			return;
		}
		else if (ShouldJump())
		{
			vsm.ChangeState(VS_Jumping);
			return;
		}

		MoveForward(walkSpeed());
		if (ShouldIdle())
		{
			vsm.ChangeState(VS_Idle);
		}
		else if (ShouldRun())
		{
			vsm.ChangeState(VS_Running);
		}
	}

	//Running
	bool VenomController::ShouldRun()
	{
		XMVECTOR len = XMVector3Length(leftStick);
		float l = len.m128_f32[0];
		return l > runThreshold();
	}

	void VenomController::EnterRunning()
	{
		venom->SetCurrentAnimation("Run", 0.0f, 1.0f, true, true);
	}

	void VenomController::Running()
	{
		if (ShouldAttackX())
		{
			vsm.ChangeState(VS_Attack_1);
			return;
		}

		MoveForward(runSpeed());
		if (ShouldIdle())
		{
			vsm.ChangeState(VS_Idle);
		}
		else if (ShouldWalk())
		{
			vsm.ChangeState(VS_Walking);
		}
		else if (ShouldJump())
		{
			vsm.ChangeState(VS_RunningJump);
		}
	}

	//Jumping
	bool VenomController::ShouldJump()
	{
		return (buttons.a == GamePad::ButtonStateTracker::PRESSED) && touchingDown && canJump && !jumping;
	}

	void VenomController::EnterJumping()
	{
		venom->animationUseTransformation(true);
		venom->SetCurrentAnimation("JumpBegin");
	}

	void VenomController::VenomBeginJump()
	{
		venom->SetCurrentAnimation("JumpLoop", 0.0f, 1.0f, true, true);
		jumping = true;
		canJump = false;
		downSpeed += jumpSpeed();
		touchingDown = false;
	}

	void VenomController::Jumping()
	{
		if (ShouldJumpKick())
		{
			vsm.ChangeState(VS_JumpKick);
			return;
		}

		XMVECTOR len = XMVector3Length(leftStick);
		float l = XMVectorGetX(len);
		JumpingMoveForward(walkSpeed() * l);
		if (touchingDown && jumping && !canJump)
		{
			jumping = false;
			canJump = true;
			vsm.ChangeState(VS_Idle);
		}
	}

	//RunningJump
	void VenomController::EnterRunningJump()
	{
		runningJumpLeftStick = leftStick;
		venom->animationUseTransformation(true);
		venom->SetCurrentAnimation("RunJumpBegin");
	}

	void VenomController::VenomBeginRunJump()
	{
		venom->SetCurrentAnimation("RunJumpLoop");
		runningJumpTimeLeft = runningJumpTime();
	}

	void VenomController::RunningJump()
	{
		if (ShouldJumpDash())
		{
			vsm.ChangeState(VS_JumpDash);
			return;
		}

		if (runningJumpTimeLeft != 0.0f)
		{
			float dt = static_cast<float>(timer.GetElapsedSeconds());
			runningJumpTimeLeft = std::max(runningJumpTimeLeft - dt, 0.0f);
			if (runningJumpTimeLeft == 0.0f)
			{
				venom->SetCurrentAnimation("RunJumpLanding");
			}
		}
		RunningJumpMoveForward(runSpeed());
	}

	void VenomController::VenomRunJumpLanding()
	{
		vsm.ChangeState(VS_Running);
	}

	void VenomController::VenomEndJumpLanding()
	{
		vsm.ChangeState(VS_Idle);
	}

	//Attack1
	bool VenomController::ShouldAttackX()
	{
		return (buttons.x == GamePad::ButtonStateTracker::PRESSED);
	}

	void VenomController::EnterAttack1()
	{
		auto animation = Attack1Animations.at(currentAttack1Animation);
		venom->SetCurrentAnimation(animation);
		currentAttack1Animation = (currentAttack1Animation + 1) % Attack1Animations.size();
	}

	void VenomController::Attacking1()
	{
		if (!newAttack1 && attack1Window && ShouldAttackX())
		{
			newAttack1 = true;
		}
	}

	void VenomController::StartVenomNextPunchWindow()
	{
		attack1Window = true;
	}

	void VenomController::EvaluateVenomNextPunch()
	{
		std::vector<std::pair<std::function<bool()>, std::function<void()>>> postAttackActions = {
			{
				[&]() { return ShouldRun(); },
				[&]() { vsm.ChangeState(VS_Running); }
			},
			{
				[&]() { return ShouldWalk(); },
				[&]() { vsm.ChangeState(VS_Walking); }
			},
			{
				[&]() { return ShouldIdle(); },
				[&]() { vsm.ChangeState(VS_Idle); }
			}
		};
		if (newAttack1)
		{
			EnterAttack1();
		}
		else
		{
			for (auto& [cond, action] : postAttackActions)
			{
				if (cond())
				{
					action();
					break;
				}
			}
		}
		newAttack1 = false;
		attack1Window = false;
	}

	void VenomController::LeaveAttack1()
	{
		attack1Window = false;
		newAttack1 = false;
		currentAttack1Animation = 0;
	}

	//JuumpKick
	bool VenomController::ShouldJumpKick()
	{
		return jumping && (buttons.x == GamePad::ButtonStateTracker::PRESSED);
	}

	void VenomController::EnterJumpKick()
	{
		venom->animationUseTransformation(true);
		venom->SetCurrentAnimation("JumpKick");
	}

	void VenomController::JumpKick()
	{
		XMVECTOR len = XMVector3Length(leftStick);
		float l = XMVectorGetX(len);
		JumpingMoveForward(walkSpeed() * l);
		if (touchingDown && jumping && !canJump)
		{
			jumping = false;
			canJump = true;
			vsm.ChangeState(VS_Idle);
		}
	}

	//JumpDash
	bool VenomController::ShouldJumpDash()
	{
		return (buttons.x == GamePad::ButtonStateTracker::PRESSED);
	}

	void VenomController::EnterJumpDash()
	{
		std::srand(static_cast<int>(std::time(0)));
		jumpDashTimeLeft = jumpDashTime();
		jumpDashAnimationIdx = std::rand() % DashAnimations.size();
		std::string dashAnim = DashAnimations.at(jumpDashAnimationIdx);
		venom->animationUseTransformation(true);
		venom->SetCurrentAnimation(dashAnim);
	}

	void VenomController::JumpDash()
	{
		if (jumpDashTimeLeft != 0.0f)
		{
			float dt = static_cast<float>(timer.GetElapsedSeconds());
			jumpDashTimeLeft = std::max(jumpDashTimeLeft - dt, 0.0f);
			if (jumpDashTimeLeft == 0.0f)
			{
				venom->SetCurrentAnimation(DashLandingAnimations.at(jumpDashAnimationIdx));
			}
		}
		RunningJumpMoveForward(runSpeed());
	}
}

//Animations chop analysis
//103541_Shackle
//103531_Descent_Loop
//103531_Descent_End
//Intro: 103531_Descent_Loop -> 103531_Descent_End -> 103541_Shackle

//103501_LowCrawl_Idle_R
//103501_LowCrawl_Move
//Wall:

//A4_10_R
//A4_11_R
//A4_12_R
//A4_13_R
//Jump Kick

//103571_Devouring_Loop
//Jump

//JumpKick Frames
//103551_Swing_Start
//10 frames
//key frame 0
//rot -90, 0, 90
//key frame 10
//rot -12.353, -98.824, -18.823  

//103551_Dash_L_End
//103551_Dash_R_End