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

	SoundJsonID sound;
	float volume;
	bool loop;
};