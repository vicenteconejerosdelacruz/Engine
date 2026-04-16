#include "pch.h"
#include "AnimationSequencerModal.h"
#include <imgui.h>
#include <ImGuizmo.h>
#include <ImEditor.h>
#include <nlohmann/json.hpp>
#include <Camera/Camera.h>
#include <Light/Light.h>
#include <Renderable/Renderable.h>
#include <Renderer.h>
#include <Scene.h>
#include <NoMath.h>
#include <Game.h>

extern std::unique_ptr<JRenderer> renderer;
extern DX::StepTimer timer;

namespace Editor
{
	extern bool templatesModified;
};

namespace Physics
{
	extern std::vector<std::string> GetCollisionMasks();
};

void AnimationSequencerModal::Initialize(ImVec2 seqPos, ImVec2 seqSize, JUUID uuid)
{
	using namespace Scene;

	sequencerPos = seqPos;
	sequencerSize = seqSize;
	unit = 0;
	asset = "";
	count = 0U;
	total = 0U;
	showing = false;
	initializing = true;
	destroying = false;
	model3dUUID = uuid;
	model3D = uuid;
	cameraInitialPos = XMFLOAT3({ 0.0, 5.21317195892334, -17.224170684814453 });
	cameraInitialRot = XMFLOAT3({ 12.898999214172363, 0.0, 0.0 });
	addNewSequence = false;
	newSequenceName = "";
	animationsSequences = model3D->animationSequences();
	selectedSequence = "";
	playingSequence = false;
	playingSequenceTime = 0.0f;
	playingSequenceLoop = false;
	flyMode = PreviewFlyMode_BoundingBox;
	selectedTransformationKeyframe = nullptr;
	keyFrameFrame = -1;
	selectedElementTrigger = nullptr;
	nextSelectedTransformationKeyframe = nullptr;
	nextSelectedKeyFrameFrame = -1;
	nextSelectedElementTrigger = nullptr;
	selectedSequenceRenaming = false;
	selectedSequenceNewName = "";
	selectedSequenceCloning = false;
	selectedSequenceCloneName = "";
	//preview model
	mousePreviewLeftClickPressed = false;
	mousePreviewLeftClickLastCoords = ImVec2();
	wheelCapture = false;
	//gizmo
	gizmoOperation = ImGuizmo::TRANSLATE;
	gizmoMode = ImGuizmo::WORLD;
	gizmoCentroidMx = XMFLOAT4X4();
	gizmoRotation = XMFLOAT3();
	gizmoPosition = XMFLOAT3();
	gizmoScale = XMFLOAT3();

	cameraUUID = getUUID();
	ambientLightUUID = getUUID();
	directionalLightUUID = getUUID();
	renderableUUID = getUUID();
	floorUUID = getUUID();

	CreateIsolatedSceneLevelAsync("modal", GetModalLevelJson(), [&](SceneUnitId id)
		{
			unit = id;
			renderable = MAKESUUUID(id, renderableUUID);
			floor = MAKESUUUID(id, floorUUID);
			camera = MAKESUUUID(id, cameraUUID);
			ambientLight = MAKESUUUID(id, ambientLightUUID);
			directionalLight = MAKESUUUID(id, directionalLightUUID);
			EnableSceneUnitRendering(id);
			showing = true;
			initializing = false;
			bones = nostd::GetKeysFromMap(renderable->animable->animations->bonesOffsets);
			bones.insert(bones.begin(), { "" });
			WriteFloorColorConstantsBuffer();
			Step();
			for (unsigned int frame = 0U; frame < JRenderer::numFrames; frame++)
			{
				renderable->WriteAnimationConstantsBuffer(frame);
			}
		},
		[&](std::string asset, unsigned int count, unsigned int total)
		{
			this->asset = asset;
			this->count = count;
			this->total = total;
		}
	);
}

void AnimationSequencerModal::Resize(ImVec2 seqPos, ImVec2 seqSize)
{
	sequencerPos = seqPos;
	sequencerSize = seqSize;
	for (auto& rpi : camera->renderPassesUUID)
	{
		rpi->Resize(static_cast<unsigned int>(seqSize.x), static_cast<unsigned int>(seqSize.y));
	}
	auto p = camera->perspective();
	p.width = seqSize.x;
	p.height = seqSize.y;
	camera->perspective(p);
}

nlohmann::json AnimationSequencerModal::GetModalLevelJson()
{
	ImVec2 camSize = GetModelPreviewCameraWidthHeight();

	nlohmann::json modal = {
		{ "cameras",
			{
				{
					{ "fitWindow", false },
					{ "name", "cam-preview" },
					{ "projectionType", "Perspective" },
					{ "perspective",
						{
							{ "farZ", 100.0 },
							{ "fovAngleY", 20.0 },
							{ "nearZ", 0.01 },
							{ "width", camSize.x },
							{ "height", camSize.y }
						}
					},
					{ "speed", 0.05000000074505806 },
					{ "uuid", cameraUUID },
					{
						"renderPasses", { GetRenderPassUUIDByName("ModelPreviewPass")}
					},
					{ "mouseController", false },
					{ "useSwapChain", false },
					{ "hidden", true },
					{ "wheelFactor", 0.1f },
					{ "modelDistanceScale", 1.0f },

					{ "position", FromXMFLOAT3(cameraInitialPos) },
					{ "rotation", FromXMFLOAT3(cameraInitialRot) },

					{ "freeposition", FromXMFLOAT3(cameraInitialPos) },
					{ "freerotation", FromXMFLOAT3(cameraInitialRot) },
				}
			}
		},
		{ "lights",
			{
				{
					{ "color", { 0.25000000074505806, 0.25000000074505806, 0.25000000074505806 } },
					{ "lightType", "Ambient" },
					{ "name", "light.0.amb-preview" },
					{ "uuid", ambientLightUUID},
					{ "cameras", { cameraUUID }}
				},
				{
					{ "color", { 1.0, 1.0, 1.0} },
					{ "farZ" , 100.0},
					{ "nearZ", 0.01},
					{ "hasShadowMaps", true },
					{ "shadowMapHeight", 4096},
					{ "shadowMapWidth", 4096},
					{ "rotation", {40.31087875366211, -10.30000039935112, 0.0} },
					{ "lightType", "Directional"},
					{ "name", "light.1.dir-preview"},
					{ "uuid", directionalLightUUID },
					{ "zBias", 0.000002 },
					{ "cameras", { cameraUUID } }
				}
			}
		},
		{ "renderables",
			{
				{
					{ "castShadows", true },
					{ "shadowed", false },
					{ "model", model3dUUID()},
					{ "name", "preview-model" },
					{ "position", { 0.0, 0.0, 0.0} },
					{ "rotation", { 0.0, -90.0, 0.0 }},
					{ "scale", { 0.1, 0.1, 0.1} },
					{ "uuid", renderableUUID },
					{ "cameras", { cameraUUID } },
					{ "animationUseTransformation", true }
				},
				{
					{ "castShadows", false },
					{ "shadowed", true },
					{
						"meshMaterial",
						{
							{ "material", "ecd1688c-73d6-49d0-870f-ca916a417c49"},
							{ "mesh",
								{
									{ "primitive", "d41e5c29-49bb-4f2c-aa2b-da781fbac512"}
								}
							}
						}
					},
					{ "name", "preview-floor" },
					{ "position", { 0.0, 0.0, 0.0} },
					{ "scale", { 100.0, 100.0, 100.0} },
					{ "uuid", floorUUID },
					{ "cameras", { cameraUUID } },
					{ "floorColor", { 0.5f, 0.5f, 0.5f } }
				}
			}
		}
	};

	return modal;
}

void AnimationSequencerModal::DestroyStep()
{
	if (destructionFrames > 0)
	{
		destructionFrames--;
	}
	else
	{
		DestroySceneObjects();
		destroying = false;
	}
}

void AnimationSequencerModal::DestroySceneObjects()
{
	using namespace Scene;

	auto& scene = GetSceneUnit(unit);
	scene->MarkForDelete();
	unit = 0;
	showing = false;

	renderable.clear();
	floor.clear();
	directionalLight.clear();
	ambientLight.clear();
	camera.clear();
	model3D.clear();
	model3dUUID.clear();
	sequencePlayer.DestroySequenceTriggersAvatars();
	selectedSequence.clear();
}

void AnimationSequencerModal::Step()
{
	using namespace Scene;

	auto& scene = GetSceneUnit(unit);
	floor->renderNext({ renderable.uuid() });

	if (selectedSequence != "")
	{
		SetTriggersAvatarColors();
		SetPlayerSequenceFrame();
	}

	if (!(ImGuizmo::IsOver() || ImGuizmo::IsUsing()))
	{
		if (flyMode == PreviewFlyMode_BoundingBox)
		{
			if (renderable->HasBoundingBoxComputed())
			{
				float modelDistanceScale = camera->at("modelDistanceScale");
				BoundingBox bb = renderable->GetBoundingBox();
				camera->LookAtBoundingBox(bb, modelDistanceScale);
			}
		}
		else if (flyMode == PreviewFlyMode_Bone)
		{
			float modelDistanceScale = camera->at("modelDistanceScale");
			auto [mm, pos, a, b, c] = renderable->GetBoneTransformation(selectedBoneTransformation->GetBone());
			BoundingBox bb(pos, { 0.1f,0.1f,0.1f });
			camera->LookAtBoundingBox(bb, modelDistanceScale);
		}
		else if (flyMode == PreviewFlyMode_Free)
		{
			camera->at("position") = camera->at("freeposition");
			camera->at("rotation") = camera->at("freerotation");
			camera->updateRotationQ();
		}
	}

	camera->WriteConstantsBuffer(scene->Frame());
	renderable->WriteConstantsBuffer(scene->Frame());
	WriteConstantsBuffers(unit);
}

