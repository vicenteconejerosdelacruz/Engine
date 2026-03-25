#include "pch.h"
#include "ThirdPersonController.h"
#include <GamePhysics.h>
#include <GamePad.h>
#include <Scene.h>
#if defined(_EDITOR)
#include <Editor.h>
#endif

namespace Editor
{
	extern CameraID GetLevelCamera(SceneUnitId id);
};

namespace Physics
{
	extern void RegisterContactCallback(PhysicsBehavior behavior, JUUID object, std::function<void(JUUID, unsigned int)> callback);
	extern void UnregisterContactCallback(PhysicsBehavior behavior, JUUID object);
}

extern std::unique_ptr<DirectX::GamePad> gamePad;
extern DirectX::GamePad::ButtonStateTracker buttons;

namespace Game
{
#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <ThirdPersonControllerAtt.h>
#include <JEnd.h>

#endif

	ThirdPersonController::ThirdPersonController(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include <ThirdPersonControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <ThirdPersonControllerAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void ThirdPersonController::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <ThirdPersonControllerAtt.h>
#include <JEnd.h>
		j.erase("uuid");
	}
#endif

	void ThirdPersonController::SetInitialConditions()
	{
		yaw = 0.0f;
		pitch = 0.0f;
		canJump = false;
		jumping = false;
	}

	void ThirdPersonController::Map(SUUUID so)
	{
		using namespace Scene;
		using namespace Physics;

		Controller::Map(so);

#if defined(_EDITOR)
		camera = Editor::GetLevelCamera(unit);
#else
		if (GetCountFromMouseCameras(unit) > 0ULL)
		{
			camera = MAKESUUUID(unit, *GetSwapChainCameras(unit).begin());
		}
#endif
		//OutputDebugStringA(std::string(std::string("Map Camera:") + camera->name() + "\n").c_str());

		physicScene = MAKESUUUID(unit, *GetPhysicScenes(unit).begin());
		renderable = so;
		physicObject = renderable->at("physicObject").at(0);
		RegisterContactCallback(PB_Trigger, physicObject(), [&](JUUID uuid, unsigned int event)
			{
				OnTriggerEvent(uuid, event);
			}
		);
	}

	void ThirdPersonController::Unmap()
	{
		Controller::Unmap();
		UnregisterContactCallback(PB_Trigger, physicObject());
		camera.clear();
		renderable.clear();
		physicObject.clear();
	}

	void ThirdPersonController::Step(float delta)
	{
#if defined(_EDITOR)
		if (!Editor::IsPlaying(unit) || Editor::IsPaused(unit))
			return;
#endif

		auto state = gamePad->GetState(0);
		if (state.IsConnected())
		{
			buttons.Update(state);
		}
		else
		{
			buttons.Reset();
		}

		UpdateLeftStickVector();
		UpdateRightStickVector();
		UpdateLookTo(delta);
		UpdatePosition(delta);
	}

	void ThirdPersonController::UpdateJump()
	{
		if (canJump && !jumping && buttons.a == GamePad::ButtonStateTracker::PRESSED)
		{
			downSpeed += jumpSpeed();
			jumping = true;
		}
	}

	void ThirdPersonController::UpdateLeftStickVector()
	{
		auto pad = gamePad->GetState(0);
		leftStick = pad.IsConnected() ? XMVECTOR({ pad.thumbSticks.leftX, 0.0f, pad.thumbSticks.leftY, 0.0f }) : XMVectorZero();
	}

	void ThirdPersonController::UpdateRightStickVector()
	{
		auto pad = gamePad->GetState(0);
		rightStick = pad.IsConnected() ? XMVECTOR({ pad.thumbSticks.rightX, -pad.thumbSticks.rightY, 0.0f, 0.0f }) : XMVectorZero();
	}

	void ThirdPersonController::UpdateLookTo(float delta)
	{
		pitch += rightStick.m128_f32[1] * 0.5f;
		yaw += rightStick.m128_f32[0] * 0.5f;
		pitch = std::clamp(pitch, -20.0f, 10.0f);
		XMVECTOR degs = { pitch,yaw,0.0f,0.0f };
		XMVECTOR rotQ = XMQuatFromDegrees(-pitch, +yaw, 0.0f);
		camera->rotationQ(rotQ);
		XMVECTOR fw = camera->forward();
		XMVECTOR camPos = fw * -camDistance() + renderable->positionV();
		camera->positionV(camPos);
	}

	void ThirdPersonController::UpdatePosition(float delta)
	{
		XMVECTOR fw = camera->forward();
		fw.m128_f32[1] = 0.0f;
		fw = XMVector3Normalize(fw);
		fw = XMVectorScale(fw, leftStick.m128_f32[2]);

		XMVECTOR right = camera->right();
		right.m128_f32[1] = 0.0f;
		right = XMVector3Normalize(right);
		right = XMVectorScale(right, -leftStick.m128_f32[0]);

		XMVECTOR disp = fw + right;
		disp += {0.0f, fixedDownDisplacement() + downSpeed * delta, 0.0f};
		disp *= speed();
		PxControllerCollisionFlags colFlag = physicObject->MoveCharacter(disp, delta);
		touchingDown = !!(colFlag & PxControllerCollisionFlag::Enum::eCOLLISION_DOWN);
		XMFLOAT3 gravity = physicScene->gravity();
		downSpeed = (touchingDown) ? 0.0f : (downSpeed + gravity.y * delta);
		if (touchingDown)
		{
			if (jumping)
				jumping = false;
			UpdateJump();
		}
	}

	void ThirdPersonController::OnTriggerEvent(JUUID triggerPhysicObject, unsigned int event)
	{
		PhysicObjectID physicObject = triggerPhysicObject;
		/*
		if (physicObject->trigger->collisionMask() & CF_WallGrabArea)
		{
			if (event & PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				canJump = true;
			}
			if (event & PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				canJump = false;
			}
		}
		*/
	}
}

