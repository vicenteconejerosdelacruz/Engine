#pragma once
#include <Controller.h>

namespace Game
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <ThirdPerson/ThirdPersonControllerAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <ThirdPerson/ThirdPersonControllerAtt.h>
#include <JEnd.h>

#endif

	struct ThirdPersonController : Controller
	{
#include <Attributes/JFlags.h>
#include <ThirdPerson/ThirdPersonControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <ThirdPerson/ThirdPersonControllerAtt.h>
#include <JEnd.h>

		ThirdPersonController(nlohmann::json& json);
		virtual void SetInitialConditions();
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
		DECL_CONTROLLER_DRAWER(ThirdPersonController, Controller);
#endif

		virtual void Map(SUUUID so);
		virtual void Unmap();
		virtual void Step(float delta);

		//Joystick
		void UpdateJump();
		void UpdateLeftStickVector();
		void UpdateRightStickVector();
		void UpdateLookTo(float delta);
		void UpdatePosition(float delta);

		//Interaction
		void OnTriggerEvent(JUUID triggerPhysicObject, unsigned int event);

		//object to interact with
		CameraID camera;
		RenderableID renderable;
		PhysicObjectID physicObject;
		PhysicSceneID physicScene;

		XMVECTOR leftStick;
		XMVECTOR rightStick;
		float yaw = 0.0f;
		float pitch = 0.0f;
		float downSpeed = 0.0f;
		bool touchingDown = true;
		bool canJump = false;
		bool jumping = false;
	};
}