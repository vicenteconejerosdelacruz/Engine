#pragma once

#include <Scene.h>
#include <SceneObject.h>
#include <SceneUnitId.h>

enum SceneObjectType;

namespace Scene
{
	using namespace physx;
	using namespace Physics;

#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

#endif
	struct SceneController : SceneObject
	{
		inline static const SceneObjectType sceneObjectType = SO_SceneControllers;

#include <Attributes/JFlags.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

		SceneController(SceneUnitId id, nlohmann::json& json);
		~SceneController() { Destroy(); }
		virtual void Initialize();
		virtual void BindToScene();
		virtual void UnbindFromScene();

		virtual void Destroy();
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif

		DeleteHook markedForDelete;
	};

	SODECL_FULL(SceneController);

#include <TrackUUID/JDecl.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

	void SceneControllerStep(SceneUnitId id, float step);
	void DestroySceneControllers();
	void DestroySceneControllers(SceneUnitId id);
	void DeleteSceneController(SceneUnitId id, JUUID uuid);
#if defined(_EDITOR)
	void WriteSceneControllersJson(SceneUnitId id, nlohmann::json& json);
#endif
}

using namespace Scene;
DEF_SCENEOBJECT_ID_HASH(SceneController);