#ifndef _SEQUENCE_CHANNEL_ELEMENT_ANIMATION_H
#define _SEQUENCE_CHANNEL_ELEMENT_ANIMATION_H

#include "SequenceChannelElement.h"

struct SequenceChannelElementAnimation : SequenceChannelElement
{
	SequenceChannelElementAnimation();
	SequenceChannelElementAnimation(const nlohmann::json& j);
	bool operator==(const SequenceChannelElementAnimation& other) const;
	nlohmann::json json();
	int GetFrameStart();
	int GetFrameEnd();
	float GetTimeAtFrame(int frame);

	std::string animation;
	float startTime;
	float endTime;
	bool forward;
};
#endif