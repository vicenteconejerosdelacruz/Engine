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
extern GameInteractionMode gameInteractionMode;

namespace Game::Brawler
{
	std::vector<std::string> GetBlockedWallMovementMasks()
	{
		return nostd::GetValuesFromFlagsMap(WallMovementAxisToString);
	}

#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include "VenomAtt.h"
#include <JEnd.h>
#endif

	VenomStates Venom::GetState()
	{
		return vsm.currentState;
	}

	//Constructor and Binding
	Venom::Venom(nlohmann::json& json) : Hero(json)
	{
#include <Attributes/JInit.h>
#include "VenomAtt.h"
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include "VenomAtt.h"
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include "VenomAtt.h"
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
				{ VS_Swing, [&](auto* sm, VenomStates prevState) { EnterSwing(); }},
			},
			.onLeave = {
				{ VS_Attack_1,[&](auto* sm, VenomStates prevState) { LeaveAttack1(); }},
				{ VS_CrawlOnWall, [&](auto* sm, VenomStates prevState) { LeaveCrawlOnWall(); }},
				{ VS_Swing, [&](auto* sm, VenomStates prevState) { LeaveSwing(); }},
				{ VS_WallToSwing, [&](auto* sm, VenomStates prevState) { LeaveWallToSwing(); }},
			},
			.onStep = {
				{ VS_None, [&](auto* sm) { venomScale = renderable->scale(); vsm.ChangeState(VS_Intro); }},
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
				{ VS_WallToSwing, [&](auto* sm) { WallToSwing(); }},
				{ VS_Swing, [&](auto* sm) { Swing(); }},
			}
		};
		initialHealth = health();
		SetInitialConditions();
	}

	void Venom::RegisterScript(Isolate* isolate, Local<ObjectTemplate> tpl, SceneUnitScripting* script)
	{
		v8_register_method<Venom>(isolate, tpl, "PlayerReady", script, [](Venom* self) { if (self) self->VenomReady(); });
		v8_register_method<Venom>(isolate, tpl, "StartNextPunchWindow", script, [](Venom* self) { if (self) self->StartVenomNextPunchWindow(); });
		v8_register_method<Venom>(isolate, tpl, "EvaluateNextPunch", script, [](Venom* self) { if (self) self->EvaluateVenomNextPunch(); });
		v8_register_method<Venom>(isolate, tpl, "SwitchToState", script, [](Venom* self, std::string state) { if (self) self->vsm.ChangeState(stringToVenomStates.at(state)); });
		v8_register_method<Venom>(isolate, tpl, "BeginJump", script, [](Venom* self) { if (self) self->VenomBeginJump(); });
		v8_register_method<Venom>(isolate, tpl, "EndJumpLanding", script, [](Venom* self) { if (self) self->VenomEndJumpLanding(); });
		v8_register_method<Venom>(isolate, tpl, "BeginRunJump", script, [](Venom* self) { if (self) self->VenomBeginRunJump(); });
		v8_register_method<Venom>(isolate, tpl, "RunJumpLanding", script, [](Venom* self) { if (self) self->VenomRunJumpLanding(); });
		v8_register_method<Venom>(isolate, tpl, "TakeHit", script, [](Venom* self, JUUID enemyController, int damage) { if (self) self->TakeHit(enemyController, damage); });
		v8_register_method<Venom>(isolate, tpl, "OnDeathAnimationEnd", script, [](Venom* self) { if (self) self->OnDeathAnimationEnd(); });
		v8_register_method<Venom>(isolate, tpl, "PlayPunchSound", script, [](Venom* self, int punchIdx, int enemyHealth) { if (self) self->PlayPunchSound(punchIdx, enemyHealth); });
		v8_register_method<Venom>(isolate, tpl, "ThrowWeb", script, [](Venom* self) { if (self) self->ThrowWeb(); });
	}

	void Venom::SetInitialConditions()
	{
		Hero::SetInitialConditions();
		health(initialHealth);
		blockedWallMovementMask(0U);
		vsm.currentState = VS_None;
		venomScale = { 0.0f,0.0f,0.0f };
		posDelta = XMVectorZero();
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
		for (unsigned int i = 0; i < 3; i++)
		{
			webTweens[i] = nullptr;
		}
		webTweenCreated = false;
		swingTimeTween = nullptr;
		continueSwinging = false;
	}

