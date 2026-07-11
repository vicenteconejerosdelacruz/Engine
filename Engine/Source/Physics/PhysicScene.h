#pragma once

#include <Scene.h>
#include <SceneObject.h>
#include <SceneUnitId.h>
#include <PxPhysicsAPI.h>
#include <ContactCallback.h>

enum SceneObjectType;

namespace Scene
{
	using namespace physx;
	using namespace Physics;

#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include "PhysicSceneAtt.h"
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include "PhysicSceneAtt.h"
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include "PhysicSceneAtt.h"
#include <JEnd.h>

#include <Creator/JRequired.h>
#include "PhysicSceneAtt.h"
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include "PhysicSceneAtt.h"
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include "PhysicSceneAtt.h"
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include "PhysicSceneAtt.h"
#include <JEnd.h>

#endif
	struct PhysicScene : SceneObject
	{
		inline static const SceneObjectType sceneObjectType = SO_PhysicScenes;

#include <Attributes/JFlags.h>
#include "PhysicSceneAtt.h"
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include "PhysicSceneAtt.h"
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include "PhysicSceneAtt.h"
#include <JEnd.h>

		PhysicScene(SceneUnitId id, nlohmann::json& json);
		~PhysicScene() { Destroy(); }
		void Initialize() override;
		virtual void BindToScene();
		virtual void UnbindFromScene();

		virtual void Destroy();
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif

		DeleteHook markedForDelete;
		PxScene* pxScene = nullptr;
		PxControllerManager* pxControllerManager = nullptr;
		std::unique_ptr<ContactCallback> contactCallback;
	};

	SODECL_FULL(PhysicScene);

#include <TrackUUID/JDecl.h>
#include "PhysicSceneAtt.h"
#include <JEnd.h>

	void FetchPhysicsScenesResults(SceneUnitId id, float step);
	void SimulatePhysicScenes(SceneUnitId id, float step);
	void DestroyPhysicScenes();
	void DestroyPhysicScenes(SceneUnitId id);
	void DeletePhysicScene(SceneUnitId id, JUUID uuid);
#if defined(_EDITOR)
	void WritePhysicSceneJson(SceneUnitId id, nlohmann::json& json);
#endif
}

using namespace Scene;
DEF_SCENEOBJECT_ID_HASH(PhysicScene);