void AnimationSequencerModal::WriteFloorColorConstantsBuffer()
{
	using namespace Scene;

	auto& scene = GetSceneUnit(unit);
	XMFLOAT3 baseColor = ToXMFLOAT3(floor->at("floorColor"));
	for (unsigned int frame = 0U; frame < JRenderer::numFrames; frame++)
	{
		floor->WriteConstantsBuffer<XMFLOAT3>("baseColor", baseColor, frame);
		floor->WriteConstantsBuffer(frame);
	}
}

void AnimationSequencerModal::SetTriggersAvatarColors()
{
	auto& scene = GetSceneUnit(unit);
	Sequence& seq = sequencePlayer.sequence;
	auto triggers = seq.GetTriggerElements();

	renderable->renderNext({});
	for (auto* t : triggers)
	{
		if (t->triggerRenderable)
		{
			renderable->renderNext({ t->triggerRenderable.uuid() });
			t->triggerRenderable->renderNext({ t->triggerLines.uuid() });

			XMFLOAT4 rgba = t->color;
			XMFLOAT3 baseColor = { rgba.x,rgba.y,rgba.z };
			XMFLOAT3 lineBaseColor = baseColor * 1.3f;
			float alpha = rgba.w;
			t->triggerRenderable->WriteConstantsBuffer("alpha", alpha, scene->Frame());
			t->triggerRenderable->WriteConstantsBuffer("baseColor", baseColor, scene->Frame());
			t->triggerRenderable->WriteConstantsBuffer(scene->Frame());
			t->triggerLines->WriteConstantsBuffer("baseColor", lineBaseColor, scene->Frame());
			t->triggerLines->WriteConstantsBuffer(scene->Frame());
		}
	}
}

void AnimationSequencerModal::SetPlayerSequenceFrame()
{
	auto& scene = GetSceneUnit(unit);
	Sequence& seq = sequencePlayer.sequence;

	if (playingSequence)
	{
		float totalTime = static_cast<float>(seq.totalFrames) / static_cast<float>(seq.framesPerSecond);
		playingSequenceTime += static_cast<float>(timer.GetElapsedSeconds());
		bool stopPlayer = false;

		if (playingSequenceLoop)
		{
			playingSequenceTime = std::fmodf(playingSequenceTime, totalTime);
		}
		else
		{
			playingSequenceTime = std::min(playingSequenceTime, totalTime);
			stopPlayer = playingSequenceTime == totalTime;
		}
		int frame = static_cast<int>(static_cast<float>(seq.totalFrames) * (playingSequenceTime / totalTime));
		frame = std::clamp(frame, 0, seq.totalFrames);
		timelineEditor.selectedFrameInTimeline = frame;
		sequencePlayer.SetFrame(timelineEditor.GetFrame(seq));
		if (stopPlayer)
		{
			playingSequence = false;
		}
	}
	else
	{
		sequencePlayer.SetFrame(timelineEditor.GetFrame(seq), false);
	}
	sequencePlayer.ApplyFrameValues();
	sequencePlayer.ApplyFrameTriggerAvatarValues();
}

void AnimationSequencerModal::DrawLoading()
{
	std::string title = "Loading";

	ImGui::OpenPopup(title.c_str());

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImVec2 modalSize = ImVec2(300.0f, 70.0f);
	ImVec2 modalPos = ImVec2(viewport->WorkSize.x * 0.5f - modalSize.x * 0.5f, viewport->WorkSize.y * 0.5f - modalSize.y * 0.5f);
	ImGui::SetNextWindowPos(modalPos);
	ImGui::SetNextWindowSize(modalSize);

	if (ImGui::BeginPopupModal(title.c_str(), nullptr,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
	))
	{
		ImGui::Text(asset.c_str());
		float progress = static_cast<float>(count) / static_cast<float>(total);
		ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), "");

		ImGui::EndPopup();
	}
}

static ImVec2 modelPosAdj(0.0f, 21.0f);
static ImVec2 sequencerSizeAdj(0.0f, -47.0f);
static ImVec2 sequencerPosAdj(0.0f, 4.0f);
static float titleBarH = 19.0f;

ImVec2 AnimationSequencerModal::GetModelPreviewCameraWidthHeight()
{
	return ImVec2(sequencerSize.y * 0.5f * 16.0f / 9.0f, sequencerSize.y * 0.5f);
}

