#pragma once

#include <string>
#include <vector>
#include "SequenceChannel.h"
#include "Channels/Elements/SequenceChannelElementAnimation.h"

struct Sequence
{
	Sequence();

	Sequence(nlohmann::json j);
	nlohmann::json json();
	bool operator==(const Sequence& other) const;

	std::string GetAnimationNameAtFrame(int frame);
	SequenceChannelElementAnimation* GetAnimationElementAtFrame(int frame);
	XMMATRIX GetTransformationAtFrame(int frame);
	void CreateSoundFXsAtFrame(int frame);
	void RunScriptAtFrame(int frame, RenderableUUID renderable);

	int framesPerSecond;
	int totalFrames;
	//bool loop;
	std::vector<SequenceChannel> sequenceChannels;
};

inline static Sequence ToSequence(nlohmann::json j)
{
	Sequence seq(j);
	return seq;
}

inline static nlohmann::json FromSequence(Sequence s)
{
	return s.json();
}
