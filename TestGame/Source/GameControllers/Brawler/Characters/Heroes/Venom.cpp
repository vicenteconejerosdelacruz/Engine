#include "pch.h"
#include "Venom.h"
//#include "BrawlerCamera.h"
#include "../../Scene/BrawlerScene.h"
#include <GamePhysics.h>
#include <Scene.h>
#include <SceneObject.h>
#include <Renderable/Renderable.h>
#include <Mouse.h>
#include <Keyboard.h>
#include <GamePad.h>
#include <StepTimer.h>
//#include <Camera/Camera.h>
#include <NoStd.h>
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

namespace Game::Brawler
{
	std::vector<std::string> GetBlockedWallMovementMasks()
	{
		return nostd::GetValuesFromFlagsMap(WallMovementAxisToString);
	}

#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include <Brawler/VenomAtt.h>
#include <JEnd.h>
#endif

	BrawlerCamera* Venom::GetBrawlerCamera()
	{
		return Game::GetController<BrawlerScene>(unit, sceneController())->GetCameraController();
	}

	VenomStates Venom::GetState()
	{
		return vsm.currentState;
	}

	//Constructor and Binding
	Venom::Venom(nlohmann::json& json) : Hero(json)
	{
#include <Attributes/JInit.h>
#include <Brawler/VenomAtt.h>
#include <JEnd.h>
#include <Attributes/JUpdate.h>
#include <Brawler/VenomAtt.h>
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
				{ VS_GrabWall, [&](auto* sm, VenomStates prevState) { EnterGrabWall(); }},
				{ VS_WallIdle, [&](auto* sm, VenomStates prevState) { EnterWallIdle(); }},
				{ VS_CrawlOnWall, [&](auto* sm, VenomStates prevState) { EnterCrawlOnWall(); }},
				{ VS_DetachFromWall, [&](auto* sm, VenomStates prevState) { EnterDetachFromWall(); }},
				{ VS_Falling, [&](auto* sm, VenomStates prevState) { EnterFalling(); }},
				{ VS_Death, [&](auto* sm, VenomStates prevState) { EnterDeath(); }},
				{ VS_WallToSwing, [&](auto* sm, VenomStates prevState) { EnterWallToSwing(); }},
			},
			.onLeave = {
				{ VS_Attack_1,[&](auto* sm, VenomStates prevState) { LeaveAttack1(); }},
				{ VS_CrawlOnWall, [&](auto* sm, VenomStates prevState) { LeaveCrawlOnWall(); }},
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
				{ VS_WallIdle, [&](auto* sm) { WallIdle(); } },
				{ VS_CrawlOnWall, [&](auto* sm) { CrawlOnWall(); } },
				{ VS_Falling, [&](auto* sm) { Falling(); }},
			}
		};
		initialHealth = health();
		SetInitialConditions();
	}

	void Venom::SetInitialConditions()
	{
		health(initialHealth);
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
		canAttachToWall(false);
	}

#if defined(_EDITOR)
	void Venom::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <Brawler/VenomAtt.h>
#include <JEnd.h>
		Hero::WriteJson(j);
	}
