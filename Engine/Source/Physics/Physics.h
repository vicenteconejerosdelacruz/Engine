#pragma once
#include <SceneUnitId.h>
#include <UUID.h>
#include <JObject.h>
//Physx
#include <PxPhysicsAPI.h>

namespace Scene
{
	DEF_SCENEOBJECT_ID_DEP(PhysicScene);
};

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

using namespace physx;
namespace Physics
{
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
		void CreatePhysicsBehavior();
		void DestroyPhisicsBehavior();
		void CreateStaticFromMesh();
		void CreateDynamicFromMesh();
		void CreateStaticFromModel3D();
		void CreateDynamicFromModel3D();
		void SetInitialConditions();

		//dynamics
		void UpdateFromGlobalPose();

#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
		std::vector<std::string> GetPhysicBehaviorAttributes();
#endif
		SUUUID sceneObject;
		PxGeometryHolder geometry;
		PxMaterial* material;
		PxRigidActor* actor;
		PxShape* shape;
	};

	void InitializePhysics();
	void DestroyPhysics();
	void CreatePhysicsScene(PhysicSceneID physicScene);
	void CreatePhysicsObjectsBehaviors(SceneUnitId id);
	void UpdateFromGlobalPose(SceneUnitId id);
	void UpdatePhysicObjects(SceneUnitId id);

	std::unique_ptr<PhysicObject>& GetPhysicObject(JUUID uuid);
	void DestroyPhysicObject(JUUID uuid);
	std::set<JUUID> GetPhysicsObjectsBySceneObjectUUID(SUUUID uuid);
	JUUID CreatePhysicObject(std::string name, SUUUID sceneObject, nlohmann::json& json);
};