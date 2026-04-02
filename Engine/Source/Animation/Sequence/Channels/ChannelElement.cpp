#include "pch.h"
#include "ChannelElement.h"

ChannelElement::ChannelElement(const ChannelElement& other)
{
	type = other.type;
	switch (type)
	{
	case SCET_Animation:
	{
		animation = other.animation;
	}
	break;
	case SCET_Transformation:
	{
		transformation = other.transformation;
	}
	break;
	case SCET_SoundFX:
	{
		soundfx = other.soundfx;
	}
	break;
	case SCET_Script:
	{
		script = other.script;
	}
	break;
	case SCET_Trigger:
	{
		trigger = other.trigger;
	}
	break;
	}
}

ChannelElement::ChannelElement(const nlohmann::json& j)
{
	type = StrToSequenceChannelElementType.at(j.at("type"));
	switch (type)
	{
	case SCET_Animation:
	{
		animation = SequenceChannelElementAnimation(j.at("animation"));
	}
	break;
	case SCET_Transformation:
	{
		transformation = SequenceChannelElementTransformation(j.at("transformation"));
	}
	break;
	case SCET_SoundFX:
	{
		soundfx = SequenceChannelElementSoundFX(j.at("soundfx"));
	}
	break;
	case SCET_Script:
	{
		script = SequenceChannelElementScript(j.at("script"));
	}
	break;
	case SCET_Trigger:
	{
		trigger = SequenceChannelElementTrigger(j.at("trigger"));
	}
	break;
	}
}

bool ChannelElement::InFrame(int frame)
{
	int frameStart = GetFrameStart();
	int frameEnd = GetFrameEnd() - 1;
	return nostd::in_between(frame, frameStart, frameEnd);
}

bool ChannelElement::ElementInFrame(int frame, bool& elementBoundFromLeft, bool& elementBoundFromRight)
{
	int frameStart = GetFrameStart();
	int frameEnd = GetFrameEnd();
	bool ret = nostd::in_between(frame, frameStart, frameEnd);
	if (ret)
	{
		elementBoundFromLeft = frame == frameStart;
		elementBoundFromRight = frame == frameEnd;
	}
	return ret;
}

void ChannelElement::Move(int frames, int totalFrames, int framesPerSecond)
{
	SequenceChannelElement* element = GetElementPointer();

	if (frames < 0)
	{
		frames = -std::min(static_cast<int>(GetFrameStart()), -frames);
		element->frameStart += frames;
		element->frameEnd += frames;
	}
	else if (frames > 0)
	{
		frames = std::min((totalFrames - static_cast<int>(GetFrameEnd())), frames);
		element->frameStart += frames;
		element->frameEnd += frames;
	}
	if (type == SCET_Transformation)
	{
		std::unordered_map<int, TransformationKeyFrame> newKeys;
		for (auto& [k, v] : transformation.keyFrames)
		{
			newKeys.insert_or_assign(k + frames, v);
		}
		transformation.keyFrames = newKeys;
	}
}

std::tuple<ChannelElement, ChannelElement> ChannelElement::Split(int frame)
{
	frame = std::max(frame, 1);

	std::tuple<ChannelElement, ChannelElement> elements;
	auto& [left, right] = elements;

	left.type = type;
	right.type = type;

	SequenceChannelElement* leftPtr = left.GetElementPointer();
	SequenceChannelElement* rightPtr = right.GetElementPointer();

	leftPtr->frameStart = GetFrameStart();
	rightPtr->frameEnd = GetFrameEnd();

	leftPtr->frameEnd = frame - 1;
	rightPtr->frameStart = frame;

	switch (type)
	{
	case SCET_Animation:
	{
		left.animation.animation = animation.animation;
		right.animation.animation = animation.animation;

		left.animation.startTime = animation.startTime;
		right.animation.endTime = animation.endTime;

		float t = static_cast<float>(frame - animation.frameStart) / static_cast<float>(animation.frameEnd - animation.frameStart);
		left.animation.endTime = animation.startTime + t * (animation.endTime - animation.startTime);
		right.animation.startTime = left.animation.endTime;
	}
	break;
	case SCET_Transformation:
	{
		std::copy_if(
			transformation.keyFrames.begin(),
			transformation.keyFrames.end(),
			std::inserter(left.transformation.keyFrames, left.transformation.keyFrames.begin()),
			[frame](const std::pair<int, TransformationKeyFrame>& p) {
				return p.first < frame;
			}
		);
		std::copy_if(
			transformation.keyFrames.begin(),
			transformation.keyFrames.end(),
			std::inserter(right.transformation.keyFrames, right.transformation.keyFrames.begin()),
			[frame](const std::pair<int, TransformationKeyFrame>& p) {
				return p.first >= frame;
			}
		);
	}
	break;
	case SCET_SoundFX:
	{
		left.soundfx.sound = soundfx.sound;
		right.soundfx.sound = soundfx.sound;
		left.soundfx.volume = soundfx.volume;
		right.soundfx.volume = soundfx.volume;
		left.soundfx.loop = soundfx.loop;
		right.soundfx.loop = soundfx.loop;
	}
	break;
	case SCET_Script:
	{
		left.script.script = script.script;
		right.script.script = script.script;
	}
	break;
	case SCET_Trigger:
	{
		left.trigger.position = trigger.position;
		right.trigger.position = trigger.position;
		left.trigger.rotation = trigger.rotation;
		right.trigger.rotation = trigger.rotation;
		left.trigger.scale = trigger.scale;
		right.trigger.scale = trigger.scale;
		left.trigger.onEnter = trigger.onEnter;
		right.trigger.onEnter = trigger.onEnter;
		left.trigger.onLeave = trigger.onLeave;
		right.trigger.onLeave = trigger.onLeave;
		left.trigger.bone = trigger.bone;
		right.trigger.bone = trigger.bone;
	}
	break;
	}

	return elements;
}

