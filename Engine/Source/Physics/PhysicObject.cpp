#include "pch.h"
#include "PhysicObject.h"
#include <Primitives.h>
//Physx
#include <PxPhysicsAPI.h>

#if defined(_EDITOR)
namespace Editor
{
	extern CameraID GetLevelCamera(SceneUnitId id);
	extern void BindRenderableToPickingPass(RenderableID r);
	extern void SelectTrigger(TriggerID trigger);

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

	SceneUnitId PhysicObject::unit()
	{
		if (renderable) { return renderable.unit(); }
		if (trigger) { return trigger.unit(); }
		assert(!!!"bad Physic Object");
		return 0x2BAD5005AD;
	}

	JUUID PhysicObject::uuid()
	{
		return at("uuid");
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
		if (built) return;

		if (geometry().empty())
			return;
		PhysicGeometryJsonID jg = geometry();

		if (jg->mesh().empty() && jg->model().empty())
			return;

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

		//create the PxActor & the PxShape
		actor = gPhysics->createRigidStatic(PxTransform(ToPxVec3(pos)));
		shape = PxRigidActorExt::createExclusiveShape(*actor, physicGeometryInstance->geometry.any(), *material);
		shape->setLocalPose(PxTransform(ToPxVec3(localPosition()), localRotQ));

		//set the actor position and rotation
		actor->setGlobalPose(PxTransform(ToPxVec3(pos), ToPxQuat(rot)));

		//add the actor to the scene
		PhysicSceneID scene = MAKESUUUID(renderable.unit(), *GetPhysicScenes(renderable.unit()).begin());
		scene->pxScene->addActor(*actor);

		//assign user data to this physic object
		shape->userData = this;
		actor->userData = this;

		physicStaticBodyUUIDBySUUID[renderable()] = at("uuid");
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

	void PhysicObject::DestroyPhisicsBehavior()
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
		eraseUUIDFromSUUID(physicDynamicBodyUUIDBySUUID, renderable());
		eraseUUIDFromSUUID(physicCharacterUUIDBySUUID, renderable());
		eraseUUIDFromSUUID(physicTriggerUUIDBySUUID, trigger());
		if (renderableShape && SceneObjectExists(renderableShape())) { renderableShape->markedForDelete = true; renderableShape.clear(); }
		if (renderableLines && SceneObjectExists(renderableLines())) { renderableLines->markedForDelete = true; renderableLines.clear(); }

		built = false;
	}

