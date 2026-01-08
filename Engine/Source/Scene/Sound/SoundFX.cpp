#include "pch.h"
#include "SoundFX.h"
#include <Scene.h>
#include <AudioSystem.h>
#include <Sound/Sound.h>
//#include <Renderer.h>
//#include <SceneObjectDef.h>

//extern std::unique_ptr<Renderer> renderer;

#if defined(_EDITOR)
namespace Editor
{
	extern void SelectSoundEffect(SceneUnitId id, JUUID suuid);
	extern JUUID CreateBillboardFromMaterials(SceneUnitId id, CameraSUUUID camera, std::string name, std::string material, std::string pickingMaterial);
	extern void RegisterBillboard(SceneUnitId id, JUUID sceneObject);
	//extern JUUID GetBillboard(JUUID sceneObject);
	extern void DestroyBillboard(SceneUnitId id, JUUID sceneObject);
	extern void MarkScenePanelAssetsAsDirty();
	extern void DeleteFromScenePanelSelection(SceneUnitId id, JUUID sceneObject);
	extern bool IsPlaying(SceneUnitId unit);
	extern bool IsPaused(SceneUnitId unit);
}
#endif

namespace Scene
{
	SODEF_FULL(SoundFX);

#include <TrackUUID/JDef.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#endif

#if defined(_EDITOR)

	void WriteSoundFXsJson(SceneUnitId id, nlohmann::json& json)
	{
#include <Editor/JSaveFile.h>
#include <SoundFXAtt.h>
#include <JEnd.h>
	}
#endif

	SoundFX::SoundFX(SceneUnitId id, nlohmann::json& json) : SceneObject(id, json)
	{
#include <Attributes/JInit.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <SoundFXAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void SoundFX::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <SoundFXAtt.h>
#include <JEnd.h>
	}
#endif

	void SoundFX::Initialize()
	{
#if defined(_EDITOR)
		using namespace Editor;
#endif
#include <TrackUUID/JInsert.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)
		if (instanceFlags() & SoundEffectInstance_Use3D)
		{
			RegisterBillboard(unit, uuid());
		}
#endif
	}

	void SoundFX::BindToScene()
	{
#include <TrackUUID/JInsert.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

		using namespace Templates;

		if (!SoundTemplateExist(sound())) return;

		std::unique_ptr<SoundJson>& stmp = GetSoundTemplate(sound());

		auto OnSoundChange = [this](JUUID sound)
			{
				UnbindFromScene();
				BindToScene();
			};
		soundEffectInstance = GetSoundEffectInstance(sound(), instanceFlags(), uuid(), OnSoundChange);

		if (nostd::bytesHas(instanceFlags(), SoundEffectInstance_Use3D))
		{
			audioEmitter.SetPosition(position());
			audioEmitter.SetOrientationFromQuaternion(rotationQ());
		}
		if (!std::get<0>(soundEffectInstance).empty() && autoPlay())
		{
#if defined(_EDITOR)
			if (Editor::IsPlaying(unit) && !Editor::IsPaused(unit))
#endif
				Play();
		}
#if defined(_EDITOR)
		SceneObject::BindToScene();
#endif
	}

	void SoundFX::UnbindFromScene()
	{
#include <TrackUUID/JErase.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

		using namespace Templates;

		if (GetEffect() != nullptr)
		{
			if (dirty(SoundFX::Update_sound))
			{
				std::string prevSoundUUID = UpdatePrevValues.at("sound");
				DestroySoundEffectInstance(prevSoundUUID, soundEffectInstance);
			}
			else
			{
				DestroySoundEffectInstance(sound(), soundEffectInstance);
			}
		}
		markedForDelete = true;
	}

	bool SoundFX::Play()
	{
		auto& sfxI = GetInstance();
		if (sfxI->GetState() == DirectX::SoundState::PLAYING) return false;
		sfxI->SetVolume(volume());
		sfxI->Play(loop());
		time = 0.0f;
		hasStarted = true;
		return true;
	}

	bool SoundFX::Stop()
	{
		auto& sfxI = GetInstance();
		if (sfxI->GetState() == DirectX::SoundState::STOPPED) return false;
		sfxI->Stop();
		time = 0.0f;
		return true;
	}

	bool SoundFX::Pause()
	{
		auto& sfxI = GetInstance();
		if (sfxI->GetState() == DirectX::SoundState::PAUSED) return false;
		sfxI->Pause();
		return true;
	}

	bool SoundFX::Resume()
	{
		auto& sfxI = GetInstance();
		if (sfxI->GetState() != DirectX::SoundState::PAUSED) return false;
		sfxI->Resume();
		return true;
	}

