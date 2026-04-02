#include "pch.h"
#include "TimelinePopups.h"
#include <imgui.h>
#include <ImEditor.h>
#include <Sound/Sound.h>

static inline ImGuiWindowFlags popupChildFlag = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize |
ImGuiWindowFlags_NoMove;

void AddElementPopup::Init(RenderableID renderable, int frame)
{
	this->renderable = renderable;
	type = SCET_Animation;

	//animations
	auto& anim = renderable->animable->animations;
	animations = nostd::GetKeysFromMap(renderable->animable->animations->animationsLength);
	animations.erase(animations.begin());
	animation.animation = *animations.begin();
	animation.frameStart = frame;
	animation.startTime = 0.0f;
	animation.endTime = anim->animationsLength[animation.animation];

	//transformation
	transformation.frameStart = frame;

	//sound effects
	soundEffects = Templates::GetSoundsUUIDsNames();
	selectedSoundEffect = soundEffects.at(0);
	soundfx.sound = std::get<0>(selectedSoundEffect);
	soundfx.frameStart = frame;
	soundfx.frameEnd = frame;

	//script
	script.frameStart = frame;
	script.frameEnd = frame;

	//trigger
	trigger.frameStart = frame;
	trigger.frameEnd = frame;
	bones = nostd::GetKeysFromMap(renderable->animable->animations->bonesOffsets);
	bones.insert(bones.begin(), { "" });
}

