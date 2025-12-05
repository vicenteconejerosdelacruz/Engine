#include "pch.h"
#include "TimelineEditor.h"

void TimelineEditor::Init(RenderableUUID renderable, Sequence& sequence)
{
	this->renderable = renderable;
	channels.clear();
	for (int i = 0; i < sequence.sequenceChannels.size(); i++)
	{
		channels.push_back(TimelineChannel(i));
	}
}

void TimelineEditor::Reset()
{
	renderable = "";

	scroll = ImVec2(0.0f, 0.0f);
	scrollbarLastMousePos = ImVec2(0.0f, 0.0f);
	scrollbarMouseClicked[0] = false;
	scrollbarMouseClicked[1] = false;
	selectedFrameInTimeline = -1;
	channels.clear();
	popup = TP_None;
	popupCoords = ImVec2(0.0f, 0.0f);
	popupChannelFrame = std::make_tuple(-1, -1);
	markerMouseDrag = false;
	markerLastMousePos = ImVec2(0.0f, 0.0f);

	selectedChannelElement = std::make_tuple(-1, -1);
	elementLastMousePos = ImVec2(0.0f, 0.0f);
	elementDrag = false;
	elementDragXSum = 0.0f;

	//left boundary drag
	elementDragLeftBoundary = false;
	elementDragLeftBoundaryMousePos = ImVec2(0.0f, 0.0f);
	selectedDragLeftBoundaryChannelElement = std::make_tuple(-1, -1);
	elementDragLeftBoundaryXSum = 0.0f;

	//right boundary drag
	elementDragRightBoundary = false;
	elementDragRightBoundaryMousePos = ImVec2(0.0f, 0.0f);
	selectedDragRightBoundaryChannelElement = std::make_tuple(-1, -1);
	elementDragRightBoundaryXSum = 0.0f;
}

void TimelineEditor::DrawRect(ImVec2 pos, ImVec2 size, ImU32 color)
{
	ImVec2 p1(pos);
	ImVec2 p2(pos.x + size.x, pos.y + size.y);
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddRectFilled(p1, p2, color);
}

void TimelineEditor::DrawBackground(ImVec2 pos, ImVec2 size)
{
	//top-left(toolbars)
	ImVec2 channelsToolbarBgPos(pos.x, pos.y);
	ImVec2 channelsToolbarBgSize(TimelineChannel::channelToolbarWidth, markersBgHeight);
	DrawRect(channelsToolbarBgPos, channelsToolbarBgSize, markersBgColor);

	//bottom-left(channels)
	ImVec2 channelsBgPos(pos.x, pos.y + markersBgHeight);
	ImVec2 channelsBgSize(TimelineChannel::channelToolbarWidth, size.y - markersBgHeight);
	DrawRect(channelsBgPos, channelsBgSize, channelsBgColor);

	//bottom-right(timeline)
	ImVec2 timelineBgPos(pos.x + TimelineChannel::channelToolbarWidth + 1, pos.y + markersBgHeight);
	ImVec2 timelineBgSize(size.x - TimelineChannel::channelToolbarWidth, size.y - markersBgHeight);
	DrawRect(timelineBgPos, timelineBgSize, timelineBgColor);

	//top-right(timeline markers)
	ImVec2 timelineMarkersBgPos(pos.x + TimelineChannel::channelToolbarWidth + 1, pos.y);
	ImVec2 timelineMarkersBgSize(size.x - TimelineChannel::channelToolbarWidth, markersBgHeight);
	DrawRect(timelineMarkersBgPos, timelineMarkersBgSize, markersBgColor);
}

bool TimelineEditor::DrawPlusButton(ImVec2 pos, ImVec2 size, bool canInteract)
{
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0)); // Transparent background
	if (canInteract)
	{
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 0.5f)); // Subtle hover effect
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.7f, 0.7f, 0.7f)); // Subtle active effect
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0)); // Subtle hover effect
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0)); // Subtle active effect
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
	draw_list->AddLine(h0, h1, addChannelButtonGlyphColor);

	ImVec2 v0(pos.x + size.x * 0.5f - .5f, pos.y);
	ImVec2 v1(pos.x + size.x * 0.5f - .5f, pos.y + size.y);
	draw_list->AddLine(v0, v1, addChannelButtonGlyphColor);

	return ret;
};

void TimelineEditor::DrawAddChannelButton(Sequence& sequence, ImVec2 pos, bool canInteract)
{
	ImVec2 addChannelButtonPos(pos.x + 5, pos.y + 2);
	ImVec2 addChannelButtonSize(markersBgHeight - 6, markersBgHeight - 6);
	ImGui::PushID("AddChannelButton");
	if (DrawPlusButton(addChannelButtonPos, addChannelButtonSize, canInteract))
	{
		std::string name = "Channel " + std::to_string(1 + sequence.sequenceChannels.size());
		sequence.sequenceChannels.push_back(SequenceChannel(name));
		int last = static_cast<int>(channels.size()) - 1;
		channels.push_back(TimelineChannel());
		if (last >= 0)
		{
			channels.back().SetPosAfter(channels.at(last));
		}
	}
	ImGui::PopID();
}

