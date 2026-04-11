#pragma once

#include "Elements/SequenceChannelElementAnimation.h"
#include "Elements/SequenceChannelElementTransformation.h"
#include "Elements/SequenceChannelElementBoneTransformation.h"
#include "Elements/SequenceChannelElementSoundFX.h"
#include "Elements/SequenceChannelElementScript.h"
#include "Elements/SequenceChannelElementTrigger.h"

enum SequenceChannelElementType
{
	SCET_Animation,
	SCET_Transformation,
	SCET_BoneTransformation,
	SCET_SoundFX,
	SCET_Script,
	SCET_Trigger,
};

static inline std::unordered_map<SequenceChannelElementType, std::string> SequenceChannelElementTypeToStr =
{
	{ SCET_Animation, "animation" },
	{ SCET_Transformation, "transformation" },
	{ SCET_BoneTransformation, "boneTransformation" },
	{ SCET_SoundFX, "soundfx" },
	{ SCET_Script, "script" },
	{ SCET_Trigger, "trigger" },
};

static inline std::unordered_map<std::string, SequenceChannelElementType> StrToSequenceChannelElementType =
{
	{ "animation", SCET_Animation },
	{ "transformation", SCET_Transformation },
	{ "boneTransformation", SCET_BoneTransformation },
	{ "soundfx", SCET_SoundFX },
	{ "script", SCET_Script },
	{ "trigger", SCET_Trigger },
};

struct ChannelElement
{
	ChannelElement() {}
	ChannelElement(const ChannelElement& other);
	ChannelElement(const nlohmann::json& j);
	~ChannelElement() {}

	bool InFrame(int frame);
	bool ElementInFrame(int frame, bool& elementBoundFromLeft, bool& elementBoundFromRight);
	void Move(int frames, int totalFrames, int framesPerSecond);
	std::tuple<ChannelElement, ChannelElement> Split(int frame);
	void ExpandLeftBorder(int numFrames);
	void ExpandRightBorder(int numFrames);
	SequenceChannelElement* GetElementPointer();

	int GetFrameStart();
	int GetFrameEnd();

	bool operator==(const ChannelElement& other) const;

	nlohmann::json json();

	SequenceChannelElementType type;
	SequenceChannelElementAnimation animation;
	SequenceChannelElementTransformation transformation;
	SequenceChannelElementBoneTransformation boneTransformation;
	SequenceChannelElementSoundFX soundfx;
	SequenceChannelElementScript script;
	SequenceChannelElementTrigger trigger;
};
