#include "pch.h"
#include "AudioSystem.h"
#include <Audio.h>

namespace AudioSystem {

	std::unique_ptr<DirectX::AudioEngine> audioEngine;
	DirectX::AudioListener listener;

	void InitAudio()
	{
		//initialize DirectXTK Audio Engine
		//inicializar el Audio Engine de DirectXTK
		AUDIO_ENGINE_FLAGS audioFlags = AudioEngine_EnvironmentalReverb | AudioEngine_ReverbUseFilters;
#ifdef _DEBUG
		audioFlags |= AudioEngine_Debug;
#endif
		audioEngine = std::make_unique<DirectX::AudioEngine>(audioFlags);
	}

	void ShutdownAudio()
	{
		audioEngine = nullptr;
	}

	void UpdateAudio()
	{
#if defined(_DEVELOPMENT)
		PIXScopedEvent(0, L"UpdateAudio");
#endif
		GetAudioEngine()->Update();
	}

	DirectX::AudioListener& GetAudioListener()
	{
		return listener;
	}

	void UpdateListener(XMFLOAT3 position, XMVECTOR orientation)
	{
		listener.SetPosition(position);
		listener.SetOrientationFromQuaternion(orientation);
	}

	std::unique_ptr<DirectX::AudioEngine>& GetAudioEngine()
	{
		return audioEngine;
	}
}