void TimelineEditor::DrawTimeline(Sequence& sequence, ImVec2 timelinePos, ImVec2 timelineSize, bool canInteract,
	std::function<void(TransformationKeyFrame*, int)> setTransformationKeyFrame
)
{
	ImGuiIO& io = ImGui::GetIO();

	int deleteChannelId = -1;
	int expandChannelId = -1;
	std::tuple<int, int> selectedChannelFrame(-1, -1);
	std::tuple<int, int> actionChannelFrame(-1, -1);
	std::tuple<int, int> dragLeftBoundaryChannelElement(-1, -1);
	std::tuple<int, int> dragRightBoundaryChannelElement(-1, -1);

	int initialFrame = GetFirstVisibleFrame();
	int lastFrame = GetLastVisibleFrame(sequence, initialFrame, timelineSize);

	ImVec2 clipMin(timelinePos);
	ImVec2 clipMax(timelinePos.x + timelineSize.x, timelinePos.y + timelineSize.y);

	ImRect channelsRect(
		ImVec2(
			timelinePos.x + TimelineChannel::channelToolbarWidth,
			timelinePos.y
		),
		ImVec2(
			timelinePos.x + timelineSize.x,
			timelinePos.y + timelineSize.y
		)
	);
	ImGui::PushClipRect(clipMin, clipMax, true);
	{
		int i = 0;
		for (TimelineChannel& tlc : channels)
		{
			auto [
				deleteChannel, expandChannel,
				leftClickedFrame, rightClickedFrame,
				leftBoundaryDrag, rightBoundaryDrag
			] = tlc.Draw(sequence, i, initialFrame, lastFrame, timelinePos, timelineSize, scroll, channelsRect, canInteract);
			deleteChannelId = deleteChannel ? i : deleteChannelId;
			expandChannelId = expandChannel ? i : expandChannelId;
			selectedChannelFrame = (leftClickedFrame != -1) ? std::make_tuple(i, leftClickedFrame) : selectedChannelFrame;
			actionChannelFrame = (rightClickedFrame != -1) ? std::make_tuple(i, rightClickedFrame) : actionChannelFrame;
			if (leftClickedFrame != -1)
			{
				int elementId = sequence.sequenceChannels.at(i).GetFirstElementIndexBetweenFrames(leftClickedFrame, leftClickedFrame);
				selectedChannelElement = std::make_tuple(elementId != -1 ? i : -1, elementId);
				if (elementId == -1)
				{
					SelectElementInChannel(0, -1); //give 0 to reset channel selection something, this will reset and do nothing after actually because of the -1 frame
				}
			}
			if (leftBoundaryDrag != -1)
			{
				int elementId = sequence.sequenceChannels.at(i).GetFirstElementIndexBetweenFrames(leftBoundaryDrag, leftBoundaryDrag);
				dragLeftBoundaryChannelElement = std::make_tuple(elementId != -1 ? i : -1, elementId);
			}
			if (rightBoundaryDrag != -1)
			{
				int elementId = sequence.sequenceChannels.at(i).GetFirstElementIndexBetweenFrames(rightBoundaryDrag, rightBoundaryDrag);
				dragRightBoundaryChannelElement = std::make_tuple(elementId != -1 ? i : -1, elementId);
			}
			i++;
		}
	}
	ImGui::PopClipRect();

	if (deleteChannelId != -1)
	{
		DeleteChannel(sequence, deleteChannelId, timelinePos, timelineSize);
	}
	if (expandChannelId != -1)
	{
		ToggleChannelExpansion(expandChannelId, timelinePos, timelineSize);
	}
	if (std::get<0>(selectedChannelFrame) != -1 && std::get<1>(selectedChannelFrame) != -1)
	{
		auto& [channel, frame] = selectedChannelFrame;
		auto* elem = sequence.sequenceChannels.at(channel).GetTransformationKeyframe(frame);
		setTransformationKeyFrame(elem, frame);
		SelectFrameInChannel(channel, frame);
	}
	if (std::get<0>(actionChannelFrame) != -1 && std::get<1>(actionChannelFrame) != -1)
	{
		auto& [channel, frame] = actionChannelFrame;

		int elementId = sequence.sequenceChannels.at(channel).GetFirstElementIndexBetweenFrames(frame, frame);
		selectedChannelElement = std::make_tuple(elementId != -1 ? channel : -1, elementId);
		if (elementId == -1)
		{
			SelectElementInChannel(0, -1); //give 0 to reset channel selection something, this will reset and do nothing after actually because of the -1 frame
		}
		CreatePopupForItemAt(sequence, channel, frame, io.MousePos);
	}
	if (std::get<0>(selectedChannelElement) != -1 && std::get<1>(selectedChannelElement) != -1)
	{
		auto& [channel, elementId] = selectedChannelElement;
		SelectElementInChannel(channel, elementId);
	}
	if (std::get<0>(dragLeftBoundaryChannelElement) != -1 && std::get<1>(dragLeftBoundaryChannelElement) != -1)
	{
		auto& [channel, elementId] = dragLeftBoundaryChannelElement;
		SelectElementToDragFromLeft(channel, elementId);
	}
	if (std::get<0>(dragRightBoundaryChannelElement) != -1 && std::get<1>(dragRightBoundaryChannelElement) != -1)
	{
		auto& [channel, elementId] = dragRightBoundaryChannelElement;
		SelectElementToDragFromRight(channel, elementId);
	}
}

