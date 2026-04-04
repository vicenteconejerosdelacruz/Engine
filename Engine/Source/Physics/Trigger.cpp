#include "pch.h"
#include "Trigger.h"
#include <Scene.h>
#include <Physics.h>

namespace Editor
{
	extern void UnRegisterTrigger(PhysicObjectID phO);
};

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

	void Trigger::create_rotation(XMFLOAT3 v)
	{
		if (!contains("rotation"))
		{
			rotation(v);
		}
		else
		{
			updateRotationQ();
		}
	}

	void Trigger::rotation(XMFLOAT3 v)
	{
		(*this)["rotation"] = FromXMFLOAT3(v);
		updateRotationQ();
	}

	void Trigger::updateRotationQ()
	{
		XMFLOAT3 v = rotation();
		rotationQuaternion = XMQuaternionRotationRollPitchYaw(
			XMConvertToRadians(v.x),
			XMConvertToRadians(v.y),
			XMConvertToRadians(v.z)
		);
		if (!physicObject.empty())
		{
			physicObject->UpdateGlobalPoseFromTrigger();
		}
	}

	XMVECTOR Trigger::rotationQ()
	{
		return rotationQuaternion;
	}

	void Trigger::rotationQ(XMVECTOR q)
	{
		rotationQuaternion = q;
		if (!physicObject.empty())
		{
			physicObject->UpdateGlobalPoseFromTrigger();
		}
	}

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
		RENAME_ON_DELETION(Trigger);
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
		UnregisterTriggerContactCallback(SUuuid());

#include <Attributes/JDestroy.h>
#include <TriggerAtt.h>
#include <JEnd.h>

		SceneObject::Destroy();
	}

	void Trigger::CreatePhysicObject()
	{
		nlohmann::json data =
		{
			{ "behavior", "Trigger" },
			{ "geometry", geometry() },
			{ "color", FromXMFLOAT4(color()) },
			{ "overrideColor", overrideColor() },
			{ "skipRendering", skipRendering() },
			{ "objectMask", objectMask() },
			{ "collisionMask", collisionMask() }
		};

		std::string pOname = name() + "-physicObject";
		physicObject = Physics::CreatePhysicObject(pOname, SUuuid(), data);
		physicObject->CreatePhysicsBehavior();
#if defined(_EDITOR)
		physicObject->CreatePhysicsAvatar();
#endif
		RegisterTriggerContactCallback(SUuuid(), [&](SUUUID otherObject, unsigned int event) { OnTriggerEvent(otherObject, event); });
	}

#if defined(_EDITOR)
	BoundingBox Trigger::GetBoundingBox()
	{
		return BoundingBox(position(), { 0.1f,0.1f,0.1f });
	}

#endif

	void Trigger::OnTriggerEvent(SUUUID sceneObject, unsigned int event)
	{
		using namespace Scripting;
		SceneObject* so = GetSceneObjectPointer(sceneObject);

		auto current_bindings = bindings();

		auto extra_bindings = so->GetScriptBindings();
		bool add_extra = false;
		if (extra_bindings.size() > 0ULL)
		{
			std::set<JNAME> skips;
			std::transform(current_bindings.begin(), current_bindings.end(), std::inserter(skips, skips.begin()), [&](ScriptBinding& sb)
				{
					return sb.bindingName;
				}
			);
			for (auto& sb : extra_bindings)
			{
				if (skips.contains(sb.bindingName))
					continue;
				current_bindings.push_back(sb);
				add_extra = true;
			}
			bindings(current_bindings);
		}

		if (event & PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			Scripting::RunScript(onEnter(), SUuuid());
		}
		else if (event & PxPairFlag::eNOTIFY_TOUCH_LOST)
		{
			Scripting::RunScript(onLeave(), SUuuid());
		}

		bindings(current_bindings);
	}

	//Scripting
	v8_templates_creators Trigger::GetV8TemplatesCreators()
	{
		v8_templates_creators creators;
#include <Attributes/JV8Templates.h>
#include <TriggerAtt.h>
#include <JEnd.h>
		return creators;
	}

	v8_context_creators Trigger::GetV8ContextCreators()
	{
		v8_context_creators creators;
#include <Attributes/JV8Context.h>
#include <TriggerAtt.h>
#include <JEnd.h>
		return creators;
	}

	void TriggersStep(SceneUnitId unit)
	{
#if defined(_EDITOR)
		using namespace Editor;
#endif
		auto& Triggers = GetTriggers(unit);
		std::set<TriggerID> tr;
		std::transform(Triggers.begin(), Triggers.end(), std::inserter(tr, tr.begin()), [&](auto o) { return MAKESUUUID(unit, o); });

		auto checkForDelete = [](TriggerID t)
			{
				if (!t->markedForDelete) return;
				PhysicObjectID phO = t->physicObject();
				phO->DestroyPhysicsBehavior();
#if defined(_EDITOR)
				phO->DestroyPhysicsAvatar();
#endif
				DestroyPhysicObject(phO());
				t->physicObject.clear();
				t->clear();
				EraseTriggerFromTriggers(FROMSUUUID(t()));
				DeleteTriggerSceneObject(t);
			};
		auto checkForPosRot = [](TriggerID t)
			{
				if (!t->dirty({ Trigger::Update_position,Trigger::Update_rotation })) return;

				t->updateRotationQ();
				t->physicObject->UpdateGlobalPoseFromTrigger();
#if defined(_EDITOR)
				t->physicObject->UpdatePhysicsAvatarTransformation();
#endif
				t->clean({ Trigger::Update_position,Trigger::Update_rotation });
			};
		auto checkForScale = [](TriggerID t)
			{
				if (!t->dirty(Trigger::Update_scale)) return;
				t->physicObject->DestroyPhysicsBehavior();
				t->physicObject->CreatePhysicsBehavior();
#if defined(_EDITOR)
				t->physicObject->UpdatePhysicsAvatarTransformation();
#endif
				t->clean(Trigger::Update_scale);
			};
#if defined(_EDITOR)
		auto checkForColor = [](TriggerID t)
			{
				if (!t->dirty({ Trigger::Update_overrideColor, Trigger::Update_color })) return;

				t->physicObject->color(t->color());
				t->physicObject->overrideColor(t->overrideColor());

				t->clean({ Trigger::Update_overrideColor, Trigger::Update_color });
			};
		auto checkForSkipRendering = [](TriggerID t)
			{
				if (!t->dirty(Trigger::Update_skipRendering)) return;

				t->physicObject->skipRendering(t->skipRendering());

				t->clean(Trigger::Update_skipRendering);
			};
#endif
		auto checkObjectMask = [](TriggerID t)
			{
				if (!t->dirty(Trigger::Update_objectMask)) return;

				t->physicObject->objectMask(t->objectMask());

				t->clean(Trigger::Update_objectMask);
			};
		auto checkCollisionMask = [](TriggerID t)
			{
				if (!t->dirty(Trigger::Update_collisionMask)) return;

				t->physicObject->collisionMask(t->collisionMask());

				t->clean(Trigger::Update_collisionMask);
			};
		std::for_each(tr.begin(), tr.end(), checkForPosRot);
		std::for_each(tr.begin(), tr.end(), checkForScale);
#if defined(_EDITOR)
		std::for_each(tr.begin(), tr.end(), checkForColor);
		std::for_each(tr.begin(), tr.end(), checkForSkipRendering);
#endif
		std::for_each(tr.begin(), tr.end(), checkObjectMask);
		std::for_each(tr.begin(), tr.end(), checkCollisionMask);
		std::for_each(tr.begin(), tr.end(), checkForDelete);
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