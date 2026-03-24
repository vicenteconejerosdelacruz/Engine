#include "pch.h"
#include "PhysicObject.h"
#include <Primitives.h>
//Physx
#include <PxPhysicsAPI.h>
#include <NoMath.h>

#if defined(_EDITOR)
namespace Editor
{
	extern CameraID GetLevelCamera(SceneUnitId id);
	extern void BindRenderableToPickingPass(RenderableID r);
	extern void SelectTrigger(TriggerID trigger);
	extern void SelectBoundary(BoundaryID boundary);

	//Scene Unit Registered
	extern bool StaticBodiesSceneUnitRegistered(SceneUnitId id);
	extern bool DynamicBodiesSceneUnitRegistered(SceneUnitId id);
	extern bool CharactersSceneUnitRegistered(SceneUnitId id);
	extern bool TriggersSceneUnitRegistered(SceneUnitId id);

	//Should Draw
	extern bool StaticBodiesShouldDraw(SceneUnitId id);
	extern bool DynamicBodiesShouldDraw(SceneUnitId id);
	extern bool CharactersShouldDraw(SceneUnitId id);
	extern bool TriggersShouldDraw(SceneUnitId id);

	//Register
	extern void RegisterStaticBody(PhysicObjectID phO);
	extern void RegisterDynamicBody(PhysicObjectID phO);
	extern void RegisterCharacter(PhysicObjectID phO);
	extern void RegisterTrigger(PhysicObjectID trigger);

	//UnRegister
	extern void UnRegisterStaticBody(PhysicObjectID phO);
	extern void UnRegisterDynamicBody(PhysicObjectID phO);
	extern void UnRegisterCharacter(PhysicObjectID phO);
	extern void UnRegisterTrigger(PhysicObjectID trigger);
};
#endif

namespace Scene
{
	extern bool SceneObjectExists(SUUUID suuuid);
};

using namespace physx;
extern PxPhysics* gPhysics;

namespace Physics
{
	std::map<JUUID, std::unique_ptr<PhysicObject>> physicObjectsUUIDs;
	std::map<SceneUnitId, std::set<JUUID>> physicObjectsBySceneUnitId;
	std::map<SUUUID, std::set<JUUID>> physicObjectsUUIDBySUUUID;
	std::map<SUUUID, JUUID> physicStaticBodyUUIDBySUUID;
	std::map<SUUUID, JUUID> physicDynamicBodyUUIDBySUUID;
	std::map<SUUUID, JUUID> physicCharacterUUIDBySUUID;
	std::map<SUUUID, JUUID> physicTriggerUUIDBySUUID;
	std::map<PhysicsBehavior, std::map<JUUID, std::function<void(JUUID, unsigned int)>>> physicContactSubscribers;
	std::map<TriggerID, std::function<void(SUUUID, unsigned int)>> triggerContactSubscribers;

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

	void PhysicObject::JUpdate(nlohmann::json p)
	{
#if defined(_EDITOR)
		Editor::MarkSceneUnitAsModified(unit());
#endif
		JObject::JUpdate(p);
	}

	void PhysicObject::JPatch(nlohmann::json p)
	{
#if defined(_EDITOR)
		Editor::MarkSceneUnitAsModified(unit());
#endif
		JObject::JPatch(p);
	}

	SceneUnitId PhysicObject::unit()
	{
		if (renderable) { return renderable.unit(); }
		if (trigger) { return trigger.unit(); }
		if (boundary) { return boundary.unit(); }
		assert(!!!"bad Physic Object");
		return 0x2BAD5005AD;
	}

	JUUID PhysicObject::uuid()
	{
		return at("uuid");
	}

	bool PhysicObject::CanBuild()
	{
		if (built) return false;

		if (geometry().empty())
			return false;

		PhysicGeometryJsonID jg = geometry();
		if (jg->mesh().empty() && jg->model().empty())
			return false;

		return true;
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
		PhysicGeometryJsonID jg = geometry();

		std::map<std::tuple<PhysicsBehavior, bool>, std::function<void()>> builders =
		{
			{ std::tuple(PB_Static,false), [&] {CreateStaticMeshBehavior(); } },
			{ std::tuple(PB_Dynamic,false), [&] {CreateDynamicMeshBehavior(); } },
			{ std::tuple(PB_Trigger,false), [&] {CreateTriggerMeshBehavior(); } },
			{ std::tuple(PB_Character,false), [&] {CreateCharacterMeshBehavior(); } },
			{ std::tuple(PB_Static,true), [&] {CreateStaticModel3DBehavior(); } },
			{ std::tuple(PB_Dynamic,true), [&] {CreateDynamicModel3DBehavior(); } },
			{ std::tuple(PB_Trigger,true), [&] {CreateTriggerModel3DBehavior(); } },
			{ std::tuple(PB_Character,true), [&] {CreateCharacterModel3DBehavior(); } },
		};

		builders.at(std::tuple(behavior(), jg->mesh().empty()))();
		built = true;
	}