void TimelineEditor::DrawVerticalScrollbar(Sequence& sequence, ImVec2 timelinePos, ImVec2 timelineSize, bool canInteract)
{
	float scrollbarSize = ImGui::GetStyle().ScrollbarSize;
	ImVec2 vScrollbarPos = ImVec2(timelinePos.x + timelineSize.x - scrollbarSize, timelinePos.y);
	ImVec2 vScrollbarSize = ImVec2(scrollbarSize, timelineSize.y);

	float channelsHeightSum = std::accumulate(channels.begin(), channels.end(), 0.0f, [](float acc, TimelineChannel ch)
		{
			return acc + ch.size.y;
		}
	);

	float vThumbHeight = std::min(std::floor(timelineSize.y * (vScrollbarSize.y / channelsHeightSum)), timelineSize.y);

	//get the axis we are working
	float* scrollValue = &scroll.y;
	float maxScroll = channelsHeightSum - vScrollbarSize.y;
	float* mousePos = &scrollbarLastMousePos.y;
	float axisSize = timelineSize.y;
	bool* mouseClicked = &scrollbarMouseClicked[1];
	bool otherAxisClicked = scrollbarMouseClicked[0];

	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	//Draw the scrollbar track
	ImRect trackRect(vScrollbarPos, ImVec2(vScrollbarPos.x + vScrollbarSize.x, vScrollbarPos.y + vScrollbarSize.y));
	draw_list->AddRectFilled(trackRect.Min, trackRect.Max, scrollbarBgColor);

	//set the thumb rect depending of vertical
	float ratio = (*scrollValue) / channelsHeightSum;
	ImVec2 thumbPos0(vScrollbarPos.x, vScrollbarPos.y + ratio * vScrollbarSize.y);
	ImVec2 thumbPos1(thumbPos0.x + vScrollbarSize.x, thumbPos0.y + vThumbHeight);

	//draw the thumb
	draw_list->AddRectFilled(thumbPos0, thumbPos1, scrollbarThumbColor);

	//handle the mouse movement
	ImRect thumbRect(thumbPos0, thumbPos1);
	ImGuiIO& io = ImGui::GetIO();

	if (io.MouseWheel != 0.0f && trackRect.Contains(io.MousePos) && canInteract)
	{
		ScrollVertical(sequence, timelineSize, -io.MouseWheel * 50.0f);
	}

	if (!otherAxisClicked)
	{
		if (ImGui::IsMouseDown(0) && (thumbRect.Contains(io.MousePos) || *mouseClicked) && canInteract)
		{
			float mouseAxisPos = io.MousePos.y;
			if (*mouseClicked)
			{
				//adjust proportional to the movement to the actual available scrollable size
				float diff = (mouseAxisPos - *mousePos);
				ScrollVertical(sequence, timelineSize, diff);
			}
			*mousePos = mouseAxisPos;
			*mouseClicked = true;
		}
		else if (*mouseClicked && !ImGui::IsMouseDown(0))
		{
			*mouseClicked = false;
		}
	}
}

void TimelineEditor::DrawHorizontalScrollbar(Sequence& sequence, ImVec2 timelinePos, ImVec2 timelineSize, bool canInteract)
{
	float scrollbarSize = ImGui::GetStyle().ScrollbarSize;
	ImVec2 hScrollbarPos = ImVec2(timelinePos.x + TimelineChannel::channelToolbarWidth, timelinePos.y + timelineSize.y + 1);
	ImVec2 hScrollbarSize = ImVec2(timelineSize.x - TimelineChannel::channelToolbarWidth - scrollbarSize, scrollbarSize);

	float framesWidthSum = sequence.totalFrames * TimelineChannel::frameSize.x;

	float hThumbWidth = std::floor((timelineSize.x - TimelineChannel::channelToolbarWidth - scrollbarSize) * (hScrollbarSize.x / framesWidthSum));
	hThumbWidth = std::min(hThumbWidth, hScrollbarSize.x);

	//get the axis we are working
	float* scrollValue = &scroll.x;
	float maxScroll = framesWidthSum - hScrollbarSize.x + TimelineChannel::frameSize.x * 2;
	float* mousePos = &scrollbarLastMousePos.x;
	float axisSize = timelineSize.x;
	bool* mouseClicked = &scrollbarMouseClicked[0];
	bool otherAxisClicked = scrollbarMouseClicked[1];

	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	//Draw the scrollbar track
	ImRect trackRect(hScrollbarPos, ImVec2(hScrollbarPos.x + hScrollbarSize.x, hScrollbarPos.y + hScrollbarSize.y));
	draw_list->AddRectFilled(trackRect.Min, trackRect.Max, scrollbarBgColor);

	//set the thumb rect depending of horizontal
	//float ratio = (*scrollValue) / (framesWidthSum + TimelineChannel::frameSize.x * 2);
	float ratio = std::max((*scrollValue - TimelineChannel::frameSize.x * 2) / (framesWidthSum), 0.0f);
	ImVec2 thumbPos0(hScrollbarPos.x + ratio * hScrollbarSize.x, hScrollbarPos.y);
	ImVec2 thumbPos1(thumbPos0.x + hThumbWidth, thumbPos0.y + hScrollbarSize.y);

	//draw the thumb
	draw_list->AddRectFilled(thumbPos0, thumbPos1, scrollbarThumbColor);

	//handle the mouse movement
	ImRect thumbRect(thumbPos0, thumbPos1);
	ImGuiIO& io = ImGui::GetIO();

	if (io.MouseWheel != 0.0f && trackRect.Contains(io.MousePos) && canInteract)
	{
		ScrollHorizontal(sequence, timelineSize, -io.MouseWheel * 50.0f);
	}

	if (!otherAxisClicked)
	{
		if (ImGui::IsMouseDown(0) && (thumbRect.Contains(io.MousePos) || *mouseClicked) && canInteract)
		{
			float mouseAxisPos = io.MousePos.x;
			if (*mouseClicked)
			{
				float diff = (mouseAxisPos - *mousePos);
				ScrollHorizontal(sequence, timelineSize, diff);
			}
			*mousePos = mouseAxisPos;
			*mouseClicked = true;
		}
		else if (*mouseClicked && !ImGui::IsMouseDown(0))
		{
			*mouseClicked = false;
		}
	}
}

