#pragma once
#include <Controller.h>

namespace Game
{
	namespace ThirdPerson
	{
#if defined(_EDITOR)
#include <Attributes/JOrder.h>
#include <ThirdPerson/ThirdPersonCharacterAtt.h>
#include <JEnd.h>
#include <Editor/JDrawersDecl.h>
#include <ThirdPerson/ThirdPersonCharacterAtt.h>
#include <JEnd.h>
#endif

		struct ThirdPersonCharacter : Controller
		{
#include <Attributes/JFlags.h>
#include <ThirdPerson/ThirdPersonCharacterAtt.h>
#include <JEnd.h>
#include <Attributes/JDecl.h>
#include <ThirdPerson/ThirdPersonCharacterAtt.h>
#include <JEnd.h>

			ThirdPersonCharacter(nlohmann::json& json);
			void SetInitialConditions() override;
#if defined(_EDITOR)
			void WriteJson(nlohmann::json& j) override;
			DECL_CONTROLLER_DRAWER(ThirdPersonCharacter, Controller);
#endif

			void Map(SUUUID so) override;
			void Unmap() override;
			void Step(float delta) override;

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
	};
};