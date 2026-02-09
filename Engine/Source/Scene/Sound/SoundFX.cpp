#include "pch.h"
#include "SoundFX.h"
#include <Scene.h>
#include <AudioSystem.h>
#include <Sound/Sound.h>

#if defined(_EDITOR)
namespace Editor
{
	extern void SelectSoundEffect(SoundFXID soundfx);
	extern RenderableID CreateBillboardFromMaterials(SceneUnitId id, CameraID camera, std::string name, std::string material, std::string pickingMaterial);
	extern void RegisterBillboard(SceneUnitId id, JUUID sceneObject);
	extern void DestroyBillboard(SceneUnitId id, JUUID sceneObject);
	extern void MarkScenePanelAssetsAsDirty();
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
		if (instanceFlags() & SoundEffectInstance_Use3D && !SceneIsIsolated(unit))
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

		soundEffectInstance = GetSoundEffectInstance(sound(), instanceFlags(), uuid());

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

		if (EffectExists())
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

	bool SoundFX::EffectExists()
	{
		return SoundEffectExists(sound());
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
	RenderableID SoundFX::CreateBillboard(CameraID camera)
	{
		if (!(instanceFlags() & SoundEffectInstance_Use3D)) return RenderableID();

		RenderableID bb = Editor::CreateBillboardFromMaterials(unit, camera, at("name"), "SoundEffect", "SoundEffectPicking");
		UpdateBillboard(bb);
		bb->OnPick = [&] {Editor::SelectSoundEffect(SUuuid()); };
		return bb;
	}

	void SoundFX::UpdateBillboard(RenderableID renderable)
	{
		assert(!renderable.empty());
		if (renderable.empty()) return;

		auto& scene = GetSceneUnit(unit);

		XMFLOAT3 baseColor = { 1.0f,1.0f,1.0f };
		renderable->position(position());
		renderable->WriteConstantsBuffer<XMFLOAT3>("baseColor", baseColor, scene->Frame());
		renderable->WriteConstantsBuffer(scene->Frame());
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
		if (!SoundEffects.contains(id)) return;

		auto& SoundFxs = SoundEffects.at(id);
		std::set<SoundFXID> sfxs;
		std::transform(SoundFxs.begin(), SoundFxs.end(), std::inserter(sfxs, sfxs.end()), [&](auto o) { return MAKESUUUID(id, o); });

		std::for_each(sfxs.begin(), sfxs.end(), [step](auto sfx)
			{
				sfx->clear();
			}
		);

		std::set<SoundFXID> sfxsDelete;
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
#endif
		}
	}

	void DestroySoundEffects()
	{
		for (auto& [id, container] : SoundFXSUsceneObjects)
		{
			for (auto& [uuid, _] : container)
			{
				SoundFXID s = MAKESUUUID(id, uuid);
				DeleteSoundFXSUSceneObject(s.unit(), s.uuid());
			}
		}
#include <TrackUUID/JClear.h>
#include <SoundFXAtt.h>
#include <JEnd.h>
	}

	void DestroySoundEffects(SceneUnitId id)
	{
		std::set<JUUID> uuids;
		std::transform(SoundFXSUsceneObjects.at(id).begin(), SoundFXSUsceneObjects.at(id).end(), std::inserter(uuids, uuids.begin()), [](auto& pair) { return pair.first; });
		for (auto& uuid : uuids)
		{
			DeleteSoundFXSUSceneObject(id, uuid);
		}
#include <TrackUUID/JClearUnit.h>
#include <SoundFXAtt.h>
#include <JEnd.h>
	}

	void DeleteSoundFX(SceneUnitId id, JUUID uuid)
	{
		SoundFXID sfx = MAKESUUUID(id, uuid);
#if defined(_EDITOR)
		Editor::DestroyBillboard(id, uuid);
#endif
		sfx->markedForDelete = true;
	}

	void PlaySounds(SceneUnitId id)
	{
		for (auto& [uuid, _] : SoundFXSUsceneObjects.at(id))
		{
			SoundFXID sfx = MAKESUUUID(id, uuid);
			if (sfx->markedForDelete) continue;
			sfx->Play();
		}
	}

	void PauseSounds(SceneUnitId id)
	{
		for (auto& [uuid, _] : SoundFXSUsceneObjects.at(id))
		{
			SoundFXID sfx = MAKESUUUID(id, uuid);
			sfx->Pause();
		}
	}

	void ResumeSounds(SceneUnitId id)
	{
		for (auto& [uuid, _] : SoundFXSUsceneObjects.at(id))
		{
			SoundFXID sfx = MAKESUUUID(id, uuid);
			if (sfx->markedForDelete) continue;
			sfx->Resume();
		}
	}

	void StopSounds(SceneUnitId id)
	{
		for (auto& [uuid, _] : SoundFXSUsceneObjects.at(id))
		{
			SoundFXID sfx = MAKESUUUID(id, uuid);
			sfx->Stop();
		}
	}
}