	void PhysicObject::CreateStaticMeshBehavior()
	{
		PhysicGeometryJsonID jg = geometry();
		physicGeometryInstance = getUUID();
		nlohmann::json atts = nlohmann::json::parse(dump());
		CreatePhysicGeometryInstance(physicGeometryInstance(), [&]
			{
				if (renderable)
					return std::make_unique<PhysicGeometryInstance>(jg, renderable, atts, jg->mesh(), physicGeometryInstance(), behavior());
				else if (boundary)
					return std::make_unique<PhysicGeometryInstance>(jg, boundary, atts, jg->mesh(), physicGeometryInstance(), behavior());
				assert(!!!"bad parent scene object");
				return std::make_unique<PhysicGeometryInstance>(physicGeometryInstance());
			}
		);
		material = gPhysics->createMaterial(staticFriction(), dynamicFriction(), restitution());

		XMFLOAT3 pos = renderable ? renderable->position() : boundary->position();
		XMVECTOR rot = renderable ? renderable->rotationQ() : boundary->rotationQ();

		XMFLOAT3 localRot = localRotation();
		JNAME meshName = GetMeshName(jg->mesh());
		PxQuat localRotQ = (ApplyGeometryLocalPoseTransformation.contains(meshName)) ?
			ApplyGeometryLocalPoseTransformation.at(meshName)(localRot) : ToPxQuat(localRot);

		//create the PxActor & the PxShape
		actor = gPhysics->createRigidStatic(PxTransform(ToPxVec3(pos)));
		shape = PxRigidActorExt::createExclusiveShape(*actor, physicGeometryInstance->geometry.any(), *material);
		shape->setLocalPose(PxTransform(ToPxVec3(localPosition()), localRotQ));

		//set the actor position and rotation
		actor->setGlobalPose(PxTransform(ToPxVec3(pos), ToPxQuat(rot)));

		//add the actor to the scene
		PhysicSceneID scene = MAKESUUUID((renderable ? renderable.unit() : boundary.unit()), *GetPhysicScenes((renderable ? renderable.unit() : boundary.unit())).begin());
		scene->pxScene->addActor(*actor);

		//assign user data to this physic object
		shape->userData = this;
		actor->userData = this;

		physicStaticBodyUUIDBySUUID[(renderable ? renderable() : boundary())] = at("uuid");
	}

	void PhysicObject::CreateDynamicMeshBehavior()
	{
		PhysicGeometryJsonID jg = geometry();
		physicGeometryInstance = getUUID();
		nlohmann::json atts = nlohmann::json::parse(dump());
		CreatePhysicGeometryInstance(physicGeometryInstance(), [&]
			{
				return std::make_unique<PhysicGeometryInstance>(jg, renderable, atts, jg->mesh(), physicGeometryInstance(), behavior());
			}
		);
		material = gPhysics->createMaterial(staticFriction(), dynamicFriction(), restitution());

		XMFLOAT3 pos = renderable->position();
		XMVECTOR rot = renderable->rotationQ();

		XMFLOAT3 localRot = localRotation();
		JNAME meshName = GetMeshName(jg->mesh());
		PxQuat localRotQ = (ApplyGeometryLocalPoseTransformation.contains(meshName)) ?
			ApplyGeometryLocalPoseTransformation.at(meshName)(localRot) : ToPxQuat(localRot);

		//create the PxActor
		actor = gPhysics->createRigidDynamic(PxTransform(PxIdentity));

		//create the PxShape
		shape = PxRigidActorExt::createExclusiveShape(*actor, physicGeometryInstance->geometry.any(), *material);
		shape->setLocalPose(PxTransform(ToPxVec3(localPosition()), localRotQ));

		//set the actor position and rotation
		actor->setGlobalPose(PxTransform(ToPxVec3(pos), ToPxQuat(rot)));

		//set the body(actor) density and compute it's inertia
		PxRigidBody* pxBody = (PxRigidBody*)actor;
		PxRigidBodyExt::updateMassAndInertia(*pxBody, density());

		//set the velocity and acceleration
		PxRigidDynamic* pxDynamic = (PxRigidDynamic*)actor;
		pxDynamic->setLinearVelocity(ToPxVec3(linearVelocity()));
		pxDynamic->setAngularVelocity(ToPxVec3(angularVelocity()));

		//add the actor to the scene
		PhysicSceneID scene = MAKESUUUID(renderable.unit(), *GetPhysicScenes(renderable.unit()).begin());
		scene->pxScene->addActor(*actor);

		//assign user data to this physic object
		actor->userData = this;
		shape->userData = this;

		physicDynamicBodyUUIDBySUUID[renderable()] = at("uuid");
	}

	void PhysicObject::CreateCharacterMeshBehavior()
	{
		PhysicGeometryJsonID jg = geometry();
		JNAME controllerType = GetMeshName(jg->mesh());

		PhysicSceneID scene = MAKESUUUID(renderable.unit(), *GetPhysicScenes(renderable.unit()).begin());
		PxControllerManager* manager = scene->pxControllerManager;

		material = gPhysics->createMaterial(1.0f, 1.0f, 1.0f);

		auto buildAABB = [&]
			{
				PxBoxControllerDesc desc;
				XMFLOAT3 halfDimensions = ToXMFLOAT3(at("halfDimensions"));
				desc.halfHeight = halfDimensions.y;			// Half-height in the "up" direction
				desc.halfSideExtent = halfDimensions.x;		// Half-extent in the "side" direction
				desc.halfForwardExtent = halfDimensions.z;	// Half-extent in the "forward" direction
				desc.contactOffset = static_cast<float>(at("contactOffset"));
				desc.material = material;
				desc.position = ToPxVec3d(renderable->position() + localPosition());
				desc.userData = this;
				controller = manager->createController(desc);
			};

		auto buildCapsule = [&]
			{
				PxCapsuleControllerDesc desc;
				desc.radius = 1.0f * static_cast<float>(at("radius"));
				desc.height = 2.0f * static_cast<float>(at("halfHeight"));
				desc.contactOffset = static_cast<float>(at("contactOffset"));
				desc.material = material;
				desc.position = ToPxVec3d(renderable->position() + localPosition());
				desc.userData = this;
				controller = manager->createController(desc);
			};

		std::map<std::string, std::function<void()>> builder =
		{
			{ "cube", buildAABB },
			{ "capsule", buildCapsule }
		};

		builder.at(controllerType)();

		controller->setUserData(this);
		controller->getActor()->userData = this;

		physicCharacterUUIDBySUUID[renderable()] = at("uuid");
	}

