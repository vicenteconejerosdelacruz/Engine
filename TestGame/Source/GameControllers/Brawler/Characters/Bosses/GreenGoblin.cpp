#include "pch.h"
#include "GreenGoblin.h"
#include "../../Scene/BrawlerScene.h"
#include <StepTimer.h>
#include <NoStd.h>
#include <SimpleMath.h>
#include <GamePhysics.h>
#include <Brawler/Characters/Heroes/Hero.h>
#if defined(_EDITOR)
#include <Editor.h>
#endif

//Timer
extern DX::StepTimer timer;
extern float gameUpdateFrequency;
namespace Game::Brawler
{
#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include "GreenGoblinAtt.h"
#include <JEnd.h>
#endif

	static BrawlerScene* GetBrawlerScene(GreenGoblin* gg)
	{
		return GetController<BrawlerScene>(gg->unit, gg->sceneController());
	}

	//Constructor and Binding
	GreenGoblin::GreenGoblin(nlohmann::json& json) : Thug(json)
	{
#include <Attributes/JInit.h>
#include "GreenGoblinAtt.h"
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include "GreenGoblinAtt.h"
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include "GreenGoblinAtt.h"
#include <JEnd.h>

		gggsm = {
			.currentState = GGGS_Idle,
			.onEnter = {
				{ GGGS_GlideLeft, [&](auto* sm, GreenGoblinGliderState prevState) { GGEnterGlideLeft(); }},
				{ GGGS_GlideRight, [&](auto* sm, GreenGoblinGliderState prevState) { GGEnterGlideRight(); }},
			},
			.onLeave = {
			},
			.onStep = {
				{ GGGS_Idle, [&](auto* sm) { GGIdle(); }},
				{ GGGS_GlideLeft, [&](auto* sm) { GGGlideLeftStep(); }},
				{ GGGS_GlideRight, [&](auto* sm) { GGGlideRightStep(); }},
			}
		};
	}

	void GreenGoblin::RegisterScript(Isolate* isolate, Local<ObjectTemplate> tpl, SceneUnitScripting* script)
	{
		Thug::RegisterScript(isolate, tpl, script);
		v8_register_method<GreenGoblin>(isolate, tpl, "ThrowBombAtTarget", script, [](GreenGoblin* self) { if (self) self->ThrowBombAtTarget(); });
		v8_register_method<GreenGoblin>(isolate, tpl, "GetBackGlideIdleAnimation", script, [](GreenGoblin* self) { if (self) self->GetBackGlideIdleAnimation(); });
	}

#if defined(_EDITOR)
	void GreenGoblin::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include "GreenGoblinAtt.h"
#include <JEnd.h>
		Thug::WriteJson(j);
	}