void AnimationSequencerModal::DrawSequencer(const char* title)
{
	ImVec2 pos = sequencerPos;
	ImVec2 size = sequencerSize;

	auto& scene = GetSceneUnit(unit);

	Step();

	ImGui::OpenPopup(title);

	ImVec2 modalSize(size.x, size.y + titleBarH);
	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(modalSize);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	bool exit = false;
	bool saveexit = false;

	auto onSelectSequence = [this](std::string sequence)
		{
			selectedTransformationKeyframe = nullptr;
			keyFrameFrame = -1;
			selectedElementTrigger = nullptr;
			nextSelectedTransformationKeyframe = nullptr;
			nextSelectedKeyFrameFrame = -1;
			nextSelectedElementTrigger = nullptr;

			if (selectedSequence != "" && animationsSequences.sequences.contains(selectedSequence))
			{
				animationsSequences.sequences.insert_or_assign(selectedSequence, sequencePlayer.sequence);
			}
			sequencePlayer.DestroySequenceTriggersAvatars();
			selectedSequence = sequence;
			if (sequence != "")
			{
				Sequence& seq = animationsSequences.sequences.at(sequence);
				sequencePlayer.SetSequence(seq, renderable);
				sequencePlayer.CreateSequenceTriggersAvatars(cameraUUID);
				timelineEditor.Init(renderable, sequencePlayer.sequence);
				renderable->SetCurrentAnimation(&sequencePlayer);
			}
			playingSequence = false;
			playingSequenceTime = 0.0f;
		};
	auto onAddNewSequence = [this]()
		{
			addNewSequence = true;
			newSequenceName.clear();
			playingSequence = false;
			playingSequenceTime = 0.0f;
		};
	auto onEraseSequence = [this, onSelectSequence](std::string sequence)
		{
			animationsSequences.sequences.erase(sequence);
			onSelectSequence("");
			playingSequence = false;
			playingSequenceTime = 0.0f;
			timelineEditor.Reset();
			sequencePlayer.SetSequence(Sequence(), RenderableID());
			renderable->SetCurrentAnimation(nullptr);
			selectedTransformationKeyframe = nullptr;
			keyFrameFrame = -1;
			selectedElementTrigger = nullptr;
			nextSelectedTransformationKeyframe = nullptr;
			nextSelectedKeyFrameFrame = -1;
			nextSelectedElementTrigger = nullptr;
		};
	auto onRenameSequence = [&](std::string sequence)
		{
			selectedSequenceRenaming = true;
			selectedSequenceNewName = sequence;
		};
	auto onCloneSequence = [&](std::string sequence)
		{
			selectedSequenceCloning = true;
			selectedSequenceCloneName = sequence;
		};
	auto onAddNewSequenceClicked = [this](std::string seqName)
		{
			addNewSequence = false;
			animationsSequences.sequences.insert_or_assign(seqName, Sequence());
			selectedSequence = seqName;
			selectedTransformationKeyframe = nullptr;
			selectedElementTrigger = nullptr;
			nextSelectedTransformationKeyframe = nullptr;
			nextSelectedElementTrigger = nullptr;
			playingSequence = false;
			playingSequenceTime = 0.0f;
			Sequence& seq = animationsSequences.sequences.at(seqName);
			sequencePlayer.SetSequence(seq, renderable);
			timelineEditor.Init(renderable, sequencePlayer.sequence);
			renderable->SetCurrentAnimation(&sequencePlayer);
		};
	auto onCancelAddNewSequenceClicked = [this]()
		{
			addNewSequence = false;
		};
	auto onRenameSequenceClicked = [&](std::string newName)
		{
			animationsSequences.sequences.insert_or_assign(newName, animationsSequences.sequences.at(selectedSequence));
			animationsSequences.sequences.erase(selectedSequence);
			selectedSequence = newName;
			selectedSequenceRenaming = false;
			selectedSequenceNewName = "";
		};
	auto onCancelRenameSequenceClicked = [&]
		{
			selectedSequenceRenaming = false;
			selectedSequenceNewName = "";
		};
	auto onCloneSequenceClicked = [&](std::string newName)
		{
			animationsSequences.sequences.insert_or_assign(newName, animationsSequences.sequences.at(selectedSequence));
			selectedSequence = newName;
			selectedSequenceCloning = false;
			selectedSequenceCloneName = "";
		};
	auto onCancelCloneSequenceClicked = [&]
		{
			selectedSequenceCloning = false;
			selectedSequenceCloneName = "";
		};
	auto getTitleValues = [pos, size]()
		{
			ImVec2 titleSize(size.x - 1, titleBarH);
			ImVec2 titlePos(pos.x + 1, pos.y);
			return std::make_tuple(titlePos, titleSize);
		};
	auto getSequenceSelectorPos = [pos]()
		{
			return ImVec2(pos.x, pos.y + titleBarH);
		};
	auto getModelPreviewValues = [pos, size]()
		{
			ImVec2 modelSize(size.y * 0.5f * 16.0f / 9.0f, size.y * 0.5f);
			ImVec2 modelPos(pos.x + (size.x - modelSize.x) * 0.5f + modelPosAdj.x, pos.y + titleBarH + modelPosAdj.y);
			return std::make_tuple(modelPos, modelSize);
		};
	auto getPlayerValues = [pos, size]()
		{
			ImVec2 timeControllerPos(pos.x, pos.y + titleBarH + modelPosAdj.y + size.y * 0.5f);
			ImVec2 timeControllerSize(size.x, 20.0f);
			return std::make_tuple(timeControllerPos, timeControllerSize);
		};
	auto getTimelineEditorValues = [pos, size]()
		{
			ImVec2 sequencerPos(pos.x, pos.y + titleBarH + modelPosAdj.y + size.y * 0.5f + 20.0f + sequencerPosAdj.y);
			ImVec2 sequencerSize(size.x + sequencerSizeAdj.x, size.y - size.y * 0.5f - 20.0f + sequencerSizeAdj.y);
			return std::make_tuple(sequencerPos, sequencerSize);
		};
	auto getSaveExitButtonValues = [pos, size]()
		{
			ImVec2 saveExitBtnPos(pos.x,
				pos.y + titleBarH + modelPosAdj.y +
				size.y + sequencerPosAdj.y +
				sequencerSizeAdj.y
			);
			ImVec2 saveExitBtnSize(200.0f, 20.0f);
			return std::make_tuple(saveExitBtnPos, saveExitBtnSize);
		};
	auto getNewSequencePopupValues = [pos, size]()
		{
			ImVec2 newSeqSize(200, 75);
			ImVec2 newSeqPos(pos.x + (size.x - newSeqSize.x) * 0.5f, pos.y + (size.y - newSeqSize.y) * 0.5f);
			return std::make_tuple(newSeqPos, newSeqSize);
		};
	auto getKeyframeValues = [pos, size]
		{
			ImVec2 kpos(pos.x, pos.y + titleBarH + 65);
			ImVec2 ksize(300.0f, 300.0f);
			return std::make_tuple(kpos, ksize);
		};
	auto getScriptEditValues = [pos, size]
		{
			ImVec2 scriptPos(pos.x, pos.y + titleBarH);
			ImVec2 scriptSize(size.x, size.y - titleBarH * 2);
			return std::make_tuple(scriptPos, scriptSize);
		};

	auto [titlePos, titleSize] = getTitleValues();
	auto seqSelPos = getSequenceSelectorPos();
	auto [modelPos, modelSize] = getModelPreviewValues();
	auto [timeControllerPos, timeControllerSize] = getPlayerValues();
	auto [sequencerPos, sequencerSize] = getTimelineEditorValues();
	auto [saveExitBtnPos, saveExitBtnSize] = getSaveExitButtonValues();
	auto [newSeqPos, newSeqSize] = getNewSequencePopupValues();
	auto [keyframePos, keyframeSize] = getKeyframeValues();
	auto [scriptEditPos, scriptEditSize] = getScriptEditValues();

	auto transformationKeyFrameSelected = [&](SequenceChannelElementTransformation* transformation, TransformationKeyFrame* tkeyframe, int frame)
		{
			selectedTransformation = transformation;
			selectedBoneTransformation = nullptr;
			selectedTransformationKeyframe = nullptr;
			keyFrameFrame = -1;
			nextSelectedTransformationKeyFrameType = SCET_Transformation;
			nextSelectedTransformationKeyframe = tkeyframe;
			nextSelectedKeyFrameFrame = frame;
			if (flyMode != PreviewFlyMode_Free)
			{
				flyMode = PreviewFlyMode_BoundingBox;
			}
			else
			{
				prevFlyMode = PreviewFlyMode_BoundingBox;
			}
		};
	auto transformationKeyFrameDeleted = [&]
		{
			selectedTransformationKeyframe = nullptr;
			keyFrameFrame = -1;
		};
	auto boneTransformationKeyFrameSelected = [&](SequenceChannelElementBoneTransformation* boneTransformation, TransformationKeyFrame* tkeyframe, int frame)
		{
			selectedTransformation = nullptr;
			selectedBoneTransformation = boneTransformation;
			selectedTransformationKeyframe = nullptr;
			keyFrameFrame = -1;
			nextSelectedTransformationKeyFrameType = SCET_BoneTransformation;
			nextSelectedTransformationKeyframe = tkeyframe;
			nextSelectedKeyFrameFrame = frame;
			if (flyMode != PreviewFlyMode_Free)
			{
				flyMode = PreviewFlyMode_Bone;
			}
			else
			{
				prevFlyMode = PreviewFlyMode_Bone;
			}
		};
	auto boneTransformationKeyFrameDeleted = [&]
		{
			selectedTransformationKeyframe = nullptr;
			keyFrameFrame = -1;
		};
	auto setElementTrigger = [&](SequenceChannelElementTrigger* elementTrigger)
		{
			selectedElementTrigger = nullptr;
			nextSelectedElementTrigger = elementTrigger;
		};
	auto onDeleteChannel = [&](int channel)
		{
			nextSelectedTransformationKeyframe = nullptr;
			selectedTransformationKeyframe = nullptr;
			nextSelectedElementTrigger = nullptr;
			selectedElementTrigger = nullptr;
		};
	auto setScriptToEdit = [&](int channel, int frame, SequenceChannelElementScript* scriptToEdit)
		{
			selectedScriptChannelFrame = std::make_tuple(channel, frame);
			selectedScriptType = SCET_Script;
			selectedScriptToEdit = scriptToEdit;
			selectedScriptToEditContent = scriptToEdit->script;
		};
	auto onTriggerAdded = [&]
		{
			sequencePlayer.CreateSequenceTriggersAvatars(cameraUUID);
		};
	auto setTriggerScriptToEdit = [&](int channel, int frame, bool onEnterScript, SequenceChannelElementTrigger* scriptToEdit)
		{
			selectedScriptChannelFrame = std::make_tuple(channel, frame);
			selectedScriptType = SCET_Trigger;
			selectedScriptToEdit = scriptToEdit;
			selectedScriptToEditContent = onEnterScript ? scriptToEdit->onEnter : scriptToEdit->onLeave;
			isEnterScript = onEnterScript;
		};
	auto onSaveScriptEdit = [&]
		{
			if (selectedScriptType == SCET_Script)
			{
				((SequenceChannelElementScript*)selectedScriptToEdit)->script = selectedScriptToEditContent;
			}
			else if (selectedScriptType == SCET_Trigger)
			{
				if (this->isEnterScript)
				{
					((SequenceChannelElementTrigger*)selectedScriptToEdit)->onEnter = selectedScriptToEditContent;
				}
				else
				{
					((SequenceChannelElementTrigger*)selectedScriptToEdit)->onLeave = selectedScriptToEditContent;
				}
			}
			selectedScriptToEdit = nullptr;
		};
	auto onCancelScriptEdit = [&]
		{
			selectedScriptToEdit = nullptr;
		};

	if (ImGui::BeginPopupModal(title, nullptr, defaultChildFlag))
	{
		DrawTitleBar(title, titlePos, titleSize, exit);
		if (selectedScriptToEdit == nullptr)
		{
			DrawSequenceSelector(seqSelPos, onSelectSequence, onEraseSequence, onRenameSequence, onCloneSequence, onAddNewSequence);
			DrawModelPreview(modelPos, modelSize);
			if (!selectedSequence.empty())
			{
				DrawTimelineController(timeControllerPos, timeControllerSize, sequencePlayer.sequence);
				timelineEditor.Draw(renderable, sequencePlayer.sequence, sequencerPos, sequencerSize,
					transformationKeyFrameSelected,
					transformationKeyFrameDeleted,
					boneTransformationKeyFrameSelected,
					boneTransformationKeyFrameDeleted,
					setScriptToEdit,
					onTriggerAdded,
					setElementTrigger,
					setTriggerScriptToEdit,
					onDeleteChannel
				);
			}
			if (selectedTransformationKeyframe != nullptr)
			{
				if (selectedTransformationKeyFrameType == SCET_Transformation)
				{
					//do not use transformation for getting the world transformation as kM will already have this transformation
					XMMATRIX kM = selectedTransformation->GetTransformationInFrame(sequencePlayer.currentFrame);
					renderable->animationUseTransformation(false);
					XMMATRIX cmx = XMMatrixMultiply(kM, renderable->world());
					renderable->animationUseTransformation(true);
					XMStoreFloat4x4(&gizmoCentroidMx, cmx);
					DrawTransformationKeyFrameAttributes(*selectedTransformationKeyframe, keyFrameFrame, keyframePos, keyframeSize);
					DrawKeyFrameGuizmo(gizmoCentroidMx, *selectedTransformationKeyframe, modelPos, modelSize);
				}
				else if (selectedTransformationKeyFrameType == SCET_BoneTransformation)
				{
					DrawBoneTransformationKeyFrameAttributes(*selectedBoneTransformation, *selectedTransformationKeyframe, keyFrameFrame, keyframePos, keyframeSize);
					auto [mm, pos, a, b, c] = renderable->GetBoneTransformation(selectedBoneTransformation->GetBone());
					XMStoreFloat4x4(&gizmoCentroidMx, mm);
					DrawBoneKeyFrameGuizmo(gizmoCentroidMx, *selectedTransformationKeyframe, modelPos, modelSize);
					auto [x, y, behind] = camera->Project(XMLoadFloat3(&pos));
					if (!behind)
					{
						ImGui::PushID("bone-name");
						{
							ImVec2 boneNamePos(modelPos.x + x, modelPos.y + y);
							ImU32 color = rgba(131, 255, 139, 1);
							ImGui::GetForegroundDrawList()->AddText(boneNamePos, color, selectedBoneTransformation->GetBone().c_str());
						}
						ImGui::PopID();
					}
				}
			}

			if (selectedElementTrigger != nullptr)
			{
				DrawElementTriggerAttributes(*selectedElementTrigger, keyframePos, keyframeSize);
			}
			DrawSaveAndExitButtons(saveExitBtnPos, saveExitBtnSize, exit, saveexit);
			if (addNewSequence)
			{
				DrawAddNewSequencePopup(newSeqPos, newSeqSize, newSequenceName, onAddNewSequenceClicked, onCancelAddNewSequenceClicked);
			}
			if (selectedSequenceRenaming)
			{
				DrawSequenceRenamePopup(newSeqPos, newSeqSize, selectedSequenceNewName, onRenameSequenceClicked, onCancelRenameSequenceClicked);
			}
			if (selectedSequenceCloning)
			{
				DrawSequenceCloningPopup(newSeqPos, newSeqSize, selectedSequenceCloneName, onCloneSequenceClicked, onCancelCloneSequenceClicked);
			}
		}
		else
		{
			DrawScriptEdition(
				selectedScriptToEditContent,
				sequencePlayer.sequence,
				selectedSequence,
				selectedScriptChannelFrame,
				scriptEditPos,
				scriptEditSize,
				onSaveScriptEdit,
				onCancelScriptEdit
			);
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);

	if (nextSelectedTransformationKeyframe != nullptr)
	{
		selectedTransformationKeyFrameType = nextSelectedTransformationKeyFrameType;
		selectedTransformationKeyframe = nextSelectedTransformationKeyframe;
		keyFrameFrame = nextSelectedKeyFrameFrame;
		nextSelectedTransformationKeyframe = nullptr;
		nextSelectedKeyFrameFrame = -1;
	}

	if (nextSelectedElementTrigger != nullptr)
	{
		selectedElementTrigger = nextSelectedElementTrigger;
		nextSelectedElementTrigger = nullptr;
	}

	if (exit)
	{
		Exit();
	}
	if (saveexit)
	{
		SaveAndExit();
	}
}

void AnimationSequencerModal::DrawTitleBar(const char* title, ImVec2 pos, ImVec2 size, bool& exit)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(45.0f / 255.0f, 62.0f / 255.0f, 104.0f / 255.0f, 1.0f));

	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);
	ImGui::BeginChild("title-bar", size, 0);
	{
		float windowWidth = ImGui::GetWindowSize().x;
		float textWidth = ImGui::CalcTextSize(title).x;
		ImVec2 textScreenPos(pos.x + (windowWidth - textWidth) * 0.5f, pos.y + 4.0f);
		ImGui::SetCursorScreenPos(textScreenPos);
		ImGui::Text(title);

		ImVec2 closeButtonScreenPos(pos.x + windowWidth - 20.0f, pos.y);
		//ImGui::SetCursorPos(ImVec2(windowWidth - 19.0f, 0.0f));
		ImGui::SetCursorScreenPos(closeButtonScreenPos);
		if (ImGui::Button(ICON_FA_TIMES, ImVec2(19.0f, 19.0f)))
		{
			exit = true;
		}
	}
	ImGui::EndChild();

	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void AnimationSequencerModal::DrawSequenceSelector(
	ImVec2 screenPos,
	std::function<void(std::string)> onSelectSequence,
	std::function<void(std::string)> onEraseSequence,
	std::function<void(std::string)> onRenameSequence,
	std::function<void(std::string)> onCloneSequence,
	std::function<void()> onAddSequence
)
{
	std::string title = "Sequence";

	ImVec2 textSize = ImGui::CalcTextSize(title.c_str());
	ImVec2 selectorSize(270, 60);
	ImVec2 windowSize(textSize.x + selectorSize.x + 6, selectorSize.y);
	ImVec2 windowPos(screenPos.x + 4, screenPos.y + 2);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

	ImGui::SetNextWindowPos(windowPos);
	ImGui::SetNextWindowSize(windowSize);
	ImGui::BeginChild("sequence-selector", windowSize, 0);
	{
		ImGui::Text(title.c_str());

		std::vector<std::string> sequences = { "" };
		std::vector<std::string> modelSequences = nostd::GetKeysFromMap(animationsSequences.sequences);
		nostd::AppendToVector(sequences, modelSequences);
		std::sort(sequences.begin(), sequences.end());

		ImGui::DrawItemWithEnabledState([this, onEraseSequence]
			{
				if (ImGui::Button(ICON_FA_TIMES))
				{
					onEraseSequence(selectedSequence);
				}
			}
		, selectedSequence != "");
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_PLUS))
		{
			onAddSequence();
		}
		ImGui::SameLine();
		ImVec2 comboSize(selectorSize.x - 50, selectorSize.y);
		ImGui::SetNextItemWidth(comboSize.x);
		ImGui::DrawComboSelection(selectedSequence, sequences, onSelectSequence);
		ImGui::SameLine();
		ImGui::BeginChild("right-buttons");
		{
			ImGui::PushID("rename-button");
			ImGui::DrawItemWithEnabledState([&]
				{
					if (ImGui::Button("Rename"))
					{
						onRenameSequence(selectedSequence);
					}
				}
			, selectedSequence != "");
			ImGui::PopID();
			ImGui::PushID("clone-button");
			ImGui::DrawItemWithEnabledState([&]
				{
					if (ImGui::Button("Clone"))
					{
						onCloneSequence(selectedSequence);
					}
				}
			, selectedSequence != "");
			ImGui::PopID();
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void AnimationSequencerModal::DrawModelPreview(ImVec2 curPos, ImVec2 size)
{
	if (camera.empty()) return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

	ImGuiIO& io = ImGui::GetIO();
	ImVec2 mouse = io.MousePos;

	ImGui::SetNextWindowSize(size, 0);
	ImGui::SetNextWindowPos(curPos, 0);

	ImRect previewRect(curPos, ImVec2(curPos.x + size.x, curPos.y + size.y));

	ImGui::BeginChild("model-preview", size, 0, ImGuiWindowFlags_NoScrollbar);
	{
		auto& pass = camera->renderPassesUUID.at(0);
		bool mouseInPreviewArea = previewRect.Contains(mouse);
		if ((ImGuizmo::IsOver() || ImGuizmo::IsUsing()))
		{
			mouseInPreviewArea = false;
			mousePreviewLeftClickPressed = false;
		}

		if (mouseInPreviewArea)
		{
			if (!wheelCapture)
			{
				wheelCapture = true;
			}
			else
			{
				if (flyMode == PreviewFlyMode_BoundingBox || flyMode == PreviewFlyMode_Bone)
				{
					float modelDistanceScale = camera->at("modelDistanceScale");
					float wheelFactor = camera->at("wheelFactor");
					modelDistanceScale -= io.MouseWheel * modelDistanceScale * wheelFactor;
					modelDistanceScale = std::max(modelDistanceScale, 0.1f);
					camera->at("modelDistanceScale") = modelDistanceScale;
				}
				else if (flyMode == PreviewFlyMode_Free)
				{
					float step = io.MouseWheel;
					XMFLOAT3 pos = ToXMFLOAT3(camera->at("freeposition"));
					XMVECTOR posV = XMLoadFloat3(&pos) + camera->forward() * camera->at("wheelFactor") * step;
					XMStoreFloat3(&pos, posV);
					camera->at("freeposition") = FromXMFLOAT3(pos);
				}
			}
			if (ImGui::IsMouseDown(0) && !mousePreviewLeftClickPressed)
			{
				mousePreviewLeftClickPressed = true;
				mousePreviewLeftClickLastCoords = mouse;
			}
			if (ImGui::IsMouseDown(1) && !mousePreviewRightClickPressed)
			{
				mousePreviewRightClickPressed = true;
				mousePreviewRightClickLastCoords = mouse;
			}
		}
		else
		{
			wheelCapture = false;
		}

		if (mousePreviewLeftClickPressed)
		{
			if (!ImGui::IsMouseDown(0))
			{
				mousePreviewLeftClickPressed = false;
			}
			else if (!(ImGuizmo::IsOver() || ImGuizmo::IsUsing()))
			{
				ImVec2 delta(mouse.x - mousePreviewLeftClickLastCoords.x, mouse.y - mousePreviewLeftClickLastCoords.y);
				mousePreviewLeftClickLastCoords = mouse;
				if (flyMode == PreviewFlyMode_BoundingBox)
				{
					float modelDistanceScale = camera->at("modelDistanceScale");
					XMVECTOR qInc = XMQuaternionRotationRollPitchYaw(delta.y * 0.01f, delta.x * 0.01f, 0.0f);
					BoundingBox bb = renderable->GetBoundingBox();
					XMVECTOR bbp = XMLoadFloat3(&bb.Center);
					XMVECTOR invFw = -1.0f * camera->forward();
					invFw = XMVector3Rotate(invFw, qInc) * modelDistanceScale;
					camera->positionV(invFw + bbp);
					camera->LookAt(bbp);
				}
				else if (flyMode == PreviewFlyMode_Bone)
				{
					float modelDistanceScale = camera->at("modelDistanceScale");
					auto [mm, pos, a, b, c] = renderable->GetBoneTransformation(selectedBoneTransformation->GetBone());
					XMVECTOR qInc = XMQuaternionRotationRollPitchYaw(delta.y * 0.01f, delta.x * 0.01f, 0.0f);
					XMVECTOR bbp = XMLoadFloat3(&pos);
					XMVECTOR invFw = -1.0f * camera->forward();
					invFw = XMVector3Rotate(invFw, qInc) * modelDistanceScale;
					camera->positionV(invFw + bbp);
					camera->LookAt(bbp);
				}
				else if (flyMode == PreviewFlyMode_Free)
				{
					if (std::fabsf(delta.x) > 0.01f || std::fabsf(delta.y) > 0.01f)
					{
						XMFLOAT3 rotation = ToXMFLOAT3(camera->at("freerotation"));
						XMVECTOR rotQ = XMQuaternionRotationRollPitchYaw(
							XMConvertToRadians(rotation.x),
							XMConvertToRadians(rotation.y),
							0.0f
						);
						XMVECTOR qInc = XMQuaternionRotationRollPitchYaw(delta.y * 0.01f, delta.x * 0.01f, 0.0f);
						XMVECTOR qNew = XMQuaternionNormalize(XMQuaternionMultiply(rotQ, qInc));
						XMFLOAT3 euler = Quaternion2Euler(qNew);
						camera->at("freerotation") = FromXMFLOAT3(euler);
					}
				}
			}
		}
		if (mousePreviewRightClickPressed)
		{
			if (!ImGui::IsMouseDown(1))
			{
				mousePreviewRightClickPressed = false;
			}
			else
			{
				ImVec2 delta(mouse.x - mousePreviewRightClickLastCoords.x, mouse.y - mousePreviewRightClickLastCoords.y);
				mousePreviewRightClickLastCoords = mouse;
				if (flyMode == PreviewFlyMode_Free)
				{
					XMFLOAT3 rotation = ToXMFLOAT3(camera->at("freerotation"));
					XMVECTOR rotQ = XMQuaternionRotationRollPitchYaw(
						XMConvertToRadians(rotation.x),
						XMConvertToRadians(rotation.y),
						XMConvertToRadians(rotation.z)
					);
					XMVECTOR dir = { 0.0f, 0.0f, 1.0f,0.0f };
					XMVECTOR fw = XMVector3Normalize(XMVector3Rotate(dir, rotQ));
					XMVECTOR up = { 0.0f, 1.0f, 0.0f,0.0f };
					up = XMVector3Normalize(XMVector3Rotate(up, rotQ));
					XMVECTOR right = XMVector3Cross(fw, up);
					XMVECTOR disp = -up * delta.y * 0.01f - right * delta.x * 0.01f;
					XMFLOAT3 pos = ToXMFLOAT3(camera->at("freeposition"));
					XMVECTOR posV = XMLoadFloat3(&pos) + disp;
					XMStoreFloat3(&pos, posV);
					camera->at("freeposition") = FromXMFLOAT3(pos);
				}
			}
		}

		ImGui::DrawTextureImage(
			(ImTextureID)
			pass->renderToTexturePass->renderToTexture[0]->gpuTextureHandle.ptr,
			pass->renderToTexturePass->renderToTexture[0]->width,
			pass->renderToTexturePass->renderToTexture[0]->height
		);
	}
	ImGui::EndChild();

	ImVec2 attsPos(curPos.x + size.x + 20.0f, curPos.y);
	ImVec2 attsSize(300.0f, 200.0f);
	ImGui::SetNextWindowPos(attsPos, 0);
	ImGui::SetNextWindowSize(attsSize, 0);
	ImGui::BeginChild("camera-atts", attsSize, 0);
	{

		ImGui::Text("floor");
		std::vector<JObject*> floorV({ GetSceneObjectPointer(unit,floor.uuid()) });
		XMFLOAT3 a = ToXMFLOAT3(floor->at("floorColor"));
		DrawValue<XMFLOAT3, jedv_t_color_float3>()("floorColor", floorV);
		XMFLOAT3 b = ToXMFLOAT3(floor->at("floorColor"));
		if (!(a.x == b.x && a.y == b.y && a.z == b.z))
		{
			WriteFloorColorConstantsBuffer();
		}

		bool adjustToBoundingBox = flyMode != PreviewFlyMode_Free;
		ImGui::Text("camera");
		if (ImGui::Checkbox("Adjust camera", &adjustToBoundingBox))
		{
			if (!adjustToBoundingBox)
			{
				prevFlyMode = flyMode;
				flyMode = PreviewFlyMode_Free;
			}
			else
			{
				flyMode = prevFlyMode;
			}
		}
		if (!adjustToBoundingBox)
		{
			Camera* cam = (Camera*)GetSceneObjectPointer(unit, camera.uuid());
			std::vector<JObject*> camV({ cam });
			DrawValue<XMFLOAT3, jedv_t_float3>()("freeposition", camV);
			DrawValue<XMFLOAT3, jedv_t_float3_angle>()("freerotation", camV);
		}

	}
	ImGui::EndChild();

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void AnimationSequencerModal::ResetGizmoVariableWorkers()
{
	gizmoRotation = XMFLOAT3();
	gizmoPosition = XMFLOAT3();
	gizmoScale = XMFLOAT3(1.0f, 1.0f, 1.0f);
}

void AnimationSequencerModal::DrawKeyFrameGuizmo(XMFLOAT4X4& world4x4, TransformationKeyFrame& keyframe, ImVec2 curPos, ImVec2 size)
{
	if (camera.empty()) return;

	if (ImGui::IsKeyPressed(ImGuiKey_T)) // t ky
	{
		gizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
		gizmoMode = ImGuizmo::MODE::WORLD;
		ResetGizmoVariableWorkers();
	}
	if (ImGui::IsKeyPressed(ImGuiKey_R)) // r key
	{
		gizmoOperation = ImGuizmo::OPERATION::ROTATE;
		gizmoMode = ImGuizmo::MODE::WORLD;
		ResetGizmoVariableWorkers();
	}
	if (ImGui::IsKeyPressed(ImGuiKey_S)) // s Key
	{
		gizmoOperation = ImGuizmo::OPERATION::SCALE;
		gizmoMode = ImGuizmo::MODE::LOCAL;
		ResetGizmoVariableWorkers();
	}

	auto translateObjects = [&](XMFLOAT4X4 view, XMFLOAT4X4 proj)
		{
			XMFLOAT4X4 delta;
			ImGuizmo::Manipulate(*view.m, *proj.m, gizmoOperation, gizmoMode, *world4x4.m, *delta.m, NULL, NULL, NULL);
			XMMATRIX XMdelta = XMLoadFloat4x4(&delta);

			XMVECTOR XMtranslation, XMrotation, XMscale;
			XMMatrixDecompose(&XMscale, &XMrotation, &XMtranslation, XMdelta);

			XMVECTOR len = XMVector3Length(XMtranslation);
			if (len.m128_f32[0] < g_XMEpsilon.f[0])
				return;

			XMVECTOR det;
			renderable->animationUseTransformation(false);
			XMMATRIX invWorld = XMMatrixInverse(&det, renderable->world());
			renderable->animationUseTransformation(true);

			XMtranslation = XMVector3Transform(XMtranslation, invWorld);

			keyframe.position.x += XMtranslation.m128_f32[0];
			keyframe.position.y += XMtranslation.m128_f32[1];
			keyframe.position.z += XMtranslation.m128_f32[2];
		};
	auto rotateObjects = [&](XMFLOAT4X4 view, XMFLOAT4X4 proj)
		{
			XMFLOAT4X4 delta;
			ImGuizmo::Manipulate(*view.m, *proj.m, gizmoOperation, gizmoMode, *world4x4.m, *delta.m, NULL, NULL, NULL);
			XMMATRIX XMdelta = XMLoadFloat4x4(&delta);

			XMVECTOR XMtranslation, XMrotation, XMscale;
			XMMatrixDecompose(&XMscale, &XMrotation, &XMtranslation, XMdelta);

			XMVECTOR len = XMVector3Length(XMrotation);
			if (len.m128_f32[0] < g_XMEpsilon.f[0])
				return;

			keyframe.rotation.z += -XMConvertToDegrees(2.0f * XMrotation.m128_f32[0]);
			keyframe.rotation.y += XMConvertToDegrees(2.0f * XMrotation.m128_f32[1]);
			keyframe.rotation.x += XMConvertToDegrees(2.0f * XMrotation.m128_f32[2]);
		};
	auto scaleObjects = [&](XMFLOAT4X4 view, XMFLOAT4X4 proj)
		{
			XMFLOAT4X4 delta;
			ImGuizmo::Manipulate(*view.m, *proj.m, gizmoOperation, gizmoMode, *world4x4.m, *delta.m, NULL, NULL, NULL);
			XMMATRIX XMdelta = XMLoadFloat4x4(&delta);
			XMVECTOR XMtranslation, XMrotation, XMscale;
			XMMatrixDecompose(&XMscale, &XMrotation, &XMtranslation, XMdelta);

			XMVECTOR len = XMVector3Length(XMscale);
			if (len.m128_f32[0] < g_XMEpsilon.f[0])
				return;

			keyframe.scale.x *= XMscale.m128_f32[0];
			keyframe.scale.y *= XMscale.m128_f32[1];
			keyframe.scale.z *= XMscale.m128_f32[2];
		};

	std::unordered_map<ImGuizmo::OPERATION, std::function<void(XMFLOAT4X4, XMFLOAT4X4)>> operators =
	{
		{ ImGuizmo::OPERATION::TRANSLATE, translateObjects },
		{ ImGuizmo::OPERATION::ROTATE, rotateObjects},
		{ ImGuizmo::OPERATION::SCALE, scaleObjects }
	};

	BeginGizmoInteraction(camera, curPos, size, [&](XMFLOAT4X4 view, XMFLOAT4X4 proj)
		{
			operators.at(gizmoOperation)(view, proj);
		}
	);
}

void AnimationSequencerModal::DrawBoneKeyFrameGuizmo(XMFLOAT4X4& world4x4, TransformationKeyFrame& keyframe, ImVec2 curPos, ImVec2 size)
{
	if (camera.empty()) return;

	if (ImGui::IsKeyPressed(ImGuiKey_T)) // t ky
	{
		gizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
		gizmoMode = ImGuizmo::MODE::LOCAL;
		ResetGizmoVariableWorkers();
	}
	if (ImGui::IsKeyPressed(ImGuiKey_R)) // r key
	{
		gizmoOperation = ImGuizmo::OPERATION::ROTATE;
		gizmoMode = ImGuizmo::MODE::LOCAL;
		ResetGizmoVariableWorkers();
	}
	if (ImGui::IsKeyPressed(ImGuiKey_S)) // s Key
	{
		gizmoOperation = ImGuizmo::OPERATION::SCALE;
		gizmoMode = ImGuizmo::MODE::LOCAL;
		ResetGizmoVariableWorkers();
	}

	auto translateObjects = [&](XMFLOAT4X4 view, XMFLOAT4X4 proj)
		{
			XMFLOAT4X4 delta;
			ImGuizmo::Manipulate(*view.m, *proj.m, gizmoOperation, gizmoMode, *world4x4.m, *delta.m, NULL, NULL, NULL);
			XMMATRIX XMdelta = XMLoadFloat4x4(&delta);

			XMVECTOR XMtranslation, XMrotation, XMscale;
			XMMatrixDecompose(&XMscale, &XMrotation, &XMtranslation, XMdelta);

			XMVECTOR len = XMVector3Length(XMtranslation);
			if (len.m128_f32[0] < g_XMEpsilon.f[0])
				return;

			XMVECTOR det;
			renderable->animationUseTransformation(false);
			XMMATRIX invWorld = XMMatrixInverse(&det, renderable->world());
			renderable->animationUseTransformation(true);

			XMtranslation = XMVector3Transform(XMtranslation, invWorld);

			keyframe.position.x += XMtranslation.m128_f32[0];
			keyframe.position.y += XMtranslation.m128_f32[1];
			keyframe.position.z += XMtranslation.m128_f32[2];
		};
	auto rotateObjects = [&](XMFLOAT4X4 view, XMFLOAT4X4 proj)
		{
			XMFLOAT4X4 delta;
			ImGuizmo::Manipulate(*view.m, *proj.m, gizmoOperation, gizmoMode, *world4x4.m, *delta.m, NULL, NULL, NULL);
			XMMATRIX XMdelta = XMLoadFloat4x4(&delta);

			XMVECTOR XMtranslation, XMrotation, XMscale;
			XMMatrixDecompose(&XMscale, &XMrotation, &XMtranslation, XMdelta);

			XMVECTOR len = XMVector3Length(XMrotation);
			if (len.m128_f32[0] < g_XMEpsilon.f[0])
				return;

			keyframe.rotation.z += -XMConvertToDegrees(2.0f * XMrotation.m128_f32[0]);
			keyframe.rotation.y += XMConvertToDegrees(2.0f * XMrotation.m128_f32[1]);
			keyframe.rotation.x += XMConvertToDegrees(2.0f * XMrotation.m128_f32[2]);
		};
	auto scaleObjects = [&](XMFLOAT4X4 view, XMFLOAT4X4 proj)
		{
			XMFLOAT4X4 delta;
			ImGuizmo::Manipulate(*view.m, *proj.m, gizmoOperation, gizmoMode, *world4x4.m, *delta.m, NULL, NULL, NULL);
			XMMATRIX XMdelta = XMLoadFloat4x4(&delta);
			XMVECTOR XMtranslation, XMrotation, XMscale;
			XMMatrixDecompose(&XMscale, &XMrotation, &XMtranslation, XMdelta);

			XMVECTOR len = XMVector3Length(XMscale);
			if (len.m128_f32[0] < g_XMEpsilon.f[0])
				return;

			keyframe.scale.x *= XMscale.m128_f32[0];
			keyframe.scale.y *= XMscale.m128_f32[1];
			keyframe.scale.z *= XMscale.m128_f32[2];
		};

	std::unordered_map<ImGuizmo::OPERATION, std::function<void(XMFLOAT4X4, XMFLOAT4X4)>> operators =
	{
		{ ImGuizmo::OPERATION::TRANSLATE, translateObjects },
		{ ImGuizmo::OPERATION::ROTATE, rotateObjects},
		{ ImGuizmo::OPERATION::SCALE, scaleObjects }
	};

	BeginGizmoInteraction(camera, curPos, size, [&](XMFLOAT4X4 view, XMFLOAT4X4 proj)
		{
			operators.at(gizmoOperation)(view, proj);
		}
	);
}

void AnimationSequencerModal::BeginGizmoInteraction(CameraID camera, ImVec2 curPos, ImVec2 size, std::function<void(XMFLOAT4X4 view, XMFLOAT4X4 proj)> interaction)
{
	ImGuizmo::BeginFrame();
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::AllowAxisFlip(false);
	ImDrawList* drawList = ImGui::GetForegroundDrawList();
	ImGuizmo::SetDrawlist(drawList);
	drawList->PushClipRect(curPos, ImVec2(curPos.x + size.x, curPos.y + size.y), true);
	ImGuizmo::SetGizmoSizeClipSpace(0.2f);

	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(curPos.x, curPos.y, size.x, size.y);

	ImGuizmo::SetID(1);

	XMFLOAT4X4 view;
	XMFLOAT4X4 proj;
	XMStoreFloat4x4(&view, camera->view());
	camera->perspectiveProjection.updateProjectionMatrix();
	XMStoreFloat4x4(&proj, camera->perspectiveProjection.projectionMatrix);

	interaction(view, proj);

	ImGuizmo::SetGizmoSizeClipSpace(0.1f);
	drawList->PopClipRect();
}

void AnimationSequencerModal::DrawBoneTransformationKeyFrameAttributes(SequenceChannelElementBoneTransformation& boneTransformation, TransformationKeyFrame& keyframe, int keyFrameFrame, ImVec2 pos, ImVec2 size)
{
	std::vector<std::string> bones = nostd::GetKeysFromMap(renderable->animable->animations->bonesOffsets);

	const int defaultTableFlags = ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

	ImVec2 attsPos(pos);
	ImVec2 attsSize(size);
	ImGui::SetNextWindowPos(attsPos, 0);
	ImGui::SetNextWindowSize(attsSize, 0);
	ImGui::BeginChild("keyframe-atts", attsSize, 0);
	{
		ImGui::Text(std::string(std::string("keyframe at frame#") + std::to_string(keyFrameFrame)).c_str());;

		ImGui::Text("bone");
		ImGui::SameLine();
		ImGui::PushID("keyframe-bone");
		{
			ImGui::DrawComboSelection(boneTransformation.bone, bones, [&boneTransformation](std::string selectedBone)
				{
					boneTransformation.bone = selectedBone;
				}
			);
		}
		ImGui::PopID();

		ImGui::Text("position");
		ImGui::PushID("keyframe-position");
		if (ImGui::BeginTable("keyframe-position-table", 3, defaultTableFlags))
		{
			bool reset = false;
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			reset |= ImGui::InputFloat("x", &keyframe.position.x, 0.0f, 0.0f, "%.3f");
			ImGui::TableSetColumnIndex(1);
			reset |= ImGui::InputFloat("y", &keyframe.position.y, 0.0f, 0.0f, "%.3f");
			ImGui::TableSetColumnIndex(2);
			reset |= ImGui::InputFloat("z", &keyframe.position.z, 0.0f, 0.0f, "%.3f");
			ImGui::EndTable();
			if (reset)
			{
				ResetGizmoVariableWorkers();
			}
		}
		ImGui::PopID();

		ImGui::Text("rotation");
		ImGui::PushID("keyframe-rotation");
		if (ImGui::BeginTable("keyframe-rotation-table", 3, defaultTableFlags))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			float pitch = XMConvertToRadians(keyframe.rotation.x);
			if (ImGui::SliderAngle("pitch", &pitch, -180.0f, 180.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
			{
				keyframe.rotation.x = XMConvertToDegrees(pitch);
				ResetGizmoVariableWorkers();
			}
			ImGui::TableSetColumnIndex(1);
			float yaw = XMConvertToRadians(keyframe.rotation.y);
			if (ImGui::SliderAngle("yaw", &yaw, -180.0f, 180.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
			{
				keyframe.rotation.y = XMConvertToDegrees(yaw);
				ResetGizmoVariableWorkers();
			}
			ImGui::TableSetColumnIndex(2);
			float roll = XMConvertToRadians(keyframe.rotation.z);
			if (ImGui::SliderAngle("roll", &roll, -180.0f, 180.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
			{
				keyframe.rotation.z = XMConvertToDegrees(roll);
				ResetGizmoVariableWorkers();
			}
			ImGui::EndTable();
		}
		ImGui::PopID();

		ImGui::Text("scale");
		ImGui::PushID("keyframe-scale");
		if (ImGui::BeginTable("keyframe-scale-table", 3, defaultTableFlags))
		{
			bool reset = false;
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			reset |= ImGui::InputFloat("x", &keyframe.scale.x, 0.0f, 0.0f, "%.3f");
			ImGui::TableSetColumnIndex(1);
			reset |= ImGui::InputFloat("y", &keyframe.scale.y, 0.0f, 0.0f, "%.3f");
			ImGui::TableSetColumnIndex(2);
			reset |= ImGui::InputFloat("z", &keyframe.scale.z, 0.0f, 0.0f, "%.3f");
			ImGui::EndTable();
			if (reset)
			{
				ResetGizmoVariableWorkers();
			}
		}
		ImGui::PopID();

		ImGui::Text("ease");
		ImGui::PushID("keyframe-easing");
		std::string selectedEase = EasingToString.at(keyframe.easing);
		ImGui::DrawComboSelection(selectedEase, nostd::GetKeysFromMap(StringToEasing), [&](std::string newEase)
			{
				keyframe.easing = StringToEasing.at(newEase);
				ResetGizmoVariableWorkers();
			}
		);
		ImGui::PopID();
	}
	ImGui::EndChild();

	ImGui::PopStyleVar(2);
}

void AnimationSequencerModal::DrawTransformationKeyFrameAttributes(TransformationKeyFrame& keyframe, int keyFrameFrame, ImVec2 pos, ImVec2 size)
{
	const int defaultTableFlags = ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

	ImVec2 attsPos(pos);
	ImVec2 attsSize(size);
	ImGui::SetNextWindowPos(attsPos, 0);
	ImGui::SetNextWindowSize(attsSize, 0);
	ImGui::BeginChild("keyframe-atts", attsSize, 0);
	{
		ImGui::Text(std::string(std::string("keyframe at frame#") + std::to_string(keyFrameFrame)).c_str());;

		ImGui::Text("position");
		ImGui::PushID("keyframe-position");
		if (ImGui::BeginTable("keyframe-position-table", 3, defaultTableFlags))
		{
			bool reset = false;
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			reset |= ImGui::InputFloat("x", &keyframe.position.x, 0.0f, 0.0f, "%.3f");
			ImGui::TableSetColumnIndex(1);
			reset |= ImGui::InputFloat("y", &keyframe.position.y, 0.0f, 0.0f, "%.3f");
			ImGui::TableSetColumnIndex(2);
			reset |= ImGui::InputFloat("z", &keyframe.position.z, 0.0f, 0.0f, "%.3f");
			ImGui::EndTable();
			if (reset)
			{
				ResetGizmoVariableWorkers();
			}
		}
		ImGui::PopID();

		ImGui::Text("rotation");
		ImGui::PushID("keyframe-rotation");
		if (ImGui::BeginTable("keyframe-rotation-table", 3, defaultTableFlags))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			float pitch = XMConvertToRadians(keyframe.rotation.x);
			if (ImGui::SliderAngle("pitch", &pitch, -180.0f, 180.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
			{
				keyframe.rotation.x = XMConvertToDegrees(pitch);
				ResetGizmoVariableWorkers();
			}
			ImGui::TableSetColumnIndex(1);
			float yaw = XMConvertToRadians(keyframe.rotation.y);
			if (ImGui::SliderAngle("yaw", &yaw, -180.0f, 180.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
			{
				keyframe.rotation.y = XMConvertToDegrees(yaw);
				ResetGizmoVariableWorkers();
			}
			ImGui::TableSetColumnIndex(2);
			float roll = XMConvertToRadians(keyframe.rotation.z);
			if (ImGui::SliderAngle("roll", &roll, -180.0f, 180.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
			{
				keyframe.rotation.z = XMConvertToDegrees(roll);
				ResetGizmoVariableWorkers();
			}
			ImGui::EndTable();
		}
		ImGui::PopID();

		ImGui::Text("scale");
		ImGui::PushID("keyframe-scale");
		if (ImGui::BeginTable("keyframe-scale-table", 3, defaultTableFlags))
		{
			bool reset = false;
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			reset |= ImGui::InputFloat("x", &keyframe.scale.x, 0.0f, 0.0f, "%.3f");
			ImGui::TableSetColumnIndex(1);
			reset |= ImGui::InputFloat("y", &keyframe.scale.y, 0.0f, 0.0f, "%.3f");
			ImGui::TableSetColumnIndex(2);
			reset |= ImGui::InputFloat("z", &keyframe.scale.z, 0.0f, 0.0f, "%.3f");
			ImGui::EndTable();
			if (reset)
			{
				ResetGizmoVariableWorkers();
			}
		}
		ImGui::PopID();

		ImGui::Text("ease");
		ImGui::PushID("keyframe-easing");
		std::string selectedEase = EasingToString.at(keyframe.easing);
		ImGui::DrawComboSelection(selectedEase, nostd::GetKeysFromMap(StringToEasing), [&](std::string newEase)
			{
				keyframe.easing = StringToEasing.at(newEase);
				ResetGizmoVariableWorkers();
			}
		);
		ImGui::PopID();
	}
	ImGui::EndChild();

	ImGui::PopStyleVar(2);
}

void AnimationSequencerModal::DrawElementTriggerAttributes(SequenceChannelElementTrigger& elementTrigger, ImVec2 pos, ImVec2 size)
{
	const int defaultTableFlags = ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

	ImVec2 attsPos(pos);
	ImVec2 attsSize(size);
	ImGui::SetNextWindowPos(attsPos, 0);
	ImGui::SetNextWindowSize(attsSize, 0);
	ImGui::BeginChild("trigger-atts", attsSize, 0);
	{
		ImGui::Text("bone");
		ImGui::PushID("trigger-bone");
		if (ImGui::BeginTable("trigger-bone", 1, defaultTableFlags))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::DrawComboSelection(elementTrigger.bone, bones, [&](std::string bone)
				{
					elementTrigger.bone = bone;
				}
			);
			ImGui::EndTable();
		}
		ImGui::PopID();

		ImGui::Text("color");
		ImGui::SameLine();
		ImGui::PushID("trigger-color");
		XMFLOAT4 color = elementTrigger.color;
		if (ImGui::ColorEdit4("##", &color.x, ImGuiColorEditFlags_NoInputs))
		{
			elementTrigger.color = color;
		}
		ImGui::PopID();

		ImGui::Text("position");
		ImGui::PushID("trigger-position");
		if (ImGui::BeginTable("trigger-position-table", 3, defaultTableFlags))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::InputFloat("x", &elementTrigger.position.x, 0.0f, 0.0f, "%.3f");
			ImGui::TableSetColumnIndex(1);
			ImGui::InputFloat("y", &elementTrigger.position.y, 0.0f, 0.0f, "%.3f");
			ImGui::TableSetColumnIndex(2);
			ImGui::InputFloat("z", &elementTrigger.position.z, 0.0f, 0.0f, "%.3f");
			ImGui::EndTable();
		}
		ImGui::PopID();

		ImGui::Text("rotation");
		ImGui::PushID("trigger-rotation");
		if (ImGui::BeginTable("trigger-rotation-table", 3, defaultTableFlags))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			float pitch = XMConvertToRadians(elementTrigger.rotation.x);
			if (ImGui::SliderAngle("pitch", &pitch, -180.0f, 180.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
			{
				elementTrigger.rotation.x = XMConvertToDegrees(pitch);
			}
			ImGui::TableSetColumnIndex(1);
			float yaw = XMConvertToRadians(elementTrigger.rotation.y);
			if (ImGui::SliderAngle("yaw", &yaw, -180.0f, 180.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
			{
				elementTrigger.rotation.y = XMConvertToDegrees(yaw);
			}
			ImGui::TableSetColumnIndex(2);
			float roll = XMConvertToRadians(elementTrigger.rotation.z);
			if (ImGui::SliderAngle("roll", &roll, -180.0f, 180.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
			{
				elementTrigger.rotation.z = XMConvertToDegrees(roll);
			}
			ImGui::EndTable();
		}
		ImGui::PopID();

		ImGui::Text("scale");
		ImGui::PushID("trigger-scale");
		if (ImGui::BeginTable("trigger-scale-table", 3, defaultTableFlags))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::InputFloat("x", &elementTrigger.scale.x, 0.0f, 0.0f, "%.3f");
			ImGui::TableSetColumnIndex(1);
			ImGui::InputFloat("y", &elementTrigger.scale.y, 0.0f, 0.0f, "%.3f");
			ImGui::TableSetColumnIndex(2);
			ImGui::InputFloat("z", &elementTrigger.scale.z, 0.0f, 0.0f, "%.3f");
			ImGui::EndTable();
		}
		ImGui::PopID();

		std::vector<std::string> collisionMasks = GetCollisionMasks();
		ImGui::PushID("trigger-masks");
		if (ImGui::BeginTable("trigger-masks", 2, defaultTableFlags))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("objectMask");
			ImGui::PushID("objectMask");
			for (unsigned int i = 0; i < collisionMasks.size(); i++)
			{
				bool value = !!(elementTrigger.objectMask & (1 << i));
				if (ImGui::Checkbox(collisionMasks.at(i).c_str(), &value))
				{
					if (value)
						elementTrigger.objectMask |= (1 << i);
					else
						elementTrigger.objectMask &= ~(1 << i);
				}
			}
			ImGui::PopID();

			ImGui::TableSetColumnIndex(1);
			ImGui::Text("collisionMask");
			ImGui::PushID("collisionMask");
			for (unsigned int i = 0; i < collisionMasks.size(); i++)
			{
				bool value = !!(elementTrigger.collisionMask & (1 << i));
				if (ImGui::Checkbox(collisionMasks.at(i).c_str(), &value))
				{
					if (value)
						elementTrigger.collisionMask |= (1 << i);
					else
						elementTrigger.collisionMask &= ~(1 << i);
				}
			}
			ImGui::PopID();

			ImGui::EndTable();
		}
		ImGui::PopID();
		/*
		if (ImGui::CollapsingHeader("objectMask"))
		{
			ImGui::PushID("trigger-objectMask");
			if (ImGui::BeginTable("trigger-objectMask", 1, defaultTableFlags))
			{

			}
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("collisionMask"))
		{
			ImGui::PushID("trigger-collisionMask");
			if (ImGui::BeginTable("trigger-collisionMask", 1, defaultTableFlags))
			{

			}
			ImGui::PopID();
		}
		*/
	}
	ImGui::EndChild();

	ImGui::PopStyleVar(2);
}

void AnimationSequencerModal::DrawTimelineController(ImVec2 curPos, ImVec2 size, Sequence& sequence)
{
	float nFramesWidth = ImGui::CalcTextSize("#frames").x + ImGui::GetStyle().FramePadding.x * 2.0f;
	float inputNumFramesWidth = 100.0f;
	float fpsWidth = ImGui::CalcTextSize("FPS").x + ImGui::GetStyle().FramePadding.x * 2.0f;
	float inputFPSWidth = 100;
	float playPauseButtonWidth = ImGui::CalcTextSize(ICON_FA_PLAY).x + ImGui::GetStyle().FramePadding.x * 2.0f;
	float stopButtonWidth = ImGui::CalcTextSize(ICON_FA_STOP).x + ImGui::GetStyle().FramePadding.x * 2.0f;
	float loopWidth = ImGui::CalcTextSize("loop").x + ImGui::GetStyle().FramePadding.x * 2.0f;
	float loopCheckboxWidth = 30.0f;
	float timeWidth = ImGui::CalcTextSize(std::string(std::string("time:") + std::format("{:.2f}", renderable->animationTime() / 1000.0f) + "s").c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f + 30.0f;
	float frameWidth = ImGui::CalcTextSize(std::string(std::string("frame:") + std::to_string(renderable->animationFrame())).c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f + 30.0f;
	float animationWidth = ImGui::CalcTextSize(std::string(std::string("animation:") + std::string((renderable->animation() != "") ? renderable->animation() : "#none")).c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f + 30.0f;
	float total_width =
		nFramesWidth
		+ inputNumFramesWidth
		+ fpsWidth
		+ inputFPSWidth
		+ ImGui::GetStyle().ItemSpacing.x
		+ playPauseButtonWidth
		+ stopButtonWidth
		+ loopCheckboxWidth
		+ timeWidth
		+ frameWidth
		+ animationWidth
		;

	float window_width = ImGui::GetContentRegionAvail().x;
	//ImVec2 start(curPos.x + (window_width - total_width) * 0.5f, curPos.y);
	ImVec2 start(curPos.x + 200, curPos.y);

	ImGui::SetCursorScreenPos(start);

	ImGui::Text("#frames");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputNumFramesWidth);
	ImGui::PushID("TimeControllerTotalFrames");
	if (ImGui::InputInt("##", &sequence.totalFrames, 1, 100))
	{
		sequence.totalFrames = std::max(sequence.totalFrames, 1);
	}
	ImGui::PopID();

	ImGui::SameLine();
	ImGui::Text("FPS");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputFPSWidth);
	ImGui::PushID("TimeControllerFramesPerSecond");
	if (ImGui::InputInt("##", &sequence.framesPerSecond, 1, 100))
	{
		sequence.framesPerSecond = std::max(sequence.framesPerSecond, 1);
	}
	ImGui::PopID();

	ImGui::SameLine();
	if (!playingSequence)
	{
		if (ImGui::Button(ICON_FA_PLAY))
		{
			renderable->animationPlay(true);
			playingSequence = true;
			playingSequenceTime = timelineEditor.GetTime(sequence) / 1000.0f;
			sequencePlayer.currentFrame = timelineEditor.GetFrame(sequence);
			sequencePlayer.runningFrame = timelineEditor.GetFrame(sequence);
		}
	}
	else
	{
		if (ImGui::Button(ICON_FA_PAUSE))
		{
			renderable->animationPlay(false);
			playingSequence = false;
		}
	}

	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_STOP))
	{
		renderable->animationPlay(false);
		renderable->animationTimeFactor(0.0f);
		renderable->animationTime(0.0f);
		playingSequence = false;
		playingSequenceTime = 0.0f;
		sequencePlayer.ResetFrames();
		timelineEditor.selectedFrameInTimeline = 0;
	}

	ImGui::SameLine();
	if (ImGui::Checkbox("loop", &playingSequenceLoop))
	{
		renderable->animationLoop(playingSequenceLoop);
	}

	ImGui::SameLine();
	ImGui::SetNextItemWidth(timeWidth);
	ImGui::Text(std::string(std::string("time:") + std::format("{:.2f}", timelineEditor.GetTime(sequence) / 1000.0f) + "s").c_str());

	int frame = timelineEditor.GetFrame(sequence);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(frameWidth);
	ImGui::Text(std::string(std::string("frame:") + std::to_string(frame)).c_str());

	ImGui::SameLine();
	ImGui::SetNextItemWidth(animationWidth);
	std::string animName = sequence.GetAnimationNameAtFrame(frame);
	ImGui::Text(std::string(std::string("animation:") + ((animName == "") ? "#none" : animName)).c_str());
}

void AnimationSequencerModal::DrawSaveAndExitButtons(ImVec2 curPos, ImVec2 size, bool& exit, bool& saveexit)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

	ImGui::SetNextWindowPos(curPos, 0);
	ImGui::SetNextWindowSize(size, 0);
	ImGui::BeginChild("saveexit", size, 0);
	{
		ImGui::DrawItemWithEnabledState([this, &saveexit]
			{
				if (ImGui::Button("Save & Exit"))
				{
					saveexit = true;
				}
			}
		, true);

		ImGui::SameLine();
		if (ImGui::Button("Exit"))
		{
			exit = true;
		}
	}
	ImGui::EndChild();

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void AnimationSequencerModal::DrawAddNewSequencePopup(ImVec2 pos, ImVec2 size, std::string& newSeqName, std::function<void(std::string)> onAddNewSequenceClicked, std::function<void()> onCancelAddNewSequenceClick)
{
	std::set<std::string> existingSequences = { "" };
	for (auto& [seqName, _] : animationsSequences.sequences)
	{
		existingSequences.insert(seqName);
	}

	const char* title = "Add new sequence";
	ImGui::OpenPopup(title);

	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	if (ImGui::BeginPopupModal(title, nullptr, popupChildFlag))
	{
		ImGui::SetNextItemWidth(size.x);
		ImGui::InputText("##", &newSeqName);

		float button1_width = ImGui::CalcTextSize("Add").x + ImGui::GetStyle().FramePadding.x * 2.0f;
		float button2_width = ImGui::CalcTextSize("Cancel").x + ImGui::GetStyle().FramePadding.x * 2.0f;
		float total_width = button1_width + button2_width + ImGui::GetStyle().ItemSpacing.x;

		float window_width = ImGui::GetContentRegionAvail().x;
		float start_x = (window_width - total_width) * 0.5f;

		ImGui::SetCursorPosX(start_x);

		ImGui::DrawItemWithEnabledState([&]
			{
				if (ImGui::Button("Add"))
				{
					onAddNewSequenceClicked(newSeqName);
				}
			}
		, !existingSequences.contains(newSeqName));

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			onCancelAddNewSequenceClick();
			addNewSequence = false;
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void AnimationSequencerModal::DrawSequenceRenamePopup(ImVec2 pos, ImVec2 size, std::string& newSeqName, std::function<void(std::string)> onRenameSequenceClicked, std::function<void()> onCancelRenameSequenceClicked)
{
	std::set<std::string> existingSequences = { "" };
	for (auto& [seqName, _] : animationsSequences.sequences)
	{
		existingSequences.insert(seqName);
	}

	const char* title = "Rename sequence";
	ImGui::OpenPopup(title);

	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	if (ImGui::BeginPopupModal(title, nullptr, popupChildFlag))
	{
		ImGui::SetNextItemWidth(size.x);
		ImGui::InputText("##", &newSeqName);

		float button1_width = ImGui::CalcTextSize("Rename").x + ImGui::GetStyle().FramePadding.x * 2.0f;
		float button2_width = ImGui::CalcTextSize("Cancel").x + ImGui::GetStyle().FramePadding.x * 2.0f;
		float total_width = button1_width + button2_width + ImGui::GetStyle().ItemSpacing.x;

		float window_width = ImGui::GetContentRegionAvail().x;
		float start_x = (window_width - total_width) * 0.5f;

		ImGui::SetCursorPosX(start_x);

		ImGui::DrawItemWithEnabledState([&]
			{
				if (ImGui::Button("Rename"))
				{
					onRenameSequenceClicked(newSeqName);
				}
			}
		, !existingSequences.contains(newSeqName));

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			onCancelRenameSequenceClicked();
			selectedSequenceRenaming = false;
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void AnimationSequencerModal::DrawSequenceCloningPopup(ImVec2 pos, ImVec2 size, std::string& newSeqName, std::function<void(std::string)> onCloneSequenceClicked, std::function<void()> onCancelCloneSequenceClicked)
{
	std::set<std::string> existingSequences = { "" };
	for (auto& [seqName, _] : animationsSequences.sequences)
	{
		existingSequences.insert(seqName);
	}

	const char* title = "Clone sequence";
	ImGui::OpenPopup(title);

	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	if (ImGui::BeginPopupModal(title, nullptr, popupChildFlag))
	{
		ImGui::SetNextItemWidth(size.x);
		ImGui::InputText("##", &newSeqName);

		float button1_width = ImGui::CalcTextSize("Clone").x + ImGui::GetStyle().FramePadding.x * 2.0f;
		float button2_width = ImGui::CalcTextSize("Cancel").x + ImGui::GetStyle().FramePadding.x * 2.0f;
		float total_width = button1_width + button2_width + ImGui::GetStyle().ItemSpacing.x;

		float window_width = ImGui::GetContentRegionAvail().x;
		float start_x = (window_width - total_width) * 0.5f;

		ImGui::SetCursorPosX(start_x);

		ImGui::DrawItemWithEnabledState([&]
			{
				if (ImGui::Button("Clone"))
				{
					onCloneSequenceClicked(newSeqName);
				}
			}
		, !existingSequences.contains(newSeqName));

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			onCancelCloneSequenceClicked();
			selectedSequenceCloning = false;
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void AnimationSequencerModal::DrawScriptEdition(std::string& content, Sequence& sequence, std::string sequenceName, std::tuple<int, int> channelFrame, ImVec2 pos, ImVec2 size, std::function<void()> onSave, std::function<void()> onCancel)
{
	auto& [channel, frame] = channelFrame;
	ImVec2 titlePos(pos);
	std::string title = "sequence:" + sequenceName + " -> channel:" + sequence.sequenceChannels.at(channel).name + " -> frame:" + std::to_string(frame);
	ImGui::SetCursorScreenPos(pos);
	ImGui::Text(title.c_str());

	ImVec2 editorPos(pos.x, pos.y + 20);
	ImVec2 editorSize(size.x, size.y - 30);
	ImGui::SetNextWindowSize(editorSize, 0);
	ImGui::SetNextWindowPos(editorPos, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	ImGui::BeginChild("script-editor", editorSize, 0);
	{
		ImGui::InputTextMultiline("Script Content", &content, ImVec2(-1, editorSize.y));
	}
	ImGui::EndChild();
	ImGui::PopStyleVar(2);

	ImVec2 buttonsPos(pos.x + 10, editorPos.y + editorSize.y + 5);
	ImGui::SetCursorScreenPos(buttonsPos);
	if (ImGui::Button("Save&Exit"))
	{
		onSave();
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
	{
		onCancel();
	}
}

void AnimationSequencerModal::Exit()
{
	renderable->SetCurrentAnimation(nullptr);
	destroying = true;
	showing = false;
	destructionFrames = JRenderer::numFrames;
}

void AnimationSequencerModal::SaveAndExit()
{
	if (selectedSequence != "" && animationsSequences.sequences.contains(selectedSequence))
	{
		animationsSequences.sequences.insert_or_assign(selectedSequence, sequencePlayer.sequence);
	}

	renderable->SetCurrentAnimation(nullptr);
	destroying = true;
	destructionFrames = JRenderer::numFrames;
	showing = false;
	model3D->animationSequences(animationsSequences);
	model3D->flag(Model3DJson::Update_animationSequences);
	Editor::templatesModified = true;
}
