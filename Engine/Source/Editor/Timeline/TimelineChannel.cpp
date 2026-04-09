#include "pch.h"
#include "TimelineChannel.h"
#include <imgui_internal.h>
#include <ImEditor.h>

TimelineChannel::TimelineChannel(int index)
{
	pos.y = index * frameSize.y;
	size = frameSize;
}

std::tuple<bool, bool, int, int, int, int> TimelineChannel::Draw(Sequence& sequence, int seqChannelId, int initialFrame, int lastFrame, ImVec2 timelinePos, ImVec2 timelineSize, ImVec2 scroll, ImRect channelsRect, bool canInteract)
{
	auto [leftClickedFrame, rightClickedFrame, leftBoundaryDrag, rightBoundaryDrag] = DrawChannelFrames(sequence, seqChannelId, initialFrame, lastFrame, timelinePos, timelineSize, scroll, channelsRect, canInteract);
	auto [deleteChannel, toggle] = DrawChannelToolbar(sequence, seqChannelId, timelinePos, timelineSize, scroll, canInteract);
	return std::make_tuple(deleteChannel, toggle, leftClickedFrame, rightClickedFrame, leftBoundaryDrag, rightBoundaryDrag);
}

std::tuple<bool, bool> TimelineChannel::DrawChannelToolbar(Sequence& sequence, int seqChannelId, ImVec2 timelinePos, ImVec2 timelineSize, ImVec2 scroll, bool canInteract)
{
	bool deleteChannel = false;
	bool toggleChannelExpansion = false;

	SequenceChannel& seqChannel = sequence.sequenceChannels.at(seqChannelId);

	ImVec2 toolbarPos(timelinePos.x, timelinePos.y + pos.y - scroll.y);
	ImVec2 toolbarSize(channelToolbarWidth, size.y);

	if (toolbarPos.y > (timelinePos.y + timelineSize.y) ||
		timelinePos.y > (toolbarPos.y + toolbarSize.y)
		) return std::make_tuple(false, false);

	//draw the border
	std::vector<ImVec2> titlePos = {
		ImVec2(toolbarPos.x,toolbarPos.y),
		ImVec2(toolbarPos.x + toolbarSize.x,toolbarPos.y),
		ImVec2(toolbarPos.x + toolbarSize.x,toolbarPos.y + toolbarSize.y),
		ImVec2(toolbarPos.x,toolbarPos.y + toolbarSize.y),
	};
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddPolyline(titlePos.data(), static_cast<unsigned int>(titlePos.size()), channelToolbarColor, 0, 1.0f);

	//draw the left grey area on which the text resides
	std::vector<ImVec2> toolbarTextArea = {
		ImVec2(toolbarPos.x + 1,toolbarPos.y + 1),
		ImVec2(toolbarPos.x + toolbarSize.x - 35,toolbarPos.y + toolbarSize.y),
	};
	draw_list->AddRectFilled(toolbarTextArea.at(0), toolbarTextArea.at(1), channelToolbarNameAreaColor);

	//draw the input text so the channel name can be changed
	ImVec2 nameSize = ImGui::CalcTextSize(seqChannel.name.c_str());
	ImVec2 namePos = ImVec2(toolbarPos.x, toolbarPos.y);
	ImGui::SetCursorScreenPos(namePos);
	std::string name = seqChannel.name;
	std::string inputId = "seqChannel" + std::to_string(seqChannelId);
	float inputWidth = toolbarSize.x - 35;
	ImGui::PushID(inputId.c_str());
	ImGui::PushItemWidth(inputWidth);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4());
	ImGui::PushStyleColor(ImGuiCol_Text, channelToolbarNameColor);
	if (ImGui::InputText("##", &name))
	{
		seqChannel.name = name;
	}
	ImGui::PopStyleColor(2);
	ImGui::PopItemWidth();
	ImGui::PopID();

	//draw the delete channel button
	ImVec2 deleteChannelBtnPos(
		toolbarPos.x + toolbarSize.x - deleteChannelBtnSize.x - 20.0f,
		toolbarPos.y + 0.5f * (frameSize.y - deleteChannelBtnSize.y)
	);
	std::string deleteChannelButtonId = "DeleteChannelButton" + std::to_string(seqChannelId);
	ImGui::PushID(deleteChannelButtonId.c_str());
	if (DrawMinusButton(deleteChannelBtnPos, deleteChannelBtnSize, canInteract))
	{
		deleteChannel = true;
	}
	ImGui::PopID();

	//draw the expansion channel toggle button
	std::string expandChannelButtonId = "ExpandChannelButton" + std::to_string(seqChannelId);
	ImGui::PushID(expandChannelButtonId.c_str());
	ImVec2 expandChannelBtnSize(deleteChannelBtnSize);
	ImVec2 expandChannelBtnPos(deleteChannelBtnPos.x + 2 + deleteChannelBtnSize.x, deleteChannelBtnPos.y);
	if (DrawExpandButton(expandChannelBtnPos, expandChannelBtnSize, canInteract))
	{
		toggleChannelExpansion = true;
	}

	ImGui::PopID();

	return std::make_tuple(deleteChannel, toggleChannelExpansion);
}

