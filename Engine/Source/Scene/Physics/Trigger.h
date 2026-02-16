#pragma once

#include <Scene.h>
#include <SceneObject.h>
#include <SceneUnitId.h>
#include <PxPhysicsAPI.h>

enum SceneObjectType;

namespace Scene
{
	using namespace physx;

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
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif

		bool markedForDelete = false;
	};

	SODECL_FULL(Trigger);

#include <TrackUUID/JDecl.h>
#include <TriggerAtt.h>
#include <JEnd.h>

	void DestroyTriggers();
	void DestroyTrigger(SceneUnitId id);
	void DeleteTrigger(SceneUnitId id, JUUID uuid);
#if defined(_EDITOR)
	void WriteTriggersJson(SceneUnitId id, nlohmann::json& json);
#endif
}

using namespace Scene;
DEF_SCENEOBJECT_ID_HASH(Trigger);