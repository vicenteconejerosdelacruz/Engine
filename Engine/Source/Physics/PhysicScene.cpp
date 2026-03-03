#include "pch.h"
#include "PhysicScene.h"
#include <Scene.h>
#include <Physics.h>

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
	}

	void PhysicSceneStep(SceneUnitId id, float step)
	{
		if (GetCountFromPhysicScenes(id) == 0ULL) return;

		UpdatePhysicObjects(id, step);

		PhysicSceneID scene = MAKESUUUID(id, *GetPhysicScenes(id).begin());
		auto updateColors = [&](bool run, size_t flag, XMFLOAT4 color, auto& getPhOs)
			{
				if (!run || !scene->dirty(flag))
					return;

				auto phOs = getPhOs(id);
				for (auto& phO : phOs)
				{
					if (phO->overrideColor()) continue;
					phO->color(color);
					phO->UpdateRenderableColor();
				}

				scene->clean(flag);
			};
		updateColors(Editor::StaticBodiesSceneUnitRegistered(id), PhysicScene::Update_staticColor, scene->staticColor(), Editor::GetStaticBodies);
		updateColors(Editor::DynamicBodiesSceneUnitRegistered(id), PhysicScene::Update_dynamicColor, scene->dynamicColor(), Editor::GetDynamicBodies);
		updateColors(Editor::CharactersSceneUnitRegistered(id), PhysicScene::Update_characterColor, scene->characterColor(), Editor::GetCharacters);
		updateColors(Editor::TriggersSceneUnitRegistered(id), PhysicScene::Update_triggerColor, scene->triggerColor(), Editor::GetTriggers);

		if (step == 0.0f) return;

		scene->pxScene->simulate(1.0f / 60.0f);
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