void AddElementPopup::Draw(ImVec2 pos, std::unordered_map<SequenceChannelElementType, std::function<void(SequenceChannelElement*)>> elementBuilders, std::function<void()> closePopup)
{
	ImGui::OpenPopup("AddElementPopup");

	ImVec2 size(200, 75);

	if (type == SCET_Animation || type == SCET_SoundFX || type == SCET_Trigger)
	{
		size.y += 20;
	}

	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	if (ImGui::BeginPopupModal("AddElementPopup", nullptr, popupChildFlag))
	{
		ImGui::SetNextItemWidth(size.x);

		std::vector<std::string> selectables = nostd::GetKeysFromMap(StrToSequenceChannelElementType);
		std::string selected = SequenceChannelElementTypeToStr.at(type);
		bool changedToAnim = false;
		bool changedToSfx = false;
		bool changedToTrigger = false;
		ImGui::PushID("NewElementType");
		ImGui::DrawComboSelection(selected, selectables,
			[this, &changedToAnim,
			&changedToSfx, &changedToTrigger
			](std::string newElementType)
			{
				type = StrToSequenceChannelElementType.at(newElementType);
				changedToAnim = type == SCET_Animation;
				changedToSfx = type == SCET_SoundFX;
				changedToTrigger = type == SCET_Trigger;
			}
		);
		ImGui::PopID();

		if (type == SCET_Animation && !changedToAnim)
		{
			ImGui::SetNextItemWidth(size.x);
			ImGui::PushID("ElementTypeAnimationName");
			ImGui::DrawComboSelection(animation.animation, animations, [this](std::string selectedAnim)
				{
					auto& animations = renderable->animable->animations;
					animation.animation = selectedAnim;
					animation.startTime = 0.0f;
					animation.endTime = animations->animationsLength[selectedAnim];
				}
			);
			ImGui::PopID();
		}
		else if (type == SCET_SoundFX && !changedToSfx)
		{
			ImGui::SetNextItemWidth(size.x);
			ImGui::PushID("ElementTypeSFXName");
			ImGui::DrawComboSelection(selectedSoundEffect, soundEffects, [this](JUUIDName selected)
				{
					selectedSoundEffect = selected;
					soundfx.sound = std::get<0>(selected);
				}
			);
			ImGui::PopID();
		}
		else if (type == SCET_Trigger && !changedToTrigger)
		{
			ImGui::SetNextItemWidth(size.x);
			ImGui::PushID("ElementTypeBoneName");
			ImGui::DrawComboSelection(trigger.bone, bones, [this](std::string selectedBone)
				{
					trigger.bone = selectedBone;
				}
			);
			ImGui::PopID();
		}

		float button1_width = ImGui::CalcTextSize("Add").x + ImGui::GetStyle().FramePadding.x * 2.0f;
		float button2_width = ImGui::CalcTextSize("Cancel").x + ImGui::GetStyle().FramePadding.x * 2.0f;
		float total_width = button1_width + button2_width + ImGui::GetStyle().ItemSpacing.x;

		float window_width = ImGui::GetContentRegionAvail().x;
		float start_x = (window_width - total_width) * 0.5f;

		ImGui::SetCursorPosX(start_x);

		if (ImGui::Button("Add"))
		{
			std::unordered_map<SequenceChannelElementType, SequenceChannelElement*> typeTemplate =
			{
				{ SCET_Animation, &animation },
				{ SCET_Transformation, &transformation },
				{ SCET_SoundFX, &soundfx },
				{ SCET_Script, &script },
				{ SCET_Trigger, &trigger },
			};
			elementBuilders.at(type)(typeTemplate.at(type));
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			closePopup();
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);
}

void InteractElementPopup::Draw(ImVec2 pos, ChannelElement& element, int frame, std::unordered_map<InteractPopups, std::function<void()>> elementInteract, std::function<void()> closePopup)
{
	ImGui::OpenPopup("InteractElementPopup");

	ImVec2 size(200, 80);

	std::vector<std::tuple<std::string, InteractPopups, bool>> options =
	{
		std::make_tuple("Delete",IP_Delete, true),
		std::make_tuple("Split",IP_Split,element.GetFrameStart() != element.GetFrameEnd())
	};

	if (element.type == SCET_Animation)
	{
		if (element.animation.forward)
		{
			options.push_back(std::make_tuple("Backward", IP_Animation_Backward, true));
		}
		else
		{
			options.push_back(std::make_tuple("Forward", IP_Animation_Forward, true));
		}
		size.y += 20;
	}
	else if (element.type == SCET_Transformation)
	{
		if (element.transformation.keyFrames.contains(frame))
		{
			options.push_back(std::make_tuple("Remove Keyframe", IP_Transformation_RemoveKeyframe, true));
		}
		else
		{
			options.push_back(std::make_tuple("Add Keyframe", IP_Transformation_AddKeyframe, true));
		}
		options.push_back(std::make_tuple("Flip", IP_Transformation_Flip, true));
		size.y += 35;
	}
	else if (element.type == SCET_Script)
	{
		options.push_back(std::make_tuple("Edit Script", IP_Script_Edit, true));
		size.y += 20;
	}
	else if (element.type == SCET_Trigger)
	{
		options.push_back(std::make_tuple("Bone", IP_Pick_Bone, true));
		options.push_back(std::make_tuple("OnEnter Script", IP_Trigger_OnEnter, true));
		options.push_back(std::make_tuple("OnLeave Script", IP_Trigger_OnLeave, true));
		size.y += 50;
	}

	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 2.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	if (ImGui::BeginPopupModal("InteractElementPopup", nullptr, popupChildFlag))
	{
		for (auto& [title, key, enabled] : options)
		{
			if (enabled)
			{
				if (ImGui::MenuItem(title.c_str())) {
					elementInteract.at(key)();
				}
			}
			else
			{
				ImGui::MenuItem(title.c_str(), NULL, false, false);
			}
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Cancel")) {
			closePopup();
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);
}

void PickBonePopup::Draw(ImVec2 pos, ChannelElement& element, std::function<void()> closePopup)
{
	ImGui::OpenPopup("PickBonePopup");

	ImVec2 size(200, 75);
	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	if (ImGui::BeginPopupModal("PickBonePopup", nullptr, popupChildFlag))
	{
		ImGui::SetNextItemWidth(size.x);
		ImGui::PushID("ElementTypeBoneName");
		ImGui::DrawComboSelection(element.trigger.bone, bones, [&](std::string selectedBone)
			{
				element.trigger.bone = selectedBone;
			}
		);
		ImGui::PopID();

		ImGui::Separator();
		if (ImGui::MenuItem("Close")) {
			closePopup();
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);
}