void ChannelElement::ExpandLeftBorder(int numFrames)
{
	switch (type)
	{
	case SCET_Animation:
	{
		animation.ExpandLeftBorder(numFrames);
	}
	break;
	case SCET_Transformation:
	{
		transformation.ExpandLeftBorder(numFrames);
	}
	break;
	case SCET_SoundFX:
	{
		soundfx.ExpandLeftBorder(numFrames);
	}
	break;
	case SCET_Script:
	{
		script.ExpandLeftBorder(numFrames);
	}
	break;
	case SCET_Trigger:
	{
		trigger.ExpandLeftBorder(numFrames);
	}
	break;
	}
}
void ChannelElement::ExpandRightBorder(int numFrames)
{
	switch (type)
	{
	case SCET_Animation:
	{
		animation.ExpandRightBorder(numFrames);
	}
	break;
	case SCET_Transformation:
	{
		transformation.ExpandRightBorder(numFrames);
	}
	break;
	case SCET_SoundFX:
	{
		soundfx.ExpandRightBorder(numFrames);
	}
	break;
	case SCET_Script:
	{
		script.ExpandRightBorder(numFrames);
	}
	break;
	case SCET_Trigger:
	{
		trigger.ExpandRightBorder(numFrames);
	}
	break;
	}
}

SequenceChannelElement* ChannelElement::GetElementPointer()
{
	std::unordered_map<SequenceChannelElementType, SequenceChannelElement*> elementsByType =
	{
		{ SCET_Animation, &animation },
		{ SCET_Transformation, &transformation },
		{ SCET_SoundFX, &soundfx },
		{ SCET_Script, &script },
		{ SCET_Trigger, &trigger },
	};
	return elementsByType.at(type);
}

int ChannelElement::GetFrameStart()
{
	switch (type)
	{
	case SCET_Animation:
	{
		return animation.GetFrameStart();
	}
	break;
	case SCET_Transformation:
	{
		return transformation.frameStart;
	}
	break;
	case SCET_SoundFX:
	{
		return soundfx.GetFrameStart();
	}
	break;
	case SCET_Script:
	{
		return script.frameStart;
	}
	break;
	case SCET_Trigger:
	{
		return trigger.frameStart;
	}
	break;
	}
	return 0;
}

int ChannelElement::GetFrameEnd()
{
	switch (type)
	{
	case SCET_Animation:
	{
		return animation.GetFrameEnd();
	}
	break;
	case SCET_Transformation:
	{
		return transformation.frameEnd;
	}
	break;
	case SCET_SoundFX:
	{
		return soundfx.GetFrameEnd();
	}
	break;
	case SCET_Script:
	{
		return script.frameEnd;
	}
	break;
	case SCET_Trigger:
	{
		return trigger.frameEnd;
	}
	break;
	}
	return 0;
}

bool ChannelElement::operator==(const ChannelElement& other) const {
	if (type != other.type) return false;
	switch (type)
	{
	case SCET_Animation:
	{
		return animation == other.animation;
	}
	break;
	case SCET_Transformation:
	{
		return transformation == other.transformation;
	}
	break;
	case SCET_SoundFX:
	{
		return soundfx == other.soundfx;
	}
	break;
	case SCET_Script:
	{
		return script == other.script;
	}
	break;
	case SCET_Trigger:
	{
		return trigger == other.trigger;
	}
	break;
	}
	return false;
}

nlohmann::json ChannelElement::json()
{
	nlohmann::json j;
	j["type"] = SequenceChannelElementTypeToStr.at(type);
	switch (type)
	{
	case SCET_Animation:
	{
		j["animation"] = animation.json();
	}
	break;
	case SCET_Transformation:
	{
		j["transformation"] = transformation.json();
	}
	break;
	case SCET_SoundFX:
	{
		j["soundfx"] = soundfx.json();
	}
	break;
	case SCET_Script:
	{
		j["script"] = script.json();
	}
	break;
	case SCET_Trigger:
	{
		j["trigger"] = trigger.json();
	}
	break;
	}
	return j;
}
