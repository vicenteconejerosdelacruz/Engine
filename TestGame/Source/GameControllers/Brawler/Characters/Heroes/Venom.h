#pragma once
#include "Hero.h"
#include <GameStateMachine.h>
#include <UUID.h>

namespace Game
{
	namespace Brawler
	{
		struct BrawlerCamera;

		enum VenomStates
		{
			VS_None,
			VS_Intro,
			VS_Idle,
			VS_Walking,
			VS_Running,
			VS_Jumping,
			VS_RunningJump,
			VS_Attack_1,
			VS_JumpKick,
			VS_JumpDash,
			VS_GrabWall,
			VS_WallIdle,
			VS_CrawlOnWall,
			VS_DetachFromWall,
			VS_Falling,
			VS_Death,
			VS_WallToSwing,
			VS_Swing,
		};

		inline static std::unordered_map<std::string, VenomStates> stringToVenomStates =
		{
			{ "None", VS_None },
			{ "Intro", VS_Intro },
			{ "Idle", VS_Idle },
			{ "Walking", VS_Walking },
			{ "Running", VS_Running },
			{ "Jumping", VS_Jumping },
			{ "RunningJump", VS_RunningJump },
			{ "Attack_1", VS_Attack_1 },
			{ "JumpKick", VS_JumpKick },
			{ "JumpDash", VS_JumpDash },
			{ "GrabWall", VS_GrabWall },
			{ "WallIdle", VS_WallIdle },
			{ "CrawlOnWall", VS_CrawlOnWall },
			{ "DetachFromWall", VS_DetachFromWall },
			{ "Falling", VS_Falling },
			{ "Death", VS_Death },
			{ "WallToSwing", VS_WallToSwing },
			{ "Swing", VS_Swing },
		};

		enum WallMovementAxis
		{
			WMA_Up = 1 << 0,
			WMA_Down = 1 << 1,
			WMA_Left = 1 << 2,
			WMA_Right = 1 << 3,
			WMA_All = 0b1111
		};

		inline static std::unordered_map<WallMovementAxis, std::string> WallMovementAxisToString =
		{
			{ WMA_Up, "Up" },
			{ WMA_Down, "Down" },
			{ WMA_Left, "Left" },
			{ WMA_Right, "Right" },
		};

		inline static std::unordered_map<std::string, WallMovementAxis> StringToWallMovementAxis =
		{
			{ "Up", WMA_Up },
			{ "Down", WMA_Down },
			{ "Left", WMA_Left },
			{ "Right", WMA_Right },
		};

		extern std::vector<std::string> GetBlockedWallMovementMasks();

#if defined(_EDITOR)
#include <Attributes/JOrder.h>
#include "VenomAtt.h"
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include "VenomAtt.h"
#include <JEnd.h>
#endif
		struct Venom : Hero
		{
#include <Attributes/JFlags.h>
#include "VenomAtt.h"
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include "VenomAtt.h"
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include "VenomAtt.h"
#include <JEnd.h>

			DEF_STRING2FLAGS_FUNC(Venom, Hero);

			static inline std::vector<std::string> Attack1Animations =
			{
				"Punch1", "Punch1",
				"Punch1", "Punch1",
				"Punch2",
			};

			static inline std::vector<std::string> DashAnimations =
			{
				"JumpDash1","JumpDash2"
			};

			static inline std::vector<std::string> DashLandingAnimations =
			{
				"JumpDash1Landing","RunDash2Landing"
			};

			static inline std::map<WallMovementAxis, std::function<void(XMVECTOR& v)>> wallMovementBlocker =
			{
				{ WMA_Up, [](auto& v) { v.m128_f32[1] = std::min(v.m128_f32[1],0.0f); }},
				{ WMA_Down, [](auto& v) { v.m128_f32[1] = std::max(v.m128_f32[1],0.0f); } },
				{ WMA_Left, [](auto& v) { v.m128_f32[0] = std::max(v.m128_f32[0],0.0f); } },
				{ WMA_Right, [](auto& v) { v.m128_f32[0] = std::min(v.m128_f32[0],0.0f); } },
			};

			//BrawlerCamera* GetBrawlerCamera();
			VenomStates GetState();

			//Constructor and Binding
			Venom(nlohmann::json& json);
			static void RegisterScript(Isolate* isolate, Local<ObjectTemplate> tpl, SceneUnitScripting* script);
			void SetInitialConditions() override;
#if defined(_EDITOR)
			void WriteJson(nlohmann::json& j) override;
			DECL_CONTROLLER_DRAWER(Venom, Hero);
#endif
			void Map(SUUUID so) override;
			void Unmap() override;