bool TimelineChannel::DrawMinusButton(ImVec2 pos, ImVec2 size, bool canInteract)
{
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0)); // Transparent background
	if (canInteract)
	{
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 0.5f)); // Subtle hover effect
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.7f, 0.7f, 0.7f)); // Subtle active effect
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
	}
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // White border
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.5f); // 1 pixel border
	ImGui::SetCursorScreenPos(pos);
	bool ret = false;
	ret = ImGui::Button("##", size) && canInteract;
	ImGui::PopStyleVar(); // Pop FrameBorderSize
	ImGui::PopStyleColor(5); // Pop the four style colors

	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	ImVec2 h0(pos.x, pos.y + size.y * 0.5f - .5f);
	ImVec2 h1(pos.x + size.x, pos.y + size.y * 0.5f - .5f);
	draw_list->AddLine(h0, h1, deleteChannelButtonGlyphColor);

	return ret;
}

bool TimelineChannel::DrawExpandButton(ImVec2 pos, ImVec2 size, bool canInteract)
{
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0)); // Transparent background
	if (canInteract)
	{
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 0.5f)); // Subtle hover effect
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.7f, 0.7f, 0.7f)); // Subtle active effect
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
	}
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // White border
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.5f); // 1 pixel border
	ImGui::SetCursorScreenPos(pos);
	bool ret = ImGui::Button("##", size) && canInteract;
	ImVec2 textPos(pos);
	textPos.y -= 3.0f;
	textPos.x += 2.0f;
	ImGui::SetCursorScreenPos(textPos);
	ImGui::Text(expanded ? "c" : "e");

	ImGui::PopStyleVar(); // Pop FrameBorderSize
	ImGui::PopStyleColor(5); // Pop the four style colors

	return ret;
}

