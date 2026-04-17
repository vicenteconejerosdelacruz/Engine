#include "pch.h"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include <IconsFontAwesome5.h>
#include <ImGuizmo.h>
#include <Editor.h>
#include <Renderer.h>
#include <DeviceUtils/CommandsProcessor/CommandsProcessor.h>
#include <DeviceUtils/RenderPass/SwapChainPass.h>
#include <Game.h>
#include <Scene.h>
#include <Level.h>
#include <DeviceUtils/Resources/Resources.h>
#include <Templates.h>
#include <MousePicking.h>
#include <EditorMouseCamera.h>
#include <RightPanelComponent.h>
//Modals & Popus
#include <CreatorModal.h>
#include <DeletePrompt.h>
#include <AnimationSequencerModal.h>
#include <YesNoCancelModal.h>
#include <SceneObjectPopup.h>
#include <Modals/ScriptEditModal.h>
#include <Modals/ScriptBindingModal.h>

extern HWND hWnd;
extern RECT hWndRect;
extern std::unique_ptr<DirectX::Mouse> mouse;
extern std::unique_ptr<DirectX::Keyboard> keyboard;
extern std::unique_ptr<JRenderer> renderer;
extern std::string gameAppTitle;
extern bool inSizeMove;
extern RECT GetMaximizedAreaSize();

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

ImGuiWindowFlags panFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

static std::mutex levelLoadMutex;
static const float sequencerAdjustment = 8.0f;

using namespace DeviceUtils;

struct LoadingProgress
{
	LoadingProgress() { Reset(); }
	void Reset() {
		loadSceneUnitModal = false;
		defaultLevel = false;
		loading = false;
		levelName = "";
		asset = "";
		count = 0U;
		total = 0U;
	}
	void LoadLevel(bool defLevel, std::string name)
	{
		asset = "";
		count = 0U;
		total = 0U;
		defaultLevel = defLevel;
		levelName = name;
		loading = true;
		loadSceneUnitModal = true;
	}
	bool loadSceneUnitModal = true;
	bool defaultLevel = false;
	bool loading = false;
	std::string levelName;
	std::string asset;
	unsigned int count = 0U;
	unsigned int total = 0U;
};

struct BillboardRegistry
{
	std::map<JUUID, RenderableID> billboardRegistry; //scene object -> renderable billboard
	std::set<RenderableID> billboardsToDestroy;
};

struct GizmoInteraction
{
	GizmoInteraction()
	{
		gizmoOperation = ImGuizmo::TRANSLATE;
		gizmoMode = ImGuizmo::WORLD;
		gizmoCentroidMx = XMFLOAT4X4();
		gizmoApplyOp = false;
		soRotation = std::map<SceneObject*, XMFLOAT3>();
		soScale = std::map<SceneObject*, XMFLOAT3>();
		so2bb = std::map<SceneObject*, XMFLOAT3>();
		bb2gizmo = std::map<SceneObject*, XMFLOAT3>();
		gizmoRotation = XMFLOAT3();
		gizmoPosition = XMFLOAT3();
		gizmoScale = XMFLOAT3();
	}

	ImGuizmo::OPERATION gizmoOperation;//(ImGuizmo::TRANSLATE);
	ImGuizmo::MODE gizmoMode;// (ImGuizmo::WORLD);
	XMFLOAT4X4 gizmoCentroidMx;
	bool gizmoApplyOp;// = false;
	std::map<SceneObject*, XMFLOAT3> soRotation;
	std::map<SceneObject*, XMFLOAT3> soScale;
	std::map<SceneObject*, XMFLOAT3> so2bb;
	std::map<SceneObject*, XMFLOAT3> bb2gizmo;
	XMFLOAT3 gizmoRotation;
	XMFLOAT3 gizmoPosition;
	XMFLOAT3 gizmoScale;
};

struct PhysicsDrawState
{
	PhysicsDrawState()
	{
		draw = true;
		drawPlayState = true;
	}

	void PlayMode()
	{
		drawPlayState = draw;
		draw = false;
	}

	void EditorMode()
	{
		draw = drawPlayState;
	}

	void SwitchDraw()
	{
		bool value = !draw;
		draw = value;
		for (auto phO : levelPhysicObjects)
		{
			phO->visible(value);
		}
	}

	bool draw;
	bool drawPlayState;
	std::set<PhysicObjectID> levelPhysicObjects;
};

namespace Editor
{
	ImGui_ImplDX12_InitInfo init_info = {};
	std::unique_ptr<CommandsProcessor> commandListProcessor;
	std::unique_ptr<CommandsProcessor> commandListPickingPassProcessor;

	bool resizeRequired = false;

	//workbench & loading
	nlohmann::json workbench;
	std::string workbenchSelectedLevel;
	LoadingProgress loadingProgress;

	bool templatesModified = false;

	bool initialized = false;
	bool maximized = true;
	bool mouseClicked = false;
	bool clickedInDragArea = false;
	bool menuBarItemClicked = false;
	int lastMouseX;
	int lastMouseY;

	float titleBH = static_cast<float>(ApplicationBarBottom);
	float panW = static_cast<float>(RightPanelWidth);
	bool NonGameMode = false;
	bool lockedGameAreaInput = false;

	MouseGameAreaMode currentMouseMode = MOUSE_GAMEAREA_MODE_NONE;
	MousePicking mousePicking;
	EditorMouseCamera mouseCamera;

	//SceneUnits/levels
	SceneUnitId currentSceneUnitId;
	std::vector<std::tuple<SceneUnitId, std::string>> currentLevelName;
	std::unordered_map<SceneUnitId, bool> levelModified;
	std::unordered_map<SceneUnitId, bool> defaultLevel;
	//SceneObjects panel
	std::unordered_map<SceneUnitId, RightPanelComponent> sceneObjectEdition;
	//Gizmos
	std::unordered_map<SceneUnitId, GizmoInteraction> gizmos;
	//SceneObject selection
	std::unordered_map<SceneUnitId, std::set<JUUID>> selectedSceneObjects;
	//Game Interaction
	std::unordered_map<SceneUnitId, bool> isPlaying;
	std::unordered_map<SceneUnitId, bool> isPaused;
	std::unordered_map<SceneUnitId, std::string> editorPrePlayDump;
	//BoundingBox
	std::unordered_map<SceneUnitId, RenderableID> boundingBox;
	//Editor Camera
	std::unordered_map<SceneUnitId, CameraID> levelCameraUUID;
	std::unordered_map<SceneUnitId, CameraID> editorCameraUUID;
	//Billboards
	std::unordered_map<SceneUnitId, BillboardRegistry> billboards;
	//Physics Objects Draw
	std::unordered_map<SceneUnitId, PhysicsDrawState> drawStaticBodies;
	std::unordered_map<SceneUnitId, PhysicsDrawState> drawDynamicBodies;
	std::unordered_map<SceneUnitId, PhysicsDrawState> drawCharacters;
	std::unordered_map<SceneUnitId, PhysicsDrawState> drawTriggers;

	RightPanelComponent templateEdition("templates", { "hidden", "uuid" }, { "Templates", "Details" }, { "Templates" });

	//Modals
	CreatorModal<SceneObjectType> sceneObjectModal;
	CreatorModal<TemplateType> templateModal;
	DeletePrompt deletePrompt;
	AnimationSequencerModal animationSequencer;
	YesNoCancelModal yesNoCancelModal;
	SceneObjectPopup sceneObjectPopup;
	ScriptEditModal scriptEditModal;
	ScripBindingModal scriptBindingModal;

	void CreateSceneUnitGizmos(SceneUnitId id)
	{
		gizmos.insert_or_assign(id, GizmoInteraction());
	}

	void CreateSceneUnitSelection(SceneUnitId id)
	{
		selectedSceneObjects.insert_or_assign(id, std::set<JUUID>());
	}

	void CreateSceneUnitGameController(SceneUnitId id)
	{
		isPlaying.insert_or_assign(id, false);
		isPaused.insert_or_assign(id, false);
		editorPrePlayDump.insert_or_assign(id, "");
	}

	void CreateSceneUnitPhysicsController(SceneUnitId id)
	{
		if (!drawStaticBodies.contains(id)) drawStaticBodies.insert_or_assign(id, PhysicsDrawState());
		if (!drawDynamicBodies.contains(id)) drawDynamicBodies.insert_or_assign(id, PhysicsDrawState());
		if (!drawCharacters.contains(id)) drawCharacters.insert_or_assign(id, PhysicsDrawState());
		if (!drawTriggers.contains(id)) drawTriggers.insert_or_assign(id, PhysicsDrawState());
	}

	void CreateSceneUnitBoundingBox(SceneUnitId id)
	{
#if !defined(_EDITOR_BOUNDINGBOX)
		return;
#endif
		if (boundingBox.contains(id)) return;

		JUUID uuid = getUUID();
		boundingBox[id] = MAKESUUUID(id, uuid);
		JUUID camera = *GetSwapChainCameras(id).begin();
		nlohmann::json jbox = nlohmann::json(
			{
				{
					"meshMaterial",
					{
						{ "material", GetMaterialUUIDByName("BoundingBox") },
						{ "mesh",
							{
								{ "primitive", GetMeshUUIDByName("boxlines") }
							}
						}
					}
				},
				{ "castShadows", false },
				{ "shadowed", false },
				{ "name" , "EditorBoundingBox" },
				{ "uuid" , uuid },
				{ "position" , { 0.0f, 0.0f, 0.0f} },
				{ "topology", "LINELIST"},
				{ "rotation" , { 0.0, 0.0, 0.0 } },
				{ "scale" , { 1.0f, 1.0f, 1.0f } },
				{ "skipMeshes" , {}},
				{ "visible" , false},
				{ "hidden" , true},
				{ "cameras", { camera }},
				{ "checkBoundingBox", false },
			}
		);
		CreateRenderable(id, jbox);
		boundingBox[id]->BindToScene();
		GetLoadingProcessor(id).LoadingPoolInsert(SO_Renderables, boundingBox[id]());
	}

	void CreateSceneUnitBillboards(SceneUnitId id)
	{
		if (billboards.contains(id)) return;
		billboards.insert_or_assign(id, BillboardRegistry());
	}

	void CreateSceneUnitEditorIndependentCamera(SceneUnitId id)
	{
		auto& scene = GetSceneUnit(id);

		if (GetCountFromMouseCameras(id) > 0ULL)
		{
			if (editorCameraUUID.contains(id)) return;

			//no more than a single swapchain camera or mouse controller is allowed
			//todo handle RTT cameras that does resolving
			levelCameraUUID[id] = MAKESUUUID(id, *GetMouseCameras(id).begin());
			editorCameraUUID[id] = MAKESUUUID(id, getUUID());

			//this should be done and reversed later in the same function.
			//the purpose is to allow to create the editor camera
			//switching will be performed by switching functions later so we undo this in a few lines below
			EraseCameraFromMouseCameras(id, levelCameraUUID[id].uuid());
			EraseCameraFromSwapChainCameras(id, levelCameraUUID[id].uuid());

			//make a patch for the uuid and clone the camera
			nlohmann::json parameters = {
				{ "uuid", editorCameraUUID[id].uuid()},
				{ "name", "editorCamera" },
				{ "hidden", true },
				{ "systemCreated", true },
				{ "mouseController", true },
				{ "renderPasses", {} },
				{ "controllers", {} }
			};
			CloneSceneObject(id, levelCameraUUID[id].uuid(), parameters);

			//step out a little bit of the scene, we can came up with a better number eventually
			editorCameraUUID[id]->MoveForward(cameraEditorDistance);
			editorCameraUUID[id]->WriteConstantsBuffer(scene->Frame());

			//restore cameras mapping
			EraseCameraFromMouseCameras(id, editorCameraUUID[id].uuid());
			EraseCameraFromSwapChainCameras(id, editorCameraUUID[id].uuid());
			InsertCameraIntoMouseCameras(id, levelCameraUUID[id].uuid());
			InsertCameraIntoSwapChainCameras(id, levelCameraUUID[id].uuid());
		}
		else
		{

		}
	}

	CameraID GetLevelCamera(SceneUnitId id)
	{
		return levelCameraUUID.at(id);
	}

	void DeleteSceneUnitLevel(SceneUnitId id)
	{
		for (auto it = currentLevelName.begin(); it != currentLevelName.end();)
		{
			if (std::get<0>(*it) == id)
			{
				currentLevelName.erase(it);
				break;
			}
			it++;
		}
		levelModified.erase(id);
		defaultLevel.erase(id);
		sceneObjectEdition.erase(id);
	}

	void DeleteSceneUnitGizmos(SceneUnitId id)
	{
		gizmos.erase(id);
	}

	void DeleteSceneUnitSelection(SceneUnitId id)
	{
		selectedSceneObjects.erase(id);
	}

	void DeleteSceneUnitGameController(SceneUnitId id)
	{
		isPlaying.erase(id);
		isPaused.erase(id);
		editorPrePlayDump.erase(id);
	}

	void DeleteSceneUnitPhysicsController(SceneUnitId id)
	{
		drawStaticBodies.erase(id);
		drawDynamicBodies.erase(id);
		drawCharacters.erase(id);
		drawTriggers.erase(id);
	}

	void DeleteSceneUnitBoundingBox(SceneUnitId id)
	{
		boundingBox.erase(id);
	}

	void DeleteSceneUnitBillboards(SceneUnitId id)
	{
		billboards.erase(id);
	}

	void DeleteSceneUnitEditorIndependentCamera(SceneUnitId id)
	{
		levelCameraUUID.erase(id);
		editorCameraUUID.erase(id);
	}

	void CopySceneUnitEditorCameraRenderPasses(SceneUnitId id)
	{
		editorCameraUUID[id]->renderPasses(levelCameraUUID[id]->renderPasses());
		editorCameraUUID[id]->renderPassesUUID = levelCameraUUID[id]->renderPassesUUID;
	}

	void MarkSceneUnitAsModified(SceneUnitId id)
	{
		if (levelModified.contains(id))
			levelModified.at(id) = true;
	}

