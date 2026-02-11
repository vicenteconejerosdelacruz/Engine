#include "pch.h"
#include "PhysicScene.h"
#include <Scene.h>
#include <Physics.h>

#if defined(_EDITOR)
namespace Editor
{
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

		//		using namespace Templates;
		//
		//		if (!SoundTemplateExist(sound())) return;
		//
		//		soundEffectInstance = GetSoundEffectInstance(sound(), instanceFlags(), uuid());
		//
		//		if (nostd::bytesHas(instanceFlags(), SoundEffectInstance_Use3D))
		//		{
		//			audioEmitter.SetPosition(position());
		//			audioEmitter.SetOrientationFromQuaternion(rotationQ());
		//		}
		//		if (!std::get<0>(soundEffectInstance).empty() && autoPlay())
		//		{
		//#if defined(_EDITOR)
		//			if (Editor::IsPlaying(unit) && !Editor::IsPaused(unit))
		//#endif
		//				Play();
		//		}
		//#if defined(_EDITOR)
		//		SceneObject::BindToScene();
		//#endif
	}

	void PhysicScene::UnbindFromScene()
	{
#include <TrackUUID/JErase.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>

		//using namespace Templates;
		//
		//if (EffectExists())
		//{
		//	if (dirty(SoundFX::Update_sound))
		//	{
		//		std::string prevSoundUUID = UpdatePrevValues.at("sound");
		//		DestroySoundEffectInstance(prevSoundUUID, soundEffectInstance);
		//	}
		//	else
		//	{
		//		DestroySoundEffectInstance(sound(), soundEffectInstance);
		//	}
		//}
		//markedForDelete = true;
	}

	void PhysicScene::Destroy()
	{
#include <Attributes/JDestroy.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>
	}

	void PhysicSceneStep(SceneUnitId id, float step)
	{
		//		if (!SoundEffects.contains(id)) return;
		//
		//		auto& SoundFxs = SoundEffects.at(id);
		//		std::set<SoundFXID> sfxs;
		//		std::transform(SoundFxs.begin(), SoundFxs.end(), std::inserter(sfxs, sfxs.end()), [&](auto o) { return MAKESUUUID(id, o); });
		//
		//		std::for_each(sfxs.begin(), sfxs.end(), [step](auto sfx)
		//			{
		//				sfx->clear();
		//			}
		//		);
		//
		//		std::set<SoundFXID> sfxsDelete;
		//		std::copy_if(sfxs.begin(), sfxs.end(), std::inserter(sfxsDelete, sfxsDelete.end()), [](auto sfx)
		//			{
		//				return sfx->markedForDelete;
		//			}
		//		);
		//
		//		for (auto sfx : sfxsDelete)
		//		{
		//			EraseSoundFXFromSoundEffects(sfx.unit(), sfx.uuid());
		//			EraseSoundFXFromSound3DEffects(sfx.unit(), sfx.uuid());
		//			DeleteSoundFXSceneObject(sfx);
		//#if defined(_EDITOR)
		//			Editor::MarkScenePanelAssetsAsDirty();
		//#endif
		//		}
	}

	void DestroyPhysicScenes()
	{
		//for (auto& [id, container] : SoundFXSUsceneObjects)
		//{
		//	for (auto& [uuid, _] : container)
		//	{
		//		SoundFXID s = MAKESUUUID(id, uuid);
		//		DeleteSoundFXSceneObject(s);
		//	}
		//}
#include <TrackUUID/JClear.h>
#include <PhysicSceneAtt.h>
#include <JEnd.h>
	}

	void DestroyPhysicScene(SceneUnitId id)
	{
		//std::set<JUUID> uuids;
		//std::transform(SoundFXSUsceneObjects.at(id).begin(), SoundFXSUsceneObjects.at(id).end(), std::inserter(uuids, uuids.begin()), [](auto& pair) { return pair.first; });
		//for (auto& uuid : uuids)
		//{
		//	DeleteSoundFXSceneObject(MAKESUUUID(id, uuid));
		//}
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