#pragma once
#include "SequenceChannelElement.h"

struct SequenceChannelElementSoundFX : SequenceChannelElement
{
	SequenceChannelElementSoundFX();
	SequenceChannelElementSoundFX(const nlohmann::json& j);
	bool operator==(const SequenceChannelElementSoundFX& other) const;
	nlohmann::json json();
	int GetFrameStart();
	int GetFrameEnd();

	SoundJsonUUID sound;
	float volume;
	bool loop;
};