void TimelineEditor::DrawMarkers(Sequence& sequence, ImVec2 timelinePos, ImVec2 timelineSize, bool canInteract)
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	ImVec2 clipMin(timelinePos.x + TimelineChannel::channelToolbarWidth, timelinePos.y - markersBgHeight);
	ImVec2 clipMax(timelinePos.x + timelineSize.x, timelinePos.y);
	ImGui::PushClipRect(clipMin, clipMax, true);

	//so we don't render more than we should we get the first visible frame and last visible(or close)
	int initialFrame = GetFirstVisibleFrame();
	int lastFrame = GetLastVisibleFrame(sequence, initialFrame, timelineSize);

	//xpos of the line markers uses scroll but are adjusted to the initial frame position
	float xpos = timelinePos.x + TimelineChannel::channelToolbarWidth - scroll.x + static_cast<float>(initialFrame) * TimelineChannel::frameSize.x;
	float y0 = timelinePos.y - 1;
	float y1 = y0 - markersLineHeight;
	for (int i = initialFrame; i <= lastFrame; i++)
	{
		draw_list->AddLine(ImVec2(xpos, y0), ImVec2(xpos, y1), markersLineColor);
		xpos += TimelineChannel::frameSize.x;
	}

	ImGui::PushStyleColor(ImGuiCol_Text, markersTextColor);

	//we get the frame texts for only the visible ones, only frames divisible by 5 and 1
	//get get the number of markers and get the offset of the markers in the timeline
	std::vector<std::string> markersTexts;
	int numMarkers = (lastFrame - initialFrame) / 5;
	int markerOffsetX = static_cast<int>(std::ceil(initialFrame / 5.0f) * 5.0f);
	for (int i = 0; i <= numMarkers; i++)
	{
		int frame = i * 5 + markerOffsetX;
		markersTexts.push_back((frame == 0U) ? "1" : std::to_string(frame));
	}

	//draw the numbers and adjust the scrolling by this offset
	float textHeight = ImGui::CalcTextSize("0").y;
	ImVec2 textPos(timelinePos.x + TimelineChannel::channelToolbarWidth - scroll.x + markerOffsetX * TimelineChannel::frameSize.x, y1 - textHeight + 2);
	for (int i = 0; i < markersTexts.size(); i++)
	{
		ImGui::SetCursorScreenPos(textPos);
		ImGui::Text(markersTexts.at(i).c_str());
		textPos.x += markersFramesBetweenTexts * TimelineChannel::frameSize.x;
	}

	ImGui::PopStyleColor();
	ImGui::PopClipRect();

	ImGuiIO& io = ImGui::GetIO();
	ImRect mouseAreaRect(clipMin, clipMax);

	if (ImGui::IsMouseDown(0) && !markerMouseDrag && mouseAreaRect.Contains(io.MousePos) && canInteract)
	{
		markerMouseDrag = true;
		markerLastMousePos = io.MousePos;
	}
	else if (ImGui::IsMouseDown(0) && markerMouseDrag)
	{
		ImVec2 mouseMinMax(clipMin.x + (clipMax.x - clipMin.x) * 0.25f, clipMin.x + (clipMax.x - clipMin.x) * 0.75f);

		ImVec2 mousePos = io.MousePos;
		float diff = (mousePos.x - markerLastMousePos.x);
		markerLastMousePos = mousePos;

		if (nostd::in_between(mousePos.x, mouseMinMax.x, mouseMinMax.y))
		{
			SetFrameAtMouseXCoord(sequence, clipMin, mousePos);
		}
		else if (mousePos.x > mouseMinMax.y)
		{
			ScrollAndSetFrame(sequence, timelineSize, clipMin, mousePos, 0.1f * (mousePos.x - mouseMinMax.y), (clipMax.x - clipMin.x));
		}
		else if (mousePos.x < mouseMinMax.x)
		{
			ScrollAndSetFrame(sequence, timelineSize, clipMin, mousePos, 0.1f * (mousePos.x - mouseMinMax.x), (clipMax.x - clipMin.x));
		}

	}
	else if (!ImGui::IsMouseDown(0) && markerMouseDrag)
	{
		markerMouseDrag = false;
	}
}

void TimelineEditor::DrawSelectedFrameVerticalLine(ImVec2 timelinePos, ImVec2 timelineSize)
{
	if (selectedFrameInTimeline == -1) return;

	ImVec2 p1(
		timelinePos.x + TimelineChannel::channelToolbarWidth - scroll.x + static_cast<float>(selectedFrameInTimeline) * TimelineChannel::frameSize.x,
		timelinePos.y - markersBgHeight
	);
	ImVec2 p2(
		p1.x + TimelineChannel::frameSize.x,
		timelinePos.y + timelineSize.y
	);

	ImVec2 clipMin(timelinePos.x + TimelineChannel::channelToolbarWidth, p1.y);
	ImVec2 clipMax(clipMin.x - TimelineChannel::channelToolbarWidth + timelineSize.x/* - ImGui::GetStyle().ScrollbarSize*/, p2.y);

	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	ImGui::PushClipRect(clipMin, clipMax, true);
	{
		draw_list->AddRectFilled(p1, p2, selectedFrameVerticalColor);
	}
	ImGui::PopClipRect();
}

