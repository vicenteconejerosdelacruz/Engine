#ifndef _SEQUENCE_CHANNE_LELEMENT_H
#define _SEQUENCE_CHANNE_LELEMENT_H

struct SequenceChannelElement
{
	SequenceChannelElement();
	SequenceChannelElement(const nlohmann::json& j);
	void ExpandLeftBorder(int numFrames);
	void ExpandRightBorder(int numFrames);

	int frameStart;
	int frameEnd;
};
#endif