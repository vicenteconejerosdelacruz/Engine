#pragma once
#include <imgui.h>
#include <ImEditor.h>
#include <Sequence/AnimationSequences.h>
#include "TimelineChannel.h"
#include "TimelinePopups.h"

struct TimelineEditor
{
	void Init(RenderableUUID renderable, Sequence& sequence);
	void Reset();
	void DrawRect(ImVec2 pos, ImVec2 size, ImU32 color);
	void DrawBackground(ImVec2 pos, ImVec2 size);
	bool DrawPlusButton(ImVec2 pos, ImVec2 size, bool canInteract);
	void DrawAddChannelButton(Sequence& sequence, ImVec2 pos, bool canInteract);
	void DrawTimeline(Sequence& sequence, ImVec2 timelinePos, ImVec2 timelineSize, bool canInteract,
		std::function<void(TransformationKeyFrame*, int)> setTransformationKeyFrame
	);
	void DrawVerticalScrollbar(Sequence& sequence, ImVec2 timelinePos, ImVec2 timelineSize, bool canInteract);
	void DrawHorizontalScrollbar(Sequence& sequence, ImVec2 timelinePos, ImVec2 timelineSize, bool canInteract);
	void DrawMarkers(Sequence& sequence, ImVec2 timelinePos, ImVec2 timelineSize, bool canInteract);
	void DrawSelectedFrameVerticalLine(ImVec2 timelinePos, ImVec2 timelineSize);
	void DrawActionPopup(Sequence& sequence, std::function<void(TransformationKeyFrame*, int frame)> setTransformationKeyFrame,
		std::function<void()> deleteTransformationKeyFrame,
		std::function<void(int channel, int frame, SequenceChannelElementScript*)> setScriptToEdit);
	void Draw(Sequence& sequence, ImVec2 pos, ImVec2 size,
		std::function<void(TransformationKeyFrame*, int frame)> setTransformationKeyFrame,
		std::function<void()> deleteTransformationKeyFrame,
		std::function<void(int channel, int frame, SequenceChannelElementScript*)> setScriptToEdit
	);

	void HandleElementDrag(Sequence& sequence);
	void HandleElementDragLeftBoundary(Sequence& sequence);
	void HandleElementDragRightBoundary(Sequence& sequence);
	void DeleteChannel(Sequence& sequence, int channelId, ImVec2 timelinePos, ImVec2 timelineSize);
	void ToggleChannelExpansion(int channelId, ImVec2 timelinePos, ImVec2 timelineSize);
	void SelectFrameInChannel(int channelId, int frame);
	void SelectElementInChannel(int channelId, int elementId);
	void SelectElementToDragFromLeft(int channelId, int elementId);
	void SelectElementToDragFromRight(int channelId, int elementId);
	void CreatePopupForItemAt(Sequence& sequence, int channelId, int frame, ImVec2 popupPosition);
	void ScrollHorizontal(Sequence& sequence, ImVec2 timelineSize, float quantity);
	void ScrollVertical(Sequence& sequence, ImVec2 timelineSize, float quantity);
	void AddAnimationElementToChannel(Sequence& sequence, int channelId, SequenceChannelElementAnimation* elem);
	void AddTransformationElementToChannel(Sequence& sequence, int channelId, SequenceChannelElementTransformation* elem);
	void AddSoundFXElementToChannel(Sequence& sequence, int channelId, SequenceChannelElementSoundFX* elem);
	void AddScriptElementToChannel(Sequence& sequence, int channelId, SequenceChannelElementScript* elem);
	void DeleteElementInFrameAtChannel(Sequence& sequence, int channelId, int frame);
	void SplitElementInFrameAtChannel(Sequence& sequence, int channelId, int frame);
	void AddKeyframeToTransformationElementInFrameAtChannel(Sequence& sequence, int channelId, int frame);
	void RemoveKeyframeFromTransformationElementInFrameAtChannel(Sequence& sequence, int channelId, int frame);
	void OpenScriptEditionForElementInFrameAtChannel(Sequence& sequence, int channelId, int frame,
		std::function<void(int channel, int frame, SequenceChannelElementScript*)> setScriptToEdit);
	void SetFrameAtMouseXCoord(Sequence& sequence, ImVec2 markerPos, ImVec2 mousePos);
	void ScrollAndSetFrame(Sequence& sequence, ImVec2 timelineSize, ImVec2 markerPos, ImVec2 mousePos, float distance, float axisSize);

	float GetMaxVerticalScroll(ImVec2 timelinePos, ImVec2 timelineSize);
	void ClampVerticalScroll(ImVec2 timelinePos, ImVec2 timelineSize);
	int GetFirstVisibleFrame();
	int GetLastVisibleFrame(Sequence& sequence, int firstFrame, ImVec2 timelineSize);
	int GetAnimationNumFrames(Sequence& sequence, std::string animation);
	float GetTime(Sequence& sequence);
	int GetFrame(Sequence& sequence);

	static inline float markersBgHeight = 16.0f;
	static inline ImU32 markersBgColor = rgba(216, 216, 216, 1);
	static inline float markersLineHeight = 4.0f;
	static inline ImU32 markersLineColor = rgba(129, 129, 129, 1);
	static inline ImU32 markersTextColor = rgba(74, 74, 74, 1);
	static inline int markersFramesBetweenTexts = 5;
	static inline ImU32 channelsBgColor = rgba(255, 255, 255, 1);
	static inline ImU32 timelineBgColor = rgba(198, 198, 198, 1);
	static inline ImU32 addChannelButtonGlyphColor = rgba(32, 32, 32, 1);
	static inline ImU32 scrollbarBgColor = rgba(81, 75, 165, 1);
	static inline ImU32 scrollbarThumbColor = rgba(150, 150, 150, 1);
	static inline ImU32 selectedFrameVerticalColor = rgba(203, 73, 136, 0.3);

	RenderableUUID renderable;
	ImVec2 scroll;
	ImVec2 scrollbarLastMousePos;
	bool scrollbarMouseClicked[2];
	int selectedFrameInTimeline = -1;
	std::vector<TimelineChannel> channels;
	TimelinePopups popup = TP_None;
	ImVec2 popupCoords;
	std::tuple<int, int> popupChannelFrame;
	bool markerMouseDrag;
	ImVec2 markerLastMousePos;

	std::tuple<int, int> selectedChannelElement = std::make_tuple(-1, -1);
	ImVec2 elementLastMousePos;
	bool elementDrag;
	float elementDragXSum;

	//left boundary drag
	bool elementDragLeftBoundary;
	ImVec2 elementDragLeftBoundaryMousePos;
	std::tuple<int, int> selectedDragLeftBoundaryChannelElement = std::make_tuple(-1, -1);
	float elementDragLeftBoundaryXSum;

	//right boundary drag
	bool elementDragRightBoundary;
	ImVec2 elementDragRightBoundaryMousePos;
	std::tuple<int, int> selectedDragRightBoundaryChannelElement = std::make_tuple(-1, -1);
	float elementDragRightBoundaryXSum;

	//popups
	AddElementPopup addElementPopup;
	InteractElementPopup interactElementPopup;
};
