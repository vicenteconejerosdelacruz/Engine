#pragma once
#include "SequenceChannelElement.h"
#include <unordered_map>
#include <string>
#include <DirectXMath.h>

enum Easing {
	Easing_Linear,
	Easing_Sine_Ease_In,
	Easing_Sine_Ease_Out,
	Easing_Sine_Ease_In_Out
};

static inline std::unordered_map<Easing, std::string> EasingToString =
{
	{ Easing_Linear,"Linear"},
	{ Easing_Sine_Ease_In,"Sine_Ease_In"},
	{ Easing_Sine_Ease_Out,"Sine_Ease_Out"},
	{ Easing_Sine_Ease_In_Out,"Sine_Ease_In_Out"},
};

static inline std::unordered_map<std::string, Easing> StringToEasing =
{
	{ "Linear", Easing_Linear},
	{ "Sine_Ease_In", Easing_Sine_Ease_In},
	{ "Sine_Ease_Out", Easing_Sine_Ease_Out},
	{ "Sine_Ease_In_Out", Easing_Sine_Ease_In_Out},
};

struct TransformationKeyFrame
{
	XMFLOAT3 position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	Easing easing = Easing_Linear;

	XMMATRIX ToMatrix();

	bool operator==(const TransformationKeyFrame& other) const;
};

struct SequenceChannelElementTransformation : SequenceChannelElement
{
	SequenceChannelElementTransformation() :SequenceChannelElement() {}
	SequenceChannelElementTransformation(const nlohmann::json& j);
	bool operator==(const SequenceChannelElementTransformation& other) const;
	nlohmann::json json();
	TransformationKeyFrame GetKeyFrameBeforeFrame(int frame);
	XMMATRIX GetTransformationInFrame(int frame);
	XMMATRIX GetTransformationBeforeFrame(int frame);
	bool HasKeyframeBeforeFrame(int frame);
	bool HasKeyframeAfterFrame(int frame);
	std::tuple<int, TransformationKeyFrame, int, TransformationKeyFrame> GetKeyframesBetweenFrame(int frame);
	XMMATRIX InterpolateKeyframes(TransformationKeyFrame keyA, TransformationKeyFrame keyB, int frameAfterA, int framesBetweenKeyframes);

	std::unordered_map<int, TransformationKeyFrame> keyFrames;
};