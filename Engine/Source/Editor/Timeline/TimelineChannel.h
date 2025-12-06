#pragma once
#include <imgui.h>
#include <Sequence/AnimationSequences.h>

struct TimelineChannel
{
	TimelineChannel(int index);
	TimelineChannel() :TimelineChannel(0) {};
	std::tuple<bool, bool, int, int, int, int> Draw(Sequence& sequence, int seqChannelId, int initialFrame, int lastFrame, ImVec2 timelinePos, ImVec2 timelineSize, ImVec2 scroll, ImRect channelsRect, bool canInteract);
	std::tuple<bool, bool> DrawChannelToolbar(Sequence& sequence, int seqChannelId, ImVec2 timelinePos, ImVec2 timelineSize, ImVec2 scroll, bool canInteract);
	bool DrawMinusButton(ImVec2 pos, ImVec2 size, bool canInteract);
	bool DrawExpandButton(ImVec2 pos, ImVec2 size, bool canInteract);
	std::tuple<int, int, int, int> DrawChannelFrames(Sequence& sequence, int seqChannelId, int initialFrame, int lastFrame, ImVec2 timelinePos, ImVec2 timelineSize, ImVec2 scroll, ImRect channelsRect, bool canInteract);
	std::tuple<bool, bool, bool, bool> DrawChannelFrame(SequenceChannel& seqChannel, int frame, ImVec2 pMin, ImVec2 pMax, ImRect channelsRect, ImRect framesRect, ImVec2 timelineSize, int numDashedLines, bool canInteract);
	void ToggleExpansion();
	void SetPosAfter(const TimelineChannel& prev);
	void ResetSelection();

	static inline ImVec2 frameSize = ImVec2(10.0f, 18.0f);
	static inline ImVec2 frameSizeExpanded = ImVec2(10.0f, 100.0f);
	static inline ImU32 frameColor = rgba(255, 255, 255, 1);
	static inline ImU32 frame5Color = rgba(239, 239, 239, 1);
	static inline ImU32 frameMouseOverColor = rgba(221, 170, 195, 1);
	static inline ImU32 frameBorderColor = rgba(229, 229, 229, 1);
	static inline ImU32 frameWithElementColor = rgba(148, 148, 148, 1);
	static inline ImU32 frameWithElementBorderColor = rgba(43, 43, 43, 1);
	static inline ImU32 frameCircleColor = rgba(43, 43, 43, 1);
	static inline ImU32 frameTransformationKeyFrameCircleColor = rgba(34, 97, 41, 1);
	static inline float frameBorderMouseDistance = 2.0f;
	static inline float frameBorderMouseThickness = 2.0f;
	static inline float frameBorderLeftThicknessAdjustment = 1.0f;
	static inline float frameBorderRightThicknessAdjustment = -1.0f;
	static inline float verticalDistanceBetweenDashedLines = 9.0f;
	static inline ImU32 selectedElementLinesColor = rgba(227, 25, 160, 0.43);
	static inline std::tuple<int, int, int, int> frameWithNoAction = std::make_tuple(-1, -1, -1, -1);
	static inline float channelToolbarWidth = 200.0f;
	static inline ImU32 channelToolbarColor = rgba(27, 28, 26, 1);
	static inline ImU32 channelToolbarNameColor = rgba(20, 20, 20, 1);
	static inline ImU32 channelToolbarNameAreaColor = rgba(180, 180, 180, 1);
	static inline ImVec2 deleteChannelBtnSize = ImVec2(10.0f, 10.0f);
	static inline ImU32 deleteChannelButtonGlyphColor = rgba(32, 32, 32, 1);
	static inline ImU32 animationLineColor = rgba(10, 10, 10, 1);

	ImVec2 pos;
	ImVec2 size;
	bool expanded = false;
	int selectedFrame = -1;
	int selectedElement = -1;
};