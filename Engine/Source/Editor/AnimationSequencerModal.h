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

	void Initialize(ImVec2 seqPos, ImVec2 seqSize, JUUID uuid);
	void Resize(ImVec2 seqPos, ImVec2 seqSize);
	nlohmann::json GetModalLevelJson();
	void DestroyStep();
	void DestroySceneObjects();
	void Step();
	void DrawLoading();
	void DrawSequencer(const char* title);
	void DrawTitleBar(const char* title, ImVec2 pos, ImVec2 size, bool& exit);
	void DrawSequenceSelector(ImVec2 curPos, std::function<void(std::string)> onSelectSequence, std::function<void(std::string)> onEraseSequence, std::function<void(std::string)> onRenameSequence, std::function<void(std::string)> onCloneSequence, std::function<void()> onAddSequence);
	ImVec2 GetModelPreviewCameraWidthHeight();
	void DrawModelPreview(ImVec2 curPos, ImVec2 size);
	void ResetGizmoVariableWorkers();
	void DrawKeyFrameGuizmo(XMFLOAT4X4& world4x4, TransformationKeyFrame& keyframe, ImVec2 curPos, ImVec2 size);
	void BeginGizmoInteraction(CameraID camera, ImVec2 curPos, ImVec2 size, std::function<void(XMFLOAT4X4 view, XMFLOAT4X4 proj)> interaction);
	void DrawBoneTransformationKeyFrameAttributes(SequenceChannelElementBoneTransformation& boneTransformation, TransformationKeyFrame& keyframe, int keyFrameFrame, ImVec2 pos, ImVec2 size);
	void DrawTransformationKeyFrameAttributes(TransformationKeyFrame& keyframe, int keyFrameFrame, ImVec2 pos, ImVec2 size);
	void DrawElementTriggerAttributes(SequenceChannelElementTrigger& elementTrigger, ImVec2 pos, ImVec2 size);
	void DrawTimelineController(ImVec2 curPos, ImVec2 size, Sequence& sequence);
	void DrawSaveAndExitButtons(ImVec2 curPos, ImVec2 size, bool& exit, bool& saveexit);
	void DrawAddNewSequencePopup(ImVec2 pos, ImVec2 size, std::string& newSeqName, std::function<void(std::string)> onAddNewSequenceClicked, std::function<void()> onCancelAddNewSequenceClicked);
	void DrawSequenceRenamePopup(ImVec2 pos, ImVec2 size, std::string& newSeqName, std::function<void(std::string)> onRenameSequenceClicked, std::function<void()> onCancelRenameSequenceClicked);
	void DrawSequenceCloningPopup(ImVec2 pos, ImVec2 size, std::string& newSeqName, std::function<void(std::string)> onCloneSequenceClicked, std::function<void()> onCancelCloneSequenceClicked);
	void DrawScriptEdition(std::string& content, Sequence& sequence, std::string sequenceName, std::tuple<int, int> channelFrame, ImVec2 pos, ImVec2 size, std::function<void()> onSave, std::function<void()> onCancel);

	void Exit();
	void SaveAndExit();

	ImVec2 sequencerPos;
	ImVec2 sequencerSize;

	SceneUnitId unit;
	std::string asset;
	unsigned int count;
	unsigned int total;
	bool showing = false;
	bool initializing = false;
	bool destroying = false;
	unsigned int destructionFrames = 0;
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
	std::set<JUUID> triggersUUIDs;

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
	//the "next" is because ImGui will change data if we set the pointer directly
	TimelineEditor timelineEditor;
	//keyframe transformation and bone transformation
	SequenceChannelElementType selectedTransformationKeyFrameType;
	SequenceChannelElementBoneTransformation* selectedBoneTransformation;
	TransformationKeyFrame* selectedTransformationKeyframe;
	int keyFrameFrame;
	SequenceChannelElementType nextSelectedTransformationKeyFrameType;
	TransformationKeyFrame* nextSelectedTransformationKeyframe;
	int nextSelectedKeyFrameFrame;

	//trigger
	SequenceChannelElementTrigger* selectedElementTrigger;
	SequenceChannelElementTrigger* nextSelectedElementTrigger;
	SequencePlayer sequencePlayer;

	//script editor
	std::tuple<int, int> selectedScriptChannelFrame = std::make_tuple(-1, -1);
	SequenceChannelElementType selectedScriptType;
	SequenceChannelElement* selectedScriptToEdit;
	std::string selectedScriptToEditContent;
	bool isEnterScript;

	//preview model
	bool mousePreviewLeftClickPressed;
	ImVec2 mousePreviewLeftClickLastCoords;
	bool wheelCapture;
	std::vector<std::string> bones;

	//Gizmo
	ImGuizmo::OPERATION gizmoOperation;//(ImGuizmo::TRANSLATE);
	ImGuizmo::MODE gizmoMode;// (ImGuizmo::WORLD);
	XMFLOAT4X4 gizmoCentroidMx;
	XMFLOAT3 gizmoRotation;
	XMFLOAT3 gizmoPosition;
	XMFLOAT3 gizmoScale;
};