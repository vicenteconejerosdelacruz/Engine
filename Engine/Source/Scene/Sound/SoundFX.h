#pragma once

//#include <SceneObjectDecl.h>
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

		SoundFX(nlohmann::json& json);
		~SoundFX() { Destroy(); }
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif
		virtual void Initialize();
		virtual void BindToScene();
		virtual void UnbindFromScene();

		void Destroy();

		XMVECTOR rotationQ();
		XMMATRIX world();
		XMVECTOR fw();

		std::unique_ptr<DirectX::SoundEffect>& GetEffect() { return std::get<0>(soundEffectInstance); }
		std::unique_ptr<DirectX::SoundEffectInstance>& GetInstance() { return std::get<1>(soundEffectInstance); }
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
		virtual JUUID CreateBillboard(CameraUUID camera);
		virtual void UpdateBillboard(JUUID billboard);
		BoundingBox GetBoundingBox();

		//Gizmo
		virtual bool CanInteractWithGizmo(ImGuizmo::OPERATION operation);
#endif
		bool markedForDelete = false;
		float time = 0.0f;
		bool hasStarted = false;
		//3D
		AudioEmitter audioEmitter;
		std::tuple<std::unique_ptr<DirectX::SoundEffect>, std::unique_ptr<DirectX::SoundEffectInstance>> soundEffectInstance;
	};

	SODECL_FULL(SoundFX);

#include <TrackUUID/JDecl.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

	void SoundFXsStep(float step);
	void DestroySoundEffects();
	void DestroySoundEffects(SceneUnitId unit);
	void DeleteSoundFX(JUUID uuid);
	void PlaySounds();
	void PauseSounds();
	void ResumeSounds();
	void StopSounds();
#if defined(_EDITOR)
	void WriteSoundFXsJson(nlohmann::json& json);
#endif
}
