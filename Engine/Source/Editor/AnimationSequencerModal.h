#pragma once
#include <memory>
#include <string>
#include <Scene.h>
#include <Templates.h>
#include "Timeline/TimelineEditor.h"

enum SequencerModalPopup
{
	SMP_None,
	SMP_AddElement,
	SMP_InteractWithElement
};

static inline std::unordered_map<SequencerModalPopup, std::string> SequencerModalPopupToString =
{
	{ SMP_AddElement, "Add Element" },
	{ SMP_InteractWithElement, "Interact with Element" }
};

static inline std::unordered_map<std::string, SequencerModalPopup> StringToSequencerModalPopup =
{
	{ "Add Element", SMP_AddElement },
	{ "Interact with Element", SMP_InteractWithElement }
};

struct AnimationSequencerModal
{
	void Initialize(JUUID uuid);
	void LoadSceneObjects();
	void DestroySceneObjects();
	void Step();
	void DrawSequencer(const char* title, ImVec2 pos, ImVec2 size);
	void DrawTitleBar(const char* title, ImVec2 pos, ImVec2 size, bool& exit);
	void DrawSequenceSelector(ImVec2 curPos, std::function<void(std::string)> onSelectSequence, std::function<void(std::string)> onEraseSequence, std::function<void()> onAddSequence);
	void DrawModelPreview(ImVec2 curPos, ImVec2 size);
	void DrawTransformationKeyFrameAttributes(TransformationKeyFrame& keyframe, int keyFrameFrame, ImVec2 pos, ImVec2 size);
	void DrawTimelineController(ImVec2 curPos, ImVec2 size, Sequence& sequence);
	void DrawSaveAndExitButtons(ImVec2 curPos, ImVec2 size, bool& exit, bool& saveexit);
	void DrawAddNewSequencePopup(ImVec2 pos, ImVec2 size, std::string& newSeqName, std::function<void(std::string)> onAddNewSequenceClicked, std::function<void()> onCancelAddNewSequenceClick);
	void DrawScriptEdition(std::string& content, Sequence& sequence, std::string sequenceName, std::tuple<int, int> channelFrame, ImVec2 pos, ImVec2 size, std::function<void()> onSave, std::function<void()> onCancel);

	void Exit();
	void SaveAndExit();

	static inline ImGuiWindowFlags defaultChildFlag = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove;
	static inline ImGuiWindowFlags popupChildFlag = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove;
	static inline ImGuiWindowFlags timelineWindowFlag = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar;

	bool showing = false;
	bool initializing = false;
	bool destroying = false;
	Model3DInstanceUUID model3dUUID;
	RenderableUUID renderable;
	RenderableUUID floor;
	CameraUUID camera;
	LightUUID ambientLight;
	LightUUID directionalLight;
	Model3DJsonUUID model3D;
	XMFLOAT3 cameraInitialPos;
	XMFLOAT3 cameraInitialRot;

	//Sequence selection
	bool addNewSequence;
	std::string newSequenceName;
	AnimationSequences animationsSequences;
	std::string selectedSequence;

	//player
	bool playingSequence = false;
	float playingSequenceTime = 0.0f;
	bool playingSequenceLoop = false;
	bool adjustToBoundingBox = true;

	//timeline editor
	TimelineEditor timelineEditor;
	TransformationKeyFrame* selectedTransformationKeyframe;
	int keyFrameFrame;
	//this "next" is because ImGui will change data if we set the pointer directly
	TransformationKeyFrame* nextSelectedTransformationKeyframe;
	int nextSelectedKeyFrameFrame;
	SequencePlayer sequencePlayer;

	//script editor
	std::tuple<int, int> selectedScriptChannelFrame = std::make_tuple(-1, -1);
	SequenceChannelElementScript* selectedScriptToEdit;
	std::string selectedScriptToEditContent;
};