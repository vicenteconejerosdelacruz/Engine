#include "pch.h"
#include "SceneController.h"
#include <Scene.h>

namespace Scene
{
	SODEF_FULL(SceneController);

#include <TrackUUID/JDef.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

#endif

#if defined(_EDITOR)
	void WriteSceneControllersJson(SceneUnitId id, nlohmann::json& json)
	{
#include <Editor/JSaveFile.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>
	}
#endif

	SceneController::SceneController(SceneUnitId id, nlohmann::json& json) : SceneObject(id, json)
	{
#include <Attributes/JInit.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

		RENAME_ON_DELETION(SceneController);
	}

#if defined(_EDITOR)
	void SceneController::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>
	}
#endif

	void SceneController::Initialize()
	{
		using namespace Physics;

#include <TrackUUID/JInsert.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>

		SceneObject::Initialize();
	}

	void SceneController::BindToScene()
	{
#include <TrackUUID/JInsert.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>
	}

	void SceneController::UnbindFromScene()
	{
#include <TrackUUID/JErase.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>
	}

	void SceneController::Destroy()
	{
#include <Attributes/JDestroy.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>


		SceneObject::Destroy();
	}

	void SceneControllerStep(SceneUnitId id, float step)
	{
	}

	void DestroySceneControllers()
	{
		for (auto& [id, container] : SceneControllerSUsceneObjects)
		{
			for (auto& [uuid, _] : container)
			{
				SceneControllerID ps = MAKESUUUID(id, uuid);
				DeleteSceneControllerSceneObject(ps);
			}
		}
#include <TrackUUID/JClear.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>
	}

	void DestroySceneControllers(SceneUnitId id)
	{
		std::set<JUUID> uuids;
		std::transform(SceneControllerSUsceneObjects.at(id).begin(), SceneControllerSUsceneObjects.at(id).end(), std::inserter(uuids, uuids.begin()), [](auto& pair) { return pair.first; });
		for (auto& uuid : uuids)
		{
			DeleteSceneControllerSceneObject(MAKESUUUID(id, uuid));
		}
#include <TrackUUID/JClearUnit.h>
#include <SceneControllerAtt.h>
#include <JEnd.h>
	}

	void DeleteSceneController(SceneUnitId id, JUUID uuid)
	{
		SceneControllerID ps = MAKESUUUID(id, uuid);
		ps->markedForDelete = true;
	}
}