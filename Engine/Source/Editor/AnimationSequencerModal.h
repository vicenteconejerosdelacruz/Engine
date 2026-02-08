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
	static inline ImGuiWindowFlags defaultChildFlag = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove;
	static inline ImGuiWindowFlags popupChildFlag = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove;
	static inline ImGuiWindowFlags timelineWindowFlag = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar;

	void Initialize(JUUID uuid);
	nlohmann::json GetModalLevelJson();
	void DestroySceneObjects();
	void Step();
	void DrawLoading();
	void DrawSequencer(const char* title, ImVec2 pos, ImVec2 size);
	void DrawTitleBar(const char* title, ImVec2 pos, ImVec2 size, bool& exit);
	void DrawSequenceSelector(ImVec2 curPos, std::function<void(std::string)> onSelectSequence, std::function<void(std::string)> onEraseSequence, std::function<void(std::string)> onRenameSequence, std::function<void(std::string)> onCloneSequence, std::function<void()> onAddSequence);
	void DrawModelPreview(ImVec2 curPos, ImVec2 size);
	void DrawTransformationKeyFrameAttributes(TransformationKeyFrame& keyframe, int keyFrameFrame, ImVec2 pos, ImVec2 size);
	void DrawTimelineController(ImVec2 curPos, ImVec2 size, Sequence& sequence);
	void DrawSaveAndExitButtons(ImVec2 curPos, ImVec2 size, bool& exit, bool& saveexit);
	void DrawAddNewSequencePopup(ImVec2 pos, ImVec2 size, std::string& newSeqName, std::function<void(std::string)> onAddNewSequenceClicked, std::function<void()> onCancelAddNewSequenceClicked);
	void DrawSequenceRenamePopup(ImVec2 pos, ImVec2 size, std::string& newSeqName, std::function<void(std::string)> onRenameSequenceClicked, std::function<void()> onCancelRenameSequenceClicked);
	void DrawSequenceCloningPopup(ImVec2 pos, ImVec2 size, std::string& newSeqName, std::function<void(std::string)> onCloneSequenceClicked, std::function<void()> onCancelCloneSequenceClicked);
	void DrawScriptEdition(std::string& content, Sequence& sequence, std::string sequenceName, std::tuple<int, int> channelFrame, ImVec2 pos, ImVec2 size, std::function<void()> onSave, std::function<void()> onCancel);

	void Exit();
	void SaveAndExit();

	SceneUnitId unit;
	std::string asset;
	unsigned int count;
	unsigned int total;
	bool showing = false;
	bool initializing = false;
	bool destroying = false;
	Model3DInstanceID model3dUUID;
	JUUID renderableUUID;
	RenderableID renderable;
	JUUID floorUUID;
	RenderableID floor;
	JUUID cameraUUID;
	CameraID camera;
	JUUID ambientLightUUID;
	LightID ambientLight;
	JUUID directionalLightUUID;
	LightID directionalLight;
	Model3DJsonID model3D;
	XMFLOAT3 cameraInitialPos;
	XMFLOAT3 cameraInitialRot;

	//Sequence selection
	bool addNewSequence;
	std::string newSequenceName;
	AnimationSequences animationsSequences;
	std::string selectedSequence;
	//Sequence renaming
	bool selectedSequenceRenaming;
	std::string selectedSequenceNewName;
	//Sequence cloning
	bool selectedSequenceCloning;
	std::string selectedSequenceCloneName;

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

	//preview model
	bool mousePreviewLeftClickPressed;
	ImVec2 mousePreviewLeftClickLastCoords;
	bool wheelCapture;
};