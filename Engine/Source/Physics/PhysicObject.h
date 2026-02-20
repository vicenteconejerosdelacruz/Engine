#pragma once
#include <set>
#include <UUID.h>
#include <JObject.h>
//Physx
#include <PxPhysicsAPI.h>
using namespace physx;

enum PhysicsBehavior
{
	PB_Static,
	PB_Dynamic
};

inline std::unordered_map<PhysicsBehavior, std::string> PhysicsBehaviorToString =
{
	{ PB_Static, "Static" },
	{ PB_Dynamic, "Dynamic" },
};

inline std::unordered_map<std::string, PhysicsBehavior> StringToPhysicsBehavior =
{
	{ "Static", PB_Static },
	{ "Dynamic", PB_Dynamic },
};

namespace Physics
{
	namespace Cooking
	{
		inline static const std::string cookingFolder = "Assets/cooking";
		inline static const std::string cookingSDFFolder = "Assets/cooking/sdf";
	};

#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <PhysicObjectAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <PhysicObjectAtt.h>
#include <JEnd.h>

#endif

	struct PhysicObject : JObject
	{
#include <Attributes/JFlags.h>
#include <PhysicObjectAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <PhysicObjectAtt.h>
#include <JEnd.h>

		virtual ~PhysicObject() {};
		PhysicObject(nlohmann::json& json);
		void InheritGeometryAttributes();
		void CreatePhysicsBehavior();
		void DestroyPhisicsBehavior();
		void SetInitialConditions();

		//dynamics
		void UpdateRenderableFromGlobalPose();
		void UpdateGlobalPoseFromRenderable();

#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
		std::vector<std::string> GetPhysicBehaviorAttributes();
#endif
		RenderableID renderable;

		PhysicGeometryInstanceID physicGeometryInstance;
		PxMaterial* material = nullptr;
		PxRigidActor* actor = nullptr;
		PxShape* shape = nullptr;
	};

	std::unique_ptr<PhysicObject>& GetPhysicObject(JUUID uuid);
	void DestroyPhysicObject(JUUID uuid);
	std::set<JUUID> GetPhysicsObjectsBySceneObjectUUID(SUUUID uuid);
	JUUID CreatePhysicObject(std::string name, SUUUID sceneObject, nlohmann::json& json);

	void CreatePhysicsObjectsBehaviors(SceneUnitId id);
	void UpdateRenderablesFromGlobalPose(SceneUnitId id);
	void UpdatePhysicObjects(SceneUnitId id);

	DEF_TEMPLATE_ID(PhysicObject, GetPhysicObject);
};

using namespace Physics;
DEF_TEMPLATE_ID_HASH(PhysicObject);