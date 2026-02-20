#include "pch.h"
#include "PhysicObject.h"
#include <Primitives.h>
//Physx
#include <PxPhysicsAPI.h>

using namespace physx;
extern PxPhysics* gPhysics;

namespace Physics
{
	std::unordered_map<JUUID, std::unique_ptr<PhysicObject>> physicObjectsUUIDs;
	std::unordered_map<SceneUnitId, std::set<JUUID>> physicObjectsBySceneUnitId;
	std::unordered_map<SUUUID, std::set<JUUID>> physicObjectsUUIDBySUUUID;

#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <PhysicObjectAtt.h>
#include <JEnd.h>

#endif

	PhysicObject::PhysicObject(nlohmann::json& json) : JObject(json)
	{
#include <Attributes/JInit.h>
#include <PhysicObjectAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <PhysicObjectAtt.h>
#include <JEnd.h>

		InheritGeometryAttributes();
	}

	void PhysicObject::InheritGeometryAttributes()
	{
		if (geometry().empty())
			return;
		PhysicGeometryJsonID pg = geometry();
		if (pg->mesh().empty())
			return;

		JNAME meshName = GetMeshName(pg->mesh());
		if (!GetPxGeometryAttributes.contains(meshName)) return;

		nlohmann::json atts = GetPxGeometryAttributes.at(meshName)();
		for (auto& el : atts.items())
		{
			if (contains(el.key()))
				continue;
			nlohmann::json patch = { {el.key(), el.value()} };
			merge_patch(patch);
		}
	}

	void PhysicObject::CreatePhysicsBehavior()
	{
		if (geometry().empty())
			return;
		PhysicGeometryJsonID jg = geometry();

		if (jg->mesh().empty() && jg->model().empty())
			return;

		//why using physicGeometryInstance instead of the mesh/model as key?
		//basically because as the scale of the geometries are built-in in the geometry bulding time
		//we cannot split from the instancing, so instances are unique but what we can put in a cache is the mesh soup
		physicGeometryInstance = getUUID();
		if (!jg->mesh().empty())
		{
			nlohmann::json atts = nlohmann::json::parse(dump());
			CreatePhysicGeometryInstance(physicGeometryInstance(), [&]
				{
					return std::make_unique<PhysicGeometryInstance>(jg, renderable, atts, jg->mesh(), physicGeometryInstance(), behavior());
				}
			);
		}
		else if (!jg->model().empty())
		{
			Model3DJsonID modelJ = jg->model();
			CreatePhysicGeometryInstance(physicGeometryInstance(), [&]
				{
					return std::make_unique<PhysicGeometryInstance>(jg, renderable, modelJ, physicGeometryInstance(), behavior());
				}
			);
		}

		material = gPhysics->createMaterial(staticFriction(), dynamicFriction(), restitution());

		if (behavior() == PB_Static)
		{
			//create the PxActor & the PxShape
			actor = gPhysics->createRigidStatic(PxTransform(ToPxVec3(renderable->position())));
			shape = PxRigidActorExt::createExclusiveShape(*actor, physicGeometryInstance->geometry.any(), *material);
			shape->setLocalPose(PxTransform(ToPxVec3(localPosition()), ToPxQuat(localRotation())));

			//set the actor position and rotation
			actor->setGlobalPose(PxTransform(ToPxVec3(renderable->position()), ToPxQuat(renderable->rotationQ())));
		}
		else
		{
			//create the PxActor
			actor = gPhysics->createRigidDynamic(PxTransform(PxIdentity));

			//create the PxShape
			shape = PxRigidActorExt::createExclusiveShape(*actor, physicGeometryInstance->geometry.any(), *material);
			shape->setLocalPose(PxTransform(ToPxVec3(localPosition()), ToPxQuat(localRotation())));

			//set the actor position and rotation
			actor->setGlobalPose(PxTransform(ToPxVec3(renderable->position()), ToPxQuat(renderable->rotationQ())));

			//set the body(actor) density and compute it's inertia
			PxRigidBody* pxBody = (PxRigidBody*)actor;
			PxRigidBodyExt::updateMassAndInertia(*pxBody, density());

			//set the velocity and acceleration
			PxRigidDynamic* pxDynamic = (PxRigidDynamic*)actor;
			pxDynamic->setLinearVelocity(ToPxVec3(linearVelocity()));
			pxDynamic->setAngularVelocity(ToPxVec3(angularVelocity()));
		}

		//add the actor to the pxScene
		PhysicSceneID scene = MAKESUUUID(renderable.unit(), *GetPhysicScenes(renderable.unit()).begin());
		scene->pxScene->addActor(*actor);
	}

	void PhysicObject::DestroyPhisicsBehavior()
	{
		PhysicSceneID scene = MAKESUUUID(renderable.unit(), *GetPhysicScenes(renderable.unit()).begin());
		if (actor)
		{
			actor->detachShape(*shape);
			scene->pxScene->removeActor(*actor);
		}
		//actor->release();
		//material->release();
		if (material)
		{
			PX_RELEASE(material);
		}
		if (actor)
		{
			PX_RELEASE(actor);
		}
		//PX_RELEASE(shape);
	}

	void PhysicObject::SetInitialConditions()
	{
		if (behavior() == PB_Static) return;

		PxRigidDynamic* pxDynamic = (PxRigidDynamic*)actor;
		pxDynamic->setGlobalPose(PxTransform(ToPxVec3(renderable->position()), ToPxQuat(renderable->rotationQ())));
		pxDynamic->setLinearVelocity(ToPxVec3(linearVelocity()));
		pxDynamic->setAngularVelocity(ToPxVec3(angularVelocity()));
	}

