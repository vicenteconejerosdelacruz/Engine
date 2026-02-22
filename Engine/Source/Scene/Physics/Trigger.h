#pragma once

#include <Scene.h>
#include <SceneObject.h>
#include <SceneUnitId.h>
#include <PxPhysicsAPI.h>

enum SceneObjectType;

namespace Physics
{
	DEF_TEMPLATE_ID_DEP(PhysicObject, GetPhysicObject);
}

namespace Scene
{
	using namespace physx;
	using namespace Physics;

#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#endif

	struct Trigger : SceneObject
	{
		inline static const SceneObjectType sceneObjectType = SO_Triggers;

#include <Attributes/JFlags.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <TriggerAtt.h>
#include <JEnd.h>

		Trigger(SceneUnitId id, nlohmann::json& json);
		~Trigger() { Destroy(); }
		virtual void Initialize();
		virtual void BindToScene();
		virtual void UnbindFromScene();

		void Destroy();

		void CreatePhysicObject();
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
		nlohmann::json CreateRenderableTrigger(std::string name, JUUID uuid, JUUID camId, std::string material);
		void CreateRenderableTrigger();
		virtual bool CanInteractWithGizmo(ImGuizmo::OPERATION operation) { return true; }
		BoundingBox GetBoundingBox();
#endif

		bool markedForDelete = false;
		PhysicObjectID physicObject;
#if defined(_EDITOR)
		RenderableID renderableShape;
		RenderableID renderableLines;
#endif
	};

	SODECL_FULL(Trigger);

#include <TrackUUID/JDecl.h>
#include <TriggerAtt.h>
#include <JEnd.h>

	void TriggersStep(SceneUnitId id, float dt);
	void DestroyTriggers();
	void DestroyTrigger(SceneUnitId id);
	void DeleteTrigger(SceneUnitId id, JUUID uuid);
#if defined(_EDITOR)
	void WriteTriggersJson(SceneUnitId id, nlohmann::json& json);
#endif
}

using namespace Scene;
DEF_SCENEOBJECT_ID_HASH(Trigger);