	void SoundFX::Step(float step)
	{
		using namespace std;

		if (!IsPlaying()) return;

		time += step;
		float duration = Duration();
		if (!loop())
		{
			time = min(time, duration);
		}
		else if (time > duration)
		{
			time = fmodf(time, duration);
		}
	}

	void SoundFX::Destroy()
	{
#include <Attributes/JDestroy.h>
#include <SoundFXAtt.h>
#include <JEnd.h>
	}

	XMVECTOR SoundFX::rotationQ()
	{
		XMFLOAT3 rotV = rotation();
		float roll, pitch, yaw;
		pitch = rotV.x; yaw = rotV.y; roll = rotV.z;
		XMVECTOR rotQ = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
		return rotQ;
	}

	XMMATRIX SoundFX::world()
	{
		XMFLOAT3 posV = position();
		XMMATRIX rotationM = XMMatrixRotationQuaternion(rotationQ());
		XMMATRIX positionM = XMMatrixTranslationFromVector({ posV.x, posV.y, posV.z });
		return XMMatrixMultiply(rotationM, positionM);
	}

	XMVECTOR SoundFX::fw()
	{
		FXMVECTOR dir = { 0.0f, 0.0f, 1.0f,0.0f };
		XMVECTOR fw = XMVector3Normalize(XMVector3Rotate(dir, rotationQ()));
		return fw;
	}

	std::unique_ptr<DirectX::SoundEffect>& SoundFX::GetEffect()
	{
		return GetSoundEffect(sound());
	}

	std::unique_ptr<DirectX::SoundEffectInstance>& SoundFX::GetInstance()
	{
		return std::get<1>(soundEffectInstance);
	}

	void SoundFX::UpdateEmmiter()
	{
		using namespace AudioSystem;
		GetInstance()->Apply3D(GetAudioListener(), audioEmitter, false);
	}

#if defined(_EDITOR)
	JUUID SoundFX::CreateBillboard(CameraSUUUID camera)
	{
		if (!(instanceFlags() & SoundEffectInstance_Use3D)) return "";

		JUUID uuid = Editor::CreateBillboardFromMaterials(unit, camera, at("name"), "SoundEffect", "SoundEffectPicking");
		UpdateBillboard(uuid);
		RenderableSUUUID bb = MAKESUUUID(unit, uuid);
		bb->OnPick = [this] {Editor::SelectSoundEffect(unit, this->uuid()); };
		return uuid;
	}

	void SoundFX::UpdateBillboard(JUUID uuid)
	{
		assert(!uuid.empty());
		if (uuid.empty()) return;

		auto& scene = GetSceneUnit(unit);

		XMFLOAT3 baseColor = { 1.0f,1.0f,1.0f };
		RenderableSUUUID bb = MAKESUUUID(unit, uuid);
		bb->position(position());
		bb->WriteConstantsBuffer<XMFLOAT3>("baseColor", baseColor, scene->Frame());
		bb->WriteConstantsBuffer(scene->Frame());
	}

	BoundingBox SoundFX::GetBoundingBox()
	{
		return BoundingBox(position(), { 0.1f,0.1f,0.1f });
	}
	bool SoundFX::CanInteractWithGizmo(ImGuizmo::OPERATION operation)
	{
		return nostd::bytesHas(instanceFlags(), SoundEffectInstance_Use3D);
	}
#endif

