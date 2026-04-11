#pragma once

#include "SequenceChannelElement.h"
#include <unordered_map>
#include <string>
#include <SimpleMath.h>
#include <nlohmann/json.hpp>
#include "TransformationKeyFrame.h"

struct SequenceChannelElementTransformation : SequenceChannelElement
{
	SequenceChannelElementTransformation() :SequenceChannelElement() {}
	SequenceChannelElementTransformation(const nlohmann::json& j);
	bool operator==(const SequenceChannelElementTransformation& other) const;
	virtual nlohmann::json json();
	TransformationKeyFrame GetKeyFrameBeforeFrame(int frame);
	XMMATRIX GetTransformationInFrame(int frame);
	XMMATRIX GetTransformationBeforeFrame(int frame);
	bool HasKeyframeBeforeFrame(int frame);
	bool HasKeyframeAfterFrame(int frame);
	std::tuple<int, TransformationKeyFrame, int, TransformationKeyFrame> GetKeyframesBetweenFrame(int frame);
	XMMATRIX InterpolateKeyframes(TransformationKeyFrame keyA, TransformationKeyFrame keyB, int frameAfterA, int framesBetweenKeyframes);
	void CreateInterpolatedKeyFrame(int frame);
	void FlipKeyFrames();

	std::unordered_map<int, TransformationKeyFrame> keyFrames;
};
