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
	//Thresholds
	static const float lookToThreshold = 0.03f;
	static const float walkThreshold = 0.05f;
	static const float runThreshold = 0.4f;

	//Speeds
	static const float walkSpeed = 3.0f;
	static const float runSpeed = 10.0f;

	//Scene Bounds(for now)
	static XMFLOAT2 zBounds = { -4.1f ,1.7f };

	//JS Module
	static std::unique_ptr<v8pp::module> v8ppModule = nullptr;
	static void VenomReady()
	{
		VenomController* venom = static_cast<VenomController*>(GetControllerByName("venom").get());
		venom->VenomReady();
	}
	static void StartVenomNextPunchWindow()
	{
		VenomController* venom = static_cast<VenomController*>(GetControllerByName("venom").get());
		venom->StartVenomNextPunchWindow();
	}
	static void EvaluateVenomNextPunch()
	{
		VenomController* venom = static_cast<VenomController*>(GetControllerByName("venom").get());
		venom->EvaluateVenomNextPunch();
	}
	static void SwitchVenomToState(std::string state)
	{
		VenomController* venom = static_cast<VenomController*>(GetControllerByName("venom").get());
		venom->vsm.ChangeState(stringToVenomStates.at(state));
	}
	static void VenomRunJumpLanding()
	{
		VenomController* venom = static_cast<VenomController*>(GetControllerByName("venom").get());
		venom->VenomRunJumpLanding();
	}
	static void BindV8Module()
	{
		if (v8ppModule == nullptr)
		{
			Scripting::BindModule([&](v8::Isolate* isolate)
				{

					v8ppModule = std::make_unique<v8pp::module>(isolate);
					v8ppModule->function("PlayerReady", &Game::VenomReady).
						function("StartNextPunchWindow", &Game::StartVenomNextPunchWindow).
						function("EvaluateNextPunch", &Game::EvaluateVenomNextPunch).
						function("SwitchToState", &Game::SwitchVenomToState).
						function("RunJumpLanding", &Game::VenomRunJumpLanding);
				}
			);
		}
	}

	//Constructor and Binding
	VenomController::VenomController()
	{
		vsm = {
			.currentState = VS_None,
			.onEnter = {
				{ VS_Intro, [this](VenomStates prevState) { EnterIntro(); }},
				{ VS_Idle, [this](VenomStates prevState) { EnterIdle(); }},
				{ VS_Walking, [this](VenomStates prevState) { EnterWalking(); }},
				{ VS_Running, [this](VenomStates prevState) { EnterRunning(); }},
				{ VS_Jumping, [this](VenomStates prevState) { EnterJumping(); }},
				{ VS_RunningJump, [this](VenomStates prevState) { EnterRunningJump(); }},
				{ VS_Attack_1,[this](VenomStates prevState) { EnterAttack1(); }}
			},
			.onLeave = {
				{ VS_Attack_1,[this](VenomStates prevState) { LeaveAttack1(); }}
			},
			.onStep = {
				{ VS_None, [this]() { venomScale = venom->scale(); vsm.ChangeState(VS_Intro); }},
				{ VS_Idle, [this]() { Idle(); }},
				{ VS_Walking, [this]() { Walking(); }},
				{ VS_Running, [this]() { Running(); }},
				{ VS_Jumping, [this]() { Jumping(); }},
				{ VS_RunningJump, [this]() { RunningJump(); }},
				{ VS_Attack_1, [this]() { Attacking1(); }}
			}
		};
		lastAnimPos = XMVectorZero();
		lastAnimPosDelta = XMVectorZero();
		lastAnimPosDelta2 = XMVectorZero();
		attack1Window = false;
		newAttack1 = false;
	}

	void VenomController::Map(JUUID so)
	{
		using namespace Scene;
		Controller::Map(so);
		SceneObjectType type = GetSceneObjectType(so);
		if (type == SO_Renderables)
		{
			venom = so;
		}
		if (GetCountFromMouseCameras() > 0ULL)
		{
			camera = *GetMouseCameras().begin();
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
		if (!Editor::IsPlaying() || Editor::IsPaused())
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
	void VenomController::BindToV8Context(v8pp::context& context)
	{
		context.value("venom", v8ppModule->new_instance());
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

	void VenomController::VenomRunJumpLanding()
	{
		vsm.ChangeState(VS_Running);
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

		if (fabsf(len) < lookToThreshold) return;

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
		return l < walkThreshold;
	}

	bool VenomController::ShouldWalk()
	{
		XMVECTOR len = XMVector3Length(leftStick);
		float l = len.m128_f32[0];
		return l > walkThreshold && l < runThreshold;
	}

	bool VenomController::ShouldRun()
	{
		XMVECTOR len = XMVector3Length(leftStick);
		float l = len.m128_f32[0];
		return l > runThreshold;
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
		venom->SetCurrentAnimation("Walk", 0.0f, 1.0f, true, true);
	}

	void VenomController::EnterRunning()
	{
		venom->SetCurrentAnimation("Run", 0.0f, 1.0f, true, true);
	}

	void VenomController::EnterJumping()
	{
		venom->animationUseTransformation(true);
		venom->SetCurrentAnimation("Jump");
	}

	void VenomController::EnterRunningJump()
	{
		runningJumpLeftStick = leftStick;
		venom->animationUseTransformation(false);
		venom->SetCurrentAnimation("RunJump");
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

		MoveForward(walkSpeed);
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

		MoveForward(runSpeed);
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
		XMVECTOR len = XMVector3Length(leftStick);
		float l = XMVectorGetX(len);
		JumpingMoveForward(walkSpeed * l);
	}

	void VenomController::RunningJump()
	{
		RunningJumpMoveForward(runSpeed);
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