#if defined(_EDITOR)
	void Venom::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include "VenomAtt.h"
#include <JEnd.h>
		Hero::WriteJson(j);
	}
#endif

	void Venom::Map(SUUUID so)
	{
		using namespace Scene;
		Hero::Map(so);
		SceneObjectType type = GetSceneObjectType(so);

		if (type == SO_Renderables)
		{
			renderable = so;
		}
		GetController<BrawlerScene>(unit, sceneController())->RegisterHero(uuid());
		physicScene = MAKESUUUID(unit, *GetPhysicScenes(unit).begin());
		if (renderable->physicObject().size() > 0ULL)
		{
			physicObject = renderable->at("physicObject").at(0);
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
		}
		SetInitialConditions();
	}

	void Venom::Unmap()
	{
		Hero::Unmap();
		UnregisterContactCallback(PB_Static, physicObject());
		UnregisterCharacterHitCallback(physicObject());
		physicScene.clear();
		physicObject.clear();
	}

	void Venom::TakeHit(JUUID enemyController, int damage)
	{
		//health(std::max(0, health() - damage));
		GetController<BrawlerScene>(unit, sceneController())->HeroTookHit(enemyController, health());
	}

	//Step
	void Venom::Step(float delta)
	{
#if defined(_EDITOR)
		if (!Editor::IsPlaying(unit) || Editor::IsPaused(unit))
			return;
#endif

		bool dialogOpen = GetController<BrawlerScene>(unit, sceneController())->IsDialogOpen();

		float dt = static_cast<float>(timer.GetElapsedSeconds());

		if (gameInteractionMode == GIM_Joystick)
		{
			auto state = gamePad->GetState(0);
			if (state.IsConnected() && !dialogOpen)
			{
				buttons.Update(state);
			}
			else
			{
				buttons.Reset();
			}
		}

		posDelta = XMVectorZero();
		if (vsm.currentState != VS_Death)
		{
			XMVECTOR XMpos, XMrot, XMscl;
			XMMatrixDecompose(&XMscl, &XMrot, &XMpos, renderable->animationTransformation);

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
		/*
		BrawlerCamera* brawlerCam = GetBrawlerCamera();

		if ((CM_Floor & fd.word0) && brawlerCam->followY())
		{
			brawlerCam->followY(false);
		}*/
	}

	//Joystick
	static const std::set<VenomStates> noUpdateStates({ VS_RunningJump, VS_JumpDash });
	void Venom::UpdateLeftStickVector()
	{
		if (noUpdateStates.contains(vsm.currentState)) return; //maybe Vec0?
		bool dialogOpen = GetController<BrawlerScene>(unit, sceneController())->IsDialogOpen();

		if (dialogOpen)
		{
			leftStick = XMVectorZero();
			return;
		}

		if (gameInteractionMode == GIM_Joystick)
		{
			auto pad = gamePad->GetState(0);
			if (pad.IsConnected())
			{
				leftStick = { pad.thumbSticks.leftX, 0.0f, pad.thumbSticks.leftY, 0.0f };
			}
		}
		else if (gameInteractionMode == GIM_PC)
		{
			auto keys = keyboard->GetState();
			float yaxis = (keys.IsKeyDown(Keyboard::Keys::Down) ^ keys.IsKeyDown(Keyboard::Keys::Up)) ? (keys.IsKeyDown(Keyboard::Keys::Down) ? -1.0f : 1.0f) : 0.0f;
			float xaxis = (keys.IsKeyDown(Keyboard::Keys::Left) ^ keys.IsKeyDown(Keyboard::Keys::Right)) ? (keys.IsKeyDown(Keyboard::Keys::Left) ? -1.0f : 1.0f) : 0.0f;
			leftStick = { xaxis, 0.0f, yaxis, 0.0f };
		}
	}

	static const std::set<VenomStates> noUpdateLookToStates({ VS_Intro,VS_WallToSwing, VS_Swing });
	void Venom::UpdateLookTo()
	{
		bool dialogOpen = GetController<BrawlerScene>(unit, sceneController())->IsDialogOpen();
		if (noUpdateLookToStates.contains(vsm.currentState) || dialogOpen) return;

		float len = leftStick.m128_f32[0];

		if (fabsf(len) < lookToThreshold()) return;

		if (len < 0.0f)
		{
			XMFLOAT3 scale = renderable->scale();
			if (scale.z > 0.0f)
			{
				scale.z = -venomScale.z;
				renderable->scale(scale);
				lookingTo(CLT_Left);
			}
		}
		else if (len > 0.0f)
		{
			XMFLOAT3 scale = renderable->scale();
			if (scale.z < 0.0f)
			{
				scale.z = venomScale.z;
				renderable->scale(scale);
				lookingTo(CLT_Right);
			}
		}
	}

	//Movement
	void Venom::CharacterMoveXZPlane(XMVECTOR stickDisplacement, float dt, float sideSpeed, XMFLOAT3 gravity)
	{
		PxExtendedVec3 posBefore = physicObject->controller->getPosition();

		XMVECTOR downDisp = { 0.0f, fixedDownDisplacement() + downSpeed * dt, 0.0f };
		XMVECTOR move = XMVector3Normalize(stickDisplacement) * sideSpeed * dt;
		move += downDisp;

		PxControllerCollisionFlags colFlag = physicObject->MoveCharacter(move, dt);

		PxExtendedVec3 posAfter = physicObject->controller->getPosition();

		posDelta = {
			(float)(posAfter.x - posBefore.x),
			(float)(posAfter.y - posBefore.y),
			(float)(posAfter.z - posBefore.z)
		};

		touchingDown = !!(colFlag & PxControllerCollisionFlag::Enum::eCOLLISION_DOWN);
		if (!!(colFlag & PxControllerCollisionFlag::Enum::eCOLLISION_UP))
		{
			downSpeed = 0.0f;
		}
		downSpeed = (touchingDown) ? 0.0f : (downSpeed + gravity.y * dt);
	}

	void Venom::CharacterMoveXYPlane(XMVECTOR stickDisplacement, float dt, float sideSpeed)
	{
		PxExtendedVec3 posBefore = physicObject->controller->getPosition();

		XMVECTOR move = stickDisplacement * sideSpeed * dt;
		PxControllerCollisionFlags colFlag = physicObject->MoveCharacter(move, dt);

		PxExtendedVec3 posAfter = physicObject->controller->getPosition();

		posDelta = {
			(float)(posAfter.x - posBefore.x),
			(float)(posAfter.y - posBefore.y),
			(float)(posAfter.z - posBefore.z)
		};
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
		renderable->animationUseTransformation(true);
		renderable->SetCurrentAnimation("Intro", 0.0f, 1.0f, true, false);
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
		renderable->animationUseTransformation(false);
		renderable->SetCurrentAnimation("Idle", 0.0f, 1.0f, true, true);
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
		renderable->SetCurrentAnimation("Walk", 0.0f, 1.0f, true, true);
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
		renderable->SetCurrentAnimation("Run", 0.0f, 1.0f, true, true);
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
		if (!touchingDown || !canJump || jumping)
			return false;
		if (GetController<BrawlerScene>(unit, sceneController())->IsDialogOpen())
		{
			return false;
		}
		if (gameInteractionMode == GIM_Joystick)
		{
			return (buttons.a == GamePad::ButtonStateTracker::PRESSED);
		}
		else if (gameInteractionMode == GIM_PC)
		{
			return keyboard->GetState().IsKeyDown(Keyboard::Keys::Space);
		}

		return false;
	}

	void Venom::EnterJumping()
	{
		renderable->animationUseTransformation(true);
		renderable->SetCurrentAnimation("JumpBegin");
	}

	void Venom::VenomBeginJump()
	{
		renderable->SetCurrentAnimation("JumpLoop", 0.0f, 1.0f, true, true);
		jumping = true;
		canJump = false;
		downSpeed += jumpSpeed();
		touchingDown = false;
		//GetBrawlerCamera()->followY(canAttachToWall());
	}

	void Venom::Jumping()
	{
		if (ShouldJumpKick())
		{
			vsm.ChangeState(VS_JumpKick);
			return;
		}
		else if (ShouldGrabWall())
		{
			vsm.ChangeState(VS_WallIdle);
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
		renderable->animationUseTransformation(true);
		renderable->SetCurrentAnimation("RunJumpBegin");
	}

	void Venom::VenomBeginRunJump()
	{
		renderable->SetCurrentAnimation("RunJumpLoop");
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
				renderable->SetCurrentAnimation("RunJumpLanding");
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
		if (GetController<BrawlerScene>(unit, sceneController())->IsDialogOpen())
		{
			return false;
		}

		if (gameInteractionMode == GIM_Joystick)
		{
			return (buttons.x == GamePad::ButtonStateTracker::PRESSED);
		}
		else if (gameInteractionMode == GIM_PC)
		{
			return keyboard->GetState().IsKeyDown(Keyboard::Keys::F);
		}
		return false;
	}

	void Venom::EnterAttack1()
	{
		auto animation = Attack1Animations.at(currentAttack1Animation);
		renderable->SetCurrentAnimation(animation);
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
		if (GetController<BrawlerScene>(unit, sceneController())->IsDialogOpen())
		{
			return false;
		}
		if (!jumping) return false;

		if (gameInteractionMode == GIM_Joystick)
		{
			return (buttons.x == GamePad::ButtonStateTracker::PRESSED);
		}
		else if (gameInteractionMode == GIM_PC)
		{
			return keyboard->GetState().IsKeyDown(Keyboard::Keys::F);
		}
		return false;
	}

	void Venom::EnterJumpKick()
	{
		renderable->animationUseTransformation(true);
		renderable->SetCurrentAnimation("JumpKick");
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
		if (GetController<BrawlerScene>(unit, sceneController())->IsDialogOpen())
		{
			return false;
		}
		if (gameInteractionMode == GIM_Joystick)
		{
			return (buttons.x == GamePad::ButtonStateTracker::PRESSED);
		}
		else if (gameInteractionMode == GIM_PC)
		{
			return keyboard->GetState().IsKeyDown(Keyboard::Keys::F);
		}
		return false;
	}

	void Venom::EnterJumpDash()
	{
		std::srand(static_cast<int>(std::time(0)));
		jumpDashTimeLeft = jumpDashTime();
		jumpDashAnimationIdx = std::rand() % DashAnimations.size();
		std::string dashAnim = DashAnimations.at(jumpDashAnimationIdx);
		renderable->animationUseTransformation(true);
		renderable->SetCurrentAnimation(dashAnim);
	}

	void Venom::JumpDash()
	{
		if (jumpDashTimeLeft != 0.0f)
		{
			float dt = static_cast<float>(timer.GetElapsedSeconds());
			jumpDashTimeLeft = std::max(jumpDashTimeLeft - dt, 0.0f);
			if (jumpDashTimeLeft == 0.0f)
			{
				renderable->SetCurrentAnimation(DashLandingAnimations.at(jumpDashAnimationIdx));
			}
		}
		RunningJumpMoveForward(runSpeed());
	}

	//GrabWall
	bool Venom::ShouldGrabWall()
	{
		if (GetController<BrawlerScene>(unit, sceneController())->IsDialogOpen())
		{
			return false;
		}
		if (!canAttachToWall())
			return false;

		if (gameInteractionMode == GIM_Joystick)
		{
			return (buttons.b == GamePad::ButtonStateTracker::PRESSED);
		}
		else if (gameInteractionMode == GIM_PC)
		{
			return keyboard->GetState().IsKeyDown(Keyboard::Keys::T);
		}
		return false;
	}

	void Venom::EnterGrabWall()
	{
		renderable->animationUseTransformation(true);
		renderable->SetCurrentAnimation("FloorToWall");
		//GetBrawlerCamera()->followY(true);
	}

	//WallIdle
	bool Venom::ShouldDetachFromWall()
	{
		if (GetController<BrawlerScene>(unit, sceneController())->IsDialogOpen())
		{
			return false;
		}
		if (gameInteractionMode == GIM_Joystick)
		{
			return (buttons.b == GamePad::ButtonStateTracker::PRESSED);
		}
		else if (gameInteractionMode == GIM_PC)
		{
			return keyboard->GetState().IsKeyDown(Keyboard::Keys::T);
		}
		return false;
	}

	void Venom::EnterWallIdle()
	{
		renderable->SetCurrentAnimation("WallIdle", 0.0f, 1.0f, true, true);
		//GetBrawlerCamera()->followY(true);
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
		renderable->SetCurrentAnimation("WallCrawl", 0.0f, 1.0f, true, true);
	}

	void Venom::LeaveCrawlOnWall()
	{
		renderable->animationTimeFactor(1.0f);
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
		renderable->animationTimeFactor(tf);
	}

	//DetachFromWall
	void Venom::EnterDetachFromWall()
	{
		renderable->SetCurrentAnimation("DetachFromWall");
	}

	//Fall
	void Venom::EnterFalling()
	{
		renderable->SetCurrentAnimation("JumpLoop", 0.0f, 1.0f, true, true);
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
		else if (ShouldGrabWall())
		{
			vsm.ChangeState(VS_WallIdle);
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
		renderable->SetCurrentAnimation("Death", 0.0f, deathTimeFactor());
	}

	void Venom::OnDeathAnimationEnd()
	{}

	static const std::set<VenomStates> fromWallSwingStates({ VS_WallIdle,VS_CrawlOnWall });
	bool Venom::ShouldWallToSwing()
	{
		if (GetController<BrawlerScene>(unit, sceneController())->IsDialogOpen())
		{
			return false;
		}
		if (!fromWallSwingStates.contains(vsm.currentState))
			return false;

		if (gameInteractionMode == GIM_Joystick)
		{
			return (buttons.rightShoulder == GamePad::ButtonStateTracker::PRESSED);
		}
		else if (gameInteractionMode == GIM_PC)
		{
			return keyboard->GetState().IsKeyDown(Keyboard::Keys::E);
		}
		return false;
	}

	void Venom::EnterWallToSwing()
	{
		renderable->SetCurrentAnimation("WallToSwing");
	}

	void Venom::UpdateWeb(XMVECTOR bonePos, XMVECTOR fixedPoint, XMFLOAT3 scale)
	{
		XMVECTOR dir = XMVectorScale(XMVectorSubtract(fixedPoint, bonePos), scale.y);
		XMVECTOR distVec = XMVector3Length(dir);
		float L;
		XMStoreFloat(&L, distVec);

		XMVECTOR dirNormal = XMVector3Normalize(dir);

		XMVECTOR focus = dirNormal;
		XMVECTOR upAux = XMVectorSet(0, 0, 1, 0);
		if (abs(XMVectorGetY(dirNormal)) > 0.99f) upAux = XMVectorSet(1, 0, 0, 0);

		XMMATRIX lookAt = XMMatrixLookToLH(XMVectorZero(), focus, upAux);

		XMMATRIX rotM = XMMatrixTranspose(lookAt);
		rotM = XMMatrixRotationX(XM_PIDIV2) * rotM;
		XMVECTOR rotQ = XMQuaternionRotationMatrix(rotM);

		XMVECTOR posV = bonePos + XMVectorScale(dir, webScaleAdj() + 0.5f);
		XMFLOAT3 pos;
		XMStoreFloat3(&pos, posV);

		web->scale(XMFLOAT3(scale.x, L, scale.z));
		web->rotationQ(rotQ);
		web->position(pos);
	}

	void Venom::WallToSwing()
	{
		if (webTweenCreated == true && RenderableSceneObjectExist(web))
		{
			float sclx = webTweens[0]->step();
			float scly = webTweens[1]->step();
			float sclz = webTweens[2]->step();

			auto [mm, bonePos, a, b, c] = renderable->GetBoneTransformation(webBone());

			XMVECTOR bonePosV = XMLoadFloat3(&bonePos);
			UpdateWeb(bonePosV, webAttachedPos, XMFLOAT3(sclx, scly, sclz));
			if (scly == 1.0f)
			{
				vsm.ChangeState(VS_Swing);
			}
		}
	}

	void Venom::LeaveWallToSwing()
	{
		webTweenCreated = false;
	}

	void Venom::ThrowWeb()
	{
		SoundFXID sfx = SoundFXID(unit, throwWebSound().at(0));
		sfx->Stop();
		sfx->Play();

		auto [mm, pos, a, b, c] = renderable->GetBoneTransformation(webBone());

		CharacterLookingTo clt = lookingTo();
		float angle = throwWebAngle() * ((clt == CLT_Left) ? 1.0f : -1.0f);

		XMVECTOR rot = XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, XMConvertToRadians(angle));
		XMVECTOR up = { 0.0f,1.0f,0.0f,0.0f };
		webAttachedPos = XMLoadFloat3(&pos) + XMVector3Rotate(up, rot) * throwWebMaxScale().y;
		distanceToSwing = 2.0f * std::fabsf(XMVectorGetX(webAttachedPos) - pos.x);

		web = MAKESUUUID(unit, getUUID());
		Scene::CreateSceneObjectFromMold(unit, webMold(),
			[&](SceneObjectType type, nlohmann::json json, std::string name)
			{
				return nlohmann::json(
					{
						{ "name", web.uuid() + "_ins"},
						{ "uuid", web.uuid() },
						{ "position", FromXMFLOAT3(pos) },
						{ "cameras", renderable->cameras() },
						{ "scale", FromXMFLOAT3(throwWebMinScale()) },
						{ "rotation", {0.0f, 0.0f, angle } }
					}
				);
			}
		);

		webTweens[0] = std::make_unique<tween>(throwWebMinScale().x, throwWebMaxScale().x, static_cast<int>(1000.0f * throwWebTime()), tween::easing::linear);
		webTweens[1] = std::make_unique<tween>(0.0f, 1.0f, static_cast<int>(1000.0f * throwWebTime()), tween::easing::linear);
		webTweens[2] = std::make_unique<tween>(throwWebMinScale().z, throwWebMaxScale().z, static_cast<int>(1000.0f * throwWebTime()), tween::easing::linear);
		webTweenCreated = true;
	}

	//Swing
	void Venom::EnterSwing()
	{
		renderable->SetCurrentAnimation("Swing");
		swingTimeTween = std::make_unique<tween>(0.0f, 1.0f, static_cast<int>(1000.0f * swingTime()));
		continueSwinging = false;
	}

	void Venom::Swing()
	{
		float dt = -swingTimeTween->current_value;
		dt += swingTimeTween->step();

		if (ShouldContinueSwinging())
		{
			LeaveSwing(); //<-not really i just nead to clear the resources
			ThrowWeb();
			EnterSwing();
			return;
		}
		if (ShouldFallFromSwing())
		{
			vsm.ChangeState(VS_Falling);
			return;
		}

		CharacterLookingTo clt = lookingTo();
		float dxS = (clt == CLT_Left) ? -1.0f : 1.0f;
		float dx = dxS * distanceToSwing * dt;

		XMVECTOR disp = { dx ,0.0,0.0f,0.0f };
		CharacterMoveXYPlane(disp, 1.0f, 1.0f);

		//now we can update the web
		auto [mm, bonePos, a, b, c] = renderable->GetBoneTransformation(webBone());

		XMVECTOR bonePosV = XMLoadFloat3(&bonePos);
		XMFLOAT3 scl = throwWebMaxScale();
		scl.y = 1.0f;
		UpdateWeb(bonePosV, webAttachedPos, scl);

		//capture if continue swinging
		if (swingTimeTween->current_value >= swingThreshold())
		{
			if (gameInteractionMode == GIM_Joystick)
			{
				continueSwinging = (buttons.rightShoulder == GamePad::ButtonStateTracker::PRESSED);
			}
			else if (gameInteractionMode == GIM_PC)
			{
				continueSwinging = keyboard->GetState().IsKeyDown(Keyboard::Keys::E);
			}
		}
	}

	void Venom::LeaveSwing()
	{
		continueSwinging = false;
		webTweenCreated = false;
		web->markedForDelete = true;
		web.clear();
	}

	bool Venom::ShouldFallFromSwing()
	{
		if ((swingTimeTween->current_value == swingTimeTween->target_value))
			return true;

		if (gameInteractionMode == GIM_Joystick)
		{
			return (buttons.b == GamePad::ButtonStateTracker::PRESSED);
		}
		else if (gameInteractionMode == GIM_PC)
		{
			return keyboard->GetState().IsKeyDown(Keyboard::Keys::T);
		}
		return false;
	}

	bool Venom::ShouldContinueSwinging()
	{
		return continueSwinging;
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

//??
//A5_4_L -> A5_10_L->A5_11_L->A5_12_L->A5_13_L

//103551_Fire_R -> 103551_Fire_R_Loop