	//Editor LifeCycle
	void InitEditor()
	{
		initialized = true;
		loadingProgress.loadSceneUnitModal = true;
		LoadWorkbench();

		commandListProcessor = std::make_unique<CommandsProcessor>(renderer->d3dDevice, JRenderer::numFrames, 0xed1704);
		commandListPickingPassProcessor = std::make_unique<CommandsProcessor>(renderer->d3dDevice, JRenderer::numFrames, 0x91c39455);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->AddFontDefault();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		float baseFontSize = 13.0f;
		float iconFontSize = baseFontSize * 2.0f / 3.0f;

		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
		ImFontConfig icons_config;
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		icons_config.GlyphMinAdvanceX = iconFontSize;
		std::filesystem::path fontPath("Fonts");
		fontPath /= FONT_ICON_FILE_NAME_FAS;
		io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), iconFontSize, &icons_config, icons_ranges);

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(hWnd);

		init_info.Device = renderer->d3dDevice;
		init_info.CommandQueue = renderer->commandQueue;
		init_info.NumFramesInFlight = renderer->numFrames;
		init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;

		// Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
		// (current version of the backend will only allocate one descriptor, future versions will need to allocate more)
		init_info.SrvDescriptorHeap = DeviceUtils::GetCSUDescriptorHeap();
		init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle)
			{
				::CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_xhandle;
				::CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_xhandle;
				DeviceUtils::AllocCSUDescriptor(cpu_xhandle, gpu_xhandle);
				out_cpu_handle->ptr = cpu_xhandle.ptr;
				out_gpu_handle->ptr = gpu_xhandle.ptr;
			};
		init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
			{
				CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_xhandle(cpu_handle);
				CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_xhandle(gpu_handle);
				DeviceUtils::FreeCSUDescriptor(cpu_xhandle, gpu_xhandle);
				cpu_handle.ptr = 0;
				gpu_handle.ptr = 0;
			};
		ImGuiImplRenderInit();
		SetupImGuiStyle();
	}

	void LoadWorkbench()
	{
		if (!std::filesystem::exists("workbench.json"))
		{
			workbench = { { "levels", {} } };
			return;
		}

		std::ifstream file("workbench.json");
		bool isOpen = file.is_open();
		workbench = nlohmann::json::parse(file);
	}

	void SaveWorkbench(std::string topItem)
	{
		nlohmann::json newWorkbench = { { "levels", {} } };
		if (!topItem.empty())
		{
			newWorkbench["levels"].push_back(topItem);
		}
		for (int i = 0; i < workbench["levels"].size(); i++)
		{
			if (workbench["levels"][i] == topItem) continue;
			newWorkbench["levels"].push_back(workbench["levels"][i]);
		}
		workbench = newWorkbench;

		//then create the json level file
		std::string workbenchDump = workbench.dump(2);
		std::ofstream file("workbench.json");
		file.write(workbenchDump.c_str(), workbenchDump.size());
		file.close();
	}

	void LevelLoadingProgress(std::string asset, unsigned int count, unsigned int total)
	{
		loadingProgress.asset = asset;
		loadingProgress.count = count;
		loadingProgress.total = total;
	}

	void OnLevelLoaded(SceneUnitId id)
	{
		using namespace Scene;
		std::lock_guard<std::mutex> lock(levelLoadMutex);
		currentSceneUnitId = id;
		currentLevelName.push_back(std::make_tuple(id, loadingProgress.levelName));
		levelModified.insert_or_assign(id, false);
		defaultLevel.insert_or_assign(id, loadingProgress.defaultLevel);
		AddSceneUnitToEditor(id);
		MarkScenePanelAssetsAsDirty();
		BindPickingRenderables(id);
		ResetRenderableScenes();
		EnableSceneUnitRendering(id);
		CreateSceneUnitGizmos(id);
		CreateSceneUnitSelection(id);
		CreateSceneUnitGameController(id);
		loadingProgress.Reset();
	}

	void CloseScene(SceneUnitId id, std::function<void()> onCloseScene)
	{
		if (!levelModified.contains(id)) return;

		if (levelModified.at(id) && !defaultLevel.at(id))
		{
			yesNoCancelModal.Init(
				"Save the scene?", "Do you wish to save " + GetLevelName(id) + " before closing the scene?",
				[=] {
					SaveLevelToFile(id, GetLevelName(id));
					yesNoCancelModal.Hide();
					CloseScene(id, onCloseScene);
				},
				[=] {
					levelModified.at(id) = false;
					CloseScene(id, onCloseScene);
					yesNoCancelModal.Hide();
				},
				[] {
					yesNoCancelModal.Hide();
				}
			);
			yesNoCancelModal.Show();
		}
		else
		{
			auto& scene = GetSceneUnit(id);
			SceneUnitId nextSceneUnitId = currentSceneUnitId;
			if (GetSceneUnitsCount() > 0ULL && currentSceneUnitId == id)
			{
				nextSceneUnitId = GetNextSceneUnitId(currentSceneUnitId);
			}
			scene->MarkForDelete([=]
				{
					if (GetSceneUnitsCount() == 1ULL)
					{
						loadingProgress.loadSceneUnitModal = true;
						currentSceneUnitId = 0ULL;
					}
					else
					{
						currentSceneUnitId = nextSceneUnitId;
					}
					onCloseScene();
				}
			);
		}
	}

	void QuitEditor()
	{
		if (currentLevelName.size() > 0)
		{
			CloseScene(std::get<0>(currentLevelName[0]), QuitEditor);
		}
		else if (Editor::templatesModified)
		{
			loadingProgress.loadSceneUnitModal = false;
			yesNoCancelModal.Init("Save Templates?", "Templates has been modified, do you wish to save these modifications?",
				[] {
					SaveTemplates();
					yesNoCancelModal.Hide();
					QuitEditor();
				},
				[] {
					Editor::templatesModified = false;
					yesNoCancelModal.Hide();
					QuitEditor();
				},
				[] {
					yesNoCancelModal.Hide();
				}
			);
			yesNoCancelModal.Show();
		}
		else
		{
			PostMessageA(hWnd, WM_QUIT, 0, 0);
		}
	}

	void ImGuiImplRenderInit()
	{
		if (initialized) ImGui_ImplDX12_Init(&init_info);
	}

	void SetupImGuiStyle()
	{
		// Green Font style by aiekick from ImThemes
		ImGuiStyle& style = ImGui::GetStyle();

		style.Alpha = 1.0f;
		style.DisabledAlpha = 0.6000000238418579f;
		style.WindowPadding = ImVec2(8.0f, 8.0f);
		style.WindowRounding = 0.0f;
		style.WindowBorderSize = 1.0f;
		style.WindowMinSize = ImVec2(32.0f, 32.0f);
		style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
		style.WindowMenuButtonPosition = ImGuiDir_Left;
		style.ChildRounding = 0.0f;
		style.ChildBorderSize = 1.0f;
		style.PopupRounding = 0.0f;
		style.PopupBorderSize = 1.0f;
		style.FramePadding = ImVec2(4.0f, 3.0f);
		style.FrameRounding = 0.0f;
		style.FrameBorderSize = 0.0f;
		style.ItemSpacing = ImVec2(8.0f, 4.0f);
		style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
		style.CellPadding = ImVec2(4.0f, 2.0f);
		style.IndentSpacing = 21.0f;
		style.ColumnsMinSpacing = 6.0f;
		style.ScrollbarSize = 14.0f;
		style.ScrollbarRounding = 9.0f;
		style.GrabMinSize = 10.0f;
		style.GrabRounding = 0.0f;
		style.TabRounding = 4.0f;
		style.TabBorderSize = 0.0f;
		style.TabMinWidthForCloseButton = 0.0f;
		style.ColorButtonPosition = ImGuiDir_Right;
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

		style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.4980392158031464f, 0.4980392158031464f, 0.4980392158031464f, 1.0f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.05882352963089943f, 0.05882352963089943f, 0.05882352963089943f, 0.9399999976158142f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0784313753247261f, 0.0784313753247261f, 0.0784313753247261f, 0.9399999976158142f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.4392156898975372f, 0.4392156898975372f, 0.4392156898975372f, 0.6000000238418579f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.5686274766921997f, 0.5686274766921997f, 0.5686274766921997f, 0.699999988079071f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.7568627595901489f, 0.7568627595901489f, 0.7568627595901489f, 0.800000011920929f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.03921568766236305f, 0.03921568766236305f, 0.03921568766236305f, 1.0f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 1.0f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.6000000238418579f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.01960784383118153f, 0.01960784383118153f, 0.01960784383118153f, 0.5299999713897705f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.407843142747879f, 0.407843142747879f, 0.407843142747879f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5098039507865906f, 0.5098039507865906f, 0.5098039507865906f, 1.0f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.5490196347236633f, 0.800000011920929f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.7490196228027344f, 0.800000011920929f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 1.0f, 0.800000011920929f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.5490196347236633f, 0.4000000059604645f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.7490196228027344f, 0.6000000238418579f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 1.0f, 0.800000011920929f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.5490196347236633f, 0.4000000059604645f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.7490196228027344f, 0.6000000238418579f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 1.0f, 0.800000011920929f);
		style.Colors[ImGuiCol_Separator] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.5490196347236633f, 0.4000000059604645f);
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.7490196228027344f, 0.6000000238418579f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 1.0f, 0.800000011920929f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.5490196347236633f, 0.4000000059604645f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.7490196228027344f, 0.6000000238418579f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 1.0f, 0.800000011920929f);
		style.Colors[ImGuiCol_Tab] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.5490196347236633f, 0.800000011920929f);
		style.Colors[ImGuiCol_TabHovered] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.7490196228027344f, 0.800000011920929f);
		style.Colors[ImGuiCol_TabActive] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 1.0f, 0.800000011920929f);
		style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.1764705926179886f, 0.1764705926179886f, 0.1764705926179886f, 1.0f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.3568627536296844f, 0.3568627536296844f, 0.3568627536296844f, 0.5400000214576721f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6078431606292725f, 0.6078431606292725f, 0.6078431606292725f, 1.0f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.4274509847164154f, 0.3490196168422699f, 1.0f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392217636108f, 0.6980392336845398f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6000000238418579f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
		style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
		style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
		style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.07000000029802322f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.3499999940395355f);
		style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
		style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
		style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
		style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
	}

	void DestroyEditor()
	{
		using namespace Scene;

		initialized = false;
		if (sceneObjectEdition.contains(currentSceneUnitId))
		{
			for (auto& uuid : sceneObjectEdition.at(currentSceneUnitId).editables)
			{
				SendEditorDestroyPreview(uuid, [](JUUID uuid) { return GetSceneObjectPointer(currentSceneUnitId, uuid); });
			}
		}
		for (auto& uuid : templateEdition.editables)
		{
			SendEditorDestroyPreview(uuid, GetJTemplatePointer);
		}

		auto sceneIds = GetSceneUnitIds();
		for (auto& id : sceneIds)
		{
			ClearSceneObjectsSelection(id);
		}
		for (auto& [id, _] : sceneObjectEdition)
		{
			sceneObjectEdition.at(id).Destroy();
		}
		templateEdition.Destroy();

		// Cleanup
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	bool WndProcHandlerEditor(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	}

	void EditorStep()
	{
		BuildAssetsTree();
		DestroyPendingBillboards();
	}

	void HandleEditorMouseMovements(SceneUnitId id)
	{
		if (currentSceneUnitId != id) return;

		if (GetCountFromMouseCameras(id) > 0ULL)
		{
			GameAreaMouseProcessing(mouse, MAKESUUUID(id, *GetMouseCameras(id).begin()));
		}
	}

	std::set<std::string> GetOpenedScenes(bool skipDefault)
	{
		std::set<std::tuple<SceneUnitId, std::string>> openedScenes;
		std::copy_if(currentLevelName.begin(), currentLevelName.end(), std::inserter(openedScenes, openedScenes.begin()), [=](auto& tup)
			{
				return !defaultLevel.contains(std::get<0>(tup)) && skipDefault;
			}
		);
		std::set<std::string> sceneNames;
		std::transform(openedScenes.begin(), openedScenes.end(), std::inserter(sceneNames, sceneNames.begin()), [](auto& tup)
			{
				return std::get<1>(tup);
			}
		);
		return sceneNames;
	}

	std::set<SceneUnitId> GetOpenedSceneUnitIds(bool skipDefault)
	{
		std::set<std::tuple<SceneUnitId, std::string>> openedScenes;
		std::copy_if(currentLevelName.begin(), currentLevelName.end(), std::inserter(openedScenes, openedScenes.begin()), [=](auto& tup)
			{
				return !defaultLevel.contains(std::get<0>(tup)) && skipDefault;
			}
		);
		std::set<SceneUnitId> sceneUnitIds;
		std::transform(openedScenes.begin(), openedScenes.end(), std::inserter(sceneUnitIds, sceneUnitIds.begin()), [](auto& tup)
			{
				return std::get<0>(tup);
			}
		);
		return sceneUnitIds;
	}

	void ResizeEditorResources(unsigned int width, unsigned int height)
	{
		if (animationSequencer.showing)
		{
			ImVec2 WorkSize(static_cast<float>(width), static_cast<float>(height));
			ImVec2 seqPos = ImVec2(WorkSize.x / sequencerAdjustment, WorkSize.y / sequencerAdjustment);
			ImVec2 seqSize = ImVec2(WorkSize.x * (1.0f - (2.0f / sequencerAdjustment)), WorkSize.y * (1.0f - (2.0f / sequencerAdjustment)));
			animationSequencer.Resize(seqPos, seqSize);
		}
	}

	void AddSceneUnitToEditor(SceneUnitId unit)
	{
		auto& scene = GetSceneUnit(unit);
		sceneObjectEdition.insert_or_assign(unit,
			RightPanelComponent(
				"sceneObjects",
				{ "hidden", "uuid" },
				{ "Scene Objects", "Details" },
				{ "Scene Objects" }
			)
		);
	}

	void SetCurrentSceneUnit(SceneUnitId unit)
	{
		currentSceneUnitId = unit;
	}

	//Editor Drawing
	void DrawEditor()
	{
		using namespace Scene;

		if (inSizeMove) return;

		unsigned int backBufferIndex = renderer->GetBackBufferIndex();
		RenderPassInstanceID renderPass = renderer->swapChainPass;
		SwapChainPassID pass = renderPass->swapChainPass;
		auto backBuffer = pass->renderTargets[backBufferIndex];
		commandListProcessor->ResetCommandList();
		auto& commandList = commandListProcessor->GetCommandList();
		TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

#if defined(_DEVELOPMENT)
		PIXBeginEvent(commandList.p, 0, L"Draw Editor");
#endif
		commandList->RSSetViewports(1, &renderer->screenViewport);
		commandList->RSSetScissorRects(1, &renderer->scissorRect);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(pass->rtvDescriptorHeap->descriptorHeap->GetCPUDescriptorHandleForHeapStart(), backBufferIndex, pass->rtvDescriptorHeap->descriptorSize);
		commandList->OMSetRenderTargets(1, &rtv, false, nullptr);
		if (GetSceneUnitsCount() == 0ULL)
		{
			XMVECTORF32 clearColor = DirectX::Colors::Black;
			commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
		}

		// Start the Dear ImGui frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		Editor::NonGameMode = false;

		if (currentSceneUnitId != 0 && sceneObjectEdition.contains(currentSceneUnitId) && sceneObjectEdition.at(currentSceneUnitId).selectedNextFrame != "")
		{
			OpenSceneObject(sceneObjectEdition.at(currentSceneUnitId).selectedNextFrame);
			sceneObjectEdition.at(currentSceneUnitId).selectedNextFrame = "";
		}

		if (templateEdition.selectedNextFrame != "")
		{
			OpenTemplate(templateEdition.selectedNextFrame);
			templateEdition.selectedNextFrame = "";
		}

		DrawApplicationBar();
		DrawLevelSelectorModal();
		DrawGameController();
		DrawPhysicsController();
		DrawLevelsTabs();

		if (!!currentSceneUnitId && !IsPlaying(currentSceneUnitId))
		{
			DrawRightPanel();

			SwitchToSceneUnitEditorCamera(currentSceneUnitId);
			DrawPickedObjectsGizmo(currentSceneUnitId, MAKESUUUID(Editor::currentSceneUnitId, *GetSwapChainCameras(currentSceneUnitId).begin()));
			SwitchToSceneUnitEditorPlayCamera(currentSceneUnitId);

			if (sceneObjectModal.creating)
				sceneObjectModal.DrawCreationPopup(SceneObjectsTypePanelMenuItems.at(sceneObjectModal.type));
			if (templateModal.creating)
				templateModal.DrawCreationPopup(TemplateTypePanelMenuItems.at(templateModal.type));
			if (deletePrompt.showing)
				deletePrompt.DrawPrompt("Delete Template");
			if (animationSequencer.showing)
			{
				animationSequencer.DrawSequencer("Animation Sequencer");
			}
			if (animationSequencer.destroying)
			{
				animationSequencer.DestroyStep();
			}
			else if (animationSequencer.initializing)
			{
				animationSequencer.DrawLoading();
			}
			sceneObjectPopup.Draw();
			scriptEditModal.Draw();
			scriptBindingModal.Draw();
		}
		if (yesNoCancelModal.Showing())
		{
			yesNoCancelModal.Draw();
		}

		// Rendering
		ImGui::Render();

		// Render Dear ImGui graphics
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

#if defined(_DEVELOPMENT)
		PIXEndEvent(commandList.p);
#endif

		TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		commandListProcessor->CloseCommandList();
		commandListProcessor->ExecuteCommandList();
		commandListProcessor->Next();
	}

	void WriteSceneUnitEditorPlayCameraConstantsBuffer(SceneUnitId id)
	{
		if (!levelCameraUUID.contains(id)) return;
		auto& scene = GetSceneUnit(id);
		levelCameraUUID[id]->WriteConstantsBuffer(scene->Frame());
	}

	void WriteSceneUnitDirectionalShadowMapAttributes(SceneUnitId id)
	{
		auto& scene = GetSceneUnit(id);
		for (auto uuid : GetShadowMapLights(id))
		{
			LightID l = MAKESUUUID(id, uuid);
			if (l->lightType() != LT_Directional) continue;

			l->CreateDirectionalCascadeShadowMapViewProjectionMatrices();
		}
		CameraID c = MAKESUUUID(id, *GetSwapChainCameras(id).begin());
		c->WriteShadowMapsConstantsBuffer(scene->Frame());
	}

	JUUID GetSceneUnitEditorCamera(SceneUnitId id)
	{
		return editorCameraUUID[id].uuid();
	}

	void SwitchToSceneUnitEditorCamera(SceneUnitId id)
	{
		if (!levelCameraUUID.contains(id) || !editorCameraUUID.contains(id)) return;

		auto& scene = GetSceneUnit(id);
		editorCameraUUID[id]->renderables = levelCameraUUID[id]->renderables;
		editorCameraUUID[id]->iblTextures = levelCameraUUID[id]->iblTextures;
		editorCameraUUID[id]->lights = levelCameraUUID[id]->lights;
		editorCameraUUID[id]->lightsWithShadowMaps = levelCameraUUID[id]->lightsWithShadowMaps;
		editorCameraUUID[id]->CopyProjection(levelCameraUUID[id]);
		editorCameraUUID[id]->WriteLightsConstantsBuffer(scene->Frame());
		editorCameraUUID[id]->WriteShadowMapsConstantsBuffer(scene->Frame());
		editorCameraUUID[id]->RenderReady(true);
		CopySceneUnitEditorCameraRenderPasses(id);
		EraseCameraFromMouseCameras(id, levelCameraUUID[id].uuid());
		EraseCameraFromSwapChainCameras(id, levelCameraUUID[id].uuid());
		InsertCameraIntoMouseCameras(id, editorCameraUUID[id].uuid());
		InsertCameraIntoSwapChainCameras(id, editorCameraUUID[id].uuid());
	}

	void SwitchToSceneUnitEditorPlayCamera(SceneUnitId id)
	{
		if (!levelCameraUUID.contains(id) || !editorCameraUUID.contains(id)) return;
		EraseCameraFromMouseCameras(id, editorCameraUUID[id].uuid());
		EraseCameraFromSwapChainCameras(id, editorCameraUUID[id].uuid());
		InsertCameraIntoMouseCameras(id, levelCameraUUID[id].uuid());
		InsertCameraIntoSwapChainCameras(id, levelCameraUUID[id].uuid());
		editorCameraUUID[id]->RenderReady(false);
	}

	void RemoveSceneUnitEditorCameraFromWindowCameras(SceneUnitId id)
	{
		EraseCameraFromWindowCameras(id, editorCameraUUID[id].uuid());
	}

	void AddSceneUnitEditorCameraToWindowCameras(SceneUnitId id)
	{
		InsertCameraIntoWindowCameras(id, editorCameraUUID[id].uuid());
	}

	void DrawApplicationBar()
	{
		using namespace Scene::Level;

		RECT dragRect;
		ZeroMemory(&dragRect, sizeof(dragRect));
		dragRect.bottom = ApplicationBarBottom;

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::DrawItemWithEnabledState([]
					{
						if (ImGui::MenuItem(ICON_FA_FILE "New"))
						{
							loadingProgress.loadSceneUnitModal = true;
							loadingProgress.LoadLevel(true, "default");
							LoadLevelIntoSceneUnit("default", GetDefaultLevel, OnLevelLoaded, LevelLoadingProgress);
						}
					},
					!IsPlaying(currentSceneUnitId)
				);

				ImGui::Separator();
				ImGui::DrawItemWithEnabledState([]
					{
						if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN "Open"))
						{
							loadingProgress.loadSceneUnitModal = true;
						}
					},
					!IsPlaying(currentSceneUnitId)
				);

				ImGui::DrawItemWithEnabledState([]
					{
						if (ImGui::MenuItem(ICON_FA_SAVE "Save"))
						{
							SaveLevelToFile(currentSceneUnitId, GetLevelName(currentSceneUnitId));
							SaveWorkbench(GetLevelName(currentSceneUnitId));
						}
					},
					!IsPlaying(currentSceneUnitId) && levelModified.at(currentSceneUnitId) && !defaultLevel.at(currentSceneUnitId)
				);

				ImGui::DrawItemWithEnabledState([]
					{
						if (ImGui::MenuItem(ICON_FA_SAVE "Save As.."))
						{
							SaveLevelAs();
						}
					},
					!IsPlaying(currentSceneUnitId)
				);

				ImGui::Separator();

				if (currentSceneUnitId != 0ULL)
				{
					std::string closeStr = ICON_FA_BOMB "Close Scene";
					if (levelModified.at(currentSceneUnitId))
					{
						closeStr += "*";
					}
					if (ImGui::MenuItem(closeStr.c_str()))
					{
						CloseScene(currentSceneUnitId);
					}
					ImGui::Separator();
				}

				ImGui::DrawItemWithEnabledState([]
					{
						if (ImGui::MenuItem(ICON_FA_SAVE "Save Templates"))
						{
							SaveTemplates();
						}
					}, templatesModified
				);
				ImGui::Separator();
				if (ImGui::MenuItem(ICON_FA_TIMES "Exit")) // It would be nice if this was a "X" like in the windows title bar set off to the far right
				{
					QuitEditor();
				}
				ImGui::EndMenu();

			}

			auto cursorPos = ImGui::GetCursorScreenPos();
			dragRect.left = static_cast<LONG>(cursorPos.x);

			const ImGuiViewport* viewport = ImGui::GetMainViewport();

			std::string titleBar = gameAppTitle;
			if (currentSceneUnitId != 0)
			{
				std::lock_guard<std::mutex> lock(levelLoadMutex);
				auto [_, name] = *std::find_if(currentLevelName.begin(), currentLevelName.end(), [&](auto unitLevel)
					{
						return currentSceneUnitId == std::get<0>(unitLevel);
					}
				);
				titleBar += " - " + name;
			}

			auto windowWidth = ImGui::GetWindowSize().x;
			auto textWidth = ImGui::CalcTextSize(titleBar.c_str()).x;

			ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
			ImGui::Text(titleBar.c_str());

			struct WinButtonDef {
				std::string label;
				FLOAT x;
				FLOAT y = 0.0f;
				std::function<void()> onClick;
			};

			WinButtonDef windowsButtons[] = {
				{
					.label = ICON_FA_TIMES,
					.x = viewport->WorkSize.x - 1.0f * 19.0f,
					.onClick = QuitEditor
				},
				{
					.label = ICON_FA_WINDOW_MAXIMIZE,
					.x = viewport->WorkSize.x - 2.0f * 19.0f,
					.onClick = []()
				{
					if (maximized)
					{
						SetWindowPos(hWnd, HWND_TOP, 0, 0, 640, 480, 0);
					}
					else
					{
						RECT desktopRect = GetMaximizedAreaSize();
						SetWindowPos(hWnd, HWND_TOP, 0, 0, desktopRect.right - desktopRect.left, desktopRect.bottom - desktopRect.top, 0);
					}
					maximized = !maximized;
				}},
				{
					.label = ICON_FA_WINDOW_MINIMIZE,
					.x = viewport->WorkSize.x - 3.0f * 19.0f,
					.onClick = []()
				{
					//ShowWindow(hWnd, SW_MINIMIZE);
					SendMessageA(hWnd,WM_SYSCOMMAND, SC_MINIMIZE,0);
				}},
			};

			dragRect.right = static_cast<LONG>(windowsButtons[_countof(windowsButtons) - 1].x);

			for (auto button : windowsButtons) {
				ImGui::SetCursorPos(ImVec2(button.x, button.y));
				if (ImGui::Button(button.label.c_str(), ImVec2(19.0f, 19.0f))) { button.onClick(); }
			}

			ImGui::EndMainMenuBar();
		}
		HandleApplicationDragTitleBar(dragRect);
	}

	void DrawLevelSelectorModal()
	{
		using namespace Game;
		using namespace Scene::Level;

		if (!loadingProgress.loadSceneUnitModal) return;

		std::string title = "Open Level";

		ImGui::OpenPopup(title.c_str());

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 modalSize = ImVec2(300.0f, 200.0f);
		ImVec2 modalPos = ImVec2(viewport->WorkSize.x * 0.5f - modalSize.x * 0.5f, viewport->WorkSize.y * 0.5f - modalSize.y * 0.5f);
		ImGui::SetNextWindowPos(modalPos);
		ImGui::SetNextWindowSize(modalSize);

		if (ImGui::BeginPopupModal(title.c_str(), nullptr,
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
		))
		{
			ImVec2 listBoxSize = ImVec2(modalSize.x - 15, modalSize.y - (loadingProgress.loading ? 100 : 60));
			ImGui::BeginListBox("##", listBoxSize);
			{
				nlohmann::json& levels = workbench.at("levels");
				for (unsigned int i = 0; i < levels.size(); i++)
				{
					std::string option = levels.at(i);
					bool selected = option == workbenchSelectedLevel;
					if (ImGui::Selectable(option.c_str(), &selected))
					{
						workbenchSelectedLevel = option;
					}
					if (!loadingProgress.loading && ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(ImGuiPopupFlags_MouseButtonLeft))
					{
						loadingProgress.LoadLevel(false, workbenchSelectedLevel);

						LoadLevelIntoSceneUnit(workbenchSelectedLevel,
							[&]()
							{
								return GetLevelFromFile(workbenchSelectedLevel);
							},
							[=](SceneUnitId id)
							{
								SaveWorkbench(workbenchSelectedLevel);
								OnLevelLoaded(id);
							},
							LevelLoadingProgress
						);
					}
				}
				ImGui::EndListBox();
			}

			if (loadingProgress.loading)
			{
				ImGui::Text(loadingProgress.asset.c_str());
				float progress = static_cast<float>(loadingProgress.count) / static_cast<float>(loadingProgress.total);
				ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), "");
			}

			ImVec2 cursorPos = ImGui::GetCursorPos();
			if (ImGui::Button("Exit"))
			{
				QuitEditor();
			}

			cursorPos.x += 55.0f;
			ImGui::SetCursorPos(cursorPos);
			ImGui::DrawItemWithEnabledState([&]
				{
					if (ImGui::Button("Open"))
					{
						ImGui::OpenFile([&](std::filesystem::path p)
							{
								std::filesystem::path absfilepath = std::filesystem::current_path().append(defaultLevelsFolder);
								std::filesystem::path rel = std::filesystem::relative(p, absfilepath);
								workbenchSelectedLevel = rel.generic_string();
								loadingProgress.LoadLevel(false, workbenchSelectedLevel);
								SaveWorkbench(workbenchSelectedLevel);
								LoadLevelIntoSceneUnit(workbenchSelectedLevel, [&]() { return GetLevelFromFile(workbenchSelectedLevel); }, OnLevelLoaded, LevelLoadingProgress);
							}, defaultLevelsFolder);
					}
				}, !loadingProgress.loading);

			ImGui::SameLine();
			ImGui::DrawItemWithEnabledState([&]
				{
					if (ImGui::Button("Default Level"))
					{
						loadingProgress.LoadLevel(true, "default");
						LoadLevelIntoSceneUnit("default", GetDefaultLevel, OnLevelLoaded, LevelLoadingProgress);
					}
				},
				!loadingProgress.loading
			);

			ImGui::SameLine();
			ImGui::DrawItemWithEnabledState([&]
				{
					if (ImGui::Button("Load Level"))
					{
						loadingProgress.LoadLevel(false, workbenchSelectedLevel);
						SaveWorkbench(workbenchSelectedLevel);
						LoadLevelIntoSceneUnit(workbenchSelectedLevel, [&]() { return GetLevelFromFile(workbenchSelectedLevel); }, OnLevelLoaded, LevelLoadingProgress);
					}
				},
				!workbenchSelectedLevel.empty() && !loadingProgress.loading
			);

			ImGui::EndPopup();
		}
	}

	void HandleApplicationDragTitleBar(RECT& dragRect)
	{
		auto mouseState = mouse->GetState();

		auto inDragBounds = [&dragRect, &mouseState]() {
			return (
				mouseState.y < dragRect.bottom &&
				mouseState.y > dragRect.top &&
				mouseState.x < dragRect.right &&
				mouseState.x > dragRect.left
				);
			};

		if (mouseState.leftButton && !mouseClicked)
		{
			mouseClicked = true;
			if (inDragBounds())
			{
				clickedInDragArea = true;
				lastMouseX = mouseState.x;
				lastMouseY = mouseState.y;
			}
		}
		else if (mouseState.leftButton && clickedInDragArea)
		{
			INT diffMouseX = mouseState.x - lastMouseX;
			INT diffMouseY = mouseState.y - lastMouseY;

			RECT currentRect;
			GetWindowRect(hWnd, &currentRect);

			INT newX = currentRect.left + diffMouseX;
			INT newY = currentRect.top + diffMouseY;

			SetWindowPos(hWnd, nullptr, newX, newY, 0, 0, SWP_NOSIZE);

			lastMouseX = mouseState.x - diffMouseX;
			lastMouseY = mouseState.y - diffMouseY;
		}
		else if (!mouseState.leftButton)
		{
			mouseClicked = false;
			clickedInDragArea = false;
		}
	}

	static ImVec2 gameControllerSize(200, 18);
	RECT GetGameControllerRect()
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 panPos = ImVec2(viewport->WorkSize.x - panW, viewport->WorkPos.y);

		RECT r;
		r.left = static_cast<LONG>((panPos.x - gameControllerSize.x) * 0.5f);
		r.right = r.left + static_cast<LONG>(gameControllerSize.x);
		r.top = static_cast<LONG>(panPos.y);
		r.bottom = r.top + static_cast<LONG>(gameControllerSize.y);
		return r;
	}

	void DrawGameController()
	{
		if (!currentSceneUnitId) return;

		RECT r = GetGameControllerRect();
		ImVec2 controllerPos(static_cast<float>(r.left), static_cast<float>(r.top));
		ImVec2 controllerSize(static_cast<float>(r.right - r.left), static_cast<float>(r.bottom - r.top));

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, controllerSize);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, 0);
		ImGui::SetNextWindowPos(controllerPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(controllerSize, ImGuiCond_Always);
		ImGui::Begin(
			"gamecontroller",
			(bool*)1,
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings
		);
		{
			float play_width = ImGui::CalcTextSize(ICON_FA_PLAY).x + ImGui::GetStyle().FramePadding.x * 2.0f;
			float stop_width = ImGui::CalcTextSize(ICON_FA_STOP).x + ImGui::GetStyle().FramePadding.x * 2.0f;
			float buttons_width = play_width + stop_width;
			float cursorX = 0.5f * (controllerSize.x - buttons_width);
			ImGui::SetCursorPosX(cursorX);
			if (!IsPlaying(currentSceneUnitId) || IsPaused(currentSceneUnitId))
			{
				if (ImGui::Button(ICON_FA_PLAY))
				{
					if (!IsPlaying(currentSceneUnitId))
					{
						SwitchToPlayMode(currentSceneUnitId);
					}
					else
					{
						SwitchToUnPausedMode(currentSceneUnitId);
					}
				}
			}
			else if (IsPlaying(currentSceneUnitId) && !IsPaused(currentSceneUnitId))
			{
				if (ImGui::Button(ICON_FA_PAUSE))
				{
					SwitchToPauseMode(currentSceneUnitId);
				}
			}
			ImGui::SameLine();
			ImGui::DrawItemWithEnabledState([]
				{
					if (ImGui::Button(ICON_FA_STOP))
					{
						SwitchToNonPlayMode(currentSceneUnitId);
					}
				},
				IsPlaying(currentSceneUnitId)
			);
		}
		ImGui::End();
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(3);
	}

	static ImVec2 physicsControllerSize(120, 90);
	RECT GetPhysicsControllerRect()
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 panPos = ImVec2(viewport->WorkSize.x - (IsPlaying(currentSceneUnitId) ? 0 : panW), viewport->WorkPos.y);

		RECT r;
		r.right = static_cast<LONG>(panPos.x - 1);
		r.left = static_cast<LONG>(r.right - physicsControllerSize.x);
		r.top = static_cast<LONG>(panPos.y);
		r.bottom = r.top + static_cast<LONG>(physicsControllerSize.y);
		return r;
	}

	void DrawPhysicsController()
	{
		if (!currentSceneUnitId || GetCountFromPhysicScenes(currentSceneUnitId) == 0ULL) return;

		RECT r = GetPhysicsControllerRect();
		ImVec2 controllerPos(static_cast<float>(r.left), static_cast<float>(r.top));
		ImVec2 controllerSize(static_cast<float>(r.right - r.left), static_cast<float>(r.bottom - r.top));

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, controllerSize);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, 0);
		ImGui::SetNextWindowPos(controllerPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(controllerSize, ImGuiCond_Always);
		ImGui::Begin(
			"physicscontroller",
			(bool*)1,
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings
		);
		{
			PhysicSceneID scene = MAKESUUUID(currentSceneUnitId, *(GetPhysicScenes(currentSceneUnitId).begin()));
			auto drawToggle = [&](auto& drawHolder, std::string title, auto& toggle, std::string attribute, size_t updateFlag)
				{
					XMFLOAT4 color = ToXMFLOAT4(scene->at(attribute));
					ImGui::PushID(attribute.c_str());
					if (ImGui::ColorEdit4("##", &color.x, ImGuiColorEditFlags_NoInputs))
					{
						scene->at(attribute) = FromXMFLOAT4(color);
						scene->flag(updateFlag);
					}
					ImGui::PopID();
					ImGui::SameLine();
					bool value = drawHolder.at(currentSceneUnitId).draw;
					if (ImGui::Checkbox(title.c_str(), &value))
					{
						toggle(currentSceneUnitId);
					}
				};
			drawToggle(drawStaticBodies, "Static", SwitchStaticBodiesDrawing, "staticColor", PhysicScene::Update_staticColor);
			drawToggle(drawDynamicBodies, "Dynamic", SwitchDynamicBodiesDrawing, "dynamicColor", PhysicScene::Update_dynamicColor);
			drawToggle(drawCharacters, "Characters", SwitchCharactersDrawing, "characterColor", PhysicScene::Update_characterColor);
			drawToggle(drawTriggers, "Triggers", SwitchTriggersDrawing, "triggerColor", PhysicScene::Update_triggerColor);
		}
		ImGui::End();
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(3);
	}

	void DrawLevelsTabs()
	{
		if (currentLevelName.empty()) return;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();

		// 1. Configurar la posición y tamaño de la ventana contenedora
		float height = 25.0f; // Un poco más de los 22 que usabas para dar margen
		ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - height));
		ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, height));

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar;

		if (ImGui::Begin("tabsBarWindow", nullptr, window_flags))
		{
			ImGuiIO& io = ImGui::GetIO();
			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			// 2. La posición inicial relativa a la ventana actual
			ImVec2 selectorPos = ImGui::GetCursorScreenPos();

			for (auto& [unit, name] : currentLevelName)
			{
				std::string tabS = ((unit == currentSceneUnitId) ? "*" : "") + name;
				ImVec2 nameSize = ImGui::CalcTextSize(tabS.c_str());

				// Definir rectángulos de interacción
				ImRect selectorRect(selectorPos, ImVec2(selectorPos.x + nameSize.x + 40.0f, selectorPos.y + height));
				ImRect closeButtonRect(ImVec2(selectorRect.Max.x - 20.0f, selectorPos.y), selectorRect.Max);

				// Lógica de colores (simplificada para el ejemplo)
				ImU32 filledColor = rgba(52, 67, 96, 0.8);
				if (selectorRect.Contains(io.MousePos)) {
					filledColor = (unit == currentSceneUnitId) ? rgba(20, 86, 218, 0.8) : rgba(138, 107, 164, 0.8);
					if (ImGui::IsMouseClicked(0) && !closeButtonRect.Contains(io.MousePos)) {
						PauseSounds(currentSceneUnitId);
						SetCurrentSceneUnit(unit);
						ResetRenderableScenes();
						EnableSceneUnitRendering(unit);
					}
				}

				// Dibujar fondo
				draw_list->AddRectFilled(selectorRect.Min, selectorRect.Max, filledColor);

				// Dibujar Texto
				ImGui::SetCursorScreenPos(ImVec2(selectorPos.x + 5.0f, selectorPos.y + 2.0f));
				ImGui::TextUnformatted(tabS.c_str());

				// Botón de cierre (integrado en el flujo de ImGui para que funcione el clic)
				ImGui::SameLine();
				ImGui::SetCursorScreenPos(ImVec2(selectorRect.Max.x - 18.0f, selectorPos.y + 2.0f));
				ImGui::PushID(std::to_string(unit).c_str());
				if (ImGui::SmallButton("x")) {
					CloseScene(unit);
				}
				ImGui::PopID();

				// Avanzar posición para la siguiente pestaña
				selectorPos.x = selectorRect.Max.x + 2.0f;
			}
		}
		ImGui::End();
	}

	void SaveLevelAs()
	{
		std::thread saveAs([]()
			{
				//first create the directory if needed
				std::filesystem::path directory(nostd::StringToWString(defaultLevelsFolder));
				std::filesystem::create_directory(directory);

				std::wstring path = L"";
				COMDLG_FILTERSPEC filters[] = { {.pszName = L"JSON files. (*.json)", .pszSpec = L"*.json" } };
				std::pair<COMDLG_FILTERSPEC*, int> filter_info = std::make_pair<COMDLG_FILTERSPEC*, int>(filters, _countof(filters));
				if (!SaveFileDialog(path, std::filesystem::absolute(directory), L"", &filter_info)) return;
				if (path.empty()) return;

				std::filesystem::path jsonFilePath = path;
				jsonFilePath.replace_extension(".json");

				SaveLevelToFile(currentSceneUnitId, nostd::WStringToString(jsonFilePath.filename()));
				SaveWorkbench(nostd::WStringToString(jsonFilePath.filename()));
			}
		);
		saveAs.detach();
	}

	bool SaveFileDialog(std::wstring& path, std::wstring defaultDirectory, std::wstring defaultFileName, std::pair<COMDLG_FILTERSPEC*, int>* pFilterInfo)
	{
		IFileSaveDialog* p_file_save = nullptr;
		bool are_all_operation_success = false;
		while (!are_all_operation_success)
		{
			HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
				IID_IFileSaveDialog, reinterpret_cast<void**>(&p_file_save));
			if (FAILED(hr))
				break;

			if (!pFilterInfo)
			{
				COMDLG_FILTERSPEC save_filter[1];
				save_filter[0].pszName = L"All files";
				save_filter[0].pszSpec = L"*.*";
				hr = p_file_save->SetFileTypes(1, save_filter);
				if (FAILED(hr))
					break;
				hr = p_file_save->SetFileTypeIndex(1);
				if (FAILED(hr))
					break;
			}
			else
			{
				hr = p_file_save->SetFileTypes(pFilterInfo->second, pFilterInfo->first);
				if (FAILED(hr))
					break;
				hr = p_file_save->SetFileTypeIndex(1);
				if (FAILED(hr))
					break;
			}

			if (!defaultDirectory.empty()) {
				IShellItem* pCurFolder = NULL;
				hr = SHCreateItemFromParsingName(defaultDirectory.c_str(), NULL, IID_PPV_ARGS(&pCurFolder));
				if (FAILED(hr))
					break;
				p_file_save->SetFolder(pCurFolder);
				pCurFolder->Release();
			}

			if (!defaultFileName.empty())
			{
				hr = p_file_save->SetFileName(defaultFileName.c_str());
				if (FAILED(hr))
					break;
			}

			hr = p_file_save->Show(NULL);
			if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) // No item was selected.
			{
				are_all_operation_success = true;
				break;
			}
			else if (FAILED(hr))
				break;

			IShellItem* p_item;
			hr = p_file_save->GetResult(&p_item);
			if (FAILED(hr))
				break;

			PWSTR item_path;
			hr = p_item->GetDisplayName(SIGDN_FILESYSPATH, &item_path);
			if (FAILED(hr))
				break;
			path = item_path;
			CoTaskMemFree(item_path);
			p_item->Release();

			are_all_operation_success = true;
		}

		if (p_file_save)
			p_file_save->Release();
		return are_all_operation_success;
	}

	std::string GetLevelString(SceneUnitId id)
	{
		using namespace nlohmann;
		using namespace Scene;

		nlohmann::json level;

		level[SceneObjectTypeJsonContainer.at(SO_Renderables)] = json::array();
		level[SceneObjectTypeJsonContainer.at(SO_Lights)] = json::array();
		level[SceneObjectTypeJsonContainer.at(SO_Cameras)] = json::array();
		level[SceneObjectTypeJsonContainer.at(SO_SoundEffects)] = json::array();
		level[SceneObjectTypeJsonContainer.at(SO_PhysicScenes)] = json::array();
		level[SceneObjectTypeJsonContainer.at(SO_Triggers)] = json::array();
		level[SceneObjectTypeJsonContainer.at(SO_Boundaries)] = json::array();
		level[SceneObjectTypeJsonContainer.at(SO_SceneControllers)] = json::array();

		WriteRenderablesJson(id, level[SceneObjectTypeJsonContainer.at(SO_Renderables)]);
		WriteLightsJson(id, level[SceneObjectTypeJsonContainer.at(SO_Lights)]);
		WriteCamerasJson(id, level[SceneObjectTypeJsonContainer.at(SO_Cameras)]);
		WriteSoundFXsJson(id, level[SceneObjectTypeJsonContainer.at(SO_SoundEffects)]);
		WritePhysicSceneJson(id, level[SceneObjectTypeJsonContainer.at(SO_PhysicScenes)]);
		WriteTriggersJson(id, level[SceneObjectTypeJsonContainer.at(SO_Triggers)]);
		WriteBoundariesJson(id, level[SceneObjectTypeJsonContainer.at(SO_Boundaries)]);
		WriteSceneControllersJson(id, level[SceneObjectTypeJsonContainer.at(SO_SceneControllers)]);

		std::string levelString = level.dump(4);
		return levelString;
	}

	std::string GetLevelName(SceneUnitId id)
	{
		for (auto& v : currentLevelName)
		{
			if (std::get<0>(v) == id)
				return std::get<1>(v);
		}
		assert(!!!"level name not found in vector");
		return "";
	}

	void ChangeLevelName(SceneUnitId id, std::string levelFileName)
	{
		for (auto& v : currentLevelName)
		{
			if (std::get<0>(v) == id)
			{
				std::get<1>(v) = levelFileName;
				return;
			}
		}
	}

	void SaveLevelToFile(SceneUnitId id, std::string levelFileName)
	{
		using namespace nlohmann;

		std::string levelString = GetLevelString(id);

		const std::string levelsRootFolder = "Levels/";
		const std::string filename = levelsRootFolder + levelFileName;

		//first create the directory if needed
		std::filesystem::path directory(levelsRootFolder);
		std::filesystem::create_directory(directory);

		//then create the json level file
		std::filesystem::path path(filename);
		path.replace_extension(".json");
		std::string pathStr = path.generic_string();
		std::ofstream file;
		file.open(pathStr);
		file.write(levelString.c_str(), levelString.size());
		file.close();

		ChangeLevelName(id, levelFileName);
		levelModified.at(id) = false;
		defaultLevel.at(id) = false;
	}

	void SaveTemplates()
	{
		//using namespace Templates;
		Templates::SaveTemplates(defaultTemplatesFolder, Shader::templateName, WriteShadersJson);
		Templates::SaveTemplates(defaultTemplatesFolder, Material::templateName, WriteMaterialsJson);
		Templates::SaveTemplates(defaultTemplatesFolder, Model3D::templateName, WriteModel3DsJson);
		Templates::SaveTemplates(defaultTemplatesFolder, Sound::templateName, WriteSoundsJson);
		Templates::SaveTemplates(defaultTemplatesFolder, Texture::templateName, WriteTexturesJson);
		Templates::SaveTemplates(defaultTemplatesFolder, RenderPass::templateName, WriteRenderPasssJson);
		Templates::SaveTemplates(defaultTemplatesFolder, PhysicGeometry::templateName, WritePhysicGeometrysJson);
		Templates::SaveTemplates(defaultTemplatesFolder, HtmlUI::templateName, WriteHtmlUIsJson);
		Templates::SaveTemplates(defaultTemplatesFolder, Mold::templateName, WriteMoldsJson);
		templatesModified = false;
	}

	float separatorFactor = 0.0f;
	const float panelMinHeight = 47.0f;
	void DrawRightPanel()
	{
		if (!currentSceneUnitId) return;

		auto matchSceneObjectsAttributes = []()
			{
				sceneObjectEdition.at(currentSceneUnitId).CreateEditableAttributesToMatch<SceneObjectType>(
					[](JUUID uuid) { return GetSceneObjectType(currentSceneUnitId, uuid); },
					[](JUUID uuid) { return GetSceneObjectPointer(currentSceneUnitId, uuid); },
					GetSceneObjectAttributes,
					GetSceneObjectDrawers,
					GetSceneObjectPreviewers
				);
			};

		auto matchTemplatesAttributes = []()
			{
				templateEdition.CreateEditableAttributesToMatch<TemplateType>(
					GetTemplateType,
					GetJTemplatePointer,
					GetTemplateAttributes,
					GetTemplateDrawers,
					GetTemplatePreviewers
				);
			};

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 panPos = ImVec2(viewport->WorkSize.x - panW, viewport->WorkPos.y);
		ImVec2 panSize = ImVec2(panW, viewport->WorkSize.y);

		ImGui::SetNextWindowPos(panPos);
		ImGui::SetNextWindowSize(panSize);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2, 0));
		ImGui::Begin("Right panel", (bool*)1, panFlags);
		{
			float halfY = panSize.y * 0.5f;

			float soPanelH = std::max(halfY * (1.0f - separatorFactor) - 2.5f, panelMinHeight);
			ImVec2 soPos = ImVec2(panPos.x, panPos.y);
			ImVec2 soSize = ImVec2(panSize.x, soPanelH);

			if (currentSceneUnitId != 0)
			{
				sceneObjectEdition.at(currentSceneUnitId).DrawPanel(soPos, soSize, SceneObjectsTypePanelMenuItems,
					GetSceneObjectsTypesList,
					[](JUUID uuid) { return GetSceneObjectPointer(currentSceneUnitId, uuid); },
					OnChangeSceneObjectTab,
					matchSceneObjectsAttributes,
					[](JUUID uuid, bool selected) { SetSceneObjectSelection(currentSceneUnitId, uuid, selected); },
					[](JUUID uuid) { SendEditorPreview(uuid, [](JUUID uuid) { return GetSceneObjectPointer(currentSceneUnitId, uuid); }, sceneObjectEdition.at(currentSceneUnitId).drawers); },
					[](SceneObjectType type) { StartSceneObjectCreation(type); },
					[](JUUID uuid) { DeleteSceneObjectFromEditor(currentSceneUnitId, uuid); },
					[](JUUID uuid) { OpenPopupForSceneObject(currentSceneUnitId, uuid); },
					[] { ClearSceneObjectsSelection(currentSceneUnitId); }
				);
			}

			ImGui::Button("DragableSeparator", ImVec2(-1, 5));
			if (ImGui::IsItemActive())
			{
				float deltaY = ImGui::GetMouseDragDelta().y;
				separatorFactor -= deltaY / halfY;
				float hi = 1.0f - panelMinHeight / halfY;
				float low = -1.0f + panelMinHeight / halfY;
				if (separatorFactor < hi && separatorFactor > low)
					ImGui::ResetMouseDragDelta(); // Reset delta for continuous dragging
				separatorFactor = std::clamp(separatorFactor, low, hi);
			}

			float tePanelH = std::max(panSize.y - soPanelH - 5.0f, panelMinHeight);
			ImVec2 tePos = ImVec2(panPos.x, soSize.y + 5.0f);
			ImVec2 teSize = ImVec2(panSize.x, tePanelH);

			templateEdition.DrawPanel(tePos, teSize, TemplateTypePanelMenuItems,
				GetTemplatesTypesList,
				[](JUUID uuid) { return GetJTemplatePointer(uuid); },
				OnChangeTemplateTab,
				matchTemplatesAttributes,
				[](JUUID uuid, bool selected) {},
				[](JUUID uuid) { SendEditorPreview(uuid, GetJTemplatePointer, templateEdition.drawers); },
				[](TemplateType type) { StartTemplateCreation(type); },
				[](JUUID uuid) { DeleteTemplate(uuid); },
				[](JUUID uuid) { OpenPopupForTemplate(uuid); },
				[] {}
			);
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void PromptTemplateDeletion(std::vector<nlohmann::json> references, std::function<void(std::vector<nlohmann::json>)> OnDelete, std::function<void()> OnCancel)
	{
		deletePrompt.showing = true;
		deletePrompt.references = references;
		deletePrompt.OnDelete = OnDelete;
		deletePrompt.OnCancel = OnCancel;
	}

	void CloseDeletionPrompt()
	{
		deletePrompt.showing = false;
	}

	void BuildAssetsTree()
	{
		using namespace Scene;
		for (auto& [id, panel] : sceneObjectEdition)
		{
			if (IsPlaying(id) || !SceneUnitRenderingExists(id) || !SceneCanBuildAssetsTree(id))
				continue;

			panel.BuildAssetsTree(
				[&]() {return GetSceneObjectsTypesList(id); },
				[&](JUUID uuid) {return GetSceneObjectPointer(id, uuid); }
			);
		}
		templateEdition.BuildAssetsTree(GetTemplatesTypesList, GetJTemplatePointer);
	}

	void OpenPopupForSceneObject(SceneUnitId id, JUUID uuid)
	{
		SceneObjectType type = GetSceneObjectType(id, uuid);
		SceneObject* object = GetSceneObjectPointer(id, uuid);

		sceneObjectPopup.show = true;
		sceneObjectPopup.openedCollapsableItem = 0;
		sceneObjectPopup.pos = ImGui::GetMousePos();
		sceneObjectPopup.pos.x -= 200;
		sceneObjectPopup.name = std::string(object->at("name")) + "_copy";
		sceneObjectPopup.id = id;
		sceneObjectPopup.uuid = uuid;
		sceneObjectPopup.type = type;
	}

	void OpenPopupForTemplate(JUUID uuid)
	{
	}

	//SceneObjects Panel
	void OnChangeSceneObjectTab(std::string newTab)
	{
		sceneObjectEdition.at(currentSceneUnitId).selectedTab = newTab;
		sceneObjectEdition.at(currentSceneUnitId).editables = sceneObjectEdition.at(currentSceneUnitId).selected;

		if (newTab == sceneObjectEdition.at(currentSceneUnitId).detailAbleTabs.at(1))
		{
			sceneObjectEdition.at(currentSceneUnitId).CreateEditableAttributesToMatch<SceneObjectType>(
				[](JUUID uuid) { return GetSceneObjectType(currentSceneUnitId, uuid); },
				[](JUUID uuid) { return GetSceneObjectPointer(currentSceneUnitId, uuid); },
				GetSceneObjectAttributes,
				GetSceneObjectDrawers,
				GetSceneObjectPreviewers
			);
			for (auto& uuid : sceneObjectEdition.at(currentSceneUnitId).editables)
			{
				SendEditorPreview(uuid, [](JUUID uuid) { return GetSceneObjectPointer(currentSceneUnitId, uuid); }, sceneObjectEdition.at(currentSceneUnitId).drawers);
			}
		}
		else
		{
			for (auto& uuid : sceneObjectEdition.at(currentSceneUnitId).editables)
			{
				SendEditorDestroyPreview(uuid, [](JUUID uuid) { return GetSceneObjectPointer(currentSceneUnitId, uuid); });
			}
		}
	}

	void OpenSceneObject(JUUID uuid)
	{
		if (currentSceneUnitId == 0) return;
		sceneObjectEdition.at(currentSceneUnitId).selected = { uuid };
		OnChangeSceneObjectTab(templateEdition.detailAbleTabs.at(1));
	}

	void OpenSceneObjectOnNextFrame(JUUID uuid)
	{
		if (currentSceneUnitId == 0) return;
		sceneObjectEdition.at(currentSceneUnitId).selectedNextFrame = uuid;
	}

	void MarkScenePanelAssetsAsDirty()
	{
		if (currentSceneUnitId == 0) return;
		sceneObjectEdition.at(currentSceneUnitId).dirtyAssetsTree = true;
	}

	//Templates Panel
	void OnChangeTemplateTab(std::string newTab)
	{
		templateEdition.selectedTab = newTab;
		templateEdition.editables = templateEdition.selected;

		if (newTab == templateEdition.detailAbleTabs.at(1))
		{
			templateEdition.CreateEditableAttributesToMatch<TemplateType>(
				GetTemplateType,
				GetJTemplatePointer,
				GetTemplateAttributes,
				GetTemplateDrawers,
				GetTemplatePreviewers
			);
			for (auto& uuid : templateEdition.editables)
			{
				SendEditorPreview(uuid, GetJTemplatePointer, templateEdition.drawers);
			}
		}
		else
		{
			for (auto& uuid : templateEdition.editables)
			{
				SendEditorDestroyPreview(uuid, GetJTemplatePointer);
			}
		}
	}

	void OpenTemplate(JUUID uuid)
	{
		templateEdition.selected = { uuid };
		OnChangeTemplateTab(templateEdition.detailAbleTabs.at(1));
	}

	void OpenTemplateOnNextFrame(JUUID uuid)
	{
		templateEdition.selectedNextFrame = uuid;
	}

	void MarkTemplatesPanelAssetsAsDirty()
	{
		templateEdition.dirtyAssetsTree = true;
		templatesModified = true;
	}

	void RemoveFromTemplateSelection(std::set<JUUID> uuids)
	{
		for (auto uuid : uuids)
		{
			if (templateEdition.selected.contains(uuid))
			{
				templateEdition.selected.erase(uuid);
			}
			if (templateEdition.editables.contains(uuid))
			{
				templateEdition.editables.erase(uuid);
			}
		}
	}

	//JObject's Preview Panel
	void SendEditorPreview(JUUID uuid, auto GetJObject, auto drawers)
	{
		size_t flags = 0;
		JObject* j = GetJObject(uuid);
		for (auto& [attribute, _] : drawers)
		{
			if (!j->UpdateFlagsMap.contains(attribute)) continue;
			auto& tp = j->UpdateFlagsMap.at(attribute);
			if (!std::get<1>(tp)) continue;
			flags |= std::get<0>(tp);
		}
		j->EditorPreview(flags);
	}

	void SendEditorDestroyPreview(JUUID uuid, auto GetJObject)
	{
		JObject* j = GetJObject(uuid);
		j->DestroyEditorPreview();
	}

	//Model3D Animation Sequencer
	void OpenAnimationSequencer(JUUID uuid)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 seqPos = ImVec2(viewport->WorkSize.x / sequencerAdjustment, viewport->WorkSize.y / sequencerAdjustment);
		ImVec2 seqSize = ImVec2(viewport->WorkSize.x * (1.0f - (2.0f / sequencerAdjustment)), viewport->WorkSize.y * (1.0f - (2.0f / sequencerAdjustment)));
		animationSequencer.Initialize(seqPos, seqSize, uuid);
	}

	//Gizmos
	void ResetGizmoVariableWorkers(SceneUnitId id)
	{
		gizmos.at(id).gizmoApplyOp = false;
		gizmos.at(id).soRotation.clear();
		gizmos.at(id).soScale.clear();
		gizmos.at(id).so2bb.clear();
		gizmos.at(id).bb2gizmo.clear();
		gizmos.at(id).gizmoRotation = XMFLOAT3();
		gizmos.at(id).gizmoPosition = XMFLOAT3();
		gizmos.at(id).gizmoScale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	}

	bool InteractWithGizmos(SceneUnitId id, std::set<SceneObject*>& objects2Gizmo)
	{
		std::set<SceneObject*> objects;
		std::transform(selectedSceneObjects.at(id).begin(), selectedSceneObjects.at(id).end(), std::inserter(objects, objects.begin()), [&](JUUID uuid)
			{
				return GetSceneObjectPointer(id, uuid);
			}
		);

		std::copy_if(objects.begin(), objects.end(), std::inserter(objects2Gizmo, objects2Gizmo.begin()), [&](auto so)
			{
				return so->CanInteractWithGizmo(gizmos.at(id).gizmoOperation);
			}
		);
		return !objects2Gizmo.empty();
	}

	void DrawPickedObjectsGizmo(SceneUnitId id, CameraID camera)
	{
		if (!gizmos.contains(id)) return;

		if (ImGui::IsKeyPressed(ImGuiKey_T)) // t ky
		{
			gizmos.at(id).gizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
			gizmos.at(id).gizmoMode = ImGuizmo::MODE::WORLD;
			ResetGizmoVariableWorkers(id);
		}
		if (ImGui::IsKeyPressed(ImGuiKey_R)) // r key
		{
			gizmos.at(id).gizmoOperation = ImGuizmo::OPERATION::ROTATE;
			gizmos.at(id).gizmoMode = ImGuizmo::MODE::WORLD;
			ResetGizmoVariableWorkers(id);
		}
		if (ImGui::IsKeyPressed(ImGuiKey_S)) // s Key
		{
			gizmos.at(id).gizmoOperation = ImGuizmo::OPERATION::SCALE;
			gizmos.at(id).gizmoMode = ImGuizmo::MODE::LOCAL;
			ResetGizmoVariableWorkers(id);
		}

		std::set<SceneObject*> objects2Gizmo;
		if (!InteractWithGizmos(id, objects2Gizmo))
			return;

		auto translateObjects = [&](XMFLOAT4X4 view, XMFLOAT4X4 proj)
			{
				if (!gizmos.at(id).gizmoApplyOp)
				{
					gizmos.at(id).gizmoCentroidMx = GetBoundindBoxesCentroid(objects2Gizmo);
				}
				XMFLOAT4X4 delta;
				ImGuizmo::Manipulate(*view.m, *proj.m, gizmos.at(id).gizmoOperation, gizmos.at(id).gizmoMode, *gizmos.at(id).gizmoCentroidMx.m, *delta.m, NULL, NULL, NULL);
				XMMATRIX XMdelta = XMLoadFloat4x4(&delta);
				XMVECTOR XMtranslation, XMrotation, XMscale;
				XMMatrixDecompose(&XMscale, &XMrotation, &XMtranslation, XMdelta);

				XMVECTOR len = XMVector3Length(XMtranslation);
				if (len.m128_f32[0] < g_XMEpsilon.f[0])
					return;

				gizmos.at(id).gizmoApplyOp = true;
				for (auto& o : objects2Gizmo)
				{
					if (!o->contains("position")) continue;
					XMFLOAT3 p = ToXMFLOAT3(o->at("position"));
					p.x += XMtranslation.m128_f32[0];
					p.y += XMtranslation.m128_f32[1];
					p.z += XMtranslation.m128_f32[2];
					nlohmann::json patch = { {"position", FromXMFLOAT3(p) } };
					o->JUpdate(patch);
				}
			};
		auto rotateObjects = [&](XMFLOAT4X4 view, XMFLOAT4X4 proj)
			{
				//if the rotation gizmo has not been initialized, create a map of the initial rotations
				//vector from the bounding box to the object's position
				//and vector from the bounding box to the gizmo position
				if (!gizmos.at(id).gizmoApplyOp)
				{
					gizmos.at(id).gizmoCentroidMx = GetBoundindBoxesCentroid(objects2Gizmo);
					gizmos.at(id).gizmoPosition = { gizmos.at(id).gizmoCentroidMx._41, gizmos.at(id).gizmoCentroidMx._42, gizmos.at(id).gizmoCentroidMx._43 };
					gizmos.at(id).gizmoApplyOp = true;

					for (auto& o : objects2Gizmo)
					{
						XMFLOAT3 p = ToXMFLOAT3(o->at("position"));
						XMFLOAT3 r = o->contains("rotation") ? ToXMFLOAT3(o->at("rotation")) : XMFLOAT3();
						BoundingBox bb = o->GetBoundingBox();

						gizmos.at(id).soRotation.insert_or_assign(o, r);
						gizmos.at(id).so2bb.insert_or_assign(o, p - bb.Center);
						gizmos.at(id).bb2gizmo.insert_or_assign(o, bb.Center - gizmos.at(id).gizmoPosition);
					}
				}

				XMFLOAT4X4 delta;
				ImGuizmo::Manipulate(*view.m, *proj.m, gizmos.at(id).gizmoOperation, gizmos.at(id).gizmoMode, *gizmos.at(id).gizmoCentroidMx.m, *delta.m, NULL, NULL, NULL);
				XMMATRIX XMdelta = XMLoadFloat4x4(&delta);
				XMVECTOR XMtranslation, XMrotation, XMscale;
				XMMatrixDecompose(&XMscale, &XMrotation, &XMtranslation, XMdelta);

				gizmos.at(id).gizmoRotation.x += XMConvertToDegrees(2.0f * XMrotation.m128_f32[0]);
				gizmos.at(id).gizmoRotation.y += XMConvertToDegrees(2.0f * XMrotation.m128_f32[1]);
				gizmos.at(id).gizmoRotation.z += XMConvertToDegrees(2.0f * XMrotation.m128_f32[2]);

				XMVECTOR rotQ = XMQuaternionRotationRollPitchYaw(
					XMConvertToRadians(gizmos.at(id).gizmoRotation.x),
					XMConvertToRadians(gizmos.at(id).gizmoRotation.y),
					XMConvertToRadians(gizmos.at(id).gizmoRotation.z)
				);

				auto rotateXM3 = [](XMFLOAT3 v, XMVECTOR q)
					{
						XMMATRIX R = XMMatrixRotationQuaternion(q);
						XMMATRIX T = XMMatrixTranslation(v.x, v.y, v.z);
						XMMATRIX TR = XMMatrixMultiply(T, R);
						XMVECTOR s, r, t;
						XMMatrixDecompose(&s, &r, &t, TR);
						XMFLOAT3 nv;
						XMStoreFloat3(&nv, t);
						return nv;
					};

				for (auto& o : objects2Gizmo)
				{
					XMFLOAT3 s2b = gizmos.at(id).so2bb.at(o);
					XMFLOAT3 b2g = gizmos.at(id).bb2gizmo.at(o);

					XMFLOAT3 ds2b = rotateXM3(s2b, rotQ);
					XMFLOAT3 db2g = rotateXM3(b2g, rotQ);

					XMFLOAT3 p = gizmos.at(id).gizmoPosition + ds2b + db2g;
					XMFLOAT3 r = gizmos.at(id).soRotation.at(o) + gizmos.at(id).gizmoRotation;

					nlohmann::json patch = { { "position", FromXMFLOAT3(p) } };
					if (o->contains("rotation"))
						patch["rotation"] = FromXMFLOAT3(r);

					o->JUpdate(patch);
				}
			};
		auto scaleObjects = [&](XMFLOAT4X4 view, XMFLOAT4X4 proj)
			{
				if (!gizmos.at(id).gizmoApplyOp)
				{
					gizmos.at(id).gizmoCentroidMx = GetBoundindBoxesCentroid(objects2Gizmo);
				}
				XMFLOAT4X4 delta;
				ImGuizmo::Manipulate(*view.m, *proj.m, gizmos.at(id).gizmoOperation, gizmos.at(id).gizmoMode, *gizmos.at(id).gizmoCentroidMx.m, *delta.m, NULL, NULL, NULL);
				XMMATRIX XMdelta = XMLoadFloat4x4(&delta);
				XMVECTOR XMtranslation, XMrotation, XMscale;
				XMMatrixDecompose(&XMscale, &XMrotation, &XMtranslation, XMdelta);

				XMVECTOR len = XMVector3Length(XMscale);
				if (len.m128_f32[0] < g_XMEpsilon.f[0])
					return;

				if (!gizmos.at(id).gizmoApplyOp)
				{
					gizmos.at(id).gizmoApplyOp = true;
					gizmos.at(id).gizmoPosition = { gizmos.at(id).gizmoCentroidMx._41, gizmos.at(id).gizmoCentroidMx._42, gizmos.at(id).gizmoCentroidMx._43 };

					for (auto& o : objects2Gizmo)
					{
						XMFLOAT3 p = ToXMFLOAT3(o->at("position"));
						XMFLOAT3 s = o->contains("scale") ? ToXMFLOAT3(o->at("scale")) : XMFLOAT3(1.0f, 1.0f, 1.0f);
						BoundingBox bb = o->GetBoundingBox();

						gizmos.at(id).soScale.insert_or_assign(o, s);
						gizmos.at(id).so2bb.insert_or_assign(o, p - bb.Center);
						gizmos.at(id).bb2gizmo.insert_or_assign(o, bb.Center - gizmos.at(id).gizmoPosition);
					}
				}

				gizmos.at(id).gizmoScale.x *= XMscale.m128_f32[0];
				gizmos.at(id).gizmoScale.y *= XMscale.m128_f32[1];
				gizmos.at(id).gizmoScale.z *= XMscale.m128_f32[2];

				auto scaleXM3 = [](XMFLOAT3 v, XMFLOAT3 s)
					{
						XMMATRIX S = XMMatrixScaling(s.x, s.y, s.z);
						XMMATRIX T = XMMatrixTranslation(v.x, v.y, v.z);
						XMMATRIX TS = XMMatrixMultiply(T, S);
						XMVECTOR sc, r, t;
						XMMatrixDecompose(&sc, &r, &t, TS);
						XMFLOAT3 nv;
						XMStoreFloat3(&nv, t);
						return nv;
					};

				for (auto& o : objects2Gizmo)
				{
					XMFLOAT3 s2b = gizmos.at(id).so2bb.at(o);
					XMFLOAT3 b2g = gizmos.at(id).bb2gizmo.at(o);

					XMFLOAT3 ds2b = scaleXM3(s2b, gizmos.at(id).gizmoScale);
					XMFLOAT3 db2g = scaleXM3(b2g, gizmos.at(id).gizmoScale);

					XMFLOAT3 p = gizmos.at(id).gizmoPosition + ds2b + db2g;
					XMFLOAT3 s = gizmos.at(id).soScale.at(o) * gizmos.at(id).gizmoScale;

					nlohmann::json patch = { { "position", FromXMFLOAT3(p) } };
					if (o->contains("scale"))
						patch["scale"] = FromXMFLOAT3(s);

					o->JUpdate(patch);
				}
			};

		std::unordered_map<ImGuizmo::OPERATION, std::function<void(XMFLOAT4X4, XMFLOAT4X4)>> operators =
		{
			{ ImGuizmo::OPERATION::TRANSLATE, translateObjects },
			{ ImGuizmo::OPERATION::ROTATE, rotateObjects},
			{ ImGuizmo::OPERATION::SCALE, scaleObjects }
		};

		BeginGizmoInteraction(camera, [&](XMFLOAT4X4 view, XMFLOAT4X4 proj)
			{
				operators.at(gizmos.at(id).gizmoOperation)(view, proj);
			}
		);
	}

	void BeginGizmoInteraction(CameraID camera, std::function<void(XMFLOAT4X4 view, XMFLOAT4X4 proj)> interaction)
	{
		ImGuizmo::BeginFrame();
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::AllowAxisFlip(false);

		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		ImGuizmo::SetID(0);

		XMFLOAT4X4 view;
		XMFLOAT4X4 proj;
		XMStoreFloat4x4(&view, camera->view());
		XMStoreFloat4x4(&proj, camera->perspectiveProjection.projectionMatrix);

		interaction(view, proj);
	}

	//SceneObject Selection
	void SelectSceneObject(SceneUnitId id, JUUID uuid)
	{
		if (uuid == ""
#if defined(_EDITOR_BOUNDINGBOX)
			|| (!boundingBox.at(id).empty() && boundingBox.at(id).uuid() == uuid)
#endif
			)
		{
			return;
		}

		RenderableID r = MAKESUUUID(id, uuid);
		OutputDebugStringA(std::string("picked:" + r->name() + "\n").c_str());
		r->OnPick();
	}

	void SelectRenderable(RenderableID renderable)
	{
		ToggleSceneObjectFromSelection(FROMSUUUID(renderable()));
	}

	void SelectLight(LightID light)
	{
		ToggleSceneObjectFromSelection(FROMSUUUID(light()));
	}

	void SelectCamera(CameraID camera)
	{
		ToggleSceneObjectFromSelection(FROMSUUUID(camera()));
	}

	void SelectSoundEffect(SoundFXID soundfx)
	{
		ToggleSceneObjectFromSelection(FROMSUUUID(soundfx()));
	}

	void SelectTrigger(TriggerID trigger)
	{
		ToggleSceneObjectFromSelection(FROMSUUUID(trigger()));
	}

	void SelectBoundary(BoundaryID boundary)
	{
		ToggleSceneObjectFromSelection(FROMSUUUID(boundary()));
	}

	void ToggleSceneObjectFromSelection(SceneUnitId unit, JUUID uuid)
	{
		if (!sceneObjectEdition.at(unit).selected.contains(uuid))
		{
			InsertSceneObjectToSelection(unit, uuid);
		}
		else
		{
			EraseSceneObjectFromSelection(unit, uuid);
		}
		ResetGizmoVariableWorkers(unit);
	}

	void SetSceneObjectSelection(SceneUnitId unit, JUUID uuid, bool selected)
	{
		if (selected)
		{
			selectedSceneObjects.at(unit).insert(uuid);
		}
		else
		{
			selectedSceneObjects.at(unit).erase(uuid);

		}
		gizmos.at(unit).gizmoApplyOp = false;
	}

	void InsertSceneObjectToSelection(SceneUnitId unit, JUUID uuid)
	{
		sceneObjectEdition.at(unit).selected.insert(uuid);
		SetSceneObjectSelection(unit, uuid, true);
	}

	void EraseSceneObjectFromSelection(SceneUnitId unit, JUUID uuid)
	{
		if (!sceneObjectEdition.contains(unit)) return;
		sceneObjectEdition.at(unit).selected.erase(uuid);
		SetSceneObjectSelection(unit, uuid, false);
	}

	void ClearSceneObjectsSelection(SceneUnitId unit)
	{
		selectedSceneObjects.at(unit).clear();
		ResetGizmoVariableWorkers(unit);
	}

	//BoundingBox
	void UpdateBoundingBox(SceneUnitId id)
	{
#if !defined(_EDITOR_BOUNDINGBOX)
		return;
#endif
		if (!selectedSceneObjects.contains(id)) return;

		if (selectedSceneObjects.at(id).size() == 0ULL || IsPlaying(id))
		{
			if (boundingBox.at(id))
			{
				boundingBox.at(id)->visible(false);
			}
			return;
		}

		std::set<SceneObject*> objects;
		std::transform(selectedSceneObjects.at(id).begin(), selectedSceneObjects.at(id).end(), std::inserter(objects, objects.begin()), [&](JUUID uuid) { return GetSceneObjectPointer(id, uuid); });

		BoundingBox bb = GetContainedBoundingBox(objects);
		if (!boundingBox.empty())
		{
			boundingBox.at(id)->visible(true);
			boundingBox.at(id)->scale(bb.Extents);
			boundingBox.at(id)->position(bb.Center);
			auto& scene = GetSceneUnit(id);
			boundingBox.at(id)->WriteConstantsBuffer(scene->Frame());
		}
	}

	//Mouse Processing
	bool AnyModalShowing()
	{
		return loadingProgress.loadSceneUnitModal || sceneObjectModal.creating || templateModal.creating || deletePrompt.showing ||
			animationSequencer.showing || sceneObjectPopup.show || scriptEditModal.showing || scriptBindingModal.showing;
	}

	bool MouseIsInGameArea(std::unique_ptr<DirectX::Mouse>& mouse)
	{
		auto coordsInArea = [](int x, int y, RECT area)
			{
				return (x > area.left && x<area.right && y > area.top && y < area.bottom);
			};

		const ImGuiViewport* viewport = ImGui::GetMainViewport();

		RECT gameArea;
		ZeroMemory(&gameArea, sizeof(gameArea));
		gameArea.top = ApplicationBarBottom + 1L;
		gameArea.bottom = static_cast<LONG>(viewport->Size.y);
		gameArea.right = static_cast<LONG>(viewport->Size.x - panW);
		int x = mouse->GetState().x;
		int y = mouse->GetState().y;

		RECT controllersArea = GetGameControllerRect();

		return coordsInArea(x, y, gameArea) && !coordsInArea(x, y, controllersArea);
	}

	void GameAreaMouseProcessing(std::unique_ptr<DirectX::Mouse>& mouse, CameraID camera)
	{
		if (AnyModalShowing()) return;

		DirectX::Mouse::State state = mouse->GetState();

		if (menuBarItemClicked && state.leftButton)
			return;
		menuBarItemClicked = false;

		if (!MouseIsInGameArea(mouse) && state.leftButton)
		{
			lockedGameAreaInput = true;
		}
		else if (lockedGameAreaInput && !state.leftButton)
		{
			lockedGameAreaInput = false;
		}

		if (NonGameMode || lockedGameAreaInput)
			return;

		auto resetMouseProcessing = []()
			{
				currentMouseMode = MOUSE_GAMEAREA_MODE_NONE;
				mousePicking.Reset();
				mouseCamera.Reset();
			};

		if (!MouseIsInGameArea(mouse))
		{
			resetMouseProcessing();
			return;
		}

		if (ImGuizmo::IsOver())
		{
			currentMouseMode = MOUSE_GAMEAREA_MODE_GIZMO;
		}

		switch (currentMouseMode)
		{
		case MOUSE_GAMEAREA_MODE_NONE:
		{
			if (state.leftButton)
			{
				currentMouseMode = MOUSE_GAMEAREA_MODE_PICKING;
				mousePicking.StartPicking(state);
			}
			if (state.rightButton)
			{
				currentMouseMode = MOUSE_GAMEAREA_MODE_CAMERA;
				mouseCamera.RickClick(state.x, state.y);
			}
			if (!mouseCamera.WheelCaptured())
			{
				mouseCamera.CaptureWheel(state.scrollWheelValue);
			}
			else
			{
				if (mouseCamera.Wheel() != state.scrollWheelValue)
				{
					mouseCamera.UpdateWheelMode(state.scrollWheelValue, state.x, state.y);
					currentMouseMode = MOUSE_GAMEAREA_MODE_CAMERA;
				}
			}
		}
		break;
		case MOUSE_GAMEAREA_MODE_PICKING:
		{
			if (mousePicking.CanPick(state))
			{
				mousePicking.Pick();
			}
			else if (mousePicking.MouseMoved(state))
			{
				currentMouseMode = MOUSE_GAMEAREA_MODE_CAMERA;
				mouseCamera.LeftClick(state.x, state.y);
			}
			else if (!state.leftButton)
			{
				resetMouseProcessing();
			}
		}
		break;
		case MOUSE_GAMEAREA_MODE_GIZMO:
		{
			if (!ImGuizmo::IsOver())
			{
				resetMouseProcessing();
			}
		}
		break;
		case MOUSE_GAMEAREA_MODE_CAMERA:
		{
			if (mouseCamera.WheelMode())
			{
				int wheelDelta = state.scrollWheelValue - mouseCamera.Wheel();
				mouseCamera.Wheel(state.scrollWheelValue);
				if (wheelDelta != 0)
				{
					//do something like settings.at("camera").at("speed").at("fw");
					float fwMovement = wheelDelta > 0 ? 1.0f : -1.0f;
					auto kb = keyboard->GetState();
					if (kb.LeftShift) fwMovement *= 10.0f;
					ImGuiIO& io = ImGui::GetIO();
					if (!io.WantCaptureMouse)
					{
						camera->MoveAlongFwAxis(fwMovement);
						WriteSceneUnitDirectionalShadowMapAttributes(camera.unit());
					}
				}
				if (state.leftButton || state.rightButton)
				{
					currentMouseMode = MOUSE_GAMEAREA_MODE_NONE;
					resetMouseProcessing();
				}
			}
			else
			{
				int mousedx, mousedy;
				mouseCamera.UpdateMouseXY(state.x, state.y, mousedx, mousedy);

				if (mouseCamera.LeftButton())
				{
					if (state.leftButton)
					{
						float dx = static_cast<float>(mousedx) * 0.3f;
						float dy = static_cast<float>(mousedy) * 0.3f;
						auto kb = keyboard->GetState();
						if (kb.LeftShift)
						{
							dx *= 5.0f;
							dy *= 5.0f;
						}
						camera->Rotate(dx, dy);
						WriteSceneUnitDirectionalShadowMapAttributes(camera.unit());
					}
					else
					{
						resetMouseProcessing();
					}
				}
				if (mouseCamera.RightButton())
				{
					if (state.rightButton)
					{
						float dx = -static_cast<float>(mousedx) * 0.01f;
						float dy = -static_cast<float>(mousedy) * 0.01f;
						auto kb = keyboard->GetState();
						if (kb.LeftShift)
						{
							dx *= 10.0f;
							dy *= 10.0f;
						}
						camera->MovePerpendicularFwAxis(dx, dy);
						WriteSceneUnitDirectionalShadowMapAttributes(camera.unit());
					}
					else
					{
						resetMouseProcessing();
					}
				}
			}
		}
		break;
		}
	}

	//SceneObject Picking
	bool PickingPassExists(SceneUnitId id)
	{
		return mousePicking.pickingPass.contains(id);
	}

	void CreatePickingPass(SceneUnitId id)
	{
#if !defined(_EDITOR_PICKINGPASS)
		return;
#endif
		if (mousePicking.pickingPass.contains(id)) return;
		mousePicking.pickingPass.insert_or_assign(id, CreateRenderPassInstance(CameraID(), GetRenderPassUUIDByName("PickingPass"), 0, HWNDWIDTH, HWNDHEIGHT));
	}

	void BindPickingRenderables(SceneUnitId id)
	{
		for (auto uuid : GetRenderables(id))
		{
			BindRenderableToPickingPass(MAKESUUUID(id, uuid));
		}
	}

	static std::mutex pickPassBinding;
	void BindRenderableToPickingPass(RenderableID r)
	{
#if !defined(_EDITOR_PICKINGPASS)
		return;
#endif
		std::lock_guard<std::mutex> lock(pickPassBinding);
		auto pass = mousePicking.pickingPass.at(r.unit());
		r->CreateRenderPassMaterialsInstances(pass);
		r->CreateRenderPassConstantsBuffersInstances(pass);
		r->CreateRenderPassRootSignatures(pass);
		r->CreateRenderPassPipelineStates(pass);
	}

	void UnbindRenderableFromPickingPass(RenderableID r)
	{
#if !defined(_EDITOR_PICKINGPASS)
		return;
#endif
		auto pass = mousePicking.pickingPass.at(r.unit());
		r->DestroyRenderPassMaterialsInstances(pass);
		r->DestroyRenderPassConstantsBuffersInstances(pass);
		r->DestroyRenderPassRootSignatures(pass);
		r->DestroyRenderPassPipelineStates(pass);
	}

	void RenderPickingPass(SceneUnitId id, CameraID camera)
	{
#if !defined(_EDITOR_PICKINGPASS)
		return;
#endif

		if (camera.empty() ||
#if !defined(_EDITOR_PICKINGPASS_EVERY_FRAME)
			!mousePicking.doPicking ||
#endif
			!mousePicking.pickingPass.contains(id) ||
			mousePicking.pickingPass.at(id).empty() ||
			currentSceneUnitId != id
			) return;

		unsigned int backBufferIndex = renderer->GetBackBufferIndex();
		commandListPickingPassProcessor->frame = backBufferIndex;

		commandListPickingPassProcessor->ResetCommandList();
		auto& commandList = commandListPickingPassProcessor->GetCommandList();

#if defined(_DEVELOPMENT)
		PIXBeginEvent(commandList.p, 0, L"Scene Picker");
#endif

		mousePicking.pickingPass.at(id)->renderToTexturePass->Pass(id, [&](SceneUnitId unit)
			{
				unsigned int objectId = 1U;
				for (JUUID uuid : GetRenderables(id))
				{
					RenderableID r = MAKESUUUID(id, uuid);
					if (!r->visible()
#if defined(_EDITOR_BOUNDINGBOX)
						|| boundingBox.at(id).uuid() == r->uuid()
#endif
						) continue;

					//OutputDebugStringA(("RenderPickingPass:" + r->name() + ":" + std::to_string(objectId) + "\n").c_str());
					r->WriteConstantsBuffer("objectId", objectId, backBufferIndex);
					r->Render(id, mousePicking.pickingPass.at(id), camera);
					objectId++;
				}
			}
		);

#if defined(_DEVELOPMENT)
		PIXEndEvent(commandList.p);
#endif
		commandListPickingPassProcessor->CloseCommandList();
		commandListPickingPassProcessor->ExecuteCommandList();
		commandListPickingPassProcessor->Next();
	}

	void PickFromScene(SceneUnitId id)
	{
#if !defined(_EDITOR_PICKINGPASS)
		return;
#endif
		if (!mousePicking.doPicking) return;
		mousePicking.doPicking = false;
		currentMouseMode = MOUSE_GAMEAREA_MODE_NONE;

		if (!mousePicking.pickingPass.contains(id) || currentSceneUnitId != id) return;

		unsigned int value = DeviceUtils::CapturePickingPixelValue(
			renderer->d3dDevice,
			renderer->commandQueue,
			mousePicking.pickingPass.at(id)->renderToTexturePass->renderToTexture[0]->renderToTexture,
			mousePicking.pickingPass.at(id)->renderToTexturePass->renderToTexture[0]->width * sizeof(unsigned int),
			mousePicking.pickingPass.at(id)->renderToTexturePass->renderToTexture[0]->resourceDesc,
			mousePicking.pickingCpuBuffer,
			mousePicking.pickingX,
			mousePicking.pickingY,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);

		PickSceneObject(id, value);
	}

	void PickSceneObject(SceneUnitId id, unsigned int pickedObjectId)
	{
		ImGuiIO& io = ImGui::GetIO();
		if (pickedObjectId == 0U || io.WantCaptureMouse)
		{
			SelectSceneObject(id, "");
			return;
		}

		unsigned int objectId = 1U;
		for (JUUID uuid : GetRenderables(id))
		{
			RenderableID r = MAKESUUUID(id, uuid);
			if (!r->visible()
#if defined(_EDITOR_BOUNDINGBOX)
				|| r.uuid() == boundingBox.at(id).uuid()
#endif
				) continue;

			if (pickedObjectId == objectId)
			{
				//OutputDebugStringA(("PickSceneObject:" + r->name() + ":" + std::to_string(objectId) + "\n").c_str());
				//OutputDebugStringA("picked!\n");
				SelectSceneObject(id, r.uuid());
				break;
			}
			objectId++;
		}
	}

	//JObjects Creation
	void StartSceneObjectCreation(SceneObjectType type)
	{
		using namespace Scene;
		sceneObjectModal.json = GetSceneObjectJson(type);
		sceneObjectModal.atts = GetSceneObjectRequiredAttributes(type);
		sceneObjectModal.drawers = GetSceneObjectCreatorDrawers(type);
		sceneObjectModal.validators = GetSceneObjectValidators(type);
		sceneObjectModal.type = type;
		sceneObjectModal.unit = currentSceneUnitId;
		sceneObjectModal.creating = true;
		sceneObjectModal.onCreate = CreateSceneObject;
	}

	void StartTemplateCreation(TemplateType type)
	{
		using namespace Templates;
		templateModal.json = GetTemplateJson(type);
		templateModal.modalProperties = GetTemplateCreationModalProperties(type);
		templateModal.atts = GetTemplateRequiredAttributes(type);
		templateModal.drawers = GetTemplateCreatorDrawers(type);
		templateModal.validators = GetTemplateValidators(type);
		templateModal.type = type;
		templateModal.creating = true;
		templateModal.onCreate = [](SceneUnitId id, TemplateType t, nlohmann::json json) { CreateTemplate(t, json); };
	}

	//Billboards
	RenderableID CreateBillboardFromMaterials(SceneUnitId id, CameraID camera, std::string name, std::string material, std::string pickingMaterial)
	{
#if !defined(_EDITOR_BILLBOARD)
		return RenderableID();
#endif
		std::string jname = name;
		jname += "-billboard";
		JUUID uuid = getUUID();
		nlohmann::json jbillboard = nlohmann::json(
			{
				{
					"meshMaterial",
					{
						{ "material", GetMaterialUUIDByName(material) },
						{ "mesh",
							{
								{ "primitive", GetMeshUUIDByName("decal") }
							}
						}
					}
				},
				{ "castShadows", false },
				{ "shadowed", false },
				{ "name" , jname },
				{ "uuid" , uuid },
				{ "position" , { 0.0f, 0.0f, 0.0f} },
				{ "topology", "TRIANGLELIST"},
				{ "rotation" , { 0.0, 0.0, 0.0 } },
				{ "scale" , { 1.0f, 1.0f, 1.0f } },
				{ "skipMeshes" , {}},
				{ "visible" , true},
				{ "hidden" , true },
				{ "systemCreated", true },
				{ "cameras", { camera.uuid() }},
				{ "passMaterialOverrides",
					{
						{
							{ "meshIndex", 0 },
							{ "renderPass", GetRenderPassUUIDByName("PickingPass") },
							{ "material", GetMaterialUUIDByName(pickingMaterial) }
						}
					}
				}
			}
		);
		CreateRenderable(id, jbillboard);
		RenderableID renderable = MAKESUUUID(id, uuid);
		GetLoadingProcessor(id).LoadingPoolInsert(SO_Renderables, renderable());
		return renderable;
	}

	void RegisterBillboard(SceneUnitId id, JUUID sceneObject)
	{
#if !defined(_EDITOR_BILLBOARD)
		return;
#endif
		billboards.at(id).billboardRegistry.insert_or_assign(sceneObject, MAKESUUUID(id, ""));
	}

	RenderableID GetBillboard(SceneUnitId id, JUUID sceneObject)
	{
#if !defined(_EDITOR_BILLBOARD)
		RenderableID r;
		return r;
#endif
		return billboards.at(id).billboardRegistry.contains(sceneObject) ? billboards.at(id).billboardRegistry.at(sceneObject) : RenderableID();
	}

	void DestroyBillboard(SceneUnitId id, JUUID sceneObject)
	{
#if !defined(_EDITOR_BILLBOARD)
		return;
#endif
		RenderableID billboard = GetBillboard(id, sceneObject);
		if (billboard.empty())
			return;

		billboards.at(id).billboardsToDestroy.insert(billboard);
		billboards.at(id).billboardRegistry.erase(sceneObject);
	}

	void CreateRegisteredBillboards(SceneUnitId id)
	{
#if !defined(_EDITOR_BILLBOARD)
		return;
#endif
		if (!PendingBillboards(id)) return;

		auto& reg = billboards.at(id).billboardRegistry;

		CameraID camera = MAKESUUUID(id, *GetSwapChainCameras(id).begin());

		for (auto it = reg.begin(); it != reg.end(); it++)
		{
			if (!it->second.empty()) continue;

			auto so = GetSceneObjectPointer(id, it->first);
			if (so->contains("hidden") && so->at("hidden") == true) continue;

			it->second = so->CreateBillboard(camera);
			if (!it->second.empty())
			{
				auto& bb = it->second;
				bb->BindToScene();
				BindRenderableToPickingPass(bb);
			}
		}
	}

	bool PendingBillboards(SceneUnitId id)
	{
		if (!billboards.contains(id)) return false;

		auto& reg = billboards.at(id).billboardRegistry;

		return std::any_of(reg.begin(), reg.end(), [](auto& pair) { return pair.second.empty(); });
	}

	void ShowBillboards(SceneUnitId id)
	{
		auto& reg = billboards.at(id).billboardRegistry;

		for (auto it = reg.begin(); it != reg.end(); it++)
		{
			if (it->second.empty()) continue;

			ShowBillboard(it->second);
		}
	}

	void ShowBillboard(RenderableID billboard)
	{
		billboard->visible(true);
	}

	void HideBillboards(SceneUnitId id)
	{
		auto& reg = billboards.at(id).billboardRegistry;

		for (auto it = reg.begin(); it != reg.end(); it++)
		{
			if (it->second.empty()) continue;

			HideBillboard(it->second);
		}
	}

	void HideBillboard(RenderableID billboard)
	{
		billboard->visible(false);
	}

	void UpdateBillboards()
	{
		if (!currentSceneUnitId || GetCountFromMouseCameras(currentSceneUnitId) == 0ULL) return;

		auto& reg = billboards.at(currentSceneUnitId).billboardRegistry;

		for (auto it = reg.begin(); it != reg.end(); it++)
		{
			if (it->second.empty()) continue;

			auto so = GetSceneObjectPointer(currentSceneUnitId, it->first);
			so->UpdateBillboard(it->second);
		}
	}

	void DestroyPendingBillboards()
	{
		if (!currentSceneUnitId || GetCountFromMouseCameras(currentSceneUnitId) == 0ULL) return;

		auto& reg = billboards.at(currentSceneUnitId).billboardsToDestroy;

		for (auto b : reg)
		{
			UnbindRenderableFromPickingPass(b);
			EraseRenderableFromRenderables(b.unit(), b.uuid());
			DeleteRenderableSceneObject(b);
		}
		reg.clear();
	}

	bool IsPlaying(SceneUnitId id)
	{
		return isPlaying.contains(id) && isPlaying.at(id);
	}

	bool IsPaused(SceneUnitId id)
	{
		return isPaused.contains(id) && isPaused.at(id);
	}

	void SwitchToPlayMode(SceneUnitId id)
	{
		using namespace Scene;
		isPlaying.at(id) = true;
		editorPrePlayDump.at(id) = GetLevelString(id);
		HideBillboards(id);
		PlaySounds(id);
		drawStaticBodies.at(id).PlayMode();
		drawDynamicBodies.at(id).PlayMode();
		drawCharacters.at(id).PlayMode();
		drawTriggers.at(id).PlayMode();
	}

	void SwitchToPauseMode(SceneUnitId id)
	{
		using namespace Scene;
		isPaused.at(id) = true;
		Scene::PauseSounds(id);
	}

	void SwitchToUnPausedMode(SceneUnitId id)
	{
		using namespace Scene;
		isPaused.at(id) = false;
		ResumeSounds(id);
	}

	void SwitchToNonPlayMode(SceneUnitId id)
	{
		using namespace Scene;
		using namespace nlohmann;
		isPlaying.at(id) = false;
		isPaused.at(id) = false;
		nlohmann::json current = json::parse(GetLevelString(id));
		nlohmann::json initial = json::parse(editorPrePlayDump.at(id));

		//initial vs current
		std::map<SceneObjectType, std::set<JUUID>> initialObjects =
		{
			{ SO_Renderables, std::set<JUUID>() },
			{ SO_Lights, std::set<JUUID>() },
			{ SO_Cameras, std::set<JUUID>() },
			{ SO_SoundEffects, std::set<JUUID>() },
			{ SO_PhysicScenes, std::set<JUUID>() },
			{ SO_Triggers, std::set<JUUID>() },
			{ SO_Boundaries, std::set<JUUID>() },
			{ SO_SceneControllers, std::set<JUUID>() },
		};
		std::map<SceneObjectType, std::set<JUUID>> currentObjects =
		{
			{ SO_Renderables, std::set<JUUID>() },
			{ SO_Lights, std::set<JUUID>() },
			{ SO_Cameras, std::set<JUUID>() },
			{ SO_SoundEffects, std::set<JUUID>() },
			{ SO_PhysicScenes, std::set<JUUID>() },
			{ SO_Triggers, std::set<JUUID>() },
			{ SO_Boundaries, std::set<JUUID>() },
			{ SO_SceneControllers, std::set<JUUID>() },
		};
		//delete
		std::map<SceneObjectType, std::set<JUUID>> toDelete =
		{
			{ SO_Renderables, std::set<JUUID>() },
			{ SO_Lights, std::set<JUUID>() },
			{ SO_Cameras, std::set<JUUID>() },
			{ SO_SoundEffects, std::set<JUUID>() },
			{ SO_PhysicScenes, std::set<JUUID>() },
			{ SO_Triggers, std::set<JUUID>() },
			{ SO_Boundaries, std::set<JUUID>() },
			{ SO_SceneControllers, std::set<JUUID>() },
		};
		//create
		std::map<SceneObjectType, std::set<JUUID>> toCreate =
		{
			{ SO_Renderables, std::set<JUUID>() },
			{ SO_Lights, std::set<JUUID>() },
			{ SO_Cameras, std::set<JUUID>() },
			{ SO_SoundEffects, std::set<JUUID>() },
			{ SO_PhysicScenes, std::set<JUUID>() },
			{ SO_Triggers, std::set<JUUID>() },
			{ SO_Boundaries, std::set<JUUID>() },
			{ SO_SceneControllers, std::set<JUUID>() },
		};
		std::map<SceneObjectType, std::map<JUUID, nlohmann::json&>> toCreateRefs =
		{
			{ SO_Renderables, std::map<JUUID,nlohmann::json&>() },
			{ SO_Lights, std::map<JUUID,nlohmann::json&>() },
			{ SO_Cameras, std::map<JUUID,nlohmann::json&>() },
			{ SO_SoundEffects, std::map<JUUID,nlohmann::json&>() },
			{ SO_PhysicScenes, std::map<JUUID,nlohmann::json&>() },
			{ SO_Triggers, std::map<JUUID,nlohmann::json&>() },
			{ SO_Boundaries, std::map<JUUID,nlohmann::json&>() },
			{ SO_SceneControllers, std::map<JUUID,nlohmann::json&>() },
		};
		//replacement
		std::map<SceneObjectType, std::map<JUUID, nlohmann::json&>> initialRefs =
		{
			{ SO_Renderables, std::map<JUUID,nlohmann::json&>() },
			{ SO_Lights, std::map<JUUID,nlohmann::json&>() },
			{ SO_Cameras, std::map<JUUID,nlohmann::json&>() },
			{ SO_SoundEffects, std::map<JUUID,nlohmann::json&>() },
			{ SO_PhysicScenes, std::map<JUUID,nlohmann::json&>() },
			{ SO_Triggers, std::map<JUUID,nlohmann::json&>() },
			{ SO_Boundaries, std::map<JUUID,nlohmann::json&>() },
			{ SO_SceneControllers, std::map<JUUID,nlohmann::json&>() },
		};
		std::map<SceneObjectType, std::map<JUUID, nlohmann::json&>> currentRefs =
		{
			{ SO_Renderables, std::map<JUUID,nlohmann::json&>() },
			{ SO_Lights, std::map<JUUID,nlohmann::json&>() },
			{ SO_Cameras, std::map<JUUID,nlohmann::json&>() },
			{ SO_SoundEffects, std::map<JUUID,nlohmann::json&>() },
			{ SO_PhysicScenes, std::map<JUUID,nlohmann::json&>() },
			{ SO_Triggers, std::map<JUUID,nlohmann::json&>() },
			{ SO_Boundaries, std::map<JUUID,nlohmann::json&>() },
			{ SO_SceneControllers, std::map<JUUID,nlohmann::json&>() },
		};

		auto gatherUUIDs = [](nlohmann::json& j, std::string so_type_name, std::set<JUUID>& uuids)
			{
				if (!j.contains(so_type_name)) return;

				nlohmann::json& jarr = j.at(so_type_name);
				for (unsigned int i = 0; i < jarr.size(); i++)
				{
					uuids.insert(jarr.at(i).at("uuid"));
				}
			};
		auto gatherUUIDsNotPresentInSecond = [](auto& first, auto& second, auto& uuidset)
			{
				for (auto uuid : first)
				{
					if (!second.contains(uuid))
						uuidset.insert(uuid);
				}
			};
		auto gatherRefs = [&](nlohmann::json& j, SceneObjectType type, std::string so_type_name, std::map<JUUID, nlohmann::json&>& refs, std::function<bool(JUUID)> reject)
			{
				if (!j.contains(so_type_name)) return;

				nlohmann::json& jarr = j.at(so_type_name);
				for (unsigned int i = 0; i < jarr.size(); i++)
				{
					JUUID uuid = jarr.at(i).at("uuid");
					if (reject(uuid))
						continue;
					refs.insert_or_assign(uuid, jarr.at(i));
				}
			};
		auto dump = [](std::map<JUUID, nlohmann::json&>& objects)
			{
				nlohmann::json arr = nlohmann::json::array();
				for (auto& [_, ref] : objects)
				{
					arr.push_back(ref);
				}
				return arr;
			};

		for (auto [type, name] : SceneObjectTypeJsonContainer)
		{
			gatherUUIDs(current, name, currentObjects.at(type));
			gatherUUIDs(initial, name, initialObjects.at(type));
		}

		//create the delete list
		for (auto& [type, uuidset] : toDelete)
		{
			gatherUUIDsNotPresentInSecond(currentObjects.at(type), initialObjects.at(type), uuidset);
		}

		//create the create list
		for (auto& [type, uuidset] : toCreate)
		{
			gatherUUIDsNotPresentInSecond(initialObjects.at(type), currentObjects.at(type), uuidset);
		}
		for (auto& [type, refMap] : toCreateRefs)
		{
			gatherRefs(initial, type, SceneObjectTypeJsonContainer.at(type), refMap, [&](JUUID uuid) { return !toCreate.at(type).contains(uuid); });
		}

		//create the replace list
		for (auto& [type, refMap] : initialRefs)
		{
			gatherRefs(initial, type, SceneObjectTypeJsonContainer.at(type), refMap, [&](JUUID uuid) {return toCreate.at(type).contains(uuid) || toDelete.at(type).contains(uuid); });
		}
		for (auto& [type, refMap] : currentRefs)
		{
			gatherRefs(current, type, SceneObjectTypeJsonContainer.at(type), refMap, [&](JUUID uuid) {return toCreate.at(type).contains(uuid) || toDelete.at(type).contains(uuid); });
		}

		//make the replacements patches
		std::map<JUUID, nlohmann::json> toReplace;
		for (auto& [type, initialRefMap] : initialRefs)
		{
			auto& currentRefMap = currentRefs.at(type);
			for (auto& [uuid, initialJ] : initialRefMap)
			{
				assert(currentRefMap.contains(uuid));

				auto& currentJ = currentRefMap.at(uuid);

				json diff = json::diff(currentJ, initialJ);

				if (diff.size() == 0ULL) continue;

				for (int i = 0; i < diff.size(); i++)
				{
					nlohmann::json& j = diff.at(i);
					assert(j.at("op") == "replace");
					std::string attribute = nostd::split(j.at("path"), "/").at(1);

					nlohmann::json patch;
					if (toReplace.contains(uuid))
					{
						patch = toReplace.at(uuid);
					}
					if (!std::set<std::string>({ "controllers","physicObject" }).contains(attribute))
					{
						patch[attribute] = initialJ.at(attribute);
						toReplace.insert_or_assign(uuid, patch);
					}
				}
			}
		}

		//no more sounds and now we can display the billboards again
		StopSounds(id);
		ShowBillboards(id);

		//apply the replacements and reset the controllers and physic objects states
		for (auto& [uuid, patch] : toReplace)
		{
			SceneObject* so = GetSceneObjectPointer(id, uuid);
			so->merge_patch(patch);
			so->SetInitialConditions();
			for (PhysicObjectID phO : GetPhysicsObjectsBySceneObjectUUID(so->SUuuid()))
			{
				phO->SetInitialConditions();
			}
		}

		//delete the scene objects
		for (auto& [type, uuidset] : toDelete)
		{
			for (auto& uuid : uuidset)
			{
				DeleteSceneObjectFromEditor(id, uuid);
			}
		}

		std::map<unsigned int, std::set<JUUID>> prioritySet = GetControllersPrioritySet(id, true);

		for (auto& [_, uuidset] : prioritySet)
		{
			for (auto& uuid : uuidset)
			{
				//GetController(uuid)->SetInitialConditions();
				GetController(uuid)->Map(GetControllerSUUUID(uuid));
			}
		}

		//create the missing objects
		if (std::any_of(toCreate.begin(), toCreate.end(), [](auto& pair) { return pair.second.size() != 0; }))
		{
			nlohmann::json level = {};
			for (auto& [type, _] : toCreateRefs)
			{
				level[SceneObjectTypeJsonContainer.at(type)] = dump(toCreateRefs.at(type));
			}

			AttachLevelIntoScene(id, "restore-objects-to-editor", level, [](SceneUnitId) {});
		}

		drawStaticBodies.at(id).EditorMode();
		drawDynamicBodies.at(id).EditorMode();
		drawCharacters.at(id).EditorMode();
		drawTriggers.at(id).EditorMode();
	}

	//Physics Objects drawing
	//Register
	bool StaticBodiesSceneUnitRegistered(SceneUnitId id)
	{
		return drawStaticBodies.contains(id);
	}
	bool DynamicBodiesSceneUnitRegistered(SceneUnitId id)
	{
		return drawDynamicBodies.contains(id);
	}
	bool CharactersSceneUnitRegistered(SceneUnitId id)
	{
		return drawCharacters.contains(id);
	}
	bool TriggersSceneUnitRegistered(SceneUnitId id)
	{
		return drawTriggers.contains(id);
	}

	//Should Draw
	bool TriggersShouldDraw(SceneUnitId id)
	{
		return drawTriggers.at(id).draw;
	}
	bool StaticBodiesShouldDraw(SceneUnitId id)
	{
		return drawStaticBodies.at(id).draw;
	}
	bool DynamicBodiesShouldDraw(SceneUnitId id)
	{
		return drawDynamicBodies.at(id).draw;
	}
	bool CharactersShouldDraw(SceneUnitId id)
	{
		return drawCharacters.at(id).draw;
	}

	//Switch drawing state
	void SwitchStaticBodiesDrawing(SceneUnitId id)
	{
		drawStaticBodies.at(id).SwitchDraw();
	}
	void SwitchDynamicBodiesDrawing(SceneUnitId id)
	{
		drawDynamicBodies.at(id).SwitchDraw();
	}
	void SwitchCharactersDrawing(SceneUnitId id)
	{
		drawCharacters.at(id).SwitchDraw();
	}
	void SwitchTriggersDrawing(SceneUnitId id)
	{
		drawTriggers.at(id).SwitchDraw();
	}

	//Physics Objects registration
	void RegisterStaticBody(PhysicObjectID phO)
	{
		drawStaticBodies.at(phO->unit()).levelPhysicObjects.insert(phO);
	}
	void RegisterDynamicBody(PhysicObjectID phO)
	{
		drawDynamicBodies.at(phO->unit()).levelPhysicObjects.insert(phO);
	}
	void RegisterCharacter(PhysicObjectID phO)
	{
		drawCharacters.at(phO->unit()).levelPhysicObjects.insert(phO);
	}
	void RegisterTrigger(PhysicObjectID phO)
	{
		drawTriggers.at(phO->unit()).levelPhysicObjects.insert(phO);
	}

	//Physics Objects unregistration
	void UnRegisterStaticBody(PhysicObjectID phO)
	{
		if (drawStaticBodies.at(phO->unit()).levelPhysicObjects.contains(phO))
			drawStaticBodies.at(phO->unit()).levelPhysicObjects.erase(phO);
	}
	void UnRegisterDynamicBody(PhysicObjectID phO)
	{
		if (drawDynamicBodies.at(phO->unit()).levelPhysicObjects.contains(phO))
			drawDynamicBodies.at(phO->unit()).levelPhysicObjects.erase(phO);
	}
	void UnRegisterCharacter(PhysicObjectID phO)
	{
		if (drawCharacters.at(phO->unit()).levelPhysicObjects.contains(phO))
			drawCharacters.at(phO->unit()).levelPhysicObjects.erase(phO);
	}
	void UnRegisterTrigger(PhysicObjectID phO)
	{
		if (drawTriggers.at(phO->unit()).levelPhysicObjects.contains(phO))
			drawTriggers.at(phO->unit()).levelPhysicObjects.erase(phO);
	}

	//Physics Objects list
	std::set<PhysicObjectID> GetStaticBodies(SceneUnitId id)
	{
		return drawStaticBodies.at(id).levelPhysicObjects;
	}
	std::set<PhysicObjectID> GetDynamicBodies(SceneUnitId id)
	{
		return drawDynamicBodies.at(id).levelPhysicObjects;
	}
	std::set<PhysicObjectID> GetCharacters(SceneUnitId id)
	{
		return drawCharacters.at(id).levelPhysicObjects;
	}
	std::set<PhysicObjectID> GetTriggers(SceneUnitId id)
	{
		return drawTriggers.at(id).levelPhysicObjects;
	}

	//Script Editor
	void StartScriptEdition(JObject* object, std::string attribute)
	{
		scriptEditModal.Init(object, attribute);
	}

	void OpenScriptBindingSelector(JObject* object, std::string attribute, int index, ScriptBinding sb)
	{
		using namespace Scene;

		JObject* selected = nullptr;
		if (!sb.uuid.empty())
		{
			selected = Scene::GetSceneObjectPointer(Editor::currentSceneUnitId, sb.uuid);
		}
		scriptBindingModal.Init(
			sb,
			selected,
			[]() { return GetSceneObjectsTypesList(currentSceneUnitId); },
			[](JUUID uuid) { return GetSceneObjectPointer(currentSceneUnitId, uuid); },
			[=](nlohmann::json json)
			{
				MarkSceneUnitAsModified(currentSceneUnitId);
				if (index == -1)
				{
					object->at(attribute).push_back(json);
				}
				else
				{
					object->at(attribute).at(index) = json;
				}
			}
		);
	}
};

