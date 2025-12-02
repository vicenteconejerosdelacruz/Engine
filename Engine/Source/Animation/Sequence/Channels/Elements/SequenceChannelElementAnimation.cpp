#include "pch.h"
#include "SequenceChannelElementAnimation.h"

SequenceChannelElementAnimation::SequenceChannelElementAnimation() :SequenceChannelElement()
{
	animation = "";
	startTime = 0.0f;
	endTime = 0.0f;
}

SequenceChannelElementAnimation::SequenceChannelElementAnimation(const nlohmann::json& j) :SequenceChannelElement(j)
{
	animation = j.at("animation");
	startTime = j.at("startTime");
	endTime = j.at("endTime");
}

bool SequenceChannelElementAnimation::operator==(const SequenceChannelElementAnimation& other) const
{
	return
		frameStart == other.frameStart
		&& frameEnd == other.frameEnd
		&& startTime == other.startTime
		&& endTime == other.endTime;
}

nlohmann::json SequenceChannelElementAnimation::json()
{
	nlohmann::json j =
	{
		{ "frameStart", frameStart },
		{ "frameEnd" , frameEnd },
		{ "animation", animation },
		{ "startTime", startTime },
		{ "endTime", endTime },
	};
	return j;
}

int SequenceChannelElementAnimation::GetFrameStart()
{
	return frameStart;
}
int SequenceChannelElementAnimation::GetFrameEnd()
{
	return frameEnd;
}

float SequenceChannelElementAnimation::GetTimeAtFrame(int frame)
{
	if (frameStart == 0 && frameEnd == 0) return 0.0f;
	frame = std::clamp(frame, frameStart, frameEnd);
	float t = static_cast<float>(frame - frameStart) / static_cast<float>(frameEnd - frameStart);
	return std::clamp(startTime + t * (endTime - startTime), startTime, endTime);
}
