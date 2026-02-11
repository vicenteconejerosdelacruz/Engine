#pragma once

#include <Scene.h>
#include <SceneObject.h>
#include <SceneUnitId.h>
#include <PxPhysicsAPI.h>

namespace Scene
{
	using namespace physx;

#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

#endif

	struct PhysicScene : SceneObject
	{
		inline static const SceneObjectType sceneObjectType = SO_PhysicScenes;

#include <Attributes/JFlags.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

		PhysicScene(SceneUnitId id, nlohmann::json& json);
		~PhysicScene() { Destroy(); }
		virtual void Initialize();
		virtual void BindToScene();
		virtual void UnbindFromScene();

		void Destroy();
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif

		bool markedForDelete = false;
		PxScene* pxScene = nullptr;
	};

	SODECL_FULL(PhysicScene);

#include <TrackUUID/JDecl.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

	void PhysicSceneStep(SceneUnitId id, float step);
	void DestroyPhysicScenes();
	void DestroyPhysicScene(SceneUnitId id);
	void DeletePhysicScene(SceneUnitId id, JUUID uuid);
#if defined(_EDITOR)
	void WritePhysicSceneJson(SceneUnitId id, nlohmann::json& json);
#endif
}

using namespace Scene;
DEF_SCENEOBJECT_ID_HASH(PhysicScene);