	void SoundFXsStep(SceneUnitId id, float step)
	{
		auto& SoundFxs = SoundEffects.at(id);
		std::set<SoundFXSUUUID> sfxs;
		std::transform(SoundFxs.begin(), SoundFxs.end(), std::inserter(sfxs, sfxs.end()), [&](auto o) { return MAKESUUUID(id, o); });

		//std::for_each(sfxs.begin(), sfxs.end(), [step](auto sfx)
		//{
		//		sfx->Step(step);
		//#if defined(_EDITOR)
		//		if ((sfx->instanceFlags() & SoundEffectInstance_Use3D))
		//		{
		//			JUUID bbuuid = Editor::GetBillboard(sfx());
		//			if (!bbuuid.empty())
		//			{
		//				sfx->UpdateBillboard(bbuuid);
		//			}
		//		}
		//#endif
		//}
		//);

		std::set<SoundFXSUUUID> sfxsDestroyI;
		//std::set<SoundFXUUID> sfxsCreateI;
		std::copy_if(sfxs.begin(), sfxs.end(), std::inserter(sfxsDestroyI, sfxsDestroyI.end()), [](auto sfx)
			{
				return sfx->dirty(SoundFX::Update_sound) || sfx->dirty(SoundFX::Update_loop) || sfx->dirty(SoundFX::Update_instanceFlags) || (!sfx->IsPlaying() && sfx->HasStarted() && sfx->destroyOnCompletion());
			}
		);

		//		std::copy_if(sfxs.begin(), sfxs.end(), std::inserter(sfxsCreateI, sfxsCreateI.end()), [](auto sfx)
		//			{
		//				if (!sfx->dirty(SoundFX::Update_sound) && (sfx->dirty(SoundFX::Update_loop) || sfx->dirty(SoundFX::Update_instanceFlags)))
		//					return true;
		//
		//				return (sfx->dirty(SoundFX::Update_sound) && !sfx->sound().empty());
		//			}
		//		);

		std::for_each(sfxsDestroyI.begin(), sfxsDestroyI.end(), [](auto sfx)
			{
				sfx->UnbindFromScene();
			}
		);

		//		std::for_each(sfxsCreateI.begin(), sfxsCreateI.end(), [](auto sfx)
		//			{
		//				sfx->BindToScene();
		//			}
		//		);

		std::for_each(sfxs.begin(), sfxs.end(), [step](auto sfx)
			{
				sfx->clear();
			}
		);

		std::set<SoundFXSUUUID> sfxsDelete;
		std::copy_if(sfxs.begin(), sfxs.end(), std::inserter(sfxsDelete, sfxsDelete.end()), [](auto sfx)
			{
				return sfx->markedForDelete;
			}
		);

		for (auto sfx : sfxsDelete)
		{
			EraseSoundFXFromSoundEffects(sfx.unit(), sfx.uuid());
			EraseSoundFXFromSound3DEffects(sfx.unit(), sfx.uuid());
			DeleteSoundFXSUSceneObject(sfx.unit(), sfx.uuid());
#if defined(_EDITOR)
			Editor::MarkScenePanelAssetsAsDirty();
			Editor::DeleteFromScenePanelSelection(sfx.unit(), sfx.uuid());
#endif
			//std::shared_ptr<SoundFX> soundfx = sfx;
			//SafeDeleteSceneObject(soundfx);
		}
	}

	void DestroySoundEffects()
	{
		for (auto& [id, container] : SoundFXSUsceneObjects)
		{
			for (auto& [uuid, _] : container)
			{
				SoundFXSUUUID s = MAKESUUUID(id, uuid);
				DeleteSoundFXSUSceneObject(s.unit(), s.uuid());
			}
		}
		//auto uuids = nostd::GetUUIDS(SoundFXsceneObjects);
		//for (SoundFXUUID uuid : uuids)
		//{
		//	DeleteSoundFXSUSceneObject(uuid->unit, uuid());
		//}
#include <TrackUUID/JClear.h>
#include <SoundFXAtt.h>
#include <JEnd.h>
	}

	void DestroySoundEffects(SceneUnitId id)
	{
		for (auto& [uuid, _] : SoundFXSUsceneObjects.at(id))
		{
			SoundFXSUUUID s = MAKESUUUID(id, uuid);
			DeleteSoundFXSUSceneObject(s.unit(), s.uuid());
		}
		//auto uuids = nostd::GetUUIDS(SoundFXsceneObjects);
		//for (SoundFXUUID uuid : uuids)
		//{
		//	if (uuid->unit != unit) continue;
		//	DeleteSoundFXSUSceneObject(uuid->unit, uuid());
		//}
#include <TrackUUID/JClearUnit.h>
#include <SoundFXAtt.h>
#include <JEnd.h>
	}

	void DeleteSoundFX(SceneUnitId id, JUUID uuid)
	{
		SoundFXSUUUID sfx = MAKESUUUID(id, uuid);
#if defined(_EDITOR)
		Editor::DestroyBillboard(id, uuid);
#endif
		sfx->markedForDelete = true;
	}

	void PlaySounds(SceneUnitId id)
	{
		for (auto& [uuid, _] : SoundFXSUsceneObjects.at(id))
		{
			SoundFXSUUUID sfx = MAKESUUUID(id, uuid);
			sfx->Play();
		}
	}

	void PauseSounds(SceneUnitId id)
	{
		for (auto& [uuid, _] : SoundFXSUsceneObjects.at(id))
		{
			SoundFXSUUUID sfx = MAKESUUUID(id, uuid);
			sfx->Pause();
		}
	}

	void ResumeSounds(SceneUnitId id)
	{
		for (auto& [uuid, _] : SoundFXSUsceneObjects.at(id))
		{
			SoundFXSUUUID sfx = MAKESUUUID(id, uuid);
			sfx->Resume();
		}
	}

	void StopSounds(SceneUnitId id)
	{
		for (auto& [uuid, _] : SoundFXSUsceneObjects.at(id))
		{
			SoundFXSUUUID sfx = MAKESUUUID(id, uuid);
			sfx->Stop();
		}
	}
}