void TimelineEditor::DrawActionPopup(Sequence& sequence,
	std::function<void(TransformationKeyFrame*, int)> setTransformationKeyFrame,
	std::function<void()> deleteTransformationKeyFrame,
	std::function<void(int channel, int frame, SequenceChannelElementScript*)> setScriptToEdit
)
{
	if (popup == TP_None) return;

	if (popup == TP_AddElement)
	{
		auto& [channel, frame] = popupChannelFrame;

		std::unordered_map<SequenceChannelElementType, std::function<void(SequenceChannelElement*)>> builders = {
			{ SCET_Animation, [this, &sequence, channel](SequenceChannelElement* elem)
			{
				AddAnimationElementToChannel(sequence, channel, static_cast<SequenceChannelElementAnimation*>(elem));
				popup = TP_None;
			}
			},
			{ SCET_Transformation, [this, &sequence, channel](SequenceChannelElement* elem)
			{
				AddTransformationElementToChannel(sequence, channel, static_cast<SequenceChannelElementTransformation*>(elem));
				popup = TP_None;
			}
			},
			{ SCET_SoundFX, [this, &sequence, channel](SequenceChannelElement* elem)
			{
				AddSoundFXElementToChannel(sequence, channel, static_cast<SequenceChannelElementSoundFX*>(elem));
				popup = TP_None;
			}
			},
			{ SCET_Script, [this, &sequence, channel](SequenceChannelElement* elem)
			{
				AddScriptElementToChannel(sequence, channel, static_cast<SequenceChannelElementScript*>(elem));
				popup = TP_None;
			}
			},
		};

		addElementPopup.Draw(popupCoords, builders, [this]() { popup = TP_None; });
	}
	else if (popup == TP_InteractWithElement)
	{
		auto& [channel, frame] = popupChannelFrame;
		auto& seqChannel = sequence.sequenceChannels.at(channel);
		int elementIndex = seqChannel.GetFirstElementIndexBetweenFrames(frame, frame);
		ChannelElement& element = seqChannel.elements.at(elementIndex);

		std::unordered_map<InteractPopups, std::function<void()>> interactions =
		{
			{ IP_Delete, [this, &sequence, channel, frame]()
			{
				DeleteElementInFrameAtChannel(sequence, channel, frame);
				popup = TP_None;
			}
			},
			{ IP_Split, [this, &sequence, channel, frame]()
			{
				SplitElementInFrameAtChannel(sequence, channel, frame);
				popup = TP_None;
			}
			},
			{ IP_Transformation_AddKeyframe, [this, &sequence, channel, frame, setTransformationKeyFrame]()
			{
				AddKeyframeToTransformationElementInFrameAtChannel(sequence, channel, frame);
				TransformationKeyFrame& keyFrame = sequence.sequenceChannels.at(channel).GetTransformationElementAtFrame(frame)->keyFrames.at(frame);
				setTransformationKeyFrame(&keyFrame, frame);
				popup = TP_None;
			}
			},
			{ IP_Transformation_RemoveKeyframe, [this, &sequence, channel, frame,deleteTransformationKeyFrame,setTransformationKeyFrame]()
			{
				setTransformationKeyFrame(nullptr,-1);
				RemoveKeyframeFromTransformationElementInFrameAtChannel(sequence, channel, frame);
				deleteTransformationKeyFrame();
				popup = TP_None;
			}
			},
			{ IP_Script_Edit, [this,&sequence,channel,frame, setScriptToEdit]()
			{
				OpenScriptEditionForElementInFrameAtChannel(sequence,channel,frame, setScriptToEdit);
				popup = TP_None;
			}
			}
		};

		interactElementPopup.Draw(popupCoords, element, frame, interactions, [this]() { popup = TP_None; });
	}
}

void TimelineEditor::Draw(Sequence& sequence, ImVec2 pos, ImVec2 size,
	std::function<void(TransformationKeyFrame*, int)> setTransformationKeyFrame,
	std::function<void()> deleteTransformationKeyFrame,
	std::function<void(int channel, int frame, SequenceChannelElementScript*)> setScriptToEdit
)
{
	auto getTimelineValues = [pos, size]()
		{
			ImVec2 timelinePos(pos.x, pos.y + markersBgHeight);
			ImVec2 timelineSize(size.x, size.y - markersBgHeight);
			return std::make_tuple(timelinePos, timelineSize);
		};
	auto [timelinePos, timelineSize] = getTimelineValues();

	bool canInteract = popup == TP_None;
	bool draging = elementDrag || elementDragLeftBoundary || elementDragRightBoundary;
	DrawBackground(pos, size);
	DrawAddChannelButton(sequence, pos, canInteract && !markerMouseDrag && !draging);
	DrawTimeline(sequence, timelinePos, timelineSize, canInteract && !markerMouseDrag && !draging, setTransformationKeyFrame);
	DrawMarkers(sequence, timelinePos, timelineSize, canInteract && !draging);
	DrawSelectedFrameVerticalLine(timelinePos, timelineSize);
	DrawVerticalScrollbar(sequence, timelinePos, timelineSize, canInteract && !markerMouseDrag && !draging);
	DrawHorizontalScrollbar(sequence, timelinePos, timelineSize, canInteract && !markerMouseDrag && !draging);
	DrawActionPopup(sequence, setTransformationKeyFrame, deleteTransformationKeyFrame, setScriptToEdit);
	HandleElementDrag(sequence);
	HandleElementDragLeftBoundary(sequence);
	HandleElementDragRightBoundary(sequence);
}