	void PhysicObject::CreateTriggerMeshBehavior()
	{
		PhysicGeometryJsonID jg = geometry();
		physicGeometryInstance = getUUID();
		nlohmann::json atts = nlohmann::json::parse(dump());
		CreatePhysicGeometryInstance(physicGeometryInstance(), [&]
			{
				return std::make_unique<PhysicGeometryInstance>(jg, trigger, atts, jg->mesh(), physicGeometryInstance(), behavior());
			}
		);
		material = gPhysics->createMaterial(1.0f, 1.0f, 1.0f);

		XMFLOAT3 pos = trigger->position();
		XMVECTOR rot = trigger->rotationQ();

		XMFLOAT3 localRot = localRotation();
		JNAME meshName = GetMeshName(jg->mesh());
		PxQuat localRotQ = (ApplyGeometryLocalPoseTransformation.contains(meshName)) ?
			ApplyGeometryLocalPoseTransformation.at(meshName)(localRot) : ToPxQuat(localRot);

		//create the PxActor
		actor = gPhysics->createRigidStatic(PxTransform(ToPxVec3(pos)));

		//create the PxShape
		shape = PxRigidActorExt::createExclusiveShape(*actor, physicGeometryInstance->geometry.any(), *material, PxShapeFlag::eTRIGGER_SHAPE | PxShapeFlag::eVISUALIZATION);
		shape->setLocalPose(PxTransform(ToPxVec3(localPosition()), localRotQ));

		//set the actor position and rotation
		actor->setGlobalPose(PxTransform(ToPxVec3(pos), ToPxQuat(rot)));

		//add the actor to the pxScene
		PhysicSceneID scene = MAKESUUUID(trigger.unit(), *GetPhysicScenes(trigger.unit()).begin());
		scene->pxScene->addActor(*actor);

		//assign user data to this physic object
		actor->userData = this;
		shape->userData = this;

		physicTriggerUUIDBySUUID[trigger()] = at("uuid");
	}

	void PhysicObject::CreateStaticModel3DBehavior()
	{
		PhysicGeometryJsonID jg = geometry();
		physicGeometryInstance = getUUID();
		Model3DJsonID modelJ = jg->model();
		CreatePhysicGeometryInstance(physicGeometryInstance(), [&]
			{
				return std::make_unique<PhysicGeometryInstance>(jg, renderable, modelJ, physicGeometryInstance(), behavior());
			}
		);
		material = gPhysics->createMaterial(staticFriction(), dynamicFriction(), restitution());

		XMFLOAT3 pos = renderable->position();
		XMVECTOR rot = renderable->rotationQ();

		//create the PxActor & the PxShape
		actor = gPhysics->createRigidStatic(PxTransform(ToPxVec3(pos)));
		shape = PxRigidActorExt::createExclusiveShape(*actor, physicGeometryInstance->geometry.any(), *material);
		shape->setLocalPose(PxTransform(ToPxVec3(localPosition()), ToPxQuat(localRotation())));

		//set the actor position and rotation
		actor->setGlobalPose(PxTransform(ToPxVec3(pos), ToPxQuat(rot)));

		//add the actor to the scene
		PhysicSceneID scene = MAKESUUUID(renderable.unit(), *GetPhysicScenes(renderable.unit()).begin());
		scene->pxScene->addActor(*actor);

		//assign user data to this physic object
		actor->userData = this;
		shape->userData = this;

		physicStaticBodyUUIDBySUUID[renderable()] = at("uuid");
	}

	void PhysicObject::CreateDynamicModel3DBehavior()
	{
		PhysicGeometryJsonID jg = geometry();
		physicGeometryInstance = getUUID();
		Model3DJsonID modelJ = jg->model();
		CreatePhysicGeometryInstance(physicGeometryInstance(), [&]
			{
				return std::make_unique<PhysicGeometryInstance>(jg, renderable, modelJ, physicGeometryInstance(), behavior());
			}
		);
		material = gPhysics->createMaterial(staticFriction(), dynamicFriction(), restitution());

		XMFLOAT3 pos = renderable->position();
		XMVECTOR rot = renderable->rotationQ();

		//create the PxActor
		actor = gPhysics->createRigidDynamic(PxTransform(PxIdentity));

		//create the PxShape
		shape = PxRigidActorExt::createExclusiveShape(*actor, physicGeometryInstance->geometry.any(), *material);
		shape->setLocalPose(PxTransform(ToPxVec3(localPosition()), ToPxQuat(localRotation())));

		//set the actor position and rotation
		actor->setGlobalPose(PxTransform(ToPxVec3(pos), ToPxQuat(rot)));

		//set the body(actor) density and compute it's inertia
		PxRigidBody* pxBody = (PxRigidBody*)actor;
		PxRigidBodyExt::updateMassAndInertia(*pxBody, density());

		//set the velocity and acceleration
		PxRigidDynamic* pxDynamic = (PxRigidDynamic*)actor;
		pxDynamic->setLinearVelocity(ToPxVec3(linearVelocity()));
		pxDynamic->setAngularVelocity(ToPxVec3(angularVelocity()));

		//add the actor to the scene
		PhysicSceneID scene = MAKESUUUID(renderable.unit(), *GetPhysicScenes(renderable.unit()).begin());
		scene->pxScene->addActor(*actor);

		//assign user data to this physic object
		actor->userData = this;
		shape->userData = this;

		physicDynamicBodyUUIDBySUUID[renderable()] = at("uuid");
	}

	void PhysicObject::CreateCharacterModel3DBehavior()
	{
		assert(!!!"do not implement");
	}

	void PhysicObject::CreateTriggerModel3DBehavior()
	{
		PhysicGeometryJsonID jg = geometry();
		physicGeometryInstance = getUUID();
		Model3DJsonID modelJ = jg->model();
		CreatePhysicGeometryInstance(physicGeometryInstance(), [&]
			{
				return std::make_unique<PhysicGeometryInstance>(jg, trigger, modelJ, physicGeometryInstance(), behavior());
			}
		);
		material = gPhysics->createMaterial(1.0f, 1.0f, 1.0f);

		XMFLOAT3 pos = trigger->position();
		XMVECTOR rot = trigger->rotationQ();

		//create the PxActor
		actor = gPhysics->createRigidStatic(PxTransform(ToPxVec3(pos)));

		//create the PxShape
		shape = PxRigidActorExt::createExclusiveShape(*actor, physicGeometryInstance->geometry.any(), *material, PxShapeFlag::eTRIGGER_SHAPE | PxShapeFlag::eVISUALIZATION);
		shape->setLocalPose(PxTransform(ToPxVec3(localPosition()), ToPxQuat(localRotation())));

		//set the actor position and rotation
		actor->setGlobalPose(PxTransform(ToPxVec3(pos), ToPxQuat(rot)));

		//add the actor to the pxScene
		PhysicSceneID scene = MAKESUUUID(trigger.unit(), *GetPhysicScenes(trigger.unit()).begin());
		scene->pxScene->addActor(*actor);

		//assign user data to this physic object
		actor->userData = this;
		shape->userData = this;

		physicTriggerUUIDBySUUID[trigger()] = at("uuid");
	}

