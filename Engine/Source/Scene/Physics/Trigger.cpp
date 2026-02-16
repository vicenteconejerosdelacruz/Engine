#include "pch.h"
#include "Trigger.h"
#include <Scene.h>
#include <Physics.h>

#if defined(_EDITOR)
namespace Editor
{
}
#endif

namespace Scene
{
	SODEF_FULL(Trigger);

#include <TrackUUID/JDef.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#endif

#if defined(_EDITOR)
	void WriteTriggersJson(SceneUnitId id, nlohmann::json& json)
	{
#include <Editor/JSaveFile.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}
#endif

	Trigger::Trigger(SceneUnitId id, nlohmann::json& json) : SceneObject(id, json)
	{
#include <Attributes/JInit.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void Trigger::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}
#endif

	void Trigger::Initialize()
	{
#include <TrackUUID/JInsert.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}

	void Trigger::BindToScene()
	{
#include <TrackUUID/JInsert.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}

	void Trigger::UnbindFromScene()
	{
#include <TrackUUID/JErase.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}

	void Trigger::Destroy()
	{
#include <Attributes/JDestroy.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}

	void DestroyTriggers()
	{
#include <TrackUUID/JClear.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}

	void DestroyTrigger(SceneUnitId id)
	{
#include <TrackUUID/JClearUnit.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}

	void DeleteTrigger(SceneUnitId id, JUUID uuid)
	{
		TriggerID tg = MAKESUUUID(id, uuid);
		tg->markedForDelete = true;
	}
}