std::tuple<int, int, int, int> TimelineChannel::DrawChannelFrames(Sequence& sequence, int seqChannelId, int initialFrame, int lastFrame, ImVec2 timelinePos, ImVec2 timelineSize, ImVec2 scroll, ImRect channelsRect, bool canInteract)
{
	std::tuple<int, int, int, int> clicked = frameWithNoAction;
	SequenceChannel& seqChannel = sequence.sequenceChannels.at(seqChannelId);
	ImVec2 framePos(timelinePos.x + channelToolbarWidth - scroll.x + static_cast<float>(initialFrame) * size.x, timelinePos.y + pos.y - scroll.y);

	ImVec2 clipMin(timelinePos.x + channelToolbarWidth, timelinePos.y);
	ImVec2 clipMax(clipMin.x + timelineSize.x - channelToolbarWidth, clipMin.y + timelineSize.y);

	int dashedFirstFrame = (selectedElement != -1) ? seqChannel.elements.at(selectedElement).GetFrameStart() : -1;
	int dashedLastFrame = (selectedElement != -1) ? seqChannel.elements.at(selectedElement).GetFrameEnd() : -1;
	int numLines = (selectedElement != -1) ? static_cast<int>(std::floor(size.y / verticalDistanceBetweenDashedLines)) : -1;

	ImRect framesRect(clipMin, clipMax);
	ImGui::PushClipRect(clipMin, clipMax, true);
	{
		ImVec2 pMin(framePos.x, framePos.y);
		ImVec2 pMax(framePos.x + frameSize.x - 1, framePos.y + size.y - 1);

		for (int i = initialFrame; i <= lastFrame; i++)
		{
			bool dashed = i >= dashedFirstFrame && i <= dashedLastFrame;
			auto [leftClickFrame, rightClickFrame, leftBoundaryDrag, rightBoundaryDrag] = DrawChannelFrame(seqChannel, i, pMin, pMax, channelsRect, framesRect, timelineSize, dashed ? numLines : -1, canInteract);
			if (leftClickFrame) std::get<0>(clicked) = i;
			if (rightClickFrame) std::get<1>(clicked) = i;
			if (leftBoundaryDrag) std::get<2>(clicked) = i;
			if (rightBoundaryDrag) std::get<3>(clicked) = i;
			pMin.x += frameSize.x;
			pMax.x += frameSize.x;
		}

		std::vector<ChannelElement> animations;
		std::copy_if(seqChannel.elements.begin(), seqChannel.elements.end(), std::back_inserter(animations), [](ChannelElement& elem)
			{
				return elem.type == SCET_Animation;
			}
		);

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		for (auto& ce : animations)
		{
			ImVec2 tagPos(
				timelinePos.x + channelToolbarWidth - scroll.x + static_cast<float>(ce.animation.GetFrameStart()) * size.x,
				timelinePos.y + pos.y - scroll.y
			);
			ImVec2 minText(tagPos.x, tagPos.y);
			ImVec2 maxText(tagPos.x + (ce.animation.GetFrameEnd() - ce.animation.GetFrameStart()) * size.x, tagPos.y + size.y);
			ImGui::PushClipRect(minText, maxText, true);
			{
				draw_list->AddText(tagPos, rgba(0, 0, 0, 1.0f), ce.animation.animation.c_str());
			}
			ImGui::PopClipRect();
		}
	}
	ImGui::PopClipRect();
	return clicked;
}

