#include "pch.h"
#include "Physics.h"
#include <unordered_map>
#include <cassert>
#include <SimpleMath.h>
#include <Physics/PhysicScene.h>
#include <Primitives.h>
//Physx
#include <PxPhysicsAPI.h>
#include <extensions/PxDefaultAllocator.h>

using namespace physx;
#define PVD_HOST "127.0.0.1"
static PxDefaultAllocator gAllocator;
static PxDefaultErrorCallback gErrorCallback;
static PxFoundation* gFoundation = nullptr;
static PxPvd* gPvd = nullptr;
static PxPhysics* gPhysics = nullptr;
static PxDefaultCpuDispatcher* gDispatcher = nullptr;

using namespace Scene;
using namespace DirectX;
namespace Physics
{
	std::unordered_map<JUUID, std::unique_ptr<PhysicObject>> physicObjectsUUIDs;
	std::unordered_map<SceneUnitId, std::set<JUUID>> physicObjectsBySceneUnitId;
	//std::unordered_map<SUUUID, std::set<JUUID>> physicObjectsUUIDBySUUUID;

	void InitializePhysics()
	{
		gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
		gPvd = PxCreatePvd(*gFoundation);
		PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
		gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

		gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
		PxInitExtensions(*gPhysics, gPvd);

		gDispatcher = PxDefaultCpuDispatcherCreate(2);
	}

	void DestroyPhysics()
	{
		//PX_RELEASE(gDispatcher);
		PX_RELEASE(gPhysics);
		if (gPvd)
		{
			PxPvdTransport* transport = gPvd->getTransport();
			PX_RELEASE(gPvd);
			PX_RELEASE(transport);
		}
		PX_RELEASE(gFoundation);
	}

	void CreatePhysicsScene(PhysicSceneID physicScene)
	{
		PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
		XMFLOAT3 gravity = physicScene->gravity();
		sceneDesc.gravity = PxVec3(gravity.x, gravity.y, gravity.z);
		sceneDesc.cpuDispatcher = gDispatcher;
		sceneDesc.filterShader = PxDefaultSimulationFilterShader;
		physicScene->pxScene = gPhysics->createScene(sceneDesc);
	}

	void CreatePhysicsObjectsBehaviors(SceneUnitId id)
	{
		for (auto& uuid : physicObjectsBySceneUnitId.at(id))
		{
			physicObjectsUUIDs.at(uuid)->CreatePhysicsBehavior();
		}
	}

	void UpdateFromGlobalPose(SceneUnitId id)
	{
		if (!physicObjectsBySceneUnitId.contains(id)) return;

		for (auto& uuid : physicObjectsBySceneUnitId.at(id))
		{
			auto& phO = GetPhysicObject(uuid);
			phO->UpdateFromGlobalPose();
		}
	}

	void UpdatePhysicObjects(SceneUnitId id)
	{
		if (!physicObjectsBySceneUnitId.contains(id)) return;

		for (auto& uuid : physicObjectsBySceneUnitId.at(id))
		{
			auto& phO = GetPhysicObject(uuid);
			if (phO->dirty(PhysicObject::Update_behavior))
			{
				phO->DestroyPhisicsBehavior();
				phO->CreatePhysicsBehavior();
				phO->clean(PhysicObject::Update_behavior);
			}
		}
	}

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
		return uuid;
	}

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
		scene->pxScene->removeActor(*actor);
		//PX_RELEASE(material);
		//PX_RELEASE(actor);
		//PX_RELEASE(shape);
	}

	void PhysicObject::CreateStaticFromMesh()
	{
		RenderableID renderable = sceneObject;
		MeshInstanceID mesh = renderable->meshes.at(0);
		JNAME name = GetMeshName(mesh->uuid);
		LoadPrimitiveIntoPxGeometryFunctions.at(name)(*this);

		//create the PxActor
		actor = gPhysics->createRigidStatic(PxTransform(PxIdentity));

		//create the PxShape
		shape = PxRigidActorExt::createExclusiveShape(*actor, geometry.any(), *material);
		shape->setLocalPose(PxTransform(PxIdentity));
		actor->attachShape(*shape);

		//set the actor position and rotation
		XMFLOAT3 pos = renderable->position();
		XMVECTOR rotation = renderable->rotationQ();
		PxVec3 pxPos(pos.x, pos.y, pos.z);
		PxQuat pxQuat(rotation.m128_f32[0], rotation.m128_f32[1], rotation.m128_f32[2], rotation.m128_f32[3]);
		actor->setGlobalPose(PxTransform(pxPos, pxQuat));

		//add the actor to the pxScene
		PhysicSceneID scene = MAKESUUUID(renderable.unit(), *GetPhysicScenes(renderable.unit()).begin());
		scene->pxScene->addActor(*actor);
	}

	void PhysicObject::CreateDynamicFromMesh()
	{
		RenderableID renderable = sceneObject;
		MeshInstanceID mesh = renderable->meshes.at(0);
		JNAME name = GetMeshName(mesh->uuid);
		LoadPrimitiveIntoPxGeometryFunctions.at(name)(*this);

		//create the PxActor
		actor = gPhysics->createRigidDynamic(PxTransform(PxIdentity));

		//create the PxShape
		shape = PxRigidActorExt::createExclusiveShape(*actor, geometry.any(), *material);
		shape->setLocalPose(PxTransform(PxIdentity));
		actor->attachShape(*shape);

		//set the actor position and rotation
		XMFLOAT3 pos = renderable->position();
		XMVECTOR rotation = renderable->rotationQ();
		PxVec3 pxPos(pos.x, pos.y, pos.z);
		PxQuat pxQuat(rotation.m128_f32[0], rotation.m128_f32[1], rotation.m128_f32[2], rotation.m128_f32[3]);
		actor->setGlobalPose(PxTransform(pxPos, pxQuat));

		//set the body(actor) density and compute it's inertia
		PxRigidBody* pxBody = (PxRigidBody*)actor;
		PxRigidBodyExt::updateMassAndInertia(*pxBody, density());

		//set the initial velocity and acceleration
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

	void PhysicObject::UpdateFromGlobalPose()
	{
		if (behavior() == PB_Static) return;

		PxTransform pxT = actor->getGlobalPose();
		RenderableID r = sceneObject;
		r->position(*((XMFLOAT3*)&pxT.p.x));
		r->rotationQ(*((XMFLOAT4*)&pxT.q.x));
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
					"density", "linearVelocity", "angularVelocity",
					"linearAcceleration", "angularAcceleration"
				}
			);
		}

		return atts;
	}
#endif
};