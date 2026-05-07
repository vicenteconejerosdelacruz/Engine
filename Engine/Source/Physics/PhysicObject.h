#pragma once

#include <set>
#include <UUID.h>
#include <JObject.h>
//Physx
#include <PxPhysicsAPI.h>
using namespace physx;

namespace Scene
{
	DEF_SCENEOBJECT_ID_DEP(Renderable);
	DEF_SCENEOBJECT_ID_DEP(Trigger);
	DEF_SCENEOBJECT_ID_DEP(Boundary);
};

namespace Templates
{
	DEF_TEMPLATE_ID_DEP(PhysicGeometryInstance, GetPhysicGeometryInstance);
}

enum PhysicsBehavior
{
	PB_Static,
	PB_Dynamic,
	PB_Trigger,
	PB_Character
};

inline std::unordered_map<PhysicsBehavior, std::string> PhysicsBehaviorToString =
{
	{ PB_Static, "Static" },
	{ PB_Dynamic, "Dynamic" },
	{ PB_Trigger, "Trigger" },
	{ PB_Character, "Character" },
};

inline std::unordered_map<std::string, PhysicsBehavior> StringToPhysicsBehavior =
{
	{ "Static", PB_Static },
	{ "Dynamic", PB_Dynamic },
	{ "Trigger", PB_Trigger },
	{ "Character", PB_Character },
};

namespace Physics
{
	using namespace Scene;
	using namespace Templates;

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

#include <Attributes/JStr2Flag.h>
#include <PhysicObjectAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <PhysicObjectAtt.h>
#include <JEnd.h>

		virtual ~PhysicObject() {};
		PhysicObject(nlohmann::json& json);
		virtual void JUpdate(nlohmann::json p);
		virtual void JPatch(nlohmann::json p);
		SceneUnitId unit();
		JUUID uuid();
		bool CanBuild();
		void InheritGeometryAttributes();
		PxFilterData GetPxFilterData();
		void CreatePhysicsBehavior();
		void CreateStaticMeshBehavior();
		void CreateDynamicMeshBehavior();
		void CreateCharacterMeshBehavior();
		void CreateTriggerMeshBehavior();
		void CreateStaticModel3DBehavior();
		void CreateDynamicModel3DBehavior();
		void CreateCharacterModel3DBehavior();
		void CreateTriggerModel3DBehavior();
		void DestroyPhysicsBehavior();
		void SetInitialConditions();

		//dynamics & characters
		void UpdateRenderableFromGlobalPose();
		void UpdateGlobalPoseFromRenderable();
		void UpdateGlobalPoseFromTrigger();
		PxControllerCollisionFlags MoveCharacter(XMVECTOR disp, float delta);

#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
		std::vector<std::string> GetPhysicBehaviorAttributes();
		virtual bool CanInteractWithGizmo(ImGuizmo::OPERATION operation) { return true; }

		//Renderable representation
		std::tuple<XMFLOAT3, XMVECTOR, XMFLOAT3, XMFLOAT3> GetPhysicsAvatarTransformation();
		void CreatePhysicsAvatar();
		void DestroyPhysicsAvatar();
		void UpdatePhysicsAvatarTransformation();
		void visible(bool value);
		void UpdatePhysicsAvatarColor(unsigned int frame, XMFLOAT4 rgba);
		void CreateRenderableStatic();
		void CreateRenderableDynamic();
		void CreateRenderableCharacter();
		void CreateRenderableTrigger();
		nlohmann::json CreateFromRenderable(std::string name, JUUID uuid, JUUID camId, std::string material, bool visible, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale);
		nlohmann::json CreateFromTrigger(std::string name, JUUID uuid, JUUID camId, std::string material);
#endif

		RenderableID renderable;
		TriggerID trigger;
		BoundaryID boundary;

		PhysicGeometryInstanceID physicGeometryInstance;
		PxMaterial* material = nullptr;
		PxRigidActor* actor = nullptr;
		PxShape* shape = nullptr;
		PxController* controller = nullptr;
#if defined(_EDITOR)
		RenderableID renderableShape;
		RenderableID renderableLines;
		bool avatarBuilt = false;
#endif
		bool built = false;
	};

	std::unique_ptr<PhysicObject>& GetPhysicObject(JUUID uuid);
	void DestroyPhysicObject(JUUID uuid);
	std::set<JUUID> GetPhysicsObjectsBySceneObjectUUID(SUUUID uuid);
	JUUID CreatePhysicObject(std::string name, SUUUID sceneObject, nlohmann::json& json);

	void CreatePhysicsObjectsBehaviors(SceneUnitId id);
	void UpdateRenderablesFromGlobalPose(SceneUnitId id);
	void UpdatePhysicObjects(SceneUnitId id);

	//Contact callbacks
	void RegisterContactCallback(PhysicsBehavior behavior, JUUID object, std::function<void(JUUID, unsigned int)> callback);
	void UnregisterContactCallback(PhysicsBehavior behavior, JUUID object);
	void CallRegisteredCallbacks(PhysicsBehavior behavior, JUUID destObject, JUUID srcObject, unsigned int event);
	//Trigger Callback
	void RegisterTriggerContactCallback(TriggerID trigger, std::function<void(SUUUID, unsigned int)> callback);
	void UnregisterTriggerContactCallback(TriggerID trigger);
	void CallTriggerContactCallback(TriggerID trigger, SUUUID sceneObject, unsigned int event);
	//Character Callback
	void RegisterCharacterHitCallback(JUUID object, std::function<void(PxFilterData)> callback);
	void UnregisterCharacterHitCallback(JUUID object);
	void CallCharacterHitCallback(JUUID destObject, PxFilterData filterData);
	//Scripting
	void CreatePhysicObjectMemberFunctionTemplates(Isolate* isolate, SceneUnitId id);

	DEF_TEMPLATE_ID(PhysicObject, GetPhysicObject);
};

//using namespace Physics;
//DEF_TEMPLATE_ID_HASH(PhysicObject);

