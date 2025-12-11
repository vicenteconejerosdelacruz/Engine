#include "pch.h"
#include "SoundFX.h"
#include <Scene.h>
#include <AudioSystem.h>
#include <Sound/Sound.h>
#include <Renderer.h>
#include <SceneObjectDef.h>

extern std::unique_ptr<Renderer> renderer;

#if defined(_EDITOR)
namespace Editor
{
	extern void SelectSoundEffect(JUUID suuid);
	extern JUUID CreateBillboardFromMaterials(CameraUUID camera, std::string name, std::string material, std::string pickingMaterial);
	extern void RegisterBillboard(JUUID sceneObject);
	extern JUUID GetBillboard(JUUID sceneObject);
	extern void DestroyBillboard(JUUID sceneObject);
	extern void MarkScenePanelAssetsAsDirty();
	extern void DeleteFromScenePanelSelection(JUUID sceneObject);
	extern bool IsPlaying();
	extern bool IsPaused();
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

	void WriteSoundFXsJson(nlohmann::json& json)
	{
#include <Editor/JSaveFile.h>
#include <SoundFXAtt.h>
#include <JEnd.h>
	}
#endif

	void SoundFXsStep(float step)
	{
		auto& SoundFxs = GetSoundEffects();
		std::set<SoundFXUUID> sfxs;
		std::transform(SoundEffects.begin(), SoundEffects.end(), std::inserter(sfxs, sfxs.end()), [](auto o) { return o; });

		std::for_each(sfxs.begin(), sfxs.end(), [step](auto sfx)
			{
				sfx->Step(step);
#if defined(_EDITOR)
				if ((sfx->instanceFlags() & SoundEffectInstance_Use3D))
				{
					JUUID bbuuid = Editor::GetBillboard(sfx());
					if (!bbuuid.empty())
					{
						sfx->UpdateBillboard(bbuuid);
					}
				}
#endif
			}
		);

		std::set<SoundFXUUID> sfxsDestroyI;
		std::set<SoundFXUUID> sfxsCreateI;
		std::copy_if(sfxs.begin(), sfxs.end(), std::inserter(sfxsDestroyI, sfxsDestroyI.end()), [](auto sfx)
			{
				return sfx->dirty(SoundFX::Update_sound) || sfx->dirty(SoundFX::Update_loop) || sfx->dirty(SoundFX::Update_instanceFlags) || (!sfx->IsPlaying() && sfx->HasStarted() && sfx->destroyOnCompletion());
			}
		);

		std::copy_if(sfxs.begin(), sfxs.end(), std::inserter(sfxsCreateI, sfxsCreateI.end()), [](auto sfx)
			{
				if (!sfx->dirty(SoundFX::Update_sound) && (sfx->dirty(SoundFX::Update_loop) || sfx->dirty(SoundFX::Update_instanceFlags)))
					return true;

				return (sfx->dirty(SoundFX::Update_sound) && !sfx->sound().empty());
			}
		);

		std::for_each(sfxsDestroyI.begin(), sfxsDestroyI.end(), [](auto sfx)
			{
				sfx->UnbindFromScene();
			}
		);

		std::for_each(sfxsCreateI.begin(), sfxsCreateI.end(), [](auto sfx)
			{
				sfx->BindToScene();
			}
		);

		std::for_each(sfxs.begin(), sfxs.end(), [step](auto sfx)
			{
				sfx->clear();
			}
		);

		std::set<SoundFXUUID> sfxsDelete;
		std::copy_if(sfxs.begin(), sfxs.end(), std::inserter(sfxsDelete, sfxsDelete.end()), [](auto sfx)
			{
				return sfx->markedForDelete;
			}
		);

		for (auto sfx : sfxsDelete)
		{
			EraseSoundFXFromSoundEffects(sfx());
			EraseSoundFXFromSound3DEffects(sfx());
			DeleteSoundFXSceneObject(sfx());
#if defined(_EDITOR)
			Editor::MarkScenePanelAssetsAsDirty();
			Editor::DeleteFromScenePanelSelection(sfx());
#endif
			//std::shared_ptr<SoundFX> soundfx = sfx;
			//SafeDeleteSceneObject(soundfx);
		}
	}

	SoundFX::SoundFX(nlohmann::json& json) : SceneObject(json)
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
#include <TrackUUID/JInsert.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)
		if (instanceFlags() & SoundEffectInstance_Use3D)
			Editor::RegisterBillboard(uuid());
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
		if (std::get<0>(soundEffectInstance) != nullptr && autoPlay())
		{
#if defined(_EDITOR)
			if (Editor::IsPlaying() && !Editor::IsPaused())
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

	void SoundFX::UpdateEmmiter()
	{
		using namespace AudioSystem;
		GetInstance()->Apply3D(GetAudioListener(), audioEmitter, false);
	}

#if defined(_EDITOR)
	JUUID SoundFX::CreateBillboard(CameraUUID camera)
	{
		if (!(instanceFlags() & SoundEffectInstance_Use3D)) return "";

		JUUID uuid = Editor::CreateBillboardFromMaterials(camera, at("name"), "SoundEffect", "SoundEffectPicking");
		UpdateBillboard(uuid);
		RenderableUUID bb = uuid;
		bb->OnPick = [this] {Editor::SelectSoundEffect(this->uuid()); };
		return uuid;
	}

	void SoundFX::UpdateBillboard(JUUID uuid)
	{
		assert(!uuid.empty());
		if (uuid.empty()) return;

		XMFLOAT3 baseColor = { 1.0f,1.0f,1.0f };
		RenderableUUID bb = uuid;
		bb->position(position());
		bb->WriteConstantsBuffer<XMFLOAT3>("baseColor", baseColor, renderer->backBufferIndex);
		bb->WriteConstantsBuffer(renderer->backBufferIndex);
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

	void DestroySoundEffects()
	{
		auto uuids = nostd::GetUUIDS(SoundFXsceneObjects);
		for (auto uuid : uuids)
		{
			DeleteSoundFXSceneObject(uuid);
		}

#include <TrackUUID/JClear.h>
#include <SoundFXAtt.h>
#include <JEnd.h>
	}

	void PlaySounds()
	{
		auto& sfxs = GetSoundEffects();
		std::for_each(sfxs.begin(), sfxs.end(), [](SoundFXUUID sfx)
			{
				sfx->Play();
			}
		);
	}

	void PauseSounds()
	{
		auto& sfxs = GetSoundEffects();
		std::for_each(sfxs.begin(), sfxs.end(), [](SoundFXUUID sfx)
			{
				sfx->Pause();
			}
		);
	}

	void ResumeSounds()
	{
		auto& sfxs = GetSoundEffects();
		std::for_each(sfxs.begin(), sfxs.end(), [](SoundFXUUID sfx)
			{
				sfx->Resume();
			}
		);
	}

	void StopSounds()
	{
		auto& sfxs = GetSoundEffects();
		std::for_each(sfxs.begin(), sfxs.end(), [](SoundFXUUID sfx)
			{
				sfx->Stop();
			}
		);
	}

	void DeleteSoundFX(std::string uuid)
	{
		SoundFXUUID sfx = uuid;
#if defined(_EDITOR)
		Editor::DestroyBillboard(uuid);
#endif
		sfx->markedForDelete = true;
	}
}