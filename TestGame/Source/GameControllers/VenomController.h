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
	VS_Attack_1
};

inline static std::unordered_map<std::string, VenomStates> stringToVenomStates =
{
	{ "None", VS_None },
	{ "Intro", VS_Intro },
	{ "Idle", VS_Idle },
	{ "Walking", VS_Walking },
	{ "Running", VS_Running },
	{ "Jumping", VS_Jumping },
	{ "Attack_1", VS_Attack_1 }
};

namespace Game
{
	struct VenomController : Controller
	{
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
			"RunJumpAttack1","RunJumpAttack2"
		};
		static inline std::vector<std::string> DashLandingAnimations =
		{
			"RunJumpAttack1Landing","RunJumpAttack2Landing"
		};

		//Constructor and Binding
		VenomController();
		virtual void Map(JUUID so);
		virtual void Unmap();

		//Step
		virtual void Step(float delta);

		//JS binding
		virtual void BindToV8Context(v8pp::context& context);
		void VenomReady();
		void StartVenomNextPunchWindow();
		void EvaluateVenomNextPunch();
		void VenomBeginRunJump();
		void VenomRunJumpLanding();
		void VenomBeginJump();
		void VenomBeginFall();
		void VenomEndJumpLanding();

		//Scene Object
		void MoveForward(float step);
		void JumpingMoveForward(float step);
		void RunningJumpMoveForward(float step);

		//Joystick
		void UpdateLeftStickVector();
		void UpdateLookTo();

		//States handling
		//Shoulds
		bool ShouldIdle();
		bool ShouldWalk();
		bool ShouldRun();
		bool ShouldJump();
		bool ShouldAttackX();
		//Enter
		void EnterIntro();
		void EnterIdle();
		void EnterWalking();
		void EnterRunning();
		void EnterJumping();
		void EnterRunningJump();
		void EnterAttack1();
		//Steps
		void Idle();
		void Walking();
		void Running();
		void Jumping();
		void RunningJump();
		void Attacking1();
		//Leaves
		void LeaveAttack1();
		void LeaveJumping();
		void LeaveRunningJumping();

		//State machine
		GameStatesMachine<VenomStates> vsm;

		//Initial States
		XMFLOAT3 venomScale;
		LookingTo lookingTo = LT_Right;

		//SceneObjects
		RenderableUUID venom;
		CameraUUID camera;

		//Joystick
		XMVECTOR leftStick;
		XMVECTOR runningJumpLeftStick;

		//Movement encoding
		XMVECTOR lastAnimPos;
		XMVECTOR lastAnimPosDelta;
		XMVECTOR lastAnimPosDelta2;

		//Attack controllers
		bool attack1Window = false;
		bool newAttack1 = false;
		int currentAttack1Animation = 0;

		//jumping
		struct
		{
			bool jumping;
			bool falling;
			bool kicking;
			std::unique_ptr<tween> jumpTween;
			std::unique_ptr<tween> fallTween;
		} JumpingStateData;

		//runningJump
		struct
		{
			bool dash;
			int dashAnimationIdx;
			std::unique_ptr<tween> jumpTween;
		} RunningJumpStateData;
	};
}
