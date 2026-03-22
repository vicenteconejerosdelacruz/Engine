#ifndef _AUDIO_SYSTEM_H
#define _AUDIO_SYSTEM_H

namespace AudioSystem {

	void InitAudio();
	void ShutdownAudio();
	void UpdateAudio();
	DirectX::AudioListener& GetAudioListener();
	void UpdateListener(XMFLOAT3 position, XMVECTOR orientation);
	std::unique_ptr<DirectX::AudioEngine>& GetAudioEngine();
}

#endif