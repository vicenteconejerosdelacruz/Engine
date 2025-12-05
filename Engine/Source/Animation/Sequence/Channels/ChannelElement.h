#pragma once

#include "Elements/SequenceChannelElementAnimation.h"
#include "Elements/SequenceChannelElementTransformation.h"
#include "Elements/SequenceChannelElementSoundFX.h"
#include "Elements/SequenceChannelElementScript.h"

enum SequenceChannelElementType
{
	SCET_Animation,
	SCET_Transformation,
	SCET_SoundFX,
	SCET_Script
};

static inline std::unordered_map<SequenceChannelElementType, std::string> SequenceChannelElementTypeToStr =
{
	{ SCET_Animation, "animation" },
	{ SCET_Transformation, "transformation" },
	{ SCET_SoundFX, "soundfx" },
	{ SCET_Script, "script" },
};

static inline std::unordered_map<std::string, SequenceChannelElementType> StrToSequenceChannelElementType =
{
	{ "animation", SCET_Animation },
	{ "transformation", SCET_Transformation },
	{ "soundfx", SCET_SoundFX },
	{ "script", SCET_Script },
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
	SequenceChannelElementSoundFX soundfx;
	SequenceChannelElementScript script;
};