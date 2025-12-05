#include "pch.h"
#include "AnimationSequencerModal.h"
#include <imgui.h>
#include <ImEditor.h>
#include <nlohmann/json.hpp>
#include <Camera/Camera.h>
#include <Light/Light.h>
#include <Renderable/Renderable.h>
#include <Renderer.h>
#include <Scene.h>
#include <NoMath.h>

extern std::unique_ptr<Renderer> renderer;
extern DX::StepTimer timer;

namespace Editor
{
	extern bool templatesModified;
};

void AnimationSequencerModal::Initialize(JUUID uuid)
{
	showing = true;
	initializing = true;
	model3dUUID = uuid;
	model3D = uuid;
	animationsSequences = model3D->animationSequences();
	selectedTransformationKeyframe = nullptr;
	keyFrameFrame = -1;
	nextSelectedTransformationKeyframe = nullptr;
	nextSelectedKeyFrameFrame = -1;
}

void AnimationSequencerModal::LoadSceneObjects()
{
	using namespace Scene;

	camera = getUUID();
	ambientLight = getUUID();
	directionalLight = getUUID();
	renderable = getUUID();
	floor = getUUID();

	cameraInitialPos = XMFLOAT3({ 0.0, 5.21317195892334, -17.224170684814453 });
	cameraInitialRot = XMFLOAT3({ 12.898999214172363, 0.0, 0.0 });

	nlohmann::json cameraJson =
	{
		{ "fitWindow", false },
		{ "name", "cam-preview" },
		{ "perspective",
			{
				{ "farZ", 100.0 },
				{ "fovAngleY", 20.0 },
				{ "nearZ", 0.01 },
				{ "width", 1778 },
				{ "height", 1000 }
			}
		},
		{ "position", FromXMFLOAT3(cameraInitialPos) },
		{ "freeposition", FromXMFLOAT3(cameraInitialPos) },
		{ "projectionType", "Perspective" },
		{ "rotation", FromXMFLOAT3(cameraInitialRot) },
		{ "freerotation", FromXMFLOAT3(cameraInitialRot) },
		{ "speed", 0.05000000074505806 },
		{ "uuid", camera() },
		{
			"renderPasses", { GetRenderPassUUIDByName("ModelPreviewPass")}
		},
		{ "mouseController", false },
		{ "useSwapChain", false },
		{ "modelDistanceScale", 1.0f } //dynamic magic
	};

	nlohmann::json ambientLightJson =
	{
		{ "color", { 0.25000000074505806, 0.25000000074505806, 0.25000000074505806 } },
		{ "lightType", "Ambient" },
		{ "name", "light.0.amb-preview" },
		{ "uuid", ambientLight()},
		{ "cameras", { camera() }}
	};

	nlohmann::json directionalLightJson =
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
		{ "uuid", directionalLight() },
		{ "zBias", 0.000002 },
		{ "cameras", { camera() } }
	};

	nlohmann::json animableJson =
	{
		{ "castShadows", true },
		{ "shadowed", false },
		{ "model", model3dUUID()},
		{ "name", "preview-model" },
		{ "position", { 0.0, 0.0, 0.0} },
		{ "rotation", { 0.0, -90.0, 0.0 }},
		{ "scale", { 0.1, 0.1, 0.1} },
		{ "uuid", renderable() },
		{ "cameras", { camera() } },
		{ "animationUseTransformation", true }
	};

	nlohmann::json floorJson =
	{
		{ "castShadows", false },
		{ "shadowed", true },
				{
					"meshMaterials",
					{
						{
							{ "material", "ecd1688c-73d6-49d0-870f-ca916a417c49"},
							{ "mesh", "d41e5c29-49bb-4f2c-aa2b-da781fbac512" }
						}
					}
				},
		{ "name", "preview-floor" },
		{ "position", { 0.0, 0.0, 0.0} },
		{ "scale", { 100.0, 100.0, 100.0} },
		{ "uuid", floor() },
		{ "cameras", { camera() } },
		{ "floorColor", { 0.5f, 0.5f, 0.5f } }
	};

	CreateCamera(cameraJson);
	CreateLight(ambientLightJson);
	CreateLight(directionalLightJson);
	CreateRenderable(animableJson);
	CreateRenderable(floorJson);

	camera->BindToScene();
	renderable->BindToScene();
	floor->BindToScene();
	ambientLight->BindToScene();
	directionalLight->BindToScene();

	camera->UpdateProjection();
	directionalLight->shadowMapCameras[0]->UpdateProjection();

	selectedSequence = "";
	initializing = false;

	Step();
	camera->at("freeposition") = FromXMFLOAT3(camera->position());
	camera->at("freerotation") = FromXMFLOAT3(camera->rotation());
}

