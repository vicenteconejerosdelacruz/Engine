#include "pch.h"
#include "SequenceChannelElementSoundFX.h"

SequenceChannelElementSoundFX::SequenceChannelElementSoundFX() :SequenceChannelElement()
{
	sound = "";
	volume = 1.0f;
	loop = false;
}

SequenceChannelElementSoundFX::SequenceChannelElementSoundFX(const nlohmann::json& j) :SequenceChannelElement(j)
{
	sound = j.at("sound");
	volume = j.at("volume");
	loop = j.at("loop");
}

bool SequenceChannelElementSoundFX::operator==(const SequenceChannelElementSoundFX& other) const {
	return frameStart == other.frameStart && frameEnd == other.frameEnd &&
		sound == other.sound && volume == other.volume && loop == other.loop;
}

nlohmann::json SequenceChannelElementSoundFX::json()
{
	nlohmann::json j =
	{
		{ "frameStart", frameStart },
		{ "frameEnd", frameEnd },
		{ "sound", sound()},
		{ "volume", volume },
		{ "loop", loop }
	};

	return j;
}

int SequenceChannelElementSoundFX::GetFrameStart()
{
	return frameStart;
}
int SequenceChannelElementSoundFX::GetFrameEnd()
{
	return frameEnd;
}
