#pragma once
#include <unordered_map>
#include <string>
#include <Sequence/AnimationSequences.h>
#include <UUID.h>

enum TimelinePopups
{
	TP_None,
	TP_AddElement,
	TP_InteractWithElement
};

static inline std::unordered_map<TimelinePopups, std::string> TimelinePopupsToString =
{
	{ TP_AddElement, "Add Element" },
	{ TP_InteractWithElement, "Interact with Element" }
};

static inline std::unordered_map<std::string, TimelinePopups> StringToTimelinePopups =
{
	{ "Add Element", TP_AddElement },
	{ "Interact with Element", TP_InteractWithElement }
};

struct AddElementPopup
{
	SequenceChannelElementType type = SCET_Animation;
	SequenceChannelElementAnimation animation;
	SequenceChannelElementTransformation transformation;
	SequenceChannelElementSoundFX soundfx;
	SequenceChannelElementScript script;

	RenderableUUID renderable;
	std::vector<std::string> animations;
	JUUIDName selectedSoundEffect;
	std::vector<JUUIDName> soundEffects;

	void Init(JUUID uuid, int frame);
	void Draw(ImVec2 pos, std::unordered_map<SequenceChannelElementType, std::function<void(SequenceChannelElement*)>> elementBuilders, std::function<void()> closePopup);
};

enum InteractPopups
{
	IP_Delete,
	IP_Split,
	IP_Transformation_AddKeyframe,
	IP_Transformation_RemoveKeyframe,
	IP_Script_Edit
};

struct InteractElementPopup
{
	void Draw(ImVec2 pos, ChannelElement& element, int frame, std::unordered_map<InteractPopups, std::function<void()>> elementInteract, std::function<void()> closePopup);
};