void AnimationSequencerModal::DestroySceneObjects()
{
	using namespace Scene;

	renderable->UnbindFromScene();
	floor->UnbindFromScene();
	directionalLight->UnbindFromScene();
	ambientLight->UnbindFromScene();
	camera->UnbindFromScene();

	DeleteRenderable(renderable());
	DeleteRenderable(floor());
	DeleteLight(directionalLight());
	DeleteLight(ambientLight());
	DeleteCamera(camera());

	renderable.clear();
	floor.clear();
	directionalLight.clear();
	ambientLight.clear();
	camera.clear();
	model3D.clear();
	model3dUUID.clear();
	selectedSequence.clear();
}

void AnimationSequencerModal::Step()
{
	XMFLOAT3 baseColor = ToXMFLOAT3(floor->at("floorColor"));
	floor->WriteConstantsBuffer<XMFLOAT3>("baseColor", baseColor, renderer->backBufferIndex);

	if (adjustToBoundingBox)
	{
		//https://stackoverflow.com/a/32836605
		BoundingBox modelBB = renderable->GetBoundingBox();
		BoundingSphere modelBBS;
		BoundingSphere::CreateFromBoundingBox(modelBBS, modelBB);

		float modelDistanceScale = camera->at("modelDistanceScale");
		float fov = XMConvertToRadians(camera->perspective().fovAngleY);
		float distance = modelDistanceScale * (adjustToBoundingBox ? (modelBBS.Radius * 2.0f) / (XMScalarSin(fov) / XMScalarCos(fov)) : 1.0f);

		XMVECTOR camFwV = camera->forward();
		XMFLOAT3 BBPos = modelBB.Center;
		XMVECTOR BBPosV = XMLoadFloat3(&BBPos);
		XMVECTOR camPosV = XMVectorSubtract(BBPosV, XMVectorScale(camFwV, distance));
		XMFLOAT3 camPos;
		XMStoreFloat3(&camPos, camPosV);
		camera->position(camPos);
	}
	else
	{
		camera->at("position") = camera->at("freeposition");
		camera->at("rotation") = camera->at("freerotation");
	}

	if (selectedSequence != "")
	{
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
	}
}