	void PhysicObject::DestroyPhysicsBehavior()
	{
		PhysicSceneID scene;
		if (renderable)
		{
			scene = MAKESUUUID(renderable.unit(), *GetPhysicScenes(renderable.unit()).begin());
		}
		if (trigger)
		{
			scene = MAKESUUUID(trigger.unit(), *GetPhysicScenes(trigger.unit()).begin());
		}
		if (boundary)
		{
			scene = MAKESUUUID(boundary.unit(), *GetPhysicScenes(boundary.unit()).begin());
		}

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
		if (controller)
		{
			PX_RELEASE(controller);
		}
		//PX_RELEASE(shape);
		auto eraseUUIDFromSUUID = [](auto& SUUIDs, SUUUID uuid)
			{
				if (SUUIDs.contains(uuid))
				{
					SUUIDs.erase(uuid);
				}
			};
		eraseUUIDFromSUUID(physicStaticBodyUUIDBySUUID, renderable());
		eraseUUIDFromSUUID(physicStaticBodyUUIDBySUUID, boundary());
		eraseUUIDFromSUUID(physicDynamicBodyUUIDBySUUID, renderable());
		eraseUUIDFromSUUID(physicCharacterUUIDBySUUID, renderable());
		eraseUUIDFromSUUID(physicTriggerUUIDBySUUID, trigger());

		built = false;
	}

	void PhysicObject::SetInitialConditions()
	{
		if (behavior() == PB_Dynamic && actor && renderable)
		{
			PxRigidDynamic* pxDynamic = (PxRigidDynamic*)actor;
			pxDynamic->setGlobalPose(PxTransform(ToPxVec3(renderable->position()), ToPxQuat(renderable->rotationQ())));
			pxDynamic->setLinearVelocity(ToPxVec3(linearVelocity()));
			pxDynamic->setAngularVelocity(ToPxVec3(angularVelocity()));
#if defined(_EDITOR)
			if (renderableShape)
			{
				renderableShape->position(renderable->position());
				renderableShape->rotationQ(renderable->rotationQ());
			}
			if (renderableLines)
			{
				renderableLines->position(renderable->position());
				renderableLines->rotationQ(renderable->rotationQ());
			}
#endif
		}
		else if (behavior() == PB_Character && controller && renderable)
		{
			controller->setPosition(ToPxVec3d(renderable->position() + localPosition()));
#if defined(_EDITOR)
			if (renderableShape) { renderableShape->position(renderable->position() + localPosition()); }
			if (renderableLines) { renderableLines->position(renderable->position() + localPosition()); }
#endif
		}
	}

	void PhysicObject::UpdateRenderableFromGlobalPose()
	{
		if (behavior() == PB_Dynamic && actor && renderable)
		{
			PxTransform pxT = actor->getGlobalPose();
			renderable->position(*((XMFLOAT3*)&pxT.p.x));
			renderable->rotationQuaternion = XMLoadFloat4((XMFLOAT4*)&pxT.q.x);
#if defined(_EDITOR)
			if (renderableShape)
			{
				renderableShape->position(renderable->position() + localPosition());
				renderableShape->rotationQ(renderable->rotationQ() + localRotation());
			}
			if (renderableLines)
			{
				renderableLines->position(renderable->position() + localPosition());
				renderableLines->rotationQ(renderable->rotationQ() + localRotation());
			}
#endif
		}
		else if (behavior() == PB_Character && controller && updateFromCharacterPosition() && renderable)
		{
			XMFLOAT3 renPos = ToXMFLOAT3(useFootPosition() ? controller->getFootPosition() : controller->getPosition());
			renderable->position(renPos);
#if defined(_EDITOR)
			XMFLOAT3 shapePos = ToXMFLOAT3(controller->getPosition());
			if (renderableShape) { renderableShape->position(shapePos); }
			if (renderableLines) { renderableLines->position(shapePos); }
#endif
		}
	}

	void PhysicObject::UpdateGlobalPoseFromRenderable()
	{
		if (!renderable && !boundary)
			return;

		if (behavior() == PB_Static && actor)
		{
			XMFLOAT3 gpos = renderable ? renderable->position() : boundary->position();
			XMVECTOR grot = renderable ? renderable->rotationQ() : boundary->rotationQ();
			actor->setGlobalPose(PxTransform(ToPxVec3(gpos), ToPxQuat(grot)));
#if defined(_EDITOR)
			XMFLOAT3 shapePos = gpos + localPosition();
			auto [pos, rotQ, rot, scl] = GetPhysicsAvatarTransformation();
			if (renderableShape)
			{
				renderableShape->position(pos);
				renderableShape->rotation(rot);
				renderableShape->rotationQ(rotQ);
			}
			if (renderableLines)
			{
				renderableLines->position(pos);
				renderableLines->rotation(rot);
				renderableLines->rotationQ(rotQ);
			}
#endif
		}
		else if (behavior() == PB_Dynamic && actor)
		{
			PxRigidDynamic* pxDynamic = (PxRigidDynamic*)actor;
			pxDynamic->setGlobalPose(PxTransform(ToPxVec3(renderable->position()), ToPxQuat(renderable->rotationQ())));
#if defined(_EDITOR)
			if (renderableShape)
			{
				renderableShape->position(renderable->position());
				renderableShape->rotationQ(renderable->rotationQ());
			}
			if (renderableLines)
			{
				renderableLines->position(renderable->position());
				renderableLines->rotationQ(renderable->rotationQ());
			}
#endif
		}
		else if (behavior() == PB_Character && controller)
		{
			controller->setPosition(ToPxVec3d(renderable->position() + localPosition()));
#if defined(_EDITOR)
			if (renderableShape) { renderableShape->position(renderable->position() + localPosition()); }
			if (renderableLines) { renderableLines->position(renderable->position() + localPosition()); }
#endif
		}
	}