std::tuple<bool, bool, bool, bool> TimelineChannel::DrawChannelFrame(SequenceChannel& seqChannel, int frame, ImVec2 pMin, ImVec2 pMax, ImRect channelsRect, ImRect framesRect, ImVec2 timelineSize, int numLines, bool canInteract)
{
	ImGuiIO& io = ImGui::GetIO();
	ImVec2 mouse = io.MousePos;
	ImRect thumbRect(pMin, pMax);

	ImU32 bgColor = (frame % 5 != 0 || frame == 0) ? frameColor : frame5Color;

	bool mouseInFrame = canInteract && channelsRect.Contains(mouse) && framesRect.Contains(mouse) && thumbRect.Contains(mouse);
	bool leftBounded = false;
	bool rightBounded = false;
	bool leftBoundaryDrag = false;
	bool rightBoundaryDrag = false;
	bool frameHasElement = seqChannel.FrameHasElement(frame, leftBounded, rightBounded);
	auto animElement = seqChannel.GetAnimationElementAtFrame(frame);
	bool forward = true;
	bool animElementFirstFrame = false;
	bool animElementLastFrame = false;
	if (animElement)
	{
		forward = animElement->forward;
		animElementFirstFrame = animElement->GetFrameStart() == frame;
		animElementLastFrame = animElement->GetFrameEnd() == frame;
	}

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	auto painter = ImGui::GetForegroundDrawList();

	if (!frameHasElement)
	{
		draw_list->AddRectFilled(pMin, pMax, (mouseInFrame || selectedFrame == frame) ? frameMouseOverColor : bgColor);
		draw_list->AddRect(pMin, pMax, frameBorderColor);
	}
	else
	{
		draw_list->AddRectFilled(pMin, ImVec2(pMax.x + 1, pMax.y), (mouseInFrame || selectedFrame == frame) ? frameMouseOverColor : frameWithElementColor);

		if (numLines != -1)
		{
			ImGui::PushClipRect(pMin, pMax, true);
			{
				ImVec2 p0(pMin.x, pMax.y);
				ImVec2 p1(p0.x + size.x, p0.y - size.x);
				for (int i = 0; i < numLines; i++)
				{
					draw_list->AddLine(p0, p1, selectedElementLinesColor);
					p0.y -= verticalDistanceBetweenDashedLines + 1;
					p1.y -= verticalDistanceBetweenDashedLines + 1;
				}
			}
			ImGui::PopClipRect();
		}

		draw_list->AddLine(ImVec2(pMin.x, pMin.y), ImVec2(pMax.x + 1, pMin.y), frameWithElementBorderColor);
		draw_list->AddLine(ImVec2(pMin.x, pMax.y), ImVec2(pMax.x + 1, pMax.y), frameWithElementBorderColor);

		if (leftBounded)
		{
			leftBoundaryDrag = mouseInFrame && (mouse.x - pMin.x) < frameBorderMouseDistance;
			float thickAdj = leftBoundaryDrag ? frameBorderLeftThicknessAdjustment : 0.0f;
			float thickness = leftBoundaryDrag ? frameBorderMouseThickness : 1.0f;
			draw_list->AddLine(ImVec2(pMin.x + thickAdj, pMin.y), ImVec2(pMin.x + thickAdj, pMax.y), frameWithElementBorderColor, thickness);
		}
		if (leftBounded)
		{
			ImVec2 center(
				pMin.x + (pMax.x - pMin.x) * 0.5f,
				pMax.y - 3
			);
			float radius = (pMax.x - pMin.x) / 4.0f;
			draw_list->AddCircleFilled(center, radius, frameCircleColor, 10);
		}
		if (seqChannel.FrameHasTransformationKeyframe(frame))
		{
			ImVec2 center(
				pMin.x + (pMax.x - pMin.x) * 0.5f,
				pMin.y + 5
			);
			float radius = (pMax.x - pMin.x) / 4.0f;
			draw_list->AddCircleFilled(center, radius, frameTransformationKeyFrameCircleColor, 10);
		}
		if (rightBounded)
		{
			rightBoundaryDrag = mouseInFrame && (pMax.x - mouse.x) < frameBorderMouseDistance;
			float thickAdj = rightBoundaryDrag ? frameBorderRightThicknessAdjustment : 0.0f;
			float thickness = rightBoundaryDrag ? frameBorderMouseThickness : 1.0f;
			draw_list->AddLine(ImVec2(pMax.x + thickAdj, pMin.y), ImVec2(pMax.x + thickAdj, pMax.y), frameWithElementBorderColor, thickness);
		}

		if (animElement)
		{
			float arry = pMin.y + (pMax.y - pMin.y) * 0.8f; // + 0.5f * (pMax.y - pMin.y);// -3.0f;
			if ((!leftBounded && !rightBounded) || (leftBounded && !rightBounded && forward) || (rightBounded && !leftBounded && !forward))
			{
				draw_list->AddLine(ImVec2(pMin.x - 1, arry), ImVec2(pMax.x + 1, arry), animationLineColor);
			}
			if (rightBounded && forward)
			{
				draw_list->AddTriangleFilled(
					ImVec2(pMin.x + 1, arry - 3),
					ImVec2(pMin.x + 1, arry + 3),
					ImVec2(pMin.x + 1 + 4, arry),
					animationLineColor
				);
			}
			if (leftBounded && !forward)
			{
				draw_list->AddTriangleFilled(
					ImVec2(pMax.x, arry - 3),
					ImVec2(pMax.x, arry + 3),
					ImVec2(pMax.x - 5, arry),
					animationLineColor
				);
			}
		}
	}

	if (!mouseInFrame)
		return std::make_tuple(false, false, false, false);

	return std::make_tuple(
		ImGui::IsMouseDown(0) && !(leftBoundaryDrag || rightBoundaryDrag),
		ImGui::IsMouseDown(1),
		ImGui::IsMouseDown(0) && leftBoundaryDrag,
		ImGui::IsMouseDown(0) && rightBoundaryDrag
	);
}

void TimelineChannel::ToggleExpansion()
{
	expanded = !expanded;
	size.y = expanded ? frameSizeExpanded.y : frameSize.y;
}

void TimelineChannel::SetPosAfter(const TimelineChannel& prev)
{
	pos.y = prev.pos.y;
	if (!prev.expanded)
		pos.y += frameSize.y;
	else
		pos.y += frameSizeExpanded.y;
}

void TimelineChannel::ResetSelection()
{
	selectedFrame = -1;
	selectedElement = -1;
}
