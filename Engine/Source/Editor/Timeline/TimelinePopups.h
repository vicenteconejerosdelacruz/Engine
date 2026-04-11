#pragma once
#include <unordered_map>
#include <string>
#include <Sequence/AnimationSequences.h>
#include <UUID.h>

enum TimelinePopups
{
	TP_None,
	TP_AddElement,
	TP_InteractWithElement,
	TP_PickBone
};

static inline std::unordered_map<TimelinePopups, std::string> TimelinePopupsToString =
{
	{ TP_AddElement, "Add Element" },
	{ TP_InteractWithElement, "Interact with Element" },
	{ TP_PickBone, "Pick Bone" },
};

static inline std::unordered_map<std::string, TimelinePopups> StringToTimelinePopups =
{
	{ "Add Element", TP_AddElement },
	{ "Interact with Element", TP_InteractWithElement },
	{ "Pick Bone", TP_PickBone },
};

struct AddElementPopup
{
	SequenceChannelElementType type = SCET_Animation;
	SequenceChannelElementAnimation animation;
	SequenceChannelElementTransformation transformation;
	SequenceChannelElementBoneTransformation boneTransformation;
	SequenceChannelElementSoundFX soundfx;
	SequenceChannelElementScript script;
	SequenceChannelElementTrigger trigger;

	RenderableID renderable;
	std::vector<std::string> animations;
	JUUIDName selectedSoundEffect;
	std::vector<JUUIDName> soundEffects;
	std::vector<std::string> bones;

	void Init(RenderableID renderable, int frame);
	void Draw(ImVec2 pos, std::unordered_map<SequenceChannelElementType, std::function<void(SequenceChannelElement*)>> elementBuilders, std::function<void()> closePopup);
};

enum InteractPopups
{
	IP_Delete,
	IP_Split,
	IP_Animation_Change,
	IP_Animation_Forward,
	IP_Animation_Backward,
	IP_Transformation_AddKeyframe,
	IP_Transformation_RemoveKeyframe,
	IP_Transformation_Flip,
	IP_BoneTransformation_AddKeyframe,
	IP_BoneTransformation_RemoveKeyframe,
	IP_Script_Edit,
	IP_Trigger_OnEnter,
	IP_Trigger_OnLeave,
	IP_Pick_Bone,
};

struct InteractElementPopup
{
	void Draw(
		ImVec2 pos,
		ChannelElement& element,
		int frame,
		std::unordered_map<InteractPopups, std::function<void()>> drawers,
		std::unordered_map<InteractPopups, std::function<void()>> elementInteract,
		std::function<void()> closePopup
	);
};

struct PickBonePopup
{
	std::vector<std::string> bones;
	void Draw(ImVec2 pos, ChannelElement& element, std::function<void()> closePopup);
};