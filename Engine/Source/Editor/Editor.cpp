#include "pch.h"

#include <functional>
#include <atlbase.h>
#include <Editor.h>
#include <imgui.h>
#include <ImGuizmo.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include <IconsFontAwesome5.h>

#include <Application.h>
#include <Renderer.h>
#include <RenderPass/RenderPass.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <DeviceUtils/Resources/Resources.h>
#include <Renderable/Renderable.h>
#include <Camera/Camera.h>
#include <Light/Light.h>
#include <Sound/SoundFX.h>
#include <Templates.h>
#include <Sound/Sound.h>
#include <Textures/Texture.h>
#include <Shader/Shader.h>

#include <Level.h>
#include <Mouse.h>
#include <Keyboard.h>
#include <MousePicking.h>
#include <EditorMouseCamera.h>
#include <RightPanelComponent.h>
#include <CreatorModal.h>
#include <DeletePrompt.h>
#include <AnimationSequencerModal.h>

extern HWND hWnd;
extern RECT hWndRect;
extern std::unique_ptr<DirectX::Mouse> mouse;
extern std::unique_ptr<DirectX::Keyboard> keyboard;
extern std::unique_ptr<Renderer> renderer;
extern std::string gameAppTitle;
extern bool inSizeMove;
extern RECT GetMaximizedAreaSize();

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Editor
{
	std::string currentLevelName = defaultLevelName;
	bool levelModified = false;
	bool templatesModified = false;
	bool defaultLevel = true;

	bool initialized = false;
	bool maximized = true;
	bool mouseClicked = false;
	bool clickedInDragArea = false;
	bool menuBarItemClicked = false;
	int lastMouseX;
	int lastMouseY;

	RenderableUUID boundingBox;
	std::map<JUUID, RenderableUUID> billboardRegistry; //scene object -> renderable billboard
	std::set<RenderableUUID> billboardsToDestroy;

	float titleBH = static_cast<float>(ApplicationBarBottom);
	float panW = static_cast<float>(RightPanelWidth);
	ImGuiWindowFlags panFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	bool NonGameMode = false;
	bool lockedGameAreaInput = false;

	RightPanelComponent sceneObjectEdition("sceneObjects", { "hidden", "uuid" }, { "Scene Objects", "Details" }, { "Scene Objects" });
	RightPanelComponent templateEdition("templates", { "hidden", "uuid" }, { "Templates", "Details" }, { "Templates" });

	ImGui_ImplDX12_InitInfo init_info = {};

	enum MouseGameAreaMode
	{
		MOUSE_GAMEAREA_MODE_NONE,
		MOUSE_GAMEAREA_MODE_PICKING,
		MOUSE_GAMEAREA_MODE_GIZMO,
		MOUSE_GAMEAREA_MODE_CAMERA
	};
	MouseGameAreaMode currentMouseMode = MOUSE_GAMEAREA_MODE_NONE;
	MousePicking mousePicking;
	EditorMouseCamera mouseCamera;

	//Gizmos
	ImGuizmo::OPERATION gizmoOperation(ImGuizmo::TRANSLATE);
	ImGuizmo::MODE gizmoMode(ImGuizmo::WORLD);
	XMFLOAT4X4 gizmoCentroidMx;
	bool gizmoApplyOp = false;
	std::map<SceneObject*, XMFLOAT3> soRotation;
	std::map<SceneObject*, XMFLOAT3> soScale;
	std::map<SceneObject*, XMFLOAT3> so2bb;
	std::map<SceneObject*, XMFLOAT3> bb2gizmo;
	XMFLOAT3 gizmoRotation;
	XMFLOAT3 gizmoPosition;
	XMFLOAT3 gizmoScale;

	//Modals
	CreatorModal<SceneObjectType> sceneObjectModal;
	CreatorModal<TemplateType> templateModal;
	DeletePrompt deletePrompt;
	AnimationSequencerModal animationSequencer;

	//SceneObject selection
	std::set<JUUID> selectedSceneObjects;

	//Game Interaction
	bool isPlaying = false;
	bool isPaused = false;

	//Editor LifeCycle
	void InitEditor()
	{
		initialized = true;
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

	void DestroyEditor() {
		initialized = false;
		for (auto& uuid : sceneObjectEdition.editables)
		{
			SendEditorDestroyPreview(uuid, GetSceneObjectPointer);
		}
		for (auto& uuid : templateEdition.editables)
		{
			SendEditorDestroyPreview(uuid, GetJTemplatePointer);
		}
		ClearSceneObjectsSelection();
		sceneObjectEdition.Destroy();
		templateEdition.Destroy();

		//mousePicking.pickedObjects.clear();
		DestroyBillboards();
		DestroyPickingPass();
		DestroyRenderableBoundingBox();
		// Cleanup
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	bool WndProcHandlerEditor(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	}

	//Editor Drawing
	void DrawEditor(CameraUUID camera) {

		if (inSizeMove) return;

		unsigned int backBufferIndex = renderer->backBufferIndex;
		RenderPassInstanceUUID renderPass = renderer->swapChainPass;
		SwapChainPassUUID pass = renderPass->swapChainPass;
		auto backBuffer = pass->renderTargets[backBufferIndex];
		auto commandList = renderer->commandList;
		TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		// Start the Dear ImGui frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		Editor::NonGameMode = false;

		if (sceneObjectEdition.selectedNextFrame != "")
		{
			OpenTemplate(sceneObjectEdition.selectedNextFrame);
			sceneObjectEdition.selectedNextFrame = "";
		}

		if (templateEdition.selectedNextFrame != "")
		{
			OpenTemplate(templateEdition.selectedNextFrame);
			templateEdition.selectedNextFrame = "";
		}

		DrawApplicationBar();
		DrawGameController();

		if (!IsPlaying())
		{
			DrawRightPanel();

			if (!camera.empty())
				DrawPickedObjectsGizmo(camera);

			if (sceneObjectModal.creating)
				sceneObjectModal.DrawCreationPopup(SceneObjectsTypePanelMenuItems.at(sceneObjectModal.type));
			if (templateModal.creating)
				templateModal.DrawCreationPopup(TemplateTypePanelMenuItems.at(templateModal.type));
			if (deletePrompt.showing)
				deletePrompt.DrawPrompt("Delete Template");
			if (animationSequencer.showing)
			{
				static const float seqAdj = 8.0f;

				const ImGuiViewport* viewport = ImGui::GetMainViewport();
				ImVec2 seqPos = ImVec2(viewport->WorkSize.x / seqAdj, viewport->WorkSize.y / seqAdj);
				ImVec2 seqSize = ImVec2(viewport->WorkSize.x * (1.0f - (2.0f / seqAdj)), viewport->WorkSize.y * (1.0f - (2.0f / seqAdj)));
				animationSequencer.DrawSequencer("Animation Sequencer", seqPos, seqSize);
			}
		}
		// Rendering
		ImGui::Render();

		// Render Dear ImGui graphics
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), renderer->commandList);

		TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}

	void DrawApplicationBar()
	{
		RECT dragRect;
		ZeroMemory(&dragRect, sizeof(dragRect));
		dragRect.bottom = ApplicationBarBottom;

		auto quitEditor = []()
			{
				if ((Editor::levelModified && !Editor::defaultLevel) || Editor::templatesModified)
				{
					bool quitLevel = true;
					bool quitTemplate = true;
					if (Editor::levelModified)
					{
						int response = MessageBoxA(hWnd, "The level has been modified, do you wish to Save your work before leaving?", "Save before leaving?", MB_ICONWARNING | MB_YESNOCANCEL);
						switch (response) {
						case IDYES:
						{
							Editor::SaveLevelToFile(Editor::currentLevelName);
						}
						break;
						case IDCANCEL:
						{
							quitLevel = false;
						}
						break;
						}
					}
					if (Editor::templatesModified)
					{
						int response = MessageBoxA(hWnd, "The templates has been modified, do you wish to Save your work before leaving?", "Save before leaving?", MB_ICONWARNING | MB_YESNOCANCEL);
						switch (response) {
						case IDYES:
						{
							Editor::SaveTemplates();
						}
						break;
						case IDCANCEL:
						{
							quitTemplate = false;
						}
						break;
						}
					}

					if (quitLevel && quitTemplate)
					{
						PostMessageA(hWnd, WM_QUIT, 0, 0);
					}
				}
				else
				{
					PostMessageA(hWnd, WM_QUIT, 0, 0);
				}
			};

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::DrawItemWithEnabledState([]
					{
						if (ImGui::MenuItem(ICON_FA_FILE "New"))
						{
							Level::SetDefaultLevelToLoad();
							menuBarItemClicked = true;
						}
					},
					!IsPlaying()
				);

				ImGui::Separator();
				ImGui::DrawItemWithEnabledState([]
					{
						if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN "Open"))
						{
							OpenLevelFile();
							menuBarItemClicked = true;
						}
					},
					!IsPlaying()
				);

				ImGui::DrawItemWithEnabledState([]
					{
						if (ImGui::MenuItem(ICON_FA_SAVE "Save"))
						{
							SaveLevelToFile(currentLevelName);
							menuBarItemClicked = true;
						}
					},
					currentLevelName != "" && levelModified && !IsPlaying()
				);

				ImGui::DrawItemWithEnabledState([]
					{
						if (ImGui::MenuItem(ICON_FA_SAVE "Save As.."))
						{
							SaveLevelAs();
							menuBarItemClicked = true;
						}
					},
					!IsPlaying()
				);

				ImGui::Separator();
				ImGui::DrawItemWithEnabledState([]
					{
						if (ImGui::MenuItem(ICON_FA_SAVE "Save Templates"))
						{
							SaveTemplates();
							menuBarItemClicked = true;
						}
					}, templatesModified
				);
				ImGui::Separator();
				if (ImGui::MenuItem(ICON_FA_TIMES "Exit")) // It would be nice if this was a "X" like in the windows title bar set off to the far right
				{
					quitEditor();
				}
				ImGui::EndMenu();

			}

			auto cursorPos = ImGui::GetCursorScreenPos();
			dragRect.left = static_cast<LONG>(cursorPos.x);

			const ImGuiViewport* viewport = ImGui::GetMainViewport();

			std::string titleBar = gameAppTitle;
			if (currentLevelName != "")
			{
				titleBar += " - " + currentLevelName;
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
					.onClick = quitEditor
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
			if (!IsPlaying() || IsPaused())
			{
				if (ImGui::Button(ICON_FA_PLAY))
				{
					if (!IsPlaying())
					{
						SwitchToPlayMode();
					}
					else
					{
						SwitchToUnPausedMode();
					}
				}
			}
			else if (IsPlaying() && !IsPaused())
			{
				if (ImGui::Button(ICON_FA_PAUSE))
				{
					SwitchToPauseMode();
				}
			}
			ImGui::SameLine();
			ImGui::DrawItemWithEnabledState([]
				{
					if (ImGui::Button(ICON_FA_STOP))
					{
						SwitchToNonPlayMode();
					}
				},
				IsPlaying()
			);
		}
		ImGui::End();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
	}

	void OpenLevelFile()
	{
		using namespace Scene::Level;

		ImGui::OpenFile([](std::filesystem::path path)
			{
				std::filesystem::path jsonFilePath = path;
				jsonFilePath.replace_extension(".json");
				SetLevelToLoad(jsonFilePath.generic_string());
			},
			defaultLevelsFolder);
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

				SaveLevelToFile(nostd::WStringToString(jsonFilePath.filename()));
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

	std::string GetLevelString()
	{
		using namespace nlohmann;
		using namespace Scene;

		nlohmann::json level;

		level["renderables"] = json::array();
		level["lights"] = json::array();
		level["cameras"] = json::array();
		level["sounds"] = json::array();

		WriteRenderablesJson(level["renderables"]);
		WriteLightsJson(level["lights"]);
		WriteCamerasJson(level["cameras"]);
		WriteSoundFXsJson(level["sounds"]);

		std::string levelString = level.dump(4);
		return levelString;
	}

	void SaveLevelToFile(std::string levelFileName)
	{
		using namespace nlohmann;

		std::string levelString = GetLevelString();

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

		currentLevelName = levelFileName;
		defaultLevel = false;
		levelModified = false;
	}

	void SaveTemplates()
	{
		using namespace Templates;
		Templates::SaveTemplates(defaultTemplatesFolder, Shader::templateName, WriteShadersJson);
		Templates::SaveTemplates(defaultTemplatesFolder, Material::templateName, WriteMaterialsJson);
		Templates::SaveTemplates(defaultTemplatesFolder, Model3D::templateName, WriteModel3DsJson);
		Templates::SaveTemplates(defaultTemplatesFolder, Sound::templateName, WriteSoundsJson);
		Templates::SaveTemplates(defaultTemplatesFolder, Texture::templateName, WriteTexturesJson);
		Templates::SaveTemplates(defaultTemplatesFolder, RenderPass::templateName, WriteRenderPasssJson);
		templatesModified = false;
	}

	float separatorFactor = 0.0f;
	const float panelMinHeight = 47.0f;
	void DrawRightPanel() {

		auto matchSceneObjectsAttributes = []()
			{
				sceneObjectEdition.CreateEditableAttributesToMatch<SceneObjectType>(
					GetSceneObjectType,
					GetSceneObjectPointer,
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
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Right panel", (bool*)1, panFlags);
		{
			float halfY = panSize.y * 0.5f;

			float soPanelH = std::max(halfY * (1.0f - separatorFactor) - 2.5f, panelMinHeight);
			ImVec2 soPos = ImVec2(panPos.x, panPos.y);
			ImVec2 soSize = ImVec2(panSize.x, soPanelH);

			sceneObjectEdition.DrawPanel(soPos, soSize, SceneObjectsTypePanelMenuItems,
				GetSceneObjectsTypesList,
				[](JUUID uuid) { return GetSceneObjectPointer(uuid); },
				OnChangeSceneObjectTab,
				matchSceneObjectsAttributes,
				[](std::string uuid, bool selected) { SetSceneObjectSelection(uuid, selected); },
				[](std::string uuid) { SendEditorPreview(uuid, GetSceneObjectPointer, sceneObjectEdition.drawers); },
				[](SceneObjectType type) { StartSceneObjectCreation(type); },
				[](std::string uuid) { DeleteSceneObjectFromEditor(uuid); }
			);

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
				[](std::string uuid, bool selected) {},
				[](std::string uuid) { SendEditorPreview(uuid, GetJTemplatePointer, templateEdition.drawers); },
				[](TemplateType type) { StartTemplateCreation(type); },
				[](std::string uuid) { DeleteTemplate(uuid); }
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
		sceneObjectEdition.BuildAssetsTree(GetSceneObjectsTypesList, GetSceneObjectPointer);
		templateEdition.BuildAssetsTree(GetTemplatesTypesList, GetJTemplatePointer);
	}

	//SceneObjects Panel
	void OnChangeSceneObjectTab(std::string newTab)
	{
		sceneObjectEdition.selectedTab = newTab;
		sceneObjectEdition.editables = sceneObjectEdition.selected;

		if (newTab == sceneObjectEdition.detailAbleTabs.at(1))
		{
			sceneObjectEdition.CreateEditableAttributesToMatch<SceneObjectType>(
				GetSceneObjectType,
				GetSceneObjectPointer,
				GetSceneObjectAttributes,
				GetSceneObjectDrawers,
				GetSceneObjectPreviewers
			);
			for (auto& uuid : sceneObjectEdition.editables)
			{
				SendEditorPreview(uuid, GetSceneObjectPointer, sceneObjectEdition.drawers);
			}
		}
		else
		{
			for (auto& uuid : sceneObjectEdition.editables)
			{
				SendEditorDestroyPreview(uuid, GetSceneObjectPointer);
			}
		}
	}

	void OpenSceneObject(std::string uuid)
	{
		sceneObjectEdition.selected = { uuid };
		OnChangeSceneObjectTab(templateEdition.detailAbleTabs.at(1));
	}

	void OpenSceneObjectOnNextFrame(std::string uuid)
	{
		sceneObjectEdition.selectedNextFrame = uuid;
	}

	void MarkScenePanelAssetsAsDirty()
	{
		sceneObjectEdition.dirtyAssetsTree = true;
		levelModified = true;
	}

	void DestroyEditorSceneObjectsReferences()
	{
		ClearSceneObjectsSelection();
		gizmoOperation = ImGuizmo::TRANSLATE;
		gizmoMode = ImGuizmo::WORLD;
		sceneObjectEdition.Destroy();
		sceneObjectEdition.dirtyAssetsTree = true;
		sceneObjectEdition.selectedTab = sceneObjectEdition.detailAbleTabs.at(0);
		sceneObjectEdition.selected.clear();
		sceneObjectEdition.editables.clear();
		sceneObjectEdition.drawersOrder.clear();
		sceneObjectEdition.drawers.clear();
	}

	void DeleteFromScenePanelSelection(JUUID sceneObject)
	{
		if (sceneObjectEdition.selected.contains(sceneObject))
		{
			sceneObjectEdition.selected.erase(sceneObject);
			sceneObjectEdition.dirtyAssetsTree = true;
			sceneObjectEdition.selectedTab = sceneObjectEdition.detailAbleTabs.at(0);
		}
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

	void OpenTemplate(std::string uuid)
	{
		templateEdition.selected = { uuid };
		OnChangeTemplateTab(templateEdition.detailAbleTabs.at(1));
	}

	void OpenTemplateOnNextFrame(std::string uuid)
	{
		templateEdition.selectedNextFrame = uuid;
	}

	void MarkTemplatesPanelAssetsAsDirty()
	{
		templateEdition.dirtyAssetsTree = true;
		templatesModified = true;
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
	void OpenAnimationSequencer(std::string uuid)
	{
		animationSequencer.Initialize(uuid);
	}

	bool PendingAnimationSequencer()
	{
		return animationSequencer.initializing;
	}

	bool PendingAnimationSequencerDestruction()
	{
		return animationSequencer.destroying;
	}

	void LoadAnimationSequencer()
	{
		animationSequencer.LoadSceneObjects();
		animationSequencer.initializing = false;
	}

	void StepAnimationSequencer()
	{
		if (!animationSequencer.showing || animationSequencer.initializing || animationSequencer.destroying) return;

		animationSequencer.Step();
	}

	void DestroyAnimationSequencer()
	{
		animationSequencer.DestroySceneObjects();
		animationSequencer.destroying = false;
	}

	//Gizmos
	void ResetGizmoVariableWorkers()
	{
		gizmoApplyOp = false;
		soRotation.clear();
		soScale.clear();
		so2bb.clear();
		bb2gizmo.clear();
		gizmoRotation = XMFLOAT3();
		gizmoPosition = XMFLOAT3();
		gizmoScale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	}

	bool InteractWithGizmos(std::set<SceneObject*>& objects2Gizmo)
	{
		std::set<SceneObject*> objects;
		std::transform(selectedSceneObjects.begin(), selectedSceneObjects.end(), std::inserter(objects, objects.begin()), [](JUUID uuid)
			{
				return GetSceneObjectPointer(uuid);
			}
		);

		std::copy_if(objects.begin(), objects.end(), std::inserter(objects2Gizmo, objects2Gizmo.begin()), [](auto so)
			{
				return so->CanInteractWithGizmo(gizmoOperation);
			}
		);
		return !objects2Gizmo.empty();
	}

	void DrawPickedObjectsGizmo(CameraUUID camera)
	{
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

		std::set<SceneObject*> objects2Gizmo;
		if (!InteractWithGizmos(objects2Gizmo))
			return;

		auto translateObjects = [&objects2Gizmo](XMFLOAT4X4 view, XMFLOAT4X4 proj)
			{
				if (!gizmoApplyOp)
				{
					gizmoCentroidMx = GetBoundindBoxesCentroid(objects2Gizmo);
				}
				XMFLOAT4X4 delta;
				ImGuizmo::Manipulate(*view.m, *proj.m, gizmoOperation, gizmoMode, *gizmoCentroidMx.m, *delta.m, NULL, NULL, NULL);
				XMMATRIX XMdelta = XMLoadFloat4x4(&delta);
				XMVECTOR XMtranslation, XMrotation, XMscale;
				XMMatrixDecompose(&XMscale, &XMrotation, &XMtranslation, XMdelta);

				XMVECTOR len = XMVector3Length(XMtranslation);
				if (len.m128_f32[0] < g_XMEpsilon.f[0])
					return;

				gizmoApplyOp = true;
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
		auto rotateObjects = [&objects2Gizmo](XMFLOAT4X4 view, XMFLOAT4X4 proj)
			{
				//if the rotation gizmo has not been initialized, create a map of the initial rotations
				//vector from the bounding box to the object's position
				//and vector from the bounding box to the gizmo position
				if (!gizmoApplyOp)
				{
					gizmoCentroidMx = GetBoundindBoxesCentroid(objects2Gizmo);
					gizmoPosition = { gizmoCentroidMx._41, gizmoCentroidMx._42, gizmoCentroidMx._43 };
					gizmoApplyOp = true;

					for (auto& o : objects2Gizmo)
					{
						XMFLOAT3 p = ToXMFLOAT3(o->at("position"));
						XMFLOAT3 r = o->contains("rotation") ? ToXMFLOAT3(o->at("rotation")) : XMFLOAT3();
						BoundingBox bb = o->GetBoundingBox();

						soRotation.insert_or_assign(o, r);
						so2bb.insert_or_assign(o, p - bb.Center);
						bb2gizmo.insert_or_assign(o, bb.Center - gizmoPosition);
					}
				}

				XMFLOAT4X4 delta;
				ImGuizmo::Manipulate(*view.m, *proj.m, gizmoOperation, gizmoMode, *gizmoCentroidMx.m, *delta.m, NULL, NULL, NULL);
				XMMATRIX XMdelta = XMLoadFloat4x4(&delta);
				XMVECTOR XMtranslation, XMrotation, XMscale;
				XMMatrixDecompose(&XMscale, &XMrotation, &XMtranslation, XMdelta);

				gizmoRotation.x += XMConvertToDegrees(2.0f * XMrotation.m128_f32[0]);
				gizmoRotation.y += XMConvertToDegrees(2.0f * XMrotation.m128_f32[1]);
				gizmoRotation.z += XMConvertToDegrees(2.0f * XMrotation.m128_f32[2]);

				XMVECTOR rotQ = XMQuaternionRotationRollPitchYaw(
					XMConvertToRadians(gizmoRotation.x),
					XMConvertToRadians(gizmoRotation.y),
					XMConvertToRadians(gizmoRotation.z)
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
					XMFLOAT3 s2b = so2bb.at(o);
					XMFLOAT3 b2g = bb2gizmo.at(o);

					XMFLOAT3 ds2b = rotateXM3(s2b, rotQ);
					XMFLOAT3 db2g = rotateXM3(b2g, rotQ);

					XMFLOAT3 p = gizmoPosition + ds2b + db2g;
					XMFLOAT3 r = soRotation.at(o) + gizmoRotation;

					nlohmann::json patch = { { "position", FromXMFLOAT3(p) } };
					if (o->contains("rotation"))
						patch["rotation"] = FromXMFLOAT3(r);

					o->JUpdate(patch);
				}
			};
		auto scaleObjects = [&objects2Gizmo](XMFLOAT4X4 view, XMFLOAT4X4 proj)
			{
				if (!gizmoApplyOp)
				{
					gizmoCentroidMx = GetBoundindBoxesCentroid(objects2Gizmo);
				}
				XMFLOAT4X4 delta;
				ImGuizmo::Manipulate(*view.m, *proj.m, gizmoOperation, gizmoMode, *gizmoCentroidMx.m, *delta.m, NULL, NULL, NULL);
				XMMATRIX XMdelta = XMLoadFloat4x4(&delta);
				XMVECTOR XMtranslation, XMrotation, XMscale;
				XMMatrixDecompose(&XMscale, &XMrotation, &XMtranslation, XMdelta);

				XMVECTOR len = XMVector3Length(XMscale);
				if (len.m128_f32[0] < g_XMEpsilon.f[0])
					return;

				if (!gizmoApplyOp)
				{
					gizmoApplyOp = true;
					gizmoPosition = { gizmoCentroidMx._41, gizmoCentroidMx._42, gizmoCentroidMx._43 };

					for (auto& o : objects2Gizmo)
					{
						XMFLOAT3 p = ToXMFLOAT3(o->at("position"));
						XMFLOAT3 s = o->contains("scale") ? ToXMFLOAT3(o->at("scale")) : XMFLOAT3(1.0f, 1.0f, 1.0f);
						BoundingBox bb = o->GetBoundingBox();

						soScale.insert_or_assign(o, s);
						so2bb.insert_or_assign(o, p - bb.Center);
						bb2gizmo.insert_or_assign(o, bb.Center - gizmoPosition);
					}
				}

				gizmoScale.x *= XMscale.m128_f32[0];
				gizmoScale.y *= XMscale.m128_f32[1];
				gizmoScale.z *= XMscale.m128_f32[2];

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
					XMFLOAT3 s2b = so2bb.at(o);
					XMFLOAT3 b2g = bb2gizmo.at(o);

					XMFLOAT3 ds2b = scaleXM3(s2b, gizmoScale);
					XMFLOAT3 db2g = scaleXM3(b2g, gizmoScale);

					XMFLOAT3 p = gizmoPosition + ds2b + db2g;
					XMFLOAT3 s = soScale.at(o) * gizmoScale;

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

		BeginGizmoInteraction(camera, [&operators](XMFLOAT4X4 view, XMFLOAT4X4 proj)
			{
				operators.at(gizmoOperation)(view, proj);
			}
		);
	}

	void BeginGizmoInteraction(CameraUUID camera, std::function<void(XMFLOAT4X4 view, XMFLOAT4X4 proj)> interaction)
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
	void SelectSceneObject(JUUID uuid)
	{
		if (uuid == "" || (!boundingBox.empty() && boundingBox == uuid))
		{
			return;
		}

		RenderableUUID r = uuid;
		r->OnPick();
	}

	void SelectRenderable(JUUID ruuid)
	{
		ToggleSceneObjectFromSelection(ruuid);
	}

	void SelectLight(JUUID luuid)
	{
		ToggleSceneObjectFromSelection(luuid);
	}

	void SelectCamera(JUUID cuuid)
	{
		ToggleSceneObjectFromSelection(cuuid);
	}

	void SelectSoundEffect(JUUID suuid)
	{
		ToggleSceneObjectFromSelection(suuid);
	}

	void ToggleSceneObjectFromSelection(JUUID uuid)
	{
		if (!sceneObjectEdition.selected.contains(uuid))
		{
			InsertSceneObjectToSelection(uuid);
		}
		else
		{
			EraseSceneObjectFromSelection(uuid);
		}
		ResetGizmoVariableWorkers();
	}

	void SetSceneObjectSelection(JUUID uuid, bool selected)
	{
		if (selected)
		{
			selectedSceneObjects.insert(uuid);
		}
		else
		{
			selectedSceneObjects.erase(uuid);

		}
		gizmoApplyOp = false;
	}

	void InsertSceneObjectToSelection(JUUID uuid)
	{
		sceneObjectEdition.selected.insert(uuid);
		SetSceneObjectSelection(uuid, true);
	}

	void EraseSceneObjectFromSelection(JUUID uuid)
	{
		sceneObjectEdition.selected.erase(uuid);
		SetSceneObjectSelection(uuid, false);
	}

	void ClearSceneObjectsSelection()
	{
		selectedSceneObjects.clear();
		ResetGizmoVariableWorkers();
	}

	//BoundingBox
	bool RenderableBoundingBoxExists()
	{
		return !boundingBox.empty();
	}

	void CreateRenderableBoundingBox(CameraUUID camera)
	{
#if !defined(_EDITOR_BOUNDINGBOX)
		return;
#endif
		boundingBox = getUUID();
		nlohmann::json jbox = nlohmann::json(
			{
				{ "meshMaterials",
					{
						{
							{ "material", GetMaterialUUIDByName("BoundingBox") },
							{ "mesh", GetMeshUUIDByName("boxlines") }
						}
					}
				},
				{ "castShadows", false },
				{ "shadowed", false },
				{ "name" , "EditorBoundingBox" },
				{ "uuid" , boundingBox() },
				{ "position" , { 0.0f, 0.0f, 0.0f} },
				{ "topology", "LINELIST"},
				{ "rotation" , { 0.0, 0.0, 0.0 } },
				{ "scale" , { 1.0f, 1.0f, 1.0f } },
				{ "skipMeshes" , {}},
				{ "visible" , false},
				{ "hidden" , true},
				{ "cameras", { camera() }}
			}
		);
		CreateRenderable(jbox);
		boundingBox->BindToScene();
	}

	void DestroyRenderableBoundingBox()
	{
		if (!boundingBox.empty())
		{
			DeleteRenderableSceneObject(boundingBox());
		}
		boundingBox.clear();
	}

	void UpdateBoundingBox()
	{
		if (selectedSceneObjects.size() == 0ULL)
		{
			if (!boundingBox.empty())
			{
				boundingBox->visible(false);
			}
			return;
		}

		std::set<SceneObject*> objects;
		std::transform(selectedSceneObjects.begin(), selectedSceneObjects.end(), std::inserter(objects, objects.begin()), GetSceneObjectPointer);

		BoundingBox bb = GetContainedBoundingBox(objects);
		if (!boundingBox.empty())
		{
			boundingBox->visible(true);
			boundingBox->scale(bb.Extents);
			boundingBox->position(bb.Center);
			boundingBox->WriteConstantsBuffer();
		}
	}

	//Mouse Processing
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

	void GameAreaMouseProcessing(std::unique_ptr<DirectX::Mouse>& mouse, CameraUUID camera)
	{
		if (sceneObjectModal.creating || templateModal.creating || deletePrompt.showing || animationSequencer.showing) return;

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
					camera->MoveAlongFwAxis(fwMovement);
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
						camera->Rotate(dx, dy);
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
						camera->MovePerpendicularFwAxis(dx, dy);
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
	bool PickingPassExists()
	{
		return !mousePicking.pickingPass.empty();
	}

	void CreatePickingPass()
	{
#if !defined(_EDITOR_PICKINGPASS)
		return;
#endif
		mousePicking.pickingPass = CreateRenderPassInstance("", GetRenderPassUUIDByName("PickingPass"), 0, HWNDWIDTH, HWNDHEIGHT);
	}

	void DestroyPickingPass()
	{
		if (!mousePicking.pickingPass.empty()) {
			UnbindPickingRenderables();
			DestroyRenderPassInstance(mousePicking.pickingPass());
			mousePicking.pickingPass.clear();
		}
		if (mousePicking.pickingCpuBuffer)
		{
			mousePicking.pickingCpuBuffer = nullptr;
		}
	}

	void BindPickingRenderables()
	{
		for (auto uuid : GetRenderables())
		{
			BindRenderableToPickingPass(uuid);
		}
	}

	void BindRenderableToPickingPass(RenderableUUID r)
	{
		auto pass = mousePicking.pickingPass;
		r->CreateRenderPassMaterialsInstances(pass);
		r->CreateRenderPassConstantsBuffersInstances(pass);
		r->CreateRenderPassRootSignatures(pass);
		r->CreateRenderPassPipelineStates(pass);
	}

	void UnbindPickingRenderables()
	{
		for (auto uuid : GetRenderables())
		{
			UnbindRenderableFromPickingPass(uuid);
		}
	}

	void UnbindRenderableFromPickingPass(RenderableUUID r)
	{
		auto pass = mousePicking.pickingPass;
		r->DestroyRenderPassMaterialsInstances(pass);
		r->DestroyRenderPassConstantsBuffersInstances(pass);
		r->DestroyRenderPassRootSignatures(pass);
		r->DestroyRenderPassPipelineStates(pass);
	}

	void RenderPickingPass(CameraUUID camera)
	{
		if (camera.empty() || !mousePicking.doPicking || mousePicking.pickingPass.empty()) return;

#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, L"Scene Picker");
#endif

		mousePicking.pickingPass->renderToTexturePass->Pass([camera]()
			{
				unsigned int backBufferIndex = renderer->backBufferIndex;
				unsigned int objectId = 1U;
				for (RenderableUUID r : GetRenderables())
				{
					//OutputDebugStringA(("RenderPickingPass:" + r->name() + ":" + std::to_string(objectId) + "\n").c_str());
					if (!r->visible() || boundingBox == r->uuid()) continue;

					r->WriteConstantsBuffer("objectId", objectId, backBufferIndex);
					r->Render(mousePicking.pickingPass, camera);
					objectId++;
				}
			}
		);

#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif
	}

	void PickFromScene()
	{
		if (!mousePicking.doPicking) return;
		mousePicking.doPicking = false;
		currentMouseMode = MOUSE_GAMEAREA_MODE_NONE;

		if (mousePicking.pickingPass.empty()) return;

		unsigned int value = DeviceUtils::CapturePickingPixelValue(
			renderer->d3dDevice,
			renderer->commandQueue,
			mousePicking.pickingPass->renderToTexturePass->renderToTexture[0]->renderToTexture,
			mousePicking.pickingPass->renderToTexturePass->renderToTexture[0]->width * sizeof(unsigned int),
			mousePicking.pickingPass->renderToTexturePass->renderToTexture[0]->resourceDesc,
			mousePicking.pickingCpuBuffer,
			mousePicking.pickingX,
			mousePicking.pickingY,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);

		PickSceneObject(value);
	}

	void PickSceneObject(unsigned int pickedObjectId)
	{
		if (ImGuizmo::IsUsing())
		{
			return;
		}

		if (pickedObjectId == 0U)
		{
			SelectSceneObject("");
			return;
		}

		unsigned int objectId = 1U;
		for (RenderableUUID r : GetRenderables())
		{
			if (!r->visible() || r() == boundingBox) continue;

			if (pickedObjectId == objectId)
			{
				SelectSceneObject(r());
				break;
			}
			objectId++;
		}
	}

	void ReleasePickingPassResources()
	{
		if (!mousePicking.pickingPass.empty()) mousePicking.pickingPass->renderToTexturePass->ReleaseResources();
	}

	void ResizePickingPass(unsigned int width, unsigned int height)
	{
		if (!mousePicking.pickingPass.empty()) mousePicking.pickingPass->renderToTexturePass->Resize(width, height);
	}

	//JObjects Creation
	void StartSceneObjectCreation(SceneObjectType type)
	{
		sceneObjectModal.json = GetSceneObjectJson(type);
		sceneObjectModal.atts = GetSceneObjectRequiredAttributes(type);
		sceneObjectModal.drawers = GetSceneObjectCreatorDrawers(type);
		sceneObjectModal.validators = GetSceneObjectValidators(type);
		sceneObjectModal.type = type;
		sceneObjectModal.creating = true;
		sceneObjectModal.onCreate = CreateSceneObject;
	}

	void StartTemplateCreation(TemplateType type)
	{
		templateModal.json = GetTemplateJson(type);
		templateModal.modalProperties = GetTemplateCreationModalProperties(type);
		templateModal.atts = GetTemplateRequiredAttributes(type);
		templateModal.drawers = GetTemplateCreatorDrawers(type);
		templateModal.validators = GetTemplateValidators(type);
		templateModal.type = type;
		templateModal.creating = true;
		templateModal.onCreate = CreateTemplate;
	}

	JUUID CreateBillboardFromMaterials(CameraUUID camera, std::string name, std::string material, std::string pickingMaterial)
	{
		std::string jname = name;
		jname += "-billboard";
		JUUID uuid = getUUID();
		nlohmann::json jbillboard = nlohmann::json(
			{
				{ "meshMaterials",
					{
						{
							{ "material", GetMaterialUUIDByName(material) },
							{ "mesh", GetMeshUUIDByName("decal") }
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
				{ "hidden" , true},
				{ "cameras", { camera() }},
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
		CreateRenderable(jbillboard);
		return uuid;
	}

	void RegisterBillboard(JUUID sceneObject)
	{
#if !defined(_EDITOR_BILLBOARD)
		return;
#endif
		billboardRegistry.insert_or_assign(sceneObject, "");
	}

	JUUID GetBillboard(JUUID sceneObject)
	{
		return billboardRegistry.contains(sceneObject) ? billboardRegistry.at(sceneObject)() : "";
	}

	void DestroyBillboard(JUUID sceneObject)
	{
		JUUID billboard = GetBillboard(sceneObject);
		if (billboard.empty())
			return;

		billboardsToDestroy.insert(billboard);
		billboardRegistry.erase(sceneObject);
	}

	void CreateRegisteredBillboards(CameraUUID camera)
	{
		for (auto it = billboardRegistry.begin(); it != billboardRegistry.end(); it++)
		{
			if (!it->second.empty()) continue;

			auto so = GetSceneObjectPointer(it->first);
			if (so->contains("hidden") && so->at("hidden") == true) continue;

			it->second = so->CreateBillboard(camera);
			if (!it->second.empty())
			{
				auto& bb = it->second;
				bb->BindToScene();
				Editor::BindRenderableToPickingPass(bb);
			}
		}
	}

	bool PendingBillboards()
	{
		for (auto it = billboardRegistry.begin(); it != billboardRegistry.end(); it++)
		{
			if (it->second.empty()) return true;
		}
		return false;
	}

	bool PendingBillboardsDestruction()
	{
		return billboardsToDestroy.size() > 0ULL;
	}

	void UpdateBillboards()
	{
		if (GetCountFromMouseCameras() == 0ULL) return;

		for (auto it = billboardRegistry.begin(); it != billboardRegistry.end(); it++)
		{
			if (it->second.empty()) continue;

			auto so = GetSceneObjectPointer(it->first);
			so->UpdateBillboard(it->second());
		}
	}

	void Editor::DestroyPendingBillboards()
	{
		for (auto b : billboardsToDestroy)
		{
			Editor::UnbindRenderableFromPickingPass(b);
			EraseRenderableFromRenderables(b());
			DeleteRenderableSceneObject(b());
		}
		billboardsToDestroy.clear();
	}

	void DestroyBillboards()
	{
		for (auto it = billboardRegistry.begin(); it != billboardRegistry.end(); )
		{
			if (it->second() != "")
			{
				Editor::UnbindRenderableFromPickingPass(it->second);
				EraseRenderableFromRenderables(it->second());
				DeleteRenderableSceneObject(it->second());
			}
			it = billboardRegistry.erase(it);
		}
		billboardsToDestroy.clear();
	}

	void ClearBillboardsRegistry()
	{
		billboardRegistry.clear();
		billboardsToDestroy.clear();
	}

	bool IsPlaying()
	{
		return isPlaying;
	}

	bool IsPaused()
	{
		return isPaused;
	}

	void SwitchToPlayMode()
	{
		isPlaying = true;
		Scene::PlaySounds();
	}

	void SwitchToPauseMode()
	{
		isPaused = true;
		Scene::PauseSounds();
	}

	void SwitchToUnPausedMode()
	{
		isPaused = false;
		Scene::ResumeSounds();
	}

	void SwitchToNonPlayMode()
	{
		isPlaying = false;
		isPaused = false;
		Scene::StopSounds();
	}
};