static ImVec2 modelPosAdj(0.0f, 21.0f);
static ImVec2 sequencerSizeAdj(0.0f, -47.0f);
static ImVec2 sequencerPosAdj(0.0f, 4.0f);
static float titleBarH = 19.0f;
void AnimationSequencerModal::DrawSequencer(const char* title, ImVec2 pos, ImVec2 size)
{
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
			if (selectedSequence != "" && animationsSequences.sequences.contains(selectedSequence))
			{
				animationsSequences.sequences.insert_or_assign(selectedSequence, sequencePlayer.sequence);
			}
			selectedSequence = sequence;
			if (sequence != "")
			{
				Sequence& seq = animationsSequences.sequences.at(sequence);
				sequencePlayer.SetSequence(seq, renderable());
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
			sequencePlayer.SetSequence(Sequence(), "");
			renderable->SetCurrentAnimation(nullptr);
			selectedTransformationKeyframe = nullptr;
			keyFrameFrame = -1;
			nextSelectedTransformationKeyframe = nullptr;
			nextSelectedKeyFrameFrame = -1;
		};
	auto onAddNewSequenceClicked = [this](std::string seqName)
		{
			addNewSequence = false;
			animationsSequences.sequences.insert_or_assign(seqName, Sequence());
			selectedSequence = seqName;
			playingSequence = false;
			playingSequenceTime = 0.0f;
			Sequence& seq = animationsSequences.sequences.at(seqName);
			sequencePlayer.SetSequence(seq, renderable());
			timelineEditor.Init(renderable, sequencePlayer.sequence);
			renderable->SetCurrentAnimation(&sequencePlayer);
		};
	auto onCancelAddNewSequenceClick = [this]()
		{
			addNewSequence = false;
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
			ImVec2 kpos(pos.x, pos.y + titleBarH + 50);
			ImVec2 ksize(300.0f, 200.0f);
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

	if (ImGui::BeginPopupModal(title, nullptr, defaultChildFlag))
	{
		DrawTitleBar(title, titlePos, titleSize, exit);
		if (selectedScriptToEdit == nullptr)
		{
			DrawSequenceSelector(seqSelPos, onSelectSequence, onEraseSequence, onAddNewSequence);
			DrawModelPreview(modelPos, modelSize);
			if (!selectedSequence.empty())
			{
				DrawTimelineController(timeControllerPos, timeControllerSize, sequencePlayer.sequence);
			}
			if (!selectedSequence.empty())
			{
				timelineEditor.Draw(sequencePlayer.sequence, sequencerPos, sequencerSize,
					[&](TransformationKeyFrame* tkeyframe, int frame)
					{
						selectedTransformationKeyframe = nullptr;
						keyFrameFrame = -1;
						nextSelectedTransformationKeyframe = tkeyframe;
						nextSelectedKeyFrameFrame = frame;
					},
					[&]()
					{
						selectedTransformationKeyframe = nullptr;
						keyFrameFrame = -1;
					},
					[&](int channel, int frame, SequenceChannelElementScript* scriptToEdit)
					{
						selectedScriptChannelFrame = std::make_tuple(channel, frame);
						selectedScriptToEdit = scriptToEdit;
						selectedScriptToEditContent = scriptToEdit->script;
					}
				);
			}
			if (selectedTransformationKeyframe != nullptr)
			{
				DrawTransformationKeyFrameAttributes(*selectedTransformationKeyframe, keyFrameFrame, keyframePos, keyframeSize);
			}
			DrawSaveAndExitButtons(saveExitBtnPos, saveExitBtnSize, exit, saveexit);
			if (addNewSequence)
			{
				DrawAddNewSequencePopup(newSeqPos, newSeqSize, newSequenceName, onAddNewSequenceClicked, onCancelAddNewSequenceClick);
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
				[&]()
				{
					selectedScriptToEdit->script = selectedScriptToEditContent;
					selectedScriptToEdit = nullptr;
				},
				[&]()
				{
					selectedScriptToEdit = nullptr;
				}
			);
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);

	if (nextSelectedTransformationKeyframe != nullptr)
	{
		selectedTransformationKeyframe = nextSelectedTransformationKeyframe;
		keyFrameFrame = nextSelectedKeyFrameFrame;
		nextSelectedTransformationKeyframe = nullptr;
		nextSelectedKeyFrameFrame = -1;
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
	std::function<void()> onAddSequence
)
{
	std::string title = "Sequence";

	ImVec2 textSize = ImGui::CalcTextSize(title.c_str());
	ImVec2 selectorSize(200, 50);
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
		ImGui::SetNextItemWidth(selectorSize.x);
		ImGui::DrawComboSelection(selectedSequence, sequences, onSelectSequence);
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

	ImGui::SetNextWindowSize(size, 0);
	ImGui::SetNextWindowPos(curPos, 0);
	ImGui::BeginChild("model-preview", size, 0);
	{
		auto& pass = camera->renderPassesUUID.at(0);
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
		ImGui::Text("camera");
		ImGui::Checkbox("Adjust camera", &adjustToBoundingBox);
		if (adjustToBoundingBox)
		{
			std::vector<JObject*> camV({ GetSceneObjectPointer(camera()) });
			DrawValue<XMFLOAT3, jedv_t_float3_angle>()("rotation", camV);
			DrawValue<float, jedv_t_float>()("modelDistanceScale", camV);
		}
		else
		{
			std::vector<JObject*> camV({ GetSceneObjectPointer(camera()) });
			DrawValue<XMFLOAT3, jedv_t_float3>()("freeposition", camV);
			DrawValue<XMFLOAT3, jedv_t_float3_angle>()("freerotation", camV);
		}

		ImGui::Text("floor");
		std::vector<JObject*> floorV({ GetSceneObjectPointer(floor()) });
		DrawValue<XMFLOAT3, jedv_t_color_float3>()("floorColor", floorV);
	}
	ImGui::EndChild();

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
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
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::InputFloat("x", &keyframe.position.x, 0.0f, 0.0f, "%.3f");
			ImGui::TableSetColumnIndex(1);
			ImGui::InputFloat("y", &keyframe.position.y, 0.0f, 0.0f, "%.3f");
			ImGui::TableSetColumnIndex(2);
			ImGui::InputFloat("z", &keyframe.position.z, 0.0f, 0.0f, "%.3f");
			ImGui::EndTable();
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
			}
			ImGui::TableSetColumnIndex(1);
			float yaw = XMConvertToRadians(keyframe.rotation.y);
			if (ImGui::SliderAngle("yaw", &yaw, -180.0f, 180.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
			{
				keyframe.rotation.y = XMConvertToDegrees(yaw);
			}
			ImGui::TableSetColumnIndex(2);
			float roll = XMConvertToRadians(keyframe.rotation.z);
			if (ImGui::SliderAngle("roll", &roll, -180.0f, 180.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
			{
				keyframe.rotation.z = XMConvertToDegrees(roll);
			}
			ImGui::EndTable();
		}
		ImGui::PopID();

		ImGui::Text("scale");
		ImGui::PushID("keyframe-scale");
		if (ImGui::BeginTable("keyframe-scale-table", 3, defaultTableFlags))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::InputFloat("x", &keyframe.scale.x, 0.0f, 0.0f, "%.3f");
			ImGui::TableSetColumnIndex(1);
			ImGui::InputFloat("y", &keyframe.scale.y, 0.0f, 0.0f, "%.3f");
			ImGui::TableSetColumnIndex(2);
			ImGui::InputFloat("z", &keyframe.scale.z, 0.0f, 0.0f, "%.3f");
			ImGui::EndTable();
		}
		ImGui::PopID();

		ImGui::Text("ease");
		ImGui::PushID("keyframe-easing");
		std::string selectedEase = EasingToString.at(keyframe.easing);
		ImGui::DrawComboSelection(selectedEase, nostd::GetKeysFromMap(StringToEasing), [&keyframe](std::string newEase)
			{
				keyframe.easing = StringToEasing.at(newEase);
			}
		);
		ImGui::PopID();
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

void AnimationSequencerModal::DrawAddNewSequencePopup(ImVec2 curPos, ImVec2 size, std::string& newSeqName, std::function<void(std::string)> onAddNewSequenceClicked, std::function<void()> onCancelAddNewSequenceClick)
{
	std::set<std::string> existingSequences = { "" };
	for (auto& [seqName, _] : animationsSequences.sequences)
	{
		existingSequences.insert(seqName);
	}

	const char* title = "Add new sequence";
	ImGui::OpenPopup(title);

	ImGui::SetNextWindowPos(curPos);
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

		ImGui::DrawItemWithEnabledState([this, newSeqName, onAddNewSequenceClicked]
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
}

void AnimationSequencerModal::SaveAndExit()
{
	if (selectedSequence != "" && animationsSequences.sequences.contains(selectedSequence))
	{
		animationsSequences.sequences.insert_or_assign(selectedSequence, sequencePlayer.sequence);
	}

	renderable->SetCurrentAnimation(nullptr);
	destroying = true;
	showing = false;
	model3D->animationSequences(animationsSequences);
	model3D->flag(Model3DJson::Update_animationSequences);
	Editor::templatesModified = true;
}



