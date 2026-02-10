#pragma once
#include "SequenceChannelElement.h"
#include <UUID.h>

namespace Templates
{
	DEF_TEMPLATE_ID_DEP(SoundJson, GetSoundTemplate);
};

using namespace Templates;
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