void TimelineEditor::HandleElementDrag(Sequence& sequence)
{
	if (std::get<0>(selectedChannelElement) == -1 || std::get<1>(selectedChannelElement) == -1) return;

	ImGuiIO& io = ImGui::GetIO();

	if (ImGui::IsMouseDown(0) && !elementDrag)
	{
		elementDrag = true;
		elementLastMousePos = io.MousePos;
		elementDragXSum = 0.0f;
	}
	else if (ImGui::IsMouseDown(0) && elementDrag)
	{
		ImVec2 mouse = io.MousePos;
		float dx = mouse.x - elementLastMousePos.x;
		elementDragXSum += dx;
		elementLastMousePos = mouse;
		float framesX = elementDragXSum / TimelineChannel::frameSize.x;
		if (std::fabs(framesX) >= 1.0f)
		{
			int frames = framesX >= 1.0f ? static_cast<int>(std::floor(framesX)) : static_cast<int>(std::ceil(framesX));
			auto [channel, element] = selectedChannelElement;
			sequence.sequenceChannels.at(channel).MoveElement(element, frames, sequence.totalFrames, sequence.framesPerSecond);
			elementDragXSum = std::fmodf(elementDragXSum, TimelineChannel::frameSize.x);
		}
	}
	else if (!ImGui::IsMouseDown(0))
	{
		elementDrag = false;
		elementDragXSum = 0.0f;
		selectedChannelElement = std::make_tuple(-1, -1);
	}
}

void TimelineEditor::HandleElementDragLeftBoundary(Sequence& sequence)
{
	if (std::get<0>(selectedDragLeftBoundaryChannelElement) == -1 || std::get<1>(selectedDragLeftBoundaryChannelElement) == -1) return;

	ImGuiIO& io = ImGui::GetIO();

	if (ImGui::IsMouseDown(0) && !elementDragLeftBoundary)
	{
		elementDragLeftBoundary = true;
		elementDragLeftBoundaryMousePos = io.MousePos;
		elementDragLeftBoundaryXSum = 0.0f;
	}
	else if (ImGui::IsMouseDown(0) && elementDragLeftBoundary)
	{
		ImVec2 mouse = io.MousePos;
		float dx = mouse.x - elementDragLeftBoundaryMousePos.x;
		elementDragLeftBoundaryXSum += dx;
		elementDragLeftBoundaryMousePos = mouse;
		float framesX = elementDragLeftBoundaryXSum / TimelineChannel::frameSize.x;
		if (std::fabs(framesX) >= 1.0f)
		{
			int frames = framesX >= 1.0f ? static_cast<int>(std::floor(framesX)) : static_cast<int>(std::ceil(framesX));
			auto [channel, element] = selectedDragLeftBoundaryChannelElement;
			sequence.sequenceChannels.at(channel).DragElementLeftBoundary(element, frames, sequence.totalFrames);
			elementDragLeftBoundaryXSum = std::fmodf(elementDragLeftBoundaryXSum, TimelineChannel::frameSize.x);
		}
	}
	else if (!ImGui::IsMouseDown(0))
	{
		elementDragLeftBoundary = false;
		elementDragLeftBoundaryXSum = 0.0f;
		selectedDragLeftBoundaryChannelElement = std::make_tuple(-1, -1);
	}
}

void TimelineEditor::HandleElementDragRightBoundary(Sequence& sequence)
{
	if (std::get<0>(selectedDragRightBoundaryChannelElement) == -1 || std::get<1>(selectedDragRightBoundaryChannelElement) == -1) return;

	ImGuiIO& io = ImGui::GetIO();

	if (ImGui::IsMouseDown(0) && !elementDragRightBoundary)
	{
		elementDragRightBoundary = true;
		elementDragRightBoundaryMousePos = io.MousePos;
		elementDragRightBoundaryXSum = 0.0f;
	}
	else if (ImGui::IsMouseDown(0) && elementDragRightBoundary)
	{
		ImVec2 mouse = io.MousePos;
		float dx = mouse.x - elementDragRightBoundaryMousePos.x;
		elementDragRightBoundaryXSum += dx;
		elementDragRightBoundaryMousePos = mouse;
		float framesX = elementDragRightBoundaryXSum / TimelineChannel::frameSize.x;
		if (std::fabs(framesX) >= 1.0f)
		{
			int frames = framesX >= 1.0f ? static_cast<int>(std::floor(framesX)) : static_cast<int>(std::ceil(framesX));
			auto [channel, element] = selectedDragRightBoundaryChannelElement;
			sequence.sequenceChannels.at(channel).DragElementRightBoundary(element, frames, sequence.totalFrames);
			elementDragRightBoundaryXSum = std::fmodf(elementDragRightBoundaryXSum, TimelineChannel::frameSize.x);
		}
	}
	else if (!ImGui::IsMouseDown(0))
	{
		elementDragRightBoundary = false;
		elementDragRightBoundaryXSum = 0.0f;
		selectedDragRightBoundaryChannelElement = std::make_tuple(-1, -1);
	}
}

void TimelineEditor::DeleteChannel(Sequence& sequence, int channelId, ImVec2 timelinePos, ImVec2 timelineSize)
{
	nostd::vector_erase_index(sequence.sequenceChannels, channelId);
	nostd::vector_erase_index(channels, channelId);

	//if is the first bump it at the beginning
	if (channelId == 0)
	{
		if (channels.size() > 0ULL)
		{
			channels.begin()->pos.y = 0.0f;
		}
	}

	//recalculate positions
	for (int i = 1; i < channels.size(); i++)
	{
		channels.at(i).SetPosAfter(channels.at(i - 1));
	}

	ClampVerticalScroll(timelinePos, timelineSize);
}

void TimelineEditor::ToggleChannelExpansion(int channelId, ImVec2 timelinePos, ImVec2 timelineSize)
{
	channels.at(channelId).ToggleExpansion();

	//recalculate positions
	for (int i = 1; i < channels.size(); i++)
	{
		channels.at(i).SetPosAfter(channels.at(i - 1));
	}

	ClampVerticalScroll(timelinePos, timelineSize);
}