	void PhysicObject::UpdateGlobalPoseFromTrigger()
	{
		actor->setGlobalPose(PxTransform(ToPxVec3(trigger->position()), ToPxQuat(trigger->rotationQ())));
	}

	PxControllerCollisionFlags  PhysicObject::MoveCharacter(XMVECTOR disp, float delta)
	{
		if (!controller || !built) return (PxControllerCollisionFlags)0U;

		PxVec3 pxdisp = ToPxVec3(disp);
		PxControllerFilters filters;
		return controller->move(pxdisp, 0.01f, delta, filters);
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
		std::vector<std::string> atts = { "behavior", "color", "overrideColor", "skipRendering" };

		std::vector<std::string> staticAtts =
		{
			"staticFriction", "dynamicFriction", "restitution",
			"geometry", "localPosition", "localRotation", "localScale",
			"density"
		};

		std::vector<std::string> dynamicAtts =
		{
			"staticFriction", "dynamicFriction", "restitution",
			"geometry", "localPosition", "localRotation",
			"density", "linearVelocity", "angularVelocity"
		};

		std::vector<std::string> triggerAtts =
		{
			"geometry", "localPosition", "localRotation"
		};

		std::vector<std::string> characterAtts =
		{
			"geometry", "localPosition", "localRotation", "updateFromCharacterPosition", "useFootPosition", "contactOffset"
		};

		std::unordered_map<PhysicsBehavior, std::vector<std::string>&> attsToAdd =
		{
			{ PB_Static, staticAtts },
			{ PB_Dynamic, dynamicAtts },
			{ PB_Trigger, triggerAtts },
			{ PB_Character, characterAtts }
		};

		nostd::AppendToVector(atts, attsToAdd.at(behavior()));
		return atts;
	}

	std::tuple<XMFLOAT3, XMVECTOR, XMFLOAT3, XMFLOAT3> PhysicObject::GetPhysicsAvatarTransformation()
	{
		//calculate the scale
		XMFLOAT3 scl = localScale() * (renderable ? renderable->scale() : boundary->scale());

		XMFLOAT3 rot = localRotation() + (renderable ? renderable->rotation() : boundary->rotation());
		XMVECTOR rotQ = XMQuatFromDegrees(rot);

		XMVECTOR actorRotQ = XMQuatFromDegrees((renderable ? renderable->rotation() : boundary->rotation()));
		XMMATRIX actorRotMM = XMMatrixRotationQuaternion(actorRotQ);
		XMFLOAT3 actorPos = (renderable ? renderable->position() : boundary->position());
		XMVECTOR actorPosV = XMLoadFloat3(&actorPos);
		XMFLOAT3 localPos = localPosition();
		XMVECTOR localPosV = XMLoadFloat3(&localPos);
		XMVECTOR posV = actorPosV + XMVector3Transform(localPosV, actorRotMM);
		XMFLOAT3 pos;
		XMStoreFloat3(&pos, posV);

		return std::make_tuple(pos, rotQ, Quaternion2Euler(rotQ), scl);
	}

	//Renderable representation
	void PhysicObject::CreatePhysicsAvatar()
	{
		if (renderableShape) return;

		std::map<PhysicsBehavior, std::function<void()>> builder =
		{
			{PB_Static,[&] {CreateRenderableStatic(); }},
			{PB_Dynamic,[&] {CreateRenderableDynamic(); }},
			{PB_Character,[&] {CreateRenderableCharacter(); }},
			{PB_Trigger,[&] {CreateRenderableTrigger(); }},
		};

		builder.at(behavior())();
	}

	void PhysicObject::DestroyPhysicsAvatar()
	{
		using namespace Editor;

		auto destroy = [](RenderableID& r)
			{
				if (!r) return;
				r->visible(false);
				r->markedForDelete = true;
				r.clear();
			};
		destroy(renderableShape);
		destroy(renderableLines);

		std::map<PhysicsBehavior, std::function<void()>> unregister =
		{
			{PB_Static,[&] {UnRegisterStaticBody(uuid()); }},
			{PB_Dynamic,[&] {UnRegisterDynamicBody(uuid()); }},
			{PB_Character,[&] {UnRegisterCharacter(uuid()); }},
			{PB_Trigger,[&] {UnRegisterTrigger(uuid()); }},
		};

		unregister.at(behavior())();

		avatarBuilt = false;
	}

	void PhysicObject::UpdatePhysicsAvatarTransformation()
	{
		if (!avatarBuilt) return;

		if (std::set<PhysicsBehavior>({ PB_Static,PB_Dynamic }).contains(behavior()))
		{
			auto [pos, rotQ, rot, scl] = GetPhysicsAvatarTransformation();

			renderableLines->position(pos);
			renderableLines->rotation(rot);
			renderableLines->rotationQ(rotQ);
			renderableLines->scale(scl);
			renderableShape->position(pos);
			renderableShape->rotation(rot);
			renderableShape->rotationQ(rotQ);
			renderableShape->scale(scl);
		}
		else if (std::set<PhysicsBehavior>({ PB_Character }).contains(behavior()))
		{
			renderableLines->position(renderable->position());
			renderableShape->position(renderable->position());
		}
		else if (std::set<PhysicsBehavior>({ PB_Trigger }).contains(behavior()))
		{
			renderableLines->position(trigger->position());
			renderableLines->rotation(trigger->rotation());
			renderableLines->scale(trigger->scale());
			renderableShape->position(trigger->position());
			renderableShape->rotation(trigger->rotation());
			renderableShape->scale(trigger->scale());
		}
	}

