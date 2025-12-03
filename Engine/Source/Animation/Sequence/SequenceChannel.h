#pragma once

#include "Channels/Elements/SequenceChannelElementAnimation.h"
#include "Channels/Elements/SequenceChannelElementTransformation.h"
#include "Channels/Elements/SequenceChannelElementSoundFX.h"
#include "Channels/Elements/SequenceChannelElementScript.h"
#include "Channels/ChannelElement.h"

struct SequenceChannel
{
	SequenceChannel();
	SequenceChannel(std::string name);
	SequenceChannel(const nlohmann::json& j);
	SequenceChannel(const SequenceChannel& seqChannel);

	bool ChannelHasElementAtFrame(int frame);
	int GetAvailableFramesToLeft(int elementIndex);
	int GetAvailableFramesToRight(int elementIndex, int totalFrames);
	int GetFirstElementIndexBetweenFrames(int frameStart, int frameEnd);
	int GetElementIndexBeforeFrame(int frame);
	SequenceChannelElementAnimation* GetAnimationElementAtFrame(int frame);
	SequenceChannelElementTransformation* GetTransformationElementAtFrame(int frame);
	TransformationKeyFrame* GetTransformationKeyframe(int frame);
	SequenceChannelElementSoundFX* GetSoundFXToCreateAtFrame(int frame);
	SequenceChannelElementScript* GetScriptToRunAtFrame(int frame);

	void InsertChannelElement(ChannelElement element, int& totalFrames);
	void MoveElement(int elementIndex, int frames, int totalFrames);
	void DragElementLeftBoundary(int elementIndex, int frames, int totalFrames);
	void DragElementRightBoundary(int elementIndex, int frames, int totalFrames);
	void EraseElement(int elementIndex);
	void SplitElement(int elementIndex, int frame);
	bool FrameHasElement(int frame, bool& leftBounded, bool& rightBounded);
	bool FrameHasTransformationKeyframe(int frame);
	void EraseElementInFrame(int frame);
	void SplitElementInFrame(int frame);

	nlohmann::json json();

	bool operator==(const SequenceChannel& other) const;

	std::string name;
	//elements should be sorted ok?
	std::vector<ChannelElement> elements;
};