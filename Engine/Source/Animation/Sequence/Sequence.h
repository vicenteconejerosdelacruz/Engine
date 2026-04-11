#pragma once

#include <string>
#include <vector>
#include "SequenceChannel.h"
#include "Channels/Elements/SequenceChannelElementAnimation.h"
#include <Animated.h>

struct Sequence
{
	Sequence();

	Sequence(nlohmann::json j);
	Sequence(const Sequence& seq);
	nlohmann::json json();
	bool operator==(const Sequence& other) const;

	std::string GetAnimationNameAtFrame(int frame);
	SequenceChannelElementAnimation* GetAnimationElementAtFrame(int frame);
	std::set<SequenceChannelElementTrigger*> GetTriggerElements();
	XMMATRIX GetTransformationAtFrame(int frame);
	Animation::BonesTransformations GetBonesTransformations(int frame);

	void CreateSoundFXsAtFrame(int frame, SceneUnitId id);
	void RunScriptAtFrame(int frame, RenderableID renderable);
	bool Runnable() const;
	SequenceChannelElementType GetElementType(unsigned int channelId, unsigned int frame);

	int framesPerSecond;
	int totalFrames;
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
