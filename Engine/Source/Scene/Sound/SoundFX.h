#pragma once

#include <Scene.h>
#include <SceneObject.h>

namespace Scene
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#endif

	struct SoundFX : SceneObject
	{
		inline static const SceneObjectType sceneObjectType = SO_SoundEffects;

#include <Attributes/JFlags.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

		SoundFX(SceneUnitId id, nlohmann::json& json);
		~SoundFX() { Destroy(); }
		virtual void Initialize();
		virtual void SetInitialConditions();
		virtual void BindToScene();
		virtual void UnbindFromScene();

		void Destroy();

		void updateRotationQ();
		XMVECTOR rotationQ();
		XMMATRIX world();
		XMVECTOR fw();

		bool EffectExists();
		std::unique_ptr<DirectX::SoundEffect>& GetEffect();
		std::unique_ptr<DirectX::SoundEffectInstance>& GetInstance();
		bool Play();
		bool Stop();
		bool Pause();
		bool Resume();
		bool IsPlaying() { return GetInstance() != nullptr && GetInstance()->GetState() == DirectX::SoundState::PLAYING; }
		bool IsPaused() { return GetInstance() == nullptr || GetInstance()->GetState() == DirectX::SoundState::PAUSED; }
		float Duration() { return (GetEffect()->GetSampleDurationMS() / 1000.0f); }
		void Step(float step);
		float Time() const { return time; }
		bool HasStarted() { return hasStarted; }

		void UpdateEmmiter();

#if defined(_EDITOR)
		virtual RenderableID CreateBillboard(CameraID camera);
		virtual void UpdateBillboard(RenderableID renderable);
		BoundingBox GetBoundingBox();

		//Gizmo
		virtual bool CanInteractWithGizmo(ImGuizmo::OPERATION operation);
		virtual void WriteJson(nlohmann::json& j);
#endif
		bool markedForDelete = false;
		float time = 0.0f;
		bool hasStarted = false;
		//Transformation
		XMVECTOR rotationQuaternion;
		AudioEmitter audioEmitter;
		SoundInstance soundEffectInstance;
	};

	SODECL_FULL(SoundFX);

#include <TrackUUID/JDecl.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

	void SoundFXsStep(SceneUnitId id, float step);
	void DestroySoundEffects();
	void DestroySoundEffects(SceneUnitId id);
	void DeleteSoundFX(SceneUnitId id, JUUID uuid);
	void PlaySounds(SceneUnitId id);
	void PauseSounds(SceneUnitId id);
	void ResumeSounds(SceneUnitId id);
	void StopSounds(SceneUnitId id);
#if defined(_EDITOR)
	void WriteSoundFXsJson(SceneUnitId id, nlohmann::json& json);
#endif
}

using namespace Scene;
DEF_SCENEOBJECT_ID_HASH(SoundFX);