#endif

	void GreenGoblin::Map(SUUUID so)
	{
		using namespace Scene;
		Thug::Map(so);

		SetInitialConditions();
	}

	void GreenGoblin::Unmap()
	{
		Thug::Unmap();
	}

	void GreenGoblin::TakeHit(int damage)
	{
		if (combatState == GGCS_Glider)
		{
			damage *= 10;
		}
		Thug::TakeHit(damage);
	}

	//Step
	void GreenGoblin::Step(float delta)
	{
#if defined(_EDITOR)
		if (!Editor::IsPlaying(unit) || Editor::IsPaused(unit))
			return;
#endif
		if (physicObject.empty())
			return;

		if (combatState == GGCS_Glider)
		{
			gggsm.Step();
		}
		else
		{
			Thug::Step(delta);
		}
	}

	void GreenGoblin::GGIdle()
	{
		if (!combatEnabled())
			return;

		GetBrawlerScene(this)->UpdateEnemy(uuid());
		gggsm.ChangeState(GGGS_GlideLeft);
	}

	void GreenGoblin::SetInitialConditions()
	{
		Thug::SetInitialConditions();
		combatState = GreenGoblinCombatStates::GGCS_Glider;
		gggsm.ChangeState(GGGS_Idle);
		leftGlideTween = nullptr;
		rightGlideTween = nullptr;
		firstGlide = true;
		bombThrown = false;
	}

	void GreenGoblin::GotoToFloor()
	{
		RenderableID gliderR = MAKESUUUID(unit, glider());
		RenderableID handbombR = MAKESUUUID(unit, handbomb());
		if (SceneObjectExists(gliderR()))
		{
			gliderR->markedForDelete = true;
		}
		if (SceneObjectExists(handbombR()))
		{
			handbombR->markedForDelete = true;
		}
		health(100);
		auto* brawler = GetBrawlerScene(this);
		brawler->UpdateEnemy(uuid());

		PxRigidDynamic* internalActor = physicObject->controller->getActor();
		PxTransform pose = internalActor->getGlobalPose();

		physicObject->kinematic(false);
		physicObject->flag(PhysicObject::Update_kinematic);
		physicObject->DestroyPhysicsBehavior();
		physicObject->CreatePhysicsBehavior();
		physicObject->UpdatePhysicsAvatarTransformation();

		physicObject->clean(PhysicObject::Update_kinematic);

		PxVec3 nextPos = pose.p;
		nextPos.y = stopGlideHeight();
		renderable->position(ToXMFLOAT3(nextPos));
		PxExtendedVec3 exPos(PxVec3d(nextPos.x, nextPos.y, nextPos.z));
		physicObject->controller->setPosition(exPos);

		combatState = GreenGoblinCombatStates::GGCS_Floor;
	}

	//GlideLeft
	void GreenGoblin::GGEnterGlideLeft()
	{
		XMFLOAT3 vTo3 = leftSpot();
		XMVECTOR vTo = XMLoadFloat3(&vTo3);
		XMVECTOR vFrom;
		if (firstGlide)
		{
			vFrom = renderable->positionV();
		}
		else
		{
			lookingTo(CLT_Left);
			XMFLOAT3 scl = renderable->scale() * lookToSwapVector();
			renderable->scale(scl);

			XMFLOAT3 vFrom3 = rightSpot();
			vFrom = XMLoadFloat3(&vFrom3);
		}
		XMVECTOR len = XMVector3Length(XMVectorSubtract(vFrom, vTo));
		float dist = XMVectorGetX(len);
		float time = dist / glideSpeed();

		leftGlideTween = std::make_unique<tween>(XMVectorGetX(vFrom), XMVectorGetX(vTo), static_cast<int>(1000.0f * time), tween::easing::linear);
		firstGlide = false;
		bombThrown = false;
		renderable->SetCurrentAnimation(glideIdleAnimation(), 0.0f, 1.0f, true, true);
	}

	void GreenGoblin::GGGlideLeftStep()
	{
		if (health() == 0)
		{
			GotoToFloor();
			return;
		}

		float sclx = leftGlideTween->step();

		PxRigidDynamic* internalActor = physicObject->controller->getActor();
		PxTransform pose = internalActor->getGlobalPose();
		PxVec3 nextPos = pose.p;
		nextPos.x = sclx;

		PxTransform	newTransform(nextPos, pose.q);

		internalActor->setKinematicTarget(newTransform);

		if (leftGlideTween->current_value == leftGlideTween->target_value)
		{
			if (health() == 0)
			{
				GotoToFloor();
			}
			else
			{
				gggsm.ChangeState(GGGS_GlideRight);
			}
		}
		else if (!bombThrown)
		{
			XMFLOAT3 bombTarget{};
			if (GetBombTarget(bombTarget))
			{
				SetThrowBombTarget(bombTarget);
			}
		}
	}

	//GlideRight
	void GreenGoblin::GGEnterGlideRight()
	{
		XMFLOAT3 vTo3 = rightSpot();
		XMVECTOR vTo = XMLoadFloat3(&vTo3);
		XMVECTOR vFrom;
		if (firstGlide)
		{
			vFrom = renderable->positionV();
		}
		else
		{
			lookingTo(CLT_Right);
			XMFLOAT3 scl = renderable->scale() * lookToSwapVector();
			renderable->scale(scl);

			XMFLOAT3 vFrom3 = leftSpot();
			vFrom = XMLoadFloat3(&vFrom3);
		}
		XMVECTOR len = XMVector3Length(XMVectorSubtract(vFrom, vTo));
		float dist = XMVectorGetX(len);
		float time = dist / glideSpeed();

		rightGlideTween = std::make_unique<tween>(XMVectorGetX(vFrom), XMVectorGetX(vTo), static_cast<int>(1000.0f * time), tween::easing::linear);
		firstGlide = false;
		bombThrown = false;
		renderable->SetCurrentAnimation(glideIdleAnimation(), 0.0f, 1.0f, true, true);
	}

	void GreenGoblin::GGGlideRightStep()
	{
		if (health() == 0)
		{
			GotoToFloor();
			return;
		}

		float sclx = rightGlideTween->step();

		PxRigidDynamic* internalActor = physicObject->controller->getActor();
		PxTransform pose = internalActor->getGlobalPose();
		PxVec3 nextPos = pose.p;
		nextPos.x = sclx;

		PxTransform	newTransform(nextPos, pose.q);

		internalActor->setKinematicTarget(newTransform);

		if (rightGlideTween->current_value == rightGlideTween->target_value)
		{
			if (health() == 0)
			{
				GotoToFloor();
			}
			else
			{
				gggsm.ChangeState(GGGS_GlideLeft);
			}
		}
		else if (!bombThrown)
		{
			XMFLOAT3 bombTarget{};
			if (GetBombTarget(bombTarget))
			{
				SetThrowBombTarget(bombTarget);
			}
		}
	}

	bool GreenGoblin::GetBombTarget(XMFLOAT3& target)
	{
		std::vector<std::tuple<JUUID, XMVECTOR>> heroes_pos = GetBrawlerScene(this)->GetHeroesPositions();
		std::vector<std::tuple<JUUID, XMVECTOR>> heroes_in_range;
		float min_dist = glideMinDistancetoAttack();
		XMVECTOR my_pos = renderable->positionV();

		std::copy_if(heroes_pos.begin(), heroes_pos.end(), std::back_inserter(heroes_in_range), [min_dist, my_pos](auto& tup_uuid_pos)
			{
				XMVECTOR my_pos_y0 = XMVectorSetY(my_pos, 0.0f);
				XMVECTOR hero_pos = XMVectorSetY(std::get<1>(tup_uuid_pos), 0.0f);
				return XMVectorGetX(XMVector3Length(XMVectorSubtract(hero_pos, my_pos_y0))) <= min_dist;
			}
		);

		int sz = static_cast<int>(heroes_in_range.size());

		if (sz == 0ULL) return false;

		int index = std::rand() % sz;

		XMStoreFloat3(&target, std::get<1>(heroes_in_range.at(index)));

		return true;
	}

	void GreenGoblin::SetThrowBombTarget(XMFLOAT3 target)
	{
		renderable->SetCurrentAnimation(glideThrowAnimation(), 0.0f, 1.0f, true);
		bombThrown = true;
		bombTarget = target;
	}

	XMVECTOR GreenGoblin::GetBombInitialVelocity()
	{
		XMFLOAT3 gravity3 = physicScene->gravity();
		XMVECTOR gravity = XMLoadFloat3(&gravity3);

		auto [mm, initialPos, a, b, c] = renderable->GetBoneTransformation(glideThrowBone());
		bombInitialPos = initialPos;
		XMVECTOR initialPosV = XMLoadFloat3(&initialPos);
		XMVECTOR finalPosV = XMLoadFloat3(&bombTarget);

		XMVECTOR dxdz = XMVectorSetY(XMVectorSubtract(finalPosV, initialPosV), 0.0f);

		float h = bombMaxHeight() + XMVectorGetY(initialPosV);
		float g = std::abs(XMVectorGetY(gravity));
		float initialVy = std::sqrt(2.0f * g * (h - XMVectorGetY(initialPosV)));
		float tUp = initialVy / g;
		float tDown = std::sqrt(2.0f * (h - XMVectorGetY(finalPosV)) / g);
		float tTotal = tUp + tDown;

		XMVECTOR velocity = XMVectorScale(dxdz, 1.0f / tTotal);
		velocity = XMVectorSetY(velocity, initialVy);
		PrintXMVector(velocity, "velocity");
		return velocity;
	}

	void GreenGoblin::ThrowBombAtTarget()
	{
		XMVECTOR initialVelocity = GetBombInitialVelocity();
		//XMFLOAT3 initialVelocity3;
		XMFLOAT3 initialVelocity3 = {};
		XMStoreFloat3(&initialVelocity3, initialVelocity);
		XMFLOAT3 pos = bombInitialPos;
		auto cameras = renderable->cameras();
		JUUID en_uuid = renderable.uuid();

		Scene::CreateSceneObjectFromMold(unit, bomb(),
			[initialVelocity3, pos, en_uuid, cameras](SceneObjectType type, nlohmann::json json, std::string name)
			{
				nlohmann::json ret = json;

				ret["uuid"] = getUUID();
				ret["position"] = FromXMFLOAT3(pos);
				ret["cameras"] = cameras;
				ret["controllers"]["pumpkin-bomb"]["enemy"]["name"] = "greengoblin";
				ret["controllers"]["pumpkin-bomb"]["enemy"]["uuid"] = en_uuid;

				ret["physicObject"] = json["physicObject"];
				ret["physicObject"][0]["linearVelocity"] = FromXMFLOAT3(initialVelocity3);
				return ret;
			}
		);
	}

	void GreenGoblin::GetBackGlideIdleAnimation()
	{
		renderable->SetCurrentAnimation(glideIdleAnimation(), 0.0f, 1.0f, true, true);
	}
};