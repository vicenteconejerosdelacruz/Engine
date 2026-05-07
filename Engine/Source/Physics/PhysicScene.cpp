#include "pch.h"
#include "PhysicScene.h"
#include <Scene.h>
#include <Physics.h>

extern float gameUpdateFrequency;

#if defined(_EDITOR)
namespace Editor
{
	extern bool StaticBodiesSceneUnitRegistered(SceneUnitId id);
	extern bool DynamicBodiesSceneUnitRegistered(SceneUnitId id);
	extern bool CharactersSceneUnitRegistered(SceneUnitId id);
	extern bool TriggersSceneUnitRegistered(SceneUnitId id);

	//Physics Objects list
	extern std::set<PhysicObjectID> GetStaticBodies(SceneUnitId id);
	extern std::set<PhysicObjectID> GetDynamicBodies(SceneUnitId id);
	extern std::set<PhysicObjectID> GetCharacters(SceneUnitId id);
	extern std::set<PhysicObjectID> GetTriggers(SceneUnitId id);
}
#endif

namespace Scene
{
	SODEF_FULL(PhysicScene);

#include <TrackUUID/JDef.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

#endif

#if defined(_EDITOR)
	void WritePhysicSceneJson(SceneUnitId id, nlohmann::json& json)
	{
#include <Editor/JSaveFile.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>
	}
#endif

	PhysicScene::PhysicScene(SceneUnitId id, nlohmann::json& json) : SceneObject(id, json)
	{
#include <Attributes/JInit.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

		RENAME_ON_DELETION(PhysicScene);
	}

#if defined(_EDITOR)
	void PhysicScene::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>
	}
#endif

	void PhysicScene::Initialize()
	{
		using namespace Physics;

#include <TrackUUID/JInsert.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

		CreatePhysicsScene(SUuuid());
		SceneObject::Initialize();
	}

	void PhysicScene::BindToScene()
	{
#include <TrackUUID/JInsert.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>
	}

	void PhysicScene::UnbindFromScene()
	{
#include <TrackUUID/JErase.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>
	}

	void PhysicScene::Destroy()
	{
#include <Attributes/JDestroy.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

		if (pxControllerManager)
		{
			PX_RELEASE(pxControllerManager);
		}
		if (pxScene)
		{
			PX_RELEASE(pxScene);
		}

		SceneObject::Destroy();
	}

	void PhysicSceneStep(SceneUnitId id, float step)
	{
		if (GetCountFromPhysicScenes(id) == 0ULL) return;

		UpdatePhysicObjects(id);

		if (step == 0.0f) return;

		PhysicSceneID scene = MAKESUUUID(id, *GetPhysicScenes(id).begin());
		scene->pxScene->simulate(gameUpdateFrequency);
		scene->pxScene->fetchResults(true);
		UpdateRenderablesFromGlobalPose(id);
	}

	void DestroyPhysicScenes()
	{
		for (auto& [id, container] : PhysicSceneSUsceneObjects)
		{
			for (auto& [uuid, _] : container)
			{
				PhysicSceneID ps = MAKESUUUID(id, uuid);
				DeletePhysicSceneSceneObject(ps);
			}
		}
#include <TrackUUID/JClear.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>
	}

	void DestroyPhysicScenes(SceneUnitId id)
	{
		std::set<JUUID> uuids;
		std::transform(PhysicSceneSUsceneObjects.at(id).begin(), PhysicSceneSUsceneObjects.at(id).end(), std::inserter(uuids, uuids.begin()), [](auto& pair) { return pair.first; });
		for (auto& uuid : uuids)
		{
			DeletePhysicSceneSceneObject(MAKESUUUID(id, uuid));
		}
#include <TrackUUID/JClearUnit.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>
	}

	void DeletePhysicScene(SceneUnitId id, JUUID uuid)
	{
		PhysicSceneID ps = MAKESUUUID(id, uuid);
		ps->markedForDelete = true;
	}
}