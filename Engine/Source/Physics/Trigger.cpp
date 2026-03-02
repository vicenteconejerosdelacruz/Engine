#include "pch.h"
#include "Trigger.h"
#include <Scene.h>
#include <Physics.h>

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
		(*this)["behavior"] = PhysicsBehaviorToString.at(PB_Trigger);
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

		CreatePhysicObject();
	}

	void Trigger::UnbindFromScene()
	{
#include <TrackUUID/JErase.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}

	void Trigger::Destroy()
	{
		if (!physicObject.empty())
		{
			DestroyPhysicObject(physicObject());
		}

#include <Attributes/JDestroy.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}

	void Trigger::CreatePhysicObject()
	{
		nlohmann::json data =
		{
			{ "behavior", "Trigger" },
			{ "geometry", geometry() },
		};

		std::string pOname = name() + "-physicObject";
		physicObject = Physics::CreatePhysicObject(pOname, SUuuid(), data);
		physicObject->CreatePhysicsBehavior();
	}

#if defined(_EDITOR)
	BoundingBox Trigger::GetBoundingBox()
	{
		return BoundingBox(position(), { 0.1f,0.1f,0.1f });
	}

#endif

	void TriggersStep(SceneUnitId unit, float dt)
	{
#if defined(_EDITOR)
		using namespace Editor;
#endif
		auto& Triggers = GetTriggers(unit);
		std::set<TriggerID> tr;
		std::transform(Triggers.begin(), Triggers.end(), std::inserter(tr, tr.begin()), [&](auto o) { return MAKESUUUID(unit, o); });

		std::set<TriggerID> tr2Del;
		std::copy_if(tr.begin(), tr.end(), std::inserter(tr2Del, tr2Del.begin()), [](auto trg)
			{
				return trg->markedForDelete;
			}
		);

		for (auto trg : tr2Del)
		{
#if defined(_EDITOR)
#endif
			DestroyPhysicObject(trg->physicObject());
			EraseTriggerFromTriggers(FROMSUUUID(trg()));
			DeleteTriggerSceneObject(trg);
		}

#if defined(_EDITOR)

		std::set<TriggerID> trTransformation;
		std::copy_if(tr.begin(), tr.end(), std::inserter(trTransformation, trTransformation.begin()), [](auto trg)
			{
				return
					trg->dirty(Trigger::Update_position) ||
					trg->dirty(Trigger::Update_rotation) ||
					trg->dirty(Trigger::Update_scale);
			}
		);

		std::set<TriggerID> trColor;
		std::copy_if(tr.begin(), tr.end(), std::inserter(trColor, trColor.begin()), [](auto trg)
			{
				return trg->dirty(Trigger::Update_color);
			}
		);


		for (auto trg : trTransformation)
		{
			if (trg->dirty(Trigger::Update_scale))
			{
				trg->physicObject->DestroyPhisicsBehavior();
				trg->physicObject->CreatePhysicsBehavior();
			}
			else
			{
				trg->physicObject->UpdateGlobalPoseFromTrigger();
			}
			trg->clean(Trigger::Update_position);
			trg->clean(Trigger::Update_rotation);
			trg->clean(Trigger::Update_scale);
		}

		for (auto trg : trColor)
		{
			trg->physicObject->UpdateRenderableColor(trg->color());
			trg->clean(Trigger::Update_color);
		}
#endif
	}

	void DestroyTriggers()
	{
		for (auto& [id, container] : TriggerSUsceneObjects)
		{
			for (auto& [uuid, _] : container)
			{
				TriggerID t = MAKESUUUID(id, uuid);
				DeleteTriggerSceneObject(t);
			}
		}
#include <TrackUUID/JClear.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}

	void DestroyTriggers(SceneUnitId id)
	{
		std::set<JUUID> uuids;
		std::transform(TriggerSUsceneObjects.at(id).begin(), TriggerSUsceneObjects.at(id).end(), std::inserter(uuids, uuids.begin()), [](auto& pair) { return pair.first; });
		for (auto& uuid : uuids)
		{
			DeleteTriggerSceneObject(MAKESUUUID(id, uuid));
		}
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