	void PhysicObject::visible(bool v)
	{
		if (renderableShape) { renderableShape->visible(v); }
		if (renderableLines) { renderableLines->visible(v); }
	}

	void PhysicObject::UpdatePhysicsAvatarColor(unsigned int frame, XMFLOAT4 rgba)
	{
		if (!avatarBuilt) return;
		XMFLOAT3 baseColor = { rgba.x,rgba.y,rgba.z };
		XMFLOAT3 lineBaseColor = baseColor * 1.3f;
		float alpha = rgba.w;
		renderableShape->WriteConstantsBuffer("baseColor", baseColor, frame);
		renderableShape->WriteConstantsBuffer("alpha", alpha, frame);
		renderableLines->WriteConstantsBuffer("baseColor", lineBaseColor, frame);
	}

	void PhysicObject::CreateRenderableStatic()
	{
		renderableLines = RenderableID(MAKESUUUID(unit(), getUUID()), [&] {return avatarBuilt; });
		renderableShape = RenderableID(MAKESUUUID(unit(), getUUID()), [&] {return avatarBuilt; });
		CameraID camera = Editor::GetLevelCamera(unit());
		bool visible = Editor::StaticBodiesShouldDraw(unit());

		auto [pos, rotQ, rot, scl] = GetPhysicsAvatarTransformation();
		std::string name = renderable ? renderable->name() : boundary->name();

		nlohmann::json lines = CreateFromRenderable(name + "-static-body-lines", renderableLines.uuid(), camera.uuid(), "Translucent_wired", visible, pos, rot, scl);
		nlohmann::json shape = CreateFromRenderable(name + "-static-body-shape", renderableShape.uuid(), camera.uuid(), "Translucent", visible, pos, rot, scl);

		shape["renderNext"] = { renderableLines.uuid() };

		nlohmann::json data =
		{
			{ "renderables", { lines, shape } }
		};

		AttachLevelIntoScene(unit(), "static-body", data, [&, rotQ](SceneUnitId id)
			{
				using namespace Editor;
				if (boundary)
				{
					BindRenderableToPickingPass(renderableLines);
					BindRenderableToPickingPass(renderableShape);
					renderableLines->OnPick = [&] {Editor::SelectTrigger(boundary->SUuuid()); };
					renderableShape->OnPick = [&] {Editor::SelectTrigger(boundary->SUuuid()); };
					renderableLines->rotationQ(rotQ);
					renderableShape->rotationQ(rotQ);
				}
				RegisterStaticBody(uuid());
				avatarBuilt = true;
			}
		);
	}
	void PhysicObject::CreateRenderableDynamic()
	{
		renderableLines = RenderableID(MAKESUUUID(unit(), getUUID()), [&] {return avatarBuilt; });
		renderableShape = RenderableID(MAKESUUUID(unit(), getUUID()), [&] {return avatarBuilt; });
		CameraID camera = Editor::GetLevelCamera(unit());
		bool visible = Editor::DynamicBodiesShouldDraw(unit());

		XMFLOAT3 renScale = renderable->scale() * localScale();
		XMFLOAT3 renPosition = renderable->position() + localPosition();
		XMFLOAT3 renRotation = renderable->rotation() + localRotation();

		nlohmann::json lines = CreateFromRenderable(renderable->name() + "-dynamic-body-lines", renderableLines.uuid(), camera.uuid(), "Translucent_wired", visible, renPosition, renRotation, renScale);
		nlohmann::json shape = CreateFromRenderable(renderable->name() + "-dynamic-body-shape", renderableShape.uuid(), camera.uuid(), "Translucent", visible, renPosition, renRotation, renScale);

		shape["renderNext"] = { renderableLines.uuid() };

		nlohmann::json data =
		{
			{ "renderables", { lines, shape } }
		};

		AttachLevelIntoScene(unit(), "dynamic-body", data, [&](SceneUnitId id)
			{
				using namespace Editor;
				RegisterDynamicBody(uuid());
				avatarBuilt = true;
			}
		);
	}
	void PhysicObject::CreateRenderableCharacter()
	{
		renderableLines = RenderableID(MAKESUUUID(unit(), getUUID()), [&] {return avatarBuilt; });
		renderableShape = RenderableID(MAKESUUUID(unit(), getUUID()), [&] {return avatarBuilt; });
		CameraID camera = Editor::GetLevelCamera(unit());
		bool visible = Editor::CharactersShouldDraw(unit());

		XMFLOAT3 renPosition = renderable->position() + localPosition();

		nlohmann::json lines = CreateFromRenderable(
			renderable->name() + "-char-lines", renderableLines.uuid(), camera.uuid(),
			"Translucent_wired", visible, renPosition, { 0.0f,0.0f,0.0f }, { 1.0f,1.0f,1.0f });
		nlohmann::json shape = CreateFromRenderable(
			renderable->name() + "-char-shape", renderableShape.uuid(), camera.uuid(),
			"Translucent", visible, renPosition, { 0.0f,0.0f,0.0f }, { 1.0f,1.0f,1.0f });

		shape["renderNext"] = { renderableLines.uuid() };

		nlohmann::json data =
		{
			{ "renderables", { lines, shape } }
		};

		AttachLevelIntoScene(unit(), "characters", data, [&](SceneUnitId id)
			{
				using namespace Editor;
				RegisterCharacter(uuid());
				avatarBuilt = true;
			}
		);
	}
	void PhysicObject::CreateRenderableTrigger()
	{
		renderableLines = RenderableID(MAKESUUUID(unit(), getUUID()), [&] {return avatarBuilt; });
		renderableShape = RenderableID(MAKESUUUID(unit(), getUUID()), [&] {return avatarBuilt; });
		CameraID camera = Editor::GetLevelCamera(unit());

		nlohmann::json lines = CreateFromTrigger(trigger->name() + "-lines", renderableLines.uuid(), camera.uuid(), "Translucent_wired");
		nlohmann::json shape = CreateFromTrigger(trigger->name() + "-shape", renderableShape.uuid(), camera.uuid(), "Translucent");

		shape["renderNext"] = { renderableLines.uuid() };

		nlohmann::json data =
		{
			{ "renderables",
				{
					lines,
					shape
				}
			}
		};

		AttachLevelIntoScene(unit(), "triggers", data, [&](SceneUnitId id)
			{
				using namespace Editor;
				BindRenderableToPickingPass(renderableLines);
				BindRenderableToPickingPass(renderableShape);
				renderableLines->OnPick = [&] {Editor::SelectTrigger(trigger->SUuuid()); };
				renderableShape->OnPick = [&] {Editor::SelectTrigger(trigger->SUuuid()); };
				RegisterTrigger(uuid());
				avatarBuilt = true;
			}
		);
	}