void TimelineEditor::SelectFrameInChannel(int channelId, int frame)
{
	for (auto& c : channels)
	{
		c.selectedFrame = -1;
	}
	channels.at(channelId).selectedFrame = frame;
	selectedFrameInTimeline = frame;
}

void TimelineEditor::SelectElementInChannel(int channelId, int elementId)
{
	for (auto& c : channels)
	{
		c.selectedElement = -1;
	}
	channels.at(channelId).selectedElement = elementId;
}

void TimelineEditor::SelectElementToDragFromLeft(int channelId, int elementId)
{
	ImGuiIO& io = ImGui::GetIO();

	elementDragLeftBoundary = true;
	elementDragLeftBoundaryMousePos = io.MousePos;
	selectedDragLeftBoundaryChannelElement = std::make_tuple(channelId, elementId);
	elementDragLeftBoundaryXSum = 0.0f;
}

void TimelineEditor::SelectElementToDragFromRight(int channelId, int elementId)
{
	ImGuiIO& io = ImGui::GetIO();

	elementDragRightBoundary = true;
	elementDragRightBoundaryMousePos = io.MousePos;
	selectedDragRightBoundaryChannelElement = std::make_tuple(channelId, elementId);
	elementDragRightBoundaryXSum = 0.0f;
}

void TimelineEditor::CreatePopupForItemAt(Sequence& sequence, int channelId, int frame, ImVec2 popupPosition)
{
	popupCoords = popupPosition;
	popup = (!sequence.sequenceChannels.at(channelId).ChannelHasElementAtFrame(frame)) ? TP_AddElement : TP_InteractWithElement;
	popupChannelFrame = std::make_tuple(channelId, frame);
	if (popup == TP_AddElement)
	{
		addElementPopup.Init(renderable(), frame);
	}
}

void TimelineEditor::ScrollHorizontal(Sequence& sequence, ImVec2 timelineSize, float quantity)
{
	float scrollbarSize = ImGui::GetStyle().ScrollbarSize;
	ImVec2 hScrollbarSize = ImVec2(timelineSize.x - TimelineChannel::channelToolbarWidth - scrollbarSize, scrollbarSize);
	float framesWidthSum = sequence.totalFrames * TimelineChannel::frameSize.x;
	float maxScroll = framesWidthSum - hScrollbarSize.x + TimelineChannel::frameSize.x * 2;
	scroll.x += quantity * (framesWidthSum / hScrollbarSize.x);
	scroll.x = std::clamp((scroll.x), 0.0f, std::max(maxScroll, 0.0f));
}

void TimelineEditor::ScrollVertical(Sequence& sequence, ImVec2 timelineSize, float quantity)
{
	float scrollbarSize = ImGui::GetStyle().ScrollbarSize;
	ImVec2 vScrollbarSize = ImVec2(scrollbarSize, timelineSize.y);
	float channelsHeightSum = std::accumulate(channels.begin(), channels.end(), 0.0f, [](float acc, TimelineChannel ch)
		{
			return acc + ch.size.y;
		}
	);
	float maxScroll = channelsHeightSum - vScrollbarSize.y;
	scroll.y += quantity * (channelsHeightSum / vScrollbarSize.y);;
	scroll.y = std::clamp(scroll.y, 0.0f, std::max(maxScroll, 0.0f));
}

void TimelineEditor::AddAnimationElementToChannel(Sequence& sequence, int channelId, SequenceChannelElementAnimation* elem)
{
	SequenceChannel& seqChannel = sequence.sequenceChannels.at(channelId);
	ChannelElement chanElem;
	chanElem.type = SCET_Animation;
	SequenceChannelElementAnimation& animation = chanElem.animation;
	animation.animation = elem->animation;
	animation.frameStart = elem->frameStart;
	animation.frameEnd = elem->frameStart + GetAnimationNumFrames(sequence, animation.animation);
	animation.startTime = elem->startTime;
	animation.endTime = elem->endTime;
	seqChannel.InsertChannelElement(chanElem, sequence.totalFrames, sequence.framesPerSecond);
}

void TimelineEditor::AddTransformationElementToChannel(Sequence& sequence, int channelId, SequenceChannelElementTransformation* elem)
{
	SequenceChannel& seqChannel = sequence.sequenceChannels.at(channelId);
	ChannelElement chanElem;
	chanElem.type = SCET_Transformation;
	SequenceChannelElementTransformation& transformation = chanElem.transformation;
	transformation.frameStart = elem->frameStart;
	transformation.frameEnd = elem->frameStart;
	seqChannel.InsertChannelElement(chanElem, sequence.totalFrames, sequence.framesPerSecond);
}

void TimelineEditor::AddSoundFXElementToChannel(Sequence& sequence, int channelId, SequenceChannelElementSoundFX* elem)
{
	SequenceChannel& seqChannel = sequence.sequenceChannels.at(channelId);
	ChannelElement chanElem;
	chanElem.type = SCET_SoundFX;
	SequenceChannelElementSoundFX& soundfx = chanElem.soundfx;
	soundfx.sound = elem->sound;
	soundfx.frameStart = elem->frameStart;
	soundfx.frameEnd = elem->frameEnd;
	seqChannel.InsertChannelElement(chanElem, sequence.totalFrames, sequence.framesPerSecond);
}

void TimelineEditor::AddScriptElementToChannel(Sequence& sequence, int channelId, SequenceChannelElementScript* elem)
{
	SequenceChannel& seqChannel = sequence.sequenceChannels.at(channelId);
	ChannelElement chanElem;
	chanElem.type = SCET_Script;
	SequenceChannelElementScript& script = chanElem.script;
	script.frameStart = elem->frameStart;
	script.frameEnd = elem->frameStart;
	seqChannel.InsertChannelElement(chanElem, sequence.totalFrames, sequence.framesPerSecond);
}

