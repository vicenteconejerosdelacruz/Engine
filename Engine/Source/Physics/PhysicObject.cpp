#include "pch.h"
#include "PhysicObject.h"
#include <Primitives.h>
//Physx
#include <PxPhysicsAPI.h>
#include <extensions/PxDefaultAllocator.h>
#include <extensions/PxCudaHelpersExt.h>
#include <gpu/PxGpu.h>
#include <gpu/PxPhysicsGpu.h>

using namespace physx;
extern PxPhysics* gPhysics;
extern PxCudaContextManager* gCudaContextManager;

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
	}

	void PhysicObject::CreatePhysicsBehavior()
	{
		RenderableID renderable = sceneObject;
		MeshInstanceID mesh = renderable->meshes.at(0);
		JNAME name = GetMeshName(mesh->uuid);

		std::map<std::tuple<bool, bool>, std::function<void()>> creatorsFunctions =
		{
			{std::tuple(false,false),[&] { CreateStaticFromMesh(); }},
			{std::tuple(false,true),[&] { CreateDynamicFromMesh(); }},
			{std::tuple(true,false),[&] { CreateStaticFromModel3D(); }},
			{std::tuple(true,true),[&] { CreateDynamicFromModel3D(); }},
		};

		bool fromModel = !renderable->model().empty();
		bool isDynamic = behavior() == PB_Dynamic && PrimitiveCanBeMadeDynamic.at(name);
		material = gPhysics->createMaterial(staticFriction(), dynamicFriction(), restitution());
		creatorsFunctions.at(std::tuple(fromModel, isDynamic))();
	}

	void PhysicObject::DestroyPhisicsBehavior()
	{
		RenderableID renderable = sceneObject;
		PhysicSceneID scene = MAKESUUUID(renderable.unit(), *GetPhysicScenes(renderable.unit()).begin());
		actor->detachShape(*shape);
		scene->pxScene->removeActor(*actor);
		//actor->release();
		//material->release();
		PX_RELEASE(material);
		PX_RELEASE(actor);
		//PX_RELEASE(shape);
	}

	void PhysicObject::CreateStaticFromMesh()
	{
		RenderableID renderable = sceneObject;
		MeshInstanceID mesh = renderable->meshes.at(0);
		JNAME name = GetMeshName(mesh->uuid);
		LoadPrimitiveIntoPxGeometryFunctions.at(name)(*this, nullptr);

		//create the PxActor & the PxShape
		actor = gPhysics->createRigidStatic(PxTransform(ToPxVec3(renderable->position())));
		shape = PxRigidActorExt::createExclusiveShape(*actor, geometry.any(), *material);
		shape->setLocalPose(PxTransform(PxIdentity));

		//set the actor position and rotation
		actor->setGlobalPose(PxTransform(ToPxVec3(renderable->position()), ToPxQuat(renderable->rotationQ())));

		//add the actor to the pxScene
		PhysicSceneID scene = MAKESUUUID(renderable.unit(), *GetPhysicScenes(renderable.unit()).begin());
		scene->pxScene->addActor(*actor);
	}

	void PhysicObject::CreateDynamicFromMesh()
	{
		RenderableID renderable = sceneObject;
		MeshInstanceID mesh = renderable->meshes.at(0);
		JNAME name = GetMeshName(mesh->uuid);
		LoadPrimitiveIntoPxGeometryFunctions.at(name)(*this,
			[](PxSDFDesc& sdfDesc)
			{
				sdfDesc.spacing = 0.05f;
				sdfDesc.subgridSize = 6;
				sdfDesc.bitsPerSubgridPixel = PxSdfBitsPerSubgridPixel::e16_BIT_PER_PIXEL;
				sdfDesc.numThreadsForSdfConstruction = 8;
				sdfDesc.sdfBuilder = PxGetPhysicsGpu()->createSDFBuilder(gCudaContextManager);
			});

		//create the PxActor
		actor = gPhysics->createRigidDynamic(PxTransform(PxIdentity));

		//create the PxShape
		shape = PxRigidActorExt::createExclusiveShape(*actor, geometry.any(), *material);
		shape->setLocalPose(PxTransform(PxIdentity));

		//set the actor position and rotation
		actor->setGlobalPose(PxTransform(ToPxVec3(renderable->position()), ToPxQuat(renderable->rotationQ())));

		//set the body(actor) density and compute it's inertia
		PxRigidBody* pxBody = (PxRigidBody*)actor;
		PxRigidBodyExt::updateMassAndInertia(*pxBody, density());

		//set the velocity and acceleration
		PxRigidDynamic* pxDynamic = (PxRigidDynamic*)actor;
		pxDynamic->setLinearVelocity(ToPxVec3(linearVelocity()));
		pxDynamic->setAngularVelocity(ToPxVec3(angularVelocity()));

		//add the actor to the pxScene
		PhysicSceneID scene = MAKESUUUID(renderable.unit(), *GetPhysicScenes(renderable.unit()).begin());
		scene->pxScene->addActor(*actor);
	}

	void PhysicObject::CreateStaticFromModel3D()
	{
	}

	void PhysicObject::CreateDynamicFromModel3D()
	{
	}

	void PhysicObject::SetInitialConditions()
	{
		if (behavior() == PB_Static) return;

		RenderableID renderable = sceneObject;
		PxRigidDynamic* pxDynamic = (PxRigidDynamic*)actor;
		pxDynamic->setGlobalPose(PxTransform(ToPxVec3(renderable->position()), ToPxQuat(renderable->rotationQ())));
		pxDynamic->setLinearVelocity(ToPxVec3(linearVelocity()));
		pxDynamic->setAngularVelocity(ToPxVec3(angularVelocity()));
	}

	void PhysicObject::UpdateRenderableFromGlobalPose()
	{
		if (behavior() == PB_Static) return;

		PxTransform pxT = actor->getGlobalPose();
		RenderableID r = sceneObject;
		r->position(*((XMFLOAT3*)&pxT.p.x));
		r->rotationQ(*((XMFLOAT4*)&pxT.q.x));
	}

	void PhysicObject::UpdateGlobalPoseFromRenderable()
	{
		RenderableID renderable = sceneObject;
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
		std::vector<std::string> atts = { "behavior", "staticFriction", "dynamicFriction", "restitution", };

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
		physicObject->sceneObject = sceneObject;
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
			if (phO->dirty(PhysicObject::Update_behavior))
			{
				phO->DestroyPhisicsBehavior();
				phO->CreatePhysicsBehavior();
				phO->clean(PhysicObject::Update_behavior);
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