	nlohmann::json PhysicObject::CreateFromRenderable(std::string name, JUUID uuid, JUUID camId, std::string material, bool visible, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale)
	{
		PhysicGeometryJsonID pg = geometry();

		nlohmann::json jrenderable = nlohmann::json(
			{
				{
					"meshMaterial",
					{
						{ "material", GetMaterialUUIDByName(material) },
						{ "mesh",
							{
								{ "primitive", pg->mesh() }
							}
						}
					}
				},
				{ "castShadows", false },
				{ "shadowed", false },
				{ "name" , name },
				{ "uuid" , uuid },
				{ "position" , FromXMFLOAT3(position) },
				{ "topology", "TRIANGLELIST" },
				{ "rotation" , FromXMFLOAT3(rotation) },
				{ "scale" , FromXMFLOAT3(scale) },
				{ "skipMeshes" , {}},
				{ "visible" , visible },
				{ "hidden" , true},
				{ "cameras", { camId }},
				{ "depthStencil",
					{
						{ "BackFace",
							{
								{ "StencilDepthFailOp", "KEEP"},
								{ "StencilFailOp", "KEEP"},
								{ "StencilFunc", "ALWAYS"},
								{ "StencilPassOp", "KEEP" }
							}
						},
						{ "DepthEnable", false },
						{ "DepthFunc", "NONE" },
						{ "DepthWriteMask", "ZERO" },
						{ "FrontFace",
							{
								{ "StencilDepthFailOp", "KEEP"},
								{ "StencilFailOp", "KEEP"},
								{ "StencilFunc", "ALWAYS"},
								{ "StencilPassOp", "KEEP" }
							}
						},
						{ "StencilEnable", false},
						{ "StencilReadMask", 255},
						{ "StencilWriteMask", 255 }
					}
				}
			}
		);
		return jrenderable;
	}
	nlohmann::json PhysicObject::CreateFromTrigger(std::string name, JUUID uuid, JUUID camId, std::string material)
	{
		PhysicGeometryJsonID pg = geometry();

		bool visible = Editor::TriggersShouldDraw(unit());

		nlohmann::json jrentrigger = nlohmann::json(
			{
				{
					"meshMaterial",
					{
						{ "material", GetMaterialUUIDByName(material) },
						{ "mesh",
							{
								{ "primitive", pg->mesh() }
							}
						}
					}
				},
				{ "castShadows", false },
				{ "shadowed", false },
				{ "name" , name },
				{ "uuid" , uuid },
				{ "position", FromXMFLOAT3(trigger->position()) },
				{ "topology", "TRIANGLELIST" },
				{ "rotation" , FromXMFLOAT3(trigger->rotation()) },
				{ "scale" , FromXMFLOAT3(trigger->scale()) },
				{ "skipMeshes" , {}},
				{ "visible" , visible },
				{ "hidden" , true},
				{ "cameras", { camId }},
				{ "passMaterialOverrides",
					{
						{
							{ "meshIndex", 0 },
							{ "renderPass", GetRenderPassUUIDByName("PickingPass") },
							{ "material", GetMaterialUUIDByName("TriggerPicking") }
						}
					}
				},
				{ "depthStencil",
					{
						{ "BackFace",
							{
								{ "StencilDepthFailOp", "KEEP"},
								{ "StencilFailOp", "KEEP"},
								{ "StencilFunc", "ALWAYS"},
								{ "StencilPassOp", "KEEP" }
							}
						},
						{ "DepthEnable", false },
						{ "DepthFunc", "NONE" },
						{ "DepthWriteMask", "ZERO" },
						{ "FrontFace",
							{
								{ "StencilDepthFailOp", "KEEP"},
								{ "StencilFailOp", "KEEP"},
								{ "StencilFunc", "ALWAYS"},
								{ "StencilPassOp", "KEEP" }
							}
						},
						{ "StencilEnable", false},
						{ "StencilReadMask", 255},
						{ "StencilWriteMask", 255 }
					}
				}
			}
		);
		return jrentrigger;
	}

#endif

	std::unique_ptr<PhysicObject>& GetPhysicObject(JUUID uuid)
	{
		return physicObjectsUUIDs.at(uuid);
	}

	void DestroyPhysicObject(JUUID uuid)
	{
		if (!physicObjectsUUIDs.contains(uuid)) return;
		physicObjectsUUIDs.at(uuid)->DestroyPhysicsBehavior();
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
		for (auto it = physicObjectsUUIDBySUUUID.begin(); it != physicObjectsUUIDBySUUUID.end(); )
		{
			if (it->second.contains(uuid))
				it->second.erase(uuid);

			if (it->second.size() == 0ULL)
				it = physicObjectsUUIDBySUUUID.erase(it);
			else
				it++;
		}

		auto eraseUUUIDFromSUUID = [](auto& UUIDBySUUID, auto uuid)
			{
				for (auto it = UUIDBySUUID.begin(); it != UUIDBySUUID.end();)
				{
					if (it->second == uuid)
					{
						UUIDBySUUID.erase(it);
						break;
					}
					it++;
				}

			};

		eraseUUUIDFromSUUID(physicStaticBodyUUIDBySUUID, uuid);
		eraseUUUIDFromSUUID(physicDynamicBodyUUIDBySUUID, uuid);
		eraseUUUIDFromSUUID(physicCharacterUUIDBySUUID, uuid);
		eraseUUUIDFromSUUID(physicTriggerUUIDBySUUID, uuid);
	}