void TimelineEditor::DeleteElementInFrameAtChannel(Sequence& sequence, int channelId, int frame)
{
	SequenceChannel& seqChannel = sequence.sequenceChannels.at(channelId);
	seqChannel.EraseElementInFrame(frame);
	channels.at(channelId).ResetSelection();
}

void TimelineEditor::SplitElementInFrameAtChannel(Sequence& sequence, int channelId, int frame)
{
	SequenceChannel& seqChannel = sequence.sequenceChannels.at(channelId);
	seqChannel.SplitElementInFrame(frame);
}

void TimelineEditor::AddKeyframeToTransformationElementInFrameAtChannel(Sequence& sequence, int channelId, int frame)
{
	SequenceChannel& seqChannel = sequence.sequenceChannels.at(channelId);
	int elementIndex = seqChannel.GetFirstElementIndexBetweenFrames(frame, frame);
	if (elementIndex == -1) return;
	ChannelElement& element = seqChannel.elements.at(elementIndex);
	TransformationKeyFrame prevKeyframe = element.transformation.GetKeyFrameBeforeFrame(frame);
	if (element.transformation.HasKeyframeAfterFrame(frame))
	{
		element.transformation.CreateInterpolatedKeyFrame(frame);
	}
	else
	{
		element.transformation.keyFrames.insert_or_assign(frame, prevKeyframe);
	}
}

void TimelineEditor::RemoveKeyframeFromTransformationElementInFrameAtChannel(Sequence& sequence, int channelId, int frame)
{
	SequenceChannel& seqChannel = sequence.sequenceChannels.at(channelId);
	int elementIndex = seqChannel.GetFirstElementIndexBetweenFrames(frame, frame);
	if (elementIndex == -1) return;
	ChannelElement& element = seqChannel.elements.at(elementIndex);
	TransformationKeyFrame prevKeyframe = element.transformation.GetKeyFrameBeforeFrame(frame);
	element.transformation.keyFrames.erase(frame);
}

void TimelineEditor::OpenScriptEditionForElementInFrameAtChannel(Sequence& sequence, int channelId, int frame,
	std::function<void(int channel, int frame, SequenceChannelElementScript*)> setScriptToEdit
)
{
	SequenceChannel& seqChannel = sequence.sequenceChannels.at(channelId);
	int elementIndex = seqChannel.GetFirstElementIndexBetweenFrames(frame, frame);
	if (elementIndex == -1) return;
	ChannelElement& element = seqChannel.elements.at(elementIndex);
	setScriptToEdit(channelId, frame, &element.script);
}

void TimelineEditor::SetFrameAtMouseXCoord(Sequence& sequence, ImVec2 markerPos, ImVec2 mousePos)
{
	selectedFrameInTimeline = std::clamp(static_cast<int>(std::floor((mousePos.x - markerPos.x + scroll.x) / TimelineChannel::frameSize.x)), 0, sequence.totalFrames);
}

void TimelineEditor::ScrollAndSetFrame(Sequence& sequence, ImVec2 timelineSize, ImVec2 markerPos, ImVec2 mousePos, float distance, float axisSize)
{
	ScrollHorizontal(sequence, timelineSize, distance);
	SetFrameAtMouseXCoord(sequence, markerPos, mousePos);
}

float TimelineEditor::GetMaxVerticalScroll(ImVec2 timelinePos, ImVec2 timelineSize)
{
	float scrollbarSize = ImGui::GetStyle().ScrollbarSize;
	ImVec2 vScrollbarPos = ImVec2(timelinePos.x + timelineSize.x - scrollbarSize, timelinePos.y);
	ImVec2 vScrollbarSize = ImVec2(scrollbarSize, timelineSize.y);

	float channelsHeightSum = std::accumulate(channels.begin(), channels.end(), 0.0f, [](float acc, TimelineChannel ch)
		{
			return acc + ch.size.y;
		}
	);

	float maxScroll = channelsHeightSum - vScrollbarSize.y;
	return maxScroll;
}

void TimelineEditor::ClampVerticalScroll(ImVec2 timelinePos, ImVec2 timelineSize)
{
	scroll.y = std::clamp(scroll.y, 0.0f, std::max(0.0f, GetMaxVerticalScroll(timelinePos, timelineSize)));
}

int TimelineEditor::GetFirstVisibleFrame()
{
	return static_cast<int>(std::floor(scroll.x / TimelineChannel::frameSize.x));
}

int TimelineEditor::GetLastVisibleFrame(Sequence& sequence, int firstFrame, ImVec2 timelineSize)
{
	return std::min(
		firstFrame + static_cast<unsigned int>(std::ceil((timelineSize.x - TimelineChannel::channelToolbarWidth) / TimelineChannel::frameSize.x)),
		static_cast<unsigned int>(sequence.totalFrames)
	);
}

int TimelineEditor::GetAnimationNumFrames(Sequence& sequence, std::string animation)
{
	float time = renderable->animable->animations->animationsLength.at(animation);
	int numFrames = static_cast<int>(std::ceil(time / sequence.framesPerSecond));
	return numFrames;
}

float TimelineEditor::GetTime(Sequence& sequence)
{
	return static_cast<float>(GetFrame(sequence)) * 1000.0f / static_cast<float>(sequence.framesPerSecond);
}

int TimelineEditor::GetFrame(Sequence& sequence)
{
	return std::clamp(selectedFrameInTimeline, 0, sequence.totalFrames);
}