	void PhysicObject::SetInitialConditions()
	{
		if (!renderable)
			return;

		if (behavior() == PB_Dynamic && actor)
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
		else if (behavior() == PB_Character && controller)
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
		if (!renderable)
			return;

		if (behavior() == PB_Dynamic && actor)
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
		else if (behavior() == PB_Character && controller && updateFromCharacterPosition())
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
		if (!renderable)
			return;

		if (behavior() == PB_Static && actor)
		{
			actor->setGlobalPose(PxTransform(ToPxVec3(renderable->position()), ToPxQuat(renderable->rotationQ())));
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
		std::vector<std::string> atts = { "behavior" };

		std::vector<std::string> staticAtts =
		{
			"staticFriction", "dynamicFriction", "restitution",
			"geometry", "localPosition", "localRotation",
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
			"geometry", "localPosition", "localRotation", "updateFromCharacterPosition", "useFootPosition"
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

	//Renderable representation
	void PhysicObject::visible(bool v)
	{
		if (renderableShape) { renderableShape->visible(v); }
		if (renderableLines) { renderableLines->visible(v); }
	}

	void PhysicObject::UpdateRenderableColor(XMFLOAT4 color)
	{
		if (!renderableShape) return;

		XMFLOAT3 baseColor = { color.x,color.y,color.z };
		XMFLOAT3 lineBaseColor = baseColor * 1.3f;
		float alpha = color.w;
		for (unsigned int i = 0; i < Renderer::numFrames; i++)
		{
			renderableShape->WriteConstantsBuffer("baseColor", baseColor, i);
			renderableShape->WriteConstantsBuffer("alpha", alpha, i);
			renderableLines->WriteConstantsBuffer("baseColor", lineBaseColor, i);
		}
	}

	void PhysicObject::CreateRenderableStatic()
	{
		if (geometry().empty())
			return;

		renderableLines = MAKESUUUID(unit(), getUUID());
		renderableShape = MAKESUUUID(unit(), getUUID());
		CameraID camera = Editor::GetLevelCamera(unit());
		bool visible = Editor::StaticBodiesShouldDraw(unit());

		nlohmann::json lines = CreateFromRenderable(renderable->name() + "-dynamic-body-lines", renderableLines.uuid(), camera.uuid(), "Translucent_wired", visible, renderable->position(), renderable->rotation(), renderable->scale());
		nlohmann::json shape = CreateFromRenderable(renderable->name() + "-dynamic-body-shape", renderableShape.uuid(), camera.uuid(), "Translucent", visible, renderable->position(), renderable->rotation(), renderable->scale());

		shape["renderNext"] = { renderableLines.uuid() };

		nlohmann::json data =
		{
			{ "renderables", { lines, shape } }
		};

		AttachLevelIntoScene(unit(), "dynamic-body", data, [&](SceneUnitId id)
			{
				using namespace Editor;
				RegisterStaticBody(uuid());
			}
		);
	}
	void PhysicObject::CreateRenderableDynamic()
	{
		if (geometry().empty())
			return;

		renderableLines = MAKESUUUID(unit(), getUUID());
		renderableShape = MAKESUUUID(unit(), getUUID());
		CameraID camera = Editor::GetLevelCamera(unit());
		bool visible = Editor::DynamicBodiesShouldDraw(unit());

		nlohmann::json lines = CreateFromRenderable(renderable->name() + "-dynamic-body-lines", renderableLines.uuid(), camera.uuid(), "Translucent_wired", visible, renderable->position(), renderable->rotation(), renderable->scale());
		nlohmann::json shape = CreateFromRenderable(renderable->name() + "-dynamic-body-shape", renderableShape.uuid(), camera.uuid(), "Translucent", visible, renderable->position(), renderable->rotation(), renderable->scale());

		shape["renderNext"] = { renderableLines.uuid() };

		nlohmann::json data =
		{
			{ "renderables", { lines, shape } }
		};

		AttachLevelIntoScene(unit(), "dynamic-body", data, [&](SceneUnitId id)
			{
				using namespace Editor;
				RegisterDynamicBody(uuid());
			}
		);
	}
	void PhysicObject::CreateRenderableCharacter()
	{
		if (geometry().empty())
			return;

		renderableLines = MAKESUUUID(unit(), getUUID());
		renderableShape = MAKESUUUID(unit(), getUUID());
		CameraID camera = Editor::GetLevelCamera(unit());
		bool visible = Editor::CharactersShouldDraw(unit());

		nlohmann::json lines = CreateFromRenderable(
			renderable->name() + "-char-lines", renderableLines.uuid(), camera.uuid(),
			"Translucent_wired", visible, renderable->position() + localPosition(),
			{ 0.0f,0.0f,0.0f }, { 1.0f,1.0f,1.0f }
		);
		nlohmann::json shape = CreateFromRenderable(
			renderable->name() + "-char-shape", renderableShape.uuid(), camera.uuid(),
			"Translucent", visible, renderable->position() + localPosition(),
			{ 0.0f,0.0f,0.0f }, { 1.0f,1.0f,1.0f }
		);

		shape["renderNext"] = { renderableLines.uuid() };

		nlohmann::json data =
		{
			{ "renderables", { lines, shape } }
		};

		AttachLevelIntoScene(unit(), "characters", data, [&](SceneUnitId id)
			{
				using namespace Editor;
				RegisterCharacter(uuid());
			}
		);
	}
	void PhysicObject::CreateRenderableTrigger()
	{
		if (geometry().empty())
			return;

		renderableLines = MAKESUUUID(unit(), getUUID());
		renderableShape = MAKESUUUID(unit(), getUUID());
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
		physicObjectsUUIDs.at(uuid)->DestroyPhisicsBehavior();
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
			physicObject->renderable = sceneObject;
			break;
		case SO_Triggers:
			physicObject->trigger = sceneObject;
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

	void UpdatePhysicObjects(SceneUnitId id, float step)
	{
#if defined(_EDITOR)
		using namespace Editor;
#endif
		if (!physicObjectsBySceneUnitId.contains(id)) return;

#if defined(_EDITOR)
		auto createRenderables = [](SceneUnitId id, auto checker, auto& UUIDbySUUUID, auto build)
			{
				if (!checker(id)) return;
				for (auto [suuuid, juuid] : UUIDbySUUUID)
				{
					if (std::get<0>(suuuid) != id) continue;
					PhysicObjectID phO = juuid;

					if (!phO->built || !phO->renderableShape.empty())
						continue;
					build(phO);
				}
			};

		createRenderables(id, StaticBodiesSceneUnitRegistered, physicStaticBodyUUIDBySUUID, [](auto& phO) {phO->CreateRenderableStatic(); });
		createRenderables(id, DynamicBodiesSceneUnitRegistered, physicDynamicBodyUUIDBySUUID, [](auto& phO) {phO->CreateRenderableDynamic(); });
		createRenderables(id, CharactersSceneUnitRegistered, physicCharacterUUIDBySUUID, [](auto& phO) {phO->CreateRenderableCharacter(); });
		createRenderables(id, TriggersSceneUnitRegistered, physicTriggerUUIDBySUUID, [](auto& phO) {phO->CreateRenderableTrigger(); });
#endif

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
				if (phO->built)
				{
					PxRigidDynamic* pxDynamic = (PxRigidDynamic*)phO->actor;
					pxDynamic->setLinearVelocity(ToPxVec3(phO->linearVelocity()));
				}
				phO->clean(PhysicObject::Update_linearVelocity);
			}

			if (phO->dirty(PhysicObject::Update_angularVelocity))
			{
				if (phO->built)
				{
					PxRigidDynamic* pxDynamic = (PxRigidDynamic*)phO->actor;
					pxDynamic->setAngularVelocity(ToPxVec3(phO->angularVelocity()));
				}
				phO->clean(PhysicObject::Update_angularVelocity);
			}
		}
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
};