	void PhysicObject::UpdateRenderableFromGlobalPose()
	{
		if (behavior() == PB_Static) return;

		PxTransform pxT = actor->getGlobalPose();
		renderable->position(*((XMFLOAT3*)&pxT.p.x));
		renderable->rotationQ(*((XMFLOAT4*)&pxT.q.x));
	}

	void PhysicObject::UpdateGlobalPoseFromRenderable()
	{
		PxRigidDynamic* pxDynamic = (PxRigidDynamic*)actor;
		pxDynamic->setGlobalPose(PxTransform(ToPxVec3(renderable->position()), ToPxQuat(renderable->rotationQ())));
	}

#if defined(_EDITOR)
	void PhysicObject::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <PhysicObjectAtt.h>
#include <JEnd.h>
		j.erase("uuid");
	}

	std::vector<std::string> PhysicObject::GetPhysicBehaviorAttributes()
	{
		std::vector<std::string> atts = {
			"behavior", "staticFriction", "dynamicFriction",
			"restitution", "geometry", "localPosition",
			"localRotation"
		};

		if (behavior() == PB_Dynamic)
		{
			atts.insert(atts.end(),
				{
					"density", "linearVelocity", "angularVelocity"
				}
			);
		}

		return atts;
	}
#endif

	std::unique_ptr<PhysicObject>& GetPhysicObject(JUUID uuid)
	{
		return physicObjectsUUIDs.at(uuid);
	}

	void DestroyPhysicObject(JUUID uuid)
	{
		physicObjectsUUIDs.erase(uuid);
		for (auto it = physicObjectsBySceneUnitId.begin(); it != physicObjectsBySceneUnitId.end();)
		{
			if (it->second.contains(uuid))
				it->second.erase(uuid);

			if (it->second.size() == 0ULL)
				it = physicObjectsBySceneUnitId.erase(it);
			else
				it++;
		}
		for (auto it = physicObjectsUUIDBySUUUID.begin(); it != physicObjectsUUIDBySUUUID.end(); it++)
		{
			if (it->second.contains(uuid))
				it->second.erase(uuid);

			if (it->second.size() == 0ULL)
				it = physicObjectsUUIDBySUUUID.erase(it);
			else
				it++;
		}
	}

	std::set<JUUID> GetPhysicsObjectsBySceneObjectUUID(SUUUID uuid)
	{
		return(physicObjectsUUIDBySUUUID.contains(uuid)) ? physicObjectsUUIDBySUUUID.at(uuid) : std::set<JUUID>();
	}

	JUUID CreatePhysicObject(std::string name, SUUUID sceneObject, nlohmann::json& json)
	{
		JUUID uuid = getUUID();
		std::unique_ptr<PhysicObject> physicObject = std::make_unique<PhysicObject>(json);
		physicObject->renderable = sceneObject;
		(*physicObject)["uuid"] = uuid;
		physicObjectsUUIDs.insert_or_assign(uuid, std::move(physicObject));
		SceneUnitId id = std::get<0>(sceneObject);
		if (!physicObjectsBySceneUnitId.contains(id))
		{
			physicObjectsBySceneUnitId.insert_or_assign(id, std::set<JUUID>());
		}
		physicObjectsBySceneUnitId.at(id).insert(uuid);
		physicObjectsUUIDBySUUUID[sceneObject].insert(uuid);
		return uuid;
	}

	void CreatePhysicsObjectsBehaviors(SceneUnitId id)
	{
		if (!physicObjectsBySceneUnitId.contains(id)) return;

		for (auto& uuid : physicObjectsBySceneUnitId.at(id))
		{
			physicObjectsUUIDs.at(uuid)->CreatePhysicsBehavior();
		}
	}

	void UpdateRenderablesFromGlobalPose(SceneUnitId id)
	{
		if (!physicObjectsBySceneUnitId.contains(id)) return;

		for (PhysicObjectID phO : physicObjectsBySceneUnitId.at(id))
		{
			phO->UpdateRenderableFromGlobalPose();
		}
	}

	void UpdatePhysicObjects(SceneUnitId id)
	{
		if (!physicObjectsBySceneUnitId.contains(id)) return;

		for (PhysicObjectID phO : physicObjectsBySceneUnitId.at(id))
		{
			if (phO->dirty(PhysicObject::Update_behavior) ||
				phO->dirty(PhysicObject::Update_geometry) ||
				phO->dirty(PhysicObject::Update_localPosition) ||
				phO->dirty(PhysicObject::Update_localRotation)
				)
			{

				phO->DestroyPhisicsBehavior();
				phO->CreatePhysicsBehavior();
				phO->clean(PhysicObject::Update_behavior);
				phO->clean(PhysicObject::Update_geometry);
				phO->clean(PhysicObject::Update_localPosition);
				phO->clean(PhysicObject::Update_localRotation);
			}

			if (phO->dirty(PhysicObject::Update_linearVelocity))
			{
				PxRigidDynamic* pxDynamic = (PxRigidDynamic*)phO->actor;
				pxDynamic->setLinearVelocity(ToPxVec3(phO->linearVelocity()));
				phO->clean(PhysicObject::Update_linearVelocity);
			}

			if (phO->dirty(PhysicObject::Update_angularVelocity))
			{
				PxRigidDynamic* pxDynamic = (PxRigidDynamic*)phO->actor;
				pxDynamic->setAngularVelocity(ToPxVec3(phO->angularVelocity()));
				phO->clean(PhysicObject::Update_angularVelocity);
			}
		}
	}
};