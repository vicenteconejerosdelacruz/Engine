#pragma once
#include <Controller.h>
#include <GameStateMachine.h>

namespace Scene
{
	struct Renderable;
	struct Camera;
};

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
	VS_WallMove,
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
	{ "WallMove", VS_WallMove },
};

namespace Game
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <VenomControllerAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <VenomControllerAtt.h>
#include <JEnd.h>

#endif
	struct VenomController : Controller
	{
#include <Attributes/JFlags.h>
#include <VenomControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <VenomControllerAtt.h>
#include <JEnd.h>

		enum LookingTo
		{
			LT_Right,
			LT_Left,
		};

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

		//Constructor and Binding
		VenomController(nlohmann::json& json);
		virtual void SetInitialConditions();
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
		DECL_CONTROLLER_DRAWER(VenomController, Controller);
#endif
		virtual void Map(SUUUID so);
		virtual void Unmap();

		//Step
		virtual void Step(float delta);

		//JS binding
		virtual v8_templates_creators GetV8TemplatesCreators();
		virtual v8_context_creators GetV8ContextCreators();
		virtual v8_functions_creators GetV8FunctionsCreators();

		//Joystick
		void UpdateLeftStickVector();
		void UpdateLookTo();

		//Movement
		void CharacterMove(XMVECTOR stickDisplacement, float dt, float sideSpeed, XMFLOAT3 gravity);
		void MoveForward(float sideSpeed);
		void JumpingMoveForward(float sideSpeed);
		void RunningJumpMoveForward(float sideSpeed);

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
		//void LeaveJumping();

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

		//JuumpKick
		bool ShouldJumpKick();
		void EnterJumpKick();
		void JumpKick();

		//JumpDash
		bool ShouldJumpDash();
		void EnterJumpDash();
		void JumpDash();

		//GrabWall
		bool ShouldGrabWall();
		void EnterGrabWall();

		//WallIdle
		void EnterWallIdle();
		void WallIdle();

		//State machine
		GameStatesMachine<VenomStates> vsm;

		//Initial States
		XMFLOAT3 venomScale;
		LookingTo lookingTo = LT_Right;

		//SceneObjects
		RenderableID venom;
		CameraID camera;
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
	};
}