#endif

	void Venom::Map(SUUUID so)
	{
		using namespace Scene;
		Hero::Map(so);
		SceneObjectType type = GetSceneObjectType(FROMSUUUID(so));

		if (type == SO_Renderables)
		{
			venom = so;
		}
		GetController<BrawlerScene>(unit, sceneController())->RegisterHero(uuid());
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
		SetInitialConditions();
	}

	void Venom::Unmap()
	{
		Hero::Unmap();
		UnregisterContactCallback(PB_Static, physicObject());
		UnregisterCharacterHitCallback(physicObject());
		venom.clear();
		physicScene.clear();
		physicObject.clear();
	}

	void Venom::TakeHit(JUUID enemyController, int damage)
	{
		health(std::max(0, health() - damage));
		GetController<BrawlerScene>(unit, sceneController())->HeroTookHit(enemyController, health());
	}

	//Step
	void Venom::Step(float delta)
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

		if (vsm.currentState != VS_Death)
		{
			XMVECTOR XMpos, XMrot, XMscl;
			XMMatrixDecompose(&XMscl, &XMrot, &XMpos, venom->animationTransformation);

			UpdateLeftStickVector();
			UpdateLookTo();
		}
		vsm.Step();
		if (ShouldDie())
		{
			vsm.ChangeState(VS_Death);
		}
	}

	void Venom::OnStaticContactEvent(JUUID physicObject, unsigned int event)
	{
		PhysicObjectID phO = physicObject;

		if (std::set<VenomStates>({ VS_GrabWall, VS_WallIdle, VS_CrawlOnWall, VS_DetachFromWall }).contains(vsm.currentState))
			return;
	}

	void Venom::OnCharacterHitEvent(PxFilterData fd)
	{
		std::set<VenomStates> nonFloorStates = { VS_GrabWall, VS_WallIdle, VS_CrawlOnWall, VS_DetachFromWall };
		//skips contacts if the character is in a wall state
		if (nonFloorStates.contains(vsm.currentState))
			return;

		BrawlerCamera* brawlerCam = GetBrawlerCamera();

		if ((CM_Floor & fd.word0) && brawlerCam->followY())
		{
			brawlerCam->followY(false);
		}
	}

	//JS binding
	v8_templates_creators Venom::GetV8TemplatesCreators()
	{
		v8_templates_creators creators = Hero::GetV8TemplatesCreators();
#include <Attributes/JV8Templates.h>
#include <Brawler/VenomAtt.h>
#include <JEnd.h>
		return creators;
	}

	v8_context_creators Venom::GetV8ContextCreators()
	{
		v8_context_creators creators = Hero::GetV8ContextCreators();
#include <Attributes/JV8Context.h>
#include <Brawler/VenomAtt.h>
#include <JEnd.h>
		return creators;
	}

	v8_functions_creators Venom::GetV8FunctionsCreators()
	{
		return {
			{ "PlayerReady", v8_wrap_call([&] { VenomReady(); }) },
			{ "StartNextPunchWindow", v8_wrap_call([&] { StartVenomNextPunchWindow(); }) },
			{ "EvaluateNextPunch", v8_wrap_call([&] { EvaluateVenomNextPunch(); }) },
			{ "SwitchToState", v8_wrap_call([&](std::string state) { vsm.ChangeState(stringToVenomStates.at(state)); }) },
			{ "BeginJump", v8_wrap_call([&] { VenomBeginJump(); }) },
			{ "EndJumpLanding", v8_wrap_call([&] { VenomEndJumpLanding(); }) },
			{ "BeginRunJump", v8_wrap_call([&] { VenomBeginRunJump(); }) },
			{ "RunJumpLanding", v8_wrap_call([&] { VenomRunJumpLanding(); }) },
			{ "TakeHit", v8_wrap_call([&](JUUID enemyController, int damage) { TakeHit(enemyController, damage); }) },
			{ "OnDeathAnimationEnd", v8_wrap_call([&] { OnDeathAnimationEnd(); }) },
			{ "PlayPunchSound", v8_wrap_call([&](int punchIdx, int enemyHealth) { PlayPunchSound(punchIdx, enemyHealth); })},
		};
	}

	//Joystick
	static const std::set<VenomStates> noUpdateStates({ VS_RunningJump, VS_JumpDash });
	void Venom::UpdateLeftStickVector()
	{
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

	static const std::set<VenomStates> noUpdateLookToStates({ VS_Intro,VS_WallToSwing });
	void Venom::UpdateLookTo()
	{
		if (noUpdateLookToStates.contains(vsm.currentState)) return;

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
	void Venom::CharacterMoveXZPlane(XMVECTOR stickDisplacement, float dt, float sideSpeed, XMFLOAT3 gravity)
	{
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
	}

	void Venom::CharacterMoveXYPlane(XMVECTOR stickDisplacement, float dt, float sideSpeed)
	{
		XMVECTOR move = stickDisplacement * sideSpeed * dt;
		//PrintXMVector(move, "move");
		PxControllerCollisionFlags colFlag = physicObject->MoveCharacter(move, dt);
	}

	void Venom::MoveForward(float sideSpeed)
	{
		CharacterMoveXZPlane(leftStick, gameUpdateFrequency, sideSpeed, physicScene->gravity());
	}

	void Venom::JumpingMoveForward(float sideSpeed)
	{
		CharacterMoveXZPlane(leftStick, gameUpdateFrequency, sideSpeed, physicScene->gravity());
	}

	void Venom::RunningJumpMoveForward(float sideSpeed)
	{
		CharacterMoveXZPlane(runningJumpLeftStick, gameUpdateFrequency, sideSpeed, physicScene->gravity());
	}

	void Venom::CrawlOnWall(float sideSpeed)
	{
		XMVECTOR stickMovement = XMVectorSwizzle(leftStick, 0, 2, 1, 3);
		if (blockedWallMovementMask() != 0)
		{
			for (auto& [flag, func] : wallMovementBlocker)
			{
				if (flag & blockedWallMovementMask())
				{
					func(stickMovement);
				}
			}
		}
		CharacterMoveXYPlane(stickMovement, gameUpdateFrequency, sideSpeed);
	}

	//Intro
	void Venom::EnterIntro()
	{
		venom->animationUseTransformation(true);
		venom->SetCurrentAnimation("Intro", 0.0f, 1.0f, true, false);
	}

	void Venom::VenomReady()
	{
		vsm.ChangeState(VS_Idle);
	}

	//Idle
	bool Venom::ShouldIdle()
	{
		XMVECTOR len = XMVector3Length(leftStick);
		float l = len.m128_f32[0];
		return l < walkThreshold();
	}

	void Venom::EnterIdle()
	{
		canJump = true;
		venom->animationUseTransformation(false);
		venom->SetCurrentAnimation("Idle", 0.0f, 1.0f, true, true);
	}

	void Venom::Idle()
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
		else if (ShouldGrabWall())
		{
			vsm.ChangeState(VS_GrabWall);
		}
	}

	//Walking
	bool Venom::ShouldWalk()
	{
		XMVECTOR len = XMVector3Length(leftStick);
		float l = len.m128_f32[0];
		return l > walkThreshold() && l < runThreshold();
	}

	void Venom::EnterWalking()
	{
		venom->SetCurrentAnimation("Walk", 0.0f, 1.0f, true, true);
	}

	void Venom::Walking()
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
	bool Venom::ShouldRun()
	{
		XMVECTOR len = XMVector3Length(leftStick);
		float l = len.m128_f32[0];
		return l > runThreshold();
	}

	void Venom::EnterRunning()
	{
		venom->SetCurrentAnimation("Run", 0.0f, 1.0f, true, true);
	}

	void Venom::Running()
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
	bool Venom::ShouldJump()
	{
		return (buttons.a == GamePad::ButtonStateTracker::PRESSED) && touchingDown && canJump && !jumping;
	}

	void Venom::EnterJumping()
	{
		venom->animationUseTransformation(true);
		venom->SetCurrentAnimation("JumpBegin");
	}

	void Venom::VenomBeginJump()
	{
		venom->SetCurrentAnimation("JumpLoop", 0.0f, 1.0f, true, true);
		jumping = true;
		canJump = false;
		downSpeed += jumpSpeed();
		touchingDown = false;
	}

	void Venom::Jumping()
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
	void Venom::EnterRunningJump()
	{
		runningJumpLeftStick = leftStick;
		venom->animationUseTransformation(true);
		venom->SetCurrentAnimation("RunJumpBegin");
	}

	void Venom::VenomBeginRunJump()
	{
		venom->SetCurrentAnimation("RunJumpLoop");
		runningJumpTimeLeft = runningJumpTime();
	}

	void Venom::RunningJump()
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

	void Venom::VenomRunJumpLanding()
	{
		vsm.ChangeState(VS_Running);
	}

	void Venom::VenomEndJumpLanding()
	{
		vsm.ChangeState(VS_Idle);
	}

	//Attack1
	bool Venom::ShouldAttackX()
	{
		return (buttons.x == GamePad::ButtonStateTracker::PRESSED);
	}

	void Venom::EnterAttack1()
	{
		auto animation = Attack1Animations.at(currentAttack1Animation);
		venom->SetCurrentAnimation(animation);
		currentAttack1Animation = (currentAttack1Animation + 1) % Attack1Animations.size();
	}

	void Venom::Attacking1()
	{
		if (!newAttack1 && attack1Window && ShouldAttackX())
		{
			newAttack1 = true;
		}
	}

	void Venom::StartVenomNextPunchWindow()
	{
		attack1Window = true;
	}

	void Venom::EvaluateVenomNextPunch()
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

	void Venom::LeaveAttack1()
	{
		attack1Window = false;
		newAttack1 = false;
		currentAttack1Animation = 0;
	}

	void Venom::PlayPunchSound(int punchIdx, int enemyHealth)
	{
		SoundFXID sfx = SoundFXID(unit, enemyHealth > 0 ? punchSounds().at(punchIdx) : punchDeathSound().at(0));
		sfx->Stop();
		sfx->Play();
	}

	//JuumpKick
	bool Venom::ShouldJumpKick()
	{
		return jumping && (buttons.x == GamePad::ButtonStateTracker::PRESSED);
	}

	void Venom::EnterJumpKick()
	{
		venom->animationUseTransformation(true);
		venom->SetCurrentAnimation("JumpKick");
	}

	void Venom::JumpKick()
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
	bool Venom::ShouldJumpDash()
	{
		return (buttons.x == GamePad::ButtonStateTracker::PRESSED);
	}

	void Venom::EnterJumpDash()
	{
		std::srand(static_cast<int>(std::time(0)));
		jumpDashTimeLeft = jumpDashTime();
		jumpDashAnimationIdx = std::rand() % DashAnimations.size();
		std::string dashAnim = DashAnimations.at(jumpDashAnimationIdx);
		venom->animationUseTransformation(true);
		venom->SetCurrentAnimation(dashAnim);
	}

	void Venom::JumpDash()
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

	//GrabWall
	bool Venom::ShouldGrabWall()
	{
		return (buttons.b == GamePad::ButtonStateTracker::PRESSED && canAttachToWall());
	}

	void Venom::EnterGrabWall()
	{
		venom->animationUseTransformation(true);
		venom->SetCurrentAnimation("FloorToWall");
		GetBrawlerCamera()->followY(true);
	}

	//WallIdle
	bool Venom::ShouldDetachFromWall()
	{
		return buttons.b == GamePad::ButtonStateTracker::PRESSED;
	}

	void Venom::EnterWallIdle()
	{
		venom->SetCurrentAnimation("WallIdle", 0.0f, 1.0f, true, true);
	}

	void Venom::WallIdle()
	{
		if (ShouldCrawlOnWall())
		{
			vsm.ChangeState(VS_CrawlOnWall);
		}
		else if (ShouldDetachFromWall())
		{
			vsm.ChangeState(VS_DetachFromWall);
		}
		else if (ShouldWallToSwing())
		{
			vsm.ChangeState(VS_WallToSwing);
		}
	}

	//WallMove
	bool Venom::ShouldCrawlOnWall()
	{
		XMVECTOR len = XMVector3Length(leftStick);
		float l = len.m128_f32[0];
		return l > wallMoveThreshold();
	}

	void Venom::EnterCrawlOnWall()
	{
		venom->SetCurrentAnimation("WallCrawl", 0.0f, 1.0f, true, true);
	}

	void Venom::LeaveCrawlOnWall()
	{
		venom->animationTimeFactor(1.0f);
	}

	void Venom::CrawlOnWall()
	{
		if (!ShouldCrawlOnWall())
		{
			vsm.ChangeState(VS_WallIdle);
			return;
		}
		else if (ShouldDetachFromWall())
		{
			vsm.ChangeState(VS_DetachFromWall);
			return;
		}
		else if (ShouldWallToSwing())
		{
			vsm.ChangeState(VS_WallToSwing);
			return;
		}

		AdjustCrawlAnimationTimeFactor();
		CrawlOnWall(wallMoveSpeed());
	}

	void Venom::AdjustCrawlAnimationTimeFactor()
	{
		XMVECTOR len = XMVector3Length(leftStick);
		float l = std::clamp(len.m128_f32[0], 0.0f, 1.0f);
		float tf = std::lerp(wallMoveMinTimeFactor(), wallMoveMaxTimeFactor(), l);
		venom->animationTimeFactor(tf);
	}

	//DetachFromWall
	void Venom::EnterDetachFromWall()
	{
		venom->SetCurrentAnimation("DetachFromWall");
	}

	//Fall
	void Venom::EnterFalling()
	{
		venom->SetCurrentAnimation("JumpLoop", 0.0f, 1.0f, true, true);
		jumping = true;
		canJump = false;
		downSpeed = 0.0f;
		touchingDown = false;
	}

	void Venom::Falling()
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

	//Death
	bool Venom::ShouldDie()
	{
		return health() <= 0 && vsm.currentState != VS_Death;
	}

	void Venom::EnterDeath()
	{
		venom->SetCurrentAnimation("Death", 0.0f, deathTimeFactor());
	}

	void Venom::OnDeathAnimationEnd()
	{
	}

	static const std::set<VenomStates> fromWallSwingStates({ VS_WallIdle,VS_CrawlOnWall });
	bool Venom::ShouldWallToSwing()
	{
		return fromWallSwingStates.contains(vsm.currentState) && buttons.rightShoulder == GamePad::ButtonStateTracker::PRESSED;
	}
	void Venom::EnterWallToSwing()
	{
		venom->SetCurrentAnimation("WallToSwing");
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

//JumpToWallFloor
//FloorToWall -> Turn_L90_C -> SwingingStart_To_LowCrawl
//Onwall_Idle
//Onwall_To_Jump
//Move_To_LowCrawl_F
//Jump_To_LowCrawl_F 
//DetachFromWall: LowCrawl_To_Onwall     ->  Onwall_To_Jump
//				(mirar muro a camara)  -> backflip to fall
//LowCrawl_To_Jump
//103501_Wallrun_F
//103501_LowCrawl_Move
//103501_LowCrawl_Idle_L

//LowCrawl_To_Onwall
//Onwall_To_Spawn_Web
//A5*
//A4*

//subir con red
//A5_4_L o A4_4_R