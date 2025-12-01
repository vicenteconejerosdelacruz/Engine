#pragma once

struct SequenceChannelElement
{
	SequenceChannelElement();
	SequenceChannelElement(const nlohmann::json& j);
	void ExpandLeftBorder(int numFrames);
	void ExpandRightBorder(int numFrames);

	int frameStart;
	int frameEnd;
};