			//States
			void TakeHit(JUUID enemyController, int damage);

			//Step
			void Step(float delta) override;

			//Physics
			void OnStaticContactEvent(JUUID physicObject, unsigned int event);
			void OnCharacterHitEvent(PxFilterData fd);

			//Joystick
			void UpdateLeftStickVector();
			void UpdateLookTo();

			//Movement
			void CharacterMoveXZPlane(XMVECTOR stickDisplacement, float dt, float sideSpeed, XMFLOAT3 gravity);
			void CharacterMoveXYPlane(XMVECTOR stickDisplacement, float dt, float sideSpeed);
			void MoveForward(float sideSpeed);
			void JumpingMoveForward(float sideSpeed);
			void RunningJumpMoveForward(float sideSpeed);
			void CrawlOnWall(float sideSpeed);
			//bool AttachedToWall();

			//Web
			void UpdateWeb(XMVECTOR bonePos, XMVECTOR fixedPoint, XMFLOAT3 scale);

			//Intro
			void EnterIntro();
			void VenomReady();

			//Idle
			bool ShouldIdle();
			void EnterIdle();
			void Idle();

			//Walking
			bool ShouldWalk();
			void EnterWalking();
			void Walking();

			//Running
			bool ShouldRun();
			void EnterRunning();
			void Running();

			//Jumping
			bool ShouldJump();
			void EnterJumping();
			void VenomBeginJump();
			void Jumping();

			//RunningJump
			void EnterRunningJump();
			void VenomBeginRunJump();
			void RunningJump();
			void VenomRunJumpLanding();
			void VenomEndJumpLanding();

			//Attack1
			bool ShouldAttackX();
			void EnterAttack1();
			void Attacking1();
			void StartVenomNextPunchWindow();
			void EvaluateVenomNextPunch();
			void LeaveAttack1();
			void PlayPunchSound(int punchIdx, int enemyHealth);

			//JuumpKick
			bool ShouldJumpKick();
			void EnterJumpKick();
			void JumpKick();

			//JumpDash
			bool ShouldJumpDash();
			void EnterJumpDash();
			void JumpDash();

			//
			bool ShouldGrabWall();
			void EnterGrabWall();

			//WallIdle
			bool ShouldDetachFromWall();
			void EnterWallIdle();
			void WallIdle();

			//WallMove
			bool ShouldCrawlOnWall();
			void EnterCrawlOnWall();
			void LeaveCrawlOnWall();
			void CrawlOnWall();
			void AdjustCrawlAnimationTimeFactor();

			//DetachFromWall
			void EnterDetachFromWall();

			//Fall
			void EnterFalling();
			void Falling();

			//Death
			bool ShouldDie();
			void EnterDeath();
			void OnDeathAnimationEnd();

			//WallToSwing
			bool ShouldWallToSwing();
			void EnterWallToSwing();
			void WallToSwing();
			void LeaveWallToSwing();
			void ThrowWeb();

			//Swing
			void EnterSwing();
			void Swing();
			void LeaveSwing();
			bool ShouldFallFromSwing();
			bool ShouldContinueSwinging();

			//State machine
			GameStatesMachine<VenomStates> vsm;

			//Initial States
			XMFLOAT3 venomScale;
			int initialHealth;

			//SceneObjects
			//RenderableID venom;
			RenderableID web;
			XMVECTOR posDelta;

			//CameraID camera;
			PhysicSceneID physicScene;
			PhysicObjectID physicObject;

			//Joystick
			XMVECTOR leftStick;
			XMVECTOR runningJumpLeftStick;

			//Attack controllers
			bool attack1Window = false;
			bool newAttack1 = false;
			int currentAttack1Animation = 0;

			//Jumping
			float downSpeed = 0.0f;
			bool touchingDown = true;
			bool canJump = false;
			bool jumping = false;

			//RunningJump
			float runningJumpTimeLeft = 0.0f;

			//JumpDash
			int jumpDashAnimationIdx;
			float jumpDashTimeLeft = 0.0f;

			//WallToSwing
			std::unique_ptr<tween> webTweens[3];
			bool webTweenCreated = false;
			XMVECTOR webAttachedPos;

			//Swing
			std::unique_ptr<tween> swingTimeTween;
			float distanceToSwing;
			bool continueSwinging;
		};
	};
};
