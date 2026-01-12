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

namespace Game
{
	auto* This() { return ContextController<VenomController>(); };

#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <VenomControllerAtt.h>
#include <JEnd.h>

#endif
	//Scene Bounds(for now)
	static XMFLOAT2 zBounds = { -6.8f ,0.90f };

	//JS Module
	static std::map<JUUID, std::unique_ptr<v8pp::module>> v8ppModule;

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
				{ VS_Attack_1,[&](auto* sm, VenomStates prevState) { EnterAttack1(); }}
			},
			.onLeave = {
				{ VS_Attack_1,[&](auto* sm, VenomStates prevState) { LeaveAttack1(); }},
				{ VS_Jumping, [&](auto* sm, VenomStates prevState) { LeaveJumping(); }},
				{ VS_Jumping, [&](auto* sm, VenomStates prevState) { LeaveRunningJumping(); }}
			},
			.onStep = {
				{ VS_None, [&](auto* sm) { venomScale = venom->scale(); vsm.ChangeState(VS_Intro); }},
				{ VS_Idle, [&](auto* sm) { Idle(); }},
				{ VS_Walking, [&](auto* sm) { Walking(); }},
				{ VS_Running, [&](auto* sm) { Running(); }},
				{ VS_Jumping, [&](auto* sm) { Jumping(); }},
				{ VS_RunningJump, [&](auto* sm) { RunningJump(); }},
				{ VS_Attack_1, [&](auto* sm) { Attacking1(); }}
			}
		};

		venomScale = { 0.0f,0.0f,0.0f };
		leftStick = XMVectorZero();
		runningJumpLeftStick = XMVectorZero();
		lastAnimPos = XMVectorZero();
		lastAnimPosDelta = XMVectorZero();
		lastAnimPosDelta2 = XMVectorZero();
		attack1Window = false;
		newAttack1 = false;
		JumpingStateData.jumping = false;
		JumpingStateData.falling = false;
		JumpingStateData.kicking = false;
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
		BindV8Module();
	}

	void VenomController::Unmap()
	{
		Controller::Unmap();
		venom.clear();
		camera.clear();
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

		lastAnimPosDelta2 = lastAnimPosDelta;
		lastAnimPosDelta = XMVectorSubtract(XMpos, lastAnimPos);
		lastAnimPos = XMpos;

		UpdateLeftStickVector();
		UpdateLookTo();
		vsm.Step();
		XMFLOAT3 vpos = venom->position();
		XMFLOAT3 cpos = camera->position();
		cpos.x = vpos.x;
		camera->position(cpos);
	}

	//JS binding
	void VenomController::BindV8Module()
	{
		JUUID uuid = controller;
		if (!v8ppModule.contains(uuid))
		{
			Scripting::BindModule([&](v8::Isolate* isolate)
				{
					v8ppModule.insert_or_assign(uuid, std::make_unique<v8pp::module>(isolate));
					v8ppModule.at(uuid)->function("animationUseTransformation", [](bool value)
						{
							This()->venom->animationUseTransformation(value);
						}
					).function("PlayerReady", []
						{
							This()->VenomReady();
						}
					).function("StartNextPunchWindow", []
						{
							This()->StartVenomNextPunchWindow();
						}
					).function("EvaluateNextPunch", []
						{
							This()->EvaluateVenomNextPunch();
						}
					).function("SwitchToState", [](std::string state)
						{
							This()->vsm.ChangeState(stringToVenomStates.at(state));
						}
					).function("BeginJump", []
						{
							This()->VenomBeginJump();
						}
					).function("EndJumpLanding", []
						{
							This()->VenomEndJumpLanding();
						}
					).function("BeginRunJump", []
						{
							This()->VenomBeginRunJump();
						}
					).function("RunJumpLanding", []
						{
							This()->VenomRunJumpLanding();
						}
					);
				}
			);
		}
	}

	void VenomController::BindToV8Context(v8pp::context& context)
	{
		context.value("venom", v8ppModule.at(controller)->new_instance());

		auto* isolate = context.isolate();

		v8::HandleScope handle_scope(isolate);
		v8::MaybeLocal<v8::String> maybe_v8_string = v8::String::NewFromUtf8(
			isolate,               // The current isolate
			controller.c_str(),    // The C-style string pointer (const char*)
			v8::NewStringType::kNormal, // Type of string
			static_cast<int>(controller.length())    // Optional: length in bytes for performance
		);
		v8::Local<v8::String> v8_string = maybe_v8_string.ToLocalChecked();
		context.value("uuid", v8_string);
	}

	void VenomController::VenomReady()
	{
		vsm.ChangeState(VS_Idle);
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

	void VenomController::VenomBeginRunJump()
	{
		venom->SetCurrentAnimation("RunJumpLoop");
	}

	void VenomController::VenomRunJumpLanding()
	{
		vsm.ChangeState(VS_Running);
	}

	void VenomController::VenomBeginJump()
	{
		JumpingStateData.jumping = true;
		JumpingStateData.falling = false;
		JumpingStateData.jumpTween = std::make_unique<tween>(tween(0.0f, jumpHeight(), jumpTime(), tween::easing::sine_ease_out));
		venom->SetCurrentAnimation("JumpLoop", 0.0f, 1.0f, true, true);
	}

	void VenomController::VenomBeginFall()
	{
		JumpingStateData.jumping = false;
		JumpingStateData.falling = true;
		JumpingStateData.fallTween = std::make_unique<tween>(tween(jumpHeight(), 0.0f, jumpTime(), tween::easing::sine_ease_in));
	}

	void VenomController::VenomEndJumpLanding()
	{
		vsm.ChangeState(VS_Idle);
	}

	//Scene Object
	void VenomController::MoveForward(float step)
	{
		float delta = static_cast<float>(timer.GetElapsedSeconds() * step);

		XMFLOAT3 scale = venom->scale();
		XMVECTOR move = XMVector3Normalize(leftStick);
		move = XMVectorScale(move, delta);
		float dz = -lastAnimPosDelta.m128_f32[2];
		if (dz > 0.0f)
		{
			move.m128_f32[0] = scale.z * std::max(-lastAnimPosDelta.m128_f32[2], 0.0f);
		}
		else
		{
			move.m128_f32[0] = scale.z * std::max(-lastAnimPosDelta2.m128_f32[2], 0.0f);
		}
		XMFLOAT3 p = venom->position();
		XMVECTOR pos = XMLoadFloat3(&p);
		pos = XMVectorAdd(pos, move);
		XMStoreFloat3(&p, pos);
		p.z = std::clamp(p.z, zBounds.x, zBounds.y);
		venom->position(p);
	}

	void VenomController::JumpingMoveForward(float step)
	{
		float delta = static_cast<float>(timer.GetElapsedSeconds() * step);

		XMFLOAT3 scale = venom->scale();
		XMVECTOR move = XMVector3Normalize(leftStick);
		move = XMVectorScale(move, delta);
		XMFLOAT3 p = venom->position();
		XMVECTOR pos = XMLoadFloat3(&p);
		pos = XMVectorAdd(pos, move);
		XMStoreFloat3(&p, pos);
		p.z = std::clamp(p.z, zBounds.x, zBounds.y);
		venom->position(p);
	}

	void VenomController::RunningJumpMoveForward(float step)
	{
		float delta = static_cast<float>(timer.GetElapsedSeconds() * step);

		XMFLOAT3 scale = venom->scale();
		XMVECTOR move = XMVector3Normalize(runningJumpLeftStick);
		move = XMVectorScale(move, delta);
		XMFLOAT3 p = venom->position();
		XMVECTOR pos = XMLoadFloat3(&p);
		pos = XMVectorAdd(pos, move);
		XMStoreFloat3(&p, pos);
		p.z = std::clamp(p.z, zBounds.x, zBounds.y);
		venom->position(p);
	}

	//Joystick
	void VenomController::UpdateLeftStickVector()
	{
		std::set<VenomStates> noUpdateStates = { VS_RunningJump };

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

	//States handling
	//Shoulds
	bool VenomController::ShouldIdle()
	{
		XMVECTOR len = XMVector3Length(leftStick);
		float l = len.m128_f32[0];
		return l < walkThreshold();
	}

	bool VenomController::ShouldWalk()
	{
		XMVECTOR len = XMVector3Length(leftStick);
		float l = len.m128_f32[0];
		return l > walkThreshold() && l < runThreshold();
	}

	bool VenomController::ShouldRun()
	{
		XMVECTOR len = XMVector3Length(leftStick);
		float l = len.m128_f32[0];
		return l > runThreshold();
	}

	bool VenomController::ShouldJump()
	{
		return (buttons.a == GamePad::ButtonStateTracker::PRESSED);
	}

	bool VenomController::ShouldAttackX()
	{
		return (buttons.x == GamePad::ButtonStateTracker::PRESSED);
	}
	//Enter
	void VenomController::EnterIntro()
	{
		venom->animationUseTransformation(true);
		venom->SetCurrentAnimation("Intro", 0.0f, 1.0f, true, false);
	}

	void VenomController::EnterIdle()
	{
		venom->animationUseTransformation(false);
		venom->SetCurrentAnimation("Idle_C", 0.0f, 1.0f, true, true);
	}

	void VenomController::EnterWalking()
	{
		venom->animationUseTransformation(false);
		venom->SetCurrentAnimation("Walk", 0.0f, 1.0f, true, true);
	}

	void VenomController::EnterRunning()
	{
		venom->animationUseTransformation(false);
		venom->SetCurrentAnimation("Run", 0.0f, 1.0f, true, true);
	}

	void VenomController::EnterJumping()
	{
		venom->animationUseTransformation(true);
		venom->SetCurrentAnimation("JumpBegin");
	}

	void VenomController::EnterRunningJump()
	{
		runningJumpLeftStick = leftStick;
		venom->animationUseTransformation(true);
		venom->SetCurrentAnimation("RunJumpBegin");
		RunningJumpStateData.jumpTween = std::make_unique<tween>(tween(0.0f, 1.0f, runningJumpTime(), tween::easing::linear));
		RunningJumpStateData.dash = false;
	}

	void VenomController::EnterAttack1()
	{
		auto animation = Attack1Animations.at(currentAttack1Animation);
		venom->SetCurrentAnimation(animation);
		currentAttack1Animation = (currentAttack1Animation + 1) % Attack1Animations.size();
	}
	//Steps
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

	void VenomController::Jumping()
	{
		if (JumpingStateData.jumping)
		{
			float height = JumpingStateData.jumpTween->step();
			if (height == jumpHeight())
			{
				VenomBeginFall();
				JumpingStateData.jumping = false;
			}
			XMFLOAT3 pos = venom->position();
			pos.y = height;
			venom->position(pos);
		}
		else if (JumpingStateData.falling)
		{
			float height = JumpingStateData.fallTween->step();
			if (height == 0.0f)
			{
				venom->SetCurrentAnimation("JumpLanding");
				JumpingStateData.falling = false;
			}
			XMFLOAT3 pos = venom->position();
			pos.y = height;
			venom->position(pos);
		}

		if (!JumpingStateData.jumping && !JumpingStateData.falling) return;

		if (!JumpingStateData.kicking && ShouldAttackX())
		{
			JumpingStateData.kicking = true;
			venom->SetCurrentAnimation("JumpKick");
		}

		XMVECTOR len = XMVector3Length(leftStick);
		float l = XMVectorGetX(len);
		JumpingMoveForward(walkSpeed() * l);
	}

	void VenomController::RunningJump()
	{
		if (RunningJumpStateData.jumpTween)
		{
			float t = RunningJumpStateData.jumpTween->step();
			if (t == 1.0f)
			{
				RunningJumpStateData.jumpTween = nullptr;
				if (!RunningJumpStateData.dash)
				{
					venom->animationUseTransformation(true);
					venom->SetCurrentAnimation("RunJumpLanding");
				}
				else
				{
					std::string dashLandingAnim = DashLandingAnimations.at(RunningJumpStateData.dashAnimationIdx);
					venom->SetCurrentAnimation(dashLandingAnim);
				}
			}
			else if (!RunningJumpStateData.dash && ShouldAttackX())
			{
				std::srand(static_cast<int>(std::time(0)));
				RunningJumpStateData.dashAnimationIdx = std::rand() % DashAnimations.size();
				std::string dashAnim = DashAnimations.at(RunningJumpStateData.dashAnimationIdx);
				venom->animationUseTransformation(true);
				venom->SetCurrentAnimation(dashAnim);
				RunningJumpStateData.jumpTween = std::make_unique<tween>(tween(0.0f, 1.0f, runningJumpAttackTime(), tween::easing::linear));
				RunningJumpStateData.dash = true;
			}
		}
		RunningJumpMoveForward(runSpeed());
	}

	void VenomController::Attacking1()
	{
		if (!newAttack1 && attack1Window && ShouldAttackX())
		{
			newAttack1 = true;
		}
	}

	//Leaves
	void VenomController::LeaveAttack1()
	{
		attack1Window = false;
		newAttack1 = false;
		currentAttack1Animation = 0;
	}

	void VenomController::LeaveJumping()
	{
		JumpingStateData.jumping = false;
		JumpingStateData.falling = false;
		JumpingStateData.kicking = false;
	}

	void VenomController::LeaveRunningJumping()
	{
		RunningJumpStateData.jumpTween = nullptr;
		RunningJumpStateData.dash = false;
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