	std::set<JUUID> GetPhysicsObjectsBySceneObjectUUID(SUUUID uuid)
	{
		return(physicObjectsUUIDBySUUUID.contains(uuid)) ? physicObjectsUUIDBySUUUID.at(uuid) : std::set<JUUID>();
	}

	JUUID CreatePhysicObject(std::string name, SUUUID sceneObject, nlohmann::json& json)
	{
		JUUID uuid = getUUID();
		std::unique_ptr<PhysicObject> physicObject = std::make_unique<PhysicObject>(json);

		switch (GetSceneObjectType(FROMSUUUID(sceneObject)))
		{
		case SO_Renderables:
		{
			physicObject->renderable = sceneObject;
		}
		break;
		case SO_Triggers:
		{
			physicObject->trigger = sceneObject;
		}
		break;
		case SO_Boundaries:
		{
			physicObject->boundary = sceneObject;
		}
		break;
		}

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
			PhysicObjectID phO = uuid;
			if (!phO->CanBuild())
				continue;

			phO->CreatePhysicsBehavior();
#if defined(_EDITOR)
			phO->CreatePhysicsAvatar();
#endif
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
#if defined(_EDITOR)
		using namespace Editor;
#endif
		if (!physicObjectsBySceneUnitId.contains(id)) return;

		auto& phOs = physicObjectsBySceneUnitId.at(id);
		unsigned int frame = GetSceneUnit(id)->Frame();
		PhysicSceneID scene = MAKESUUUID(id, *GetPhysicScenes(id).begin());

		auto checkLocalPose = [](PhysicObjectID p)
			{
				std::vector<size_t> flags = { PhysicObject::Update_localPosition,PhysicObject::Update_localRotation, PhysicObject::Update_localScale };
				if (!p->dirty(flags))
					return;
				p->clean(flags);
				p->localScale(XMClamp(p->localScale(), 0.01f, 1000.0f));
				p->DestroyPhysicsBehavior();
				p->CreatePhysicsBehavior();
				p->UpdatePhysicsAvatarTransformation();
			};
		auto checkBehaviorGeom = [](PhysicObjectID p)
			{
				std::vector<size_t> flags = { PhysicObject::Update_behavior,PhysicObject::Update_geometry };
				if (!p->dirty(flags))
					return;
				p->clean(flags);
				p->DestroyPhysicsBehavior();
				p->CreatePhysicsBehavior();
				p->DestroyPhysicsAvatar();
				p->CreatePhysicsAvatar();
			};
		auto checkVelocity = [](PhysicObjectID p)
			{
				std::vector<size_t> flags = { PhysicObject::Update_linearVelocity,PhysicObject::Update_angularVelocity };
				if (!p->dirty(flags))
					return;

				if (p->dirty(PhysicObject::Update_linearVelocity))
				{
					PxRigidDynamic* pxDynamic = (PxRigidDynamic*)p->actor;
					pxDynamic->setLinearVelocity(ToPxVec3(p->linearVelocity()));
				}

				if (p->dirty(PhysicObject::Update_angularVelocity))
				{
					PxRigidDynamic* pxDynamic = (PxRigidDynamic*)p->actor;
					pxDynamic->setAngularVelocity(ToPxVec3(p->angularVelocity()));
				}

				p->clean(flags);
			};
#if defined(_EDITOR)
		std::map<PhysicsBehavior, XMFLOAT4> behaviorColors =
		{
			{ PB_Static, scene->staticColor() },
			{ PB_Dynamic, scene->dynamicColor() },
			{ PB_Character, scene->characterColor() },
			{ PB_Trigger, scene->triggerColor() },
		};

		auto updateColors = [=](PhysicObjectID p)
			{
				p->UpdatePhysicsAvatarColor(frame, p->overrideColor() ? p->color() : behaviorColors.at(p->behavior()));
			};
#endif

		std::for_each(phOs.begin(), phOs.end(), checkLocalPose);
		std::for_each(phOs.begin(), phOs.end(), checkBehaviorGeom);
		std::for_each(phOs.begin(), phOs.end(), checkVelocity);
#if defined(_EDITOR)
		std::for_each(phOs.begin(), phOs.end(), updateColors);
#endif
	}

	void RegisterContactCallback(PhysicsBehavior behavior, JUUID object, std::function<void(JUUID, unsigned int)> callback)
	{
		physicContactSubscribers[behavior][object] = callback;
	}

	void UnregisterContactCallback(PhysicsBehavior behavior, JUUID object)
	{
		if (!physicContactSubscribers.contains(behavior) || !physicContactSubscribers.at(behavior).contains(object)) return;

		physicContactSubscribers.at(behavior).erase(object);
	}

	void CallRegisteredCallbacks(PhysicsBehavior behavior, JUUID destObject, JUUID srcObject, unsigned int event)
	{
		if (!physicContactSubscribers.contains(behavior) || !physicContactSubscribers.at(behavior).contains(destObject)) return;

		physicContactSubscribers.at(behavior).at(destObject)(srcObject, event);
	}

	//Trigger Callback
	void RegisterTriggerContactCallback(TriggerID trigger, std::function<void(SUUUID, unsigned int)> callback)
	{
		triggerContactSubscribers.insert_or_assign(trigger, callback);
	}
	void UnregisterTriggerContactCallback(TriggerID trigger)
	{
		triggerContactSubscribers.erase(trigger);
	}
	void CallTriggerContactCallback(TriggerID trigger, SUUUID sceneObject, unsigned int event)
	{
		if (!triggerContactSubscribers.contains(trigger)) return;

		triggerContactSubscribers.at(trigger)(sceneObject, event);
	}
};