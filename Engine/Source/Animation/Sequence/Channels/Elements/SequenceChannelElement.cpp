#include "pch.h"
#include "SequenceChannelElement.h"

SequenceChannelElement::SequenceChannelElement()
{
	frameStart = 0;
	frameEnd = 0;
}

SequenceChannelElement::SequenceChannelElement(const nlohmann::json& j)
{
	frameStart = j.at("frameStart");
	frameEnd = j.at("frameEnd");
}

void SequenceChannelElement::ExpandLeftBorder(int numFrames)
{
	frameStart += numFrames;
}

void SequenceChannelElement::ExpandRightBorder(int numFrames)
{
	frameEnd += numFrames;
}