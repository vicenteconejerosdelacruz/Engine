#include "pch.h"
#include "Engine.h"
#include <Renderer.h>
#include <Scripting.h>
#include <Physics.h>
#include <UI.h>
#include <AudioSystem.h>
#include <ShaderCompiler.h>
#include <Templates.h>
#include <Game.h>
#if defined(_EDITOR)
#include <Editor.h>
#endif
#include <StepTimer.h>
#include <locale>

#include "GameDecl.h"

using namespace Templates;
using namespace Scene;
using namespace AudioSystem;
using namespace ShaderCompiler;
using namespace Scripting;
using namespace Physics;
using namespace UI;
using namespace Game;
#if defined(_EDITOR)
using namespace Editor;
#endif

namespace Scene
{
	extern void SceneRender();
	extern void ScenePostRender();
	extern void SceneObjectsStep(DX::StepTimer& timer);
	extern void DestroyScenes(bool inmediate);
};

//take me out from here
extern std::string gameAppTitle;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND hWnd;                                      // Window hWnd
RECT hWndRect;									// Window Rect (this can change so if you change the size of the window please rewrite this value)
HWND desktopHwnd;								// Desktop hWnd (how do you enumerate desktops with this?)
RECT desktopRect;								// Desktop Rect								
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
bool appDone = false;
bool inSizeMove = false;
bool resizeWindow = false;
bool minimized = false;
#if defined(_DEVELOPMENT)
bool inFullScreen = false;
#else
bool inFullScreen = true;
#endif
#if defined(_EDITOR)
bool editorPlayMode = false;
#endif
extern std::string gameAppTitle;
extern float gameUpdateFrequency;

std::unique_ptr<JRenderer> renderer;

//FPS
DX::StepTimer timer;

//Mouse
std::unique_ptr<DirectX::Mouse> mouse;
//Keyboard
std::unique_ptr<DirectX::Keyboard> keyboard;
DirectX::Keyboard::KeyboardStateTracker keys;
//GamePad
std::unique_ptr<DirectX::GamePad> gamePad;
DirectX::GamePad::ButtonStateTracker buttons;
//Input swap
GameInteractionMode gameInteractionMode = GIM_Gamepad;
extern bool canSwapInteractionMode;
std::map<JUUID, std::function<void(JUUID)>> onKeyboardMouseInputDetected;
std::map<JUUID, std::function<void(JUUID)>> onGamepadInputDetected;

//app destruction
bool destroyed = false;

RECT GetMaximizedAreaSize()
{
	HMONITOR monitor = MonitorFromWindow(desktopHwnd, MONITOR_DEFAULTTONULL);
	MONITORINFO monitor_info{};
	monitor_info.cbSize = sizeof(monitor_info);
	GetMonitorInfoW(monitor, &monitor_info);
	return monitor_info.rcWork;
}

// Guarda el tamaño original de la ventana al estar en modo Windowed
static WINDOWPLACEMENT g_wpPrev = { sizeof(WINDOWPLACEMENT) };

void ResetWindowStyle(bool fullscreen)
{
#if !defined(_EDITOR)
	DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);

	if (fullscreen)
	{
		// Guardar las coordenadas de ventana normal solo si NO está en Popup actualmente
		if ((dwStyle & WS_POPUP) == 0)
		{
			GetWindowPlacement(hWnd, &g_wpPrev);
		}

		MONITORINFO mi = { sizeof(mi) };
		HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

		if (GetMonitorInfoW(hMonitor, &mi))
		{
			// Quitar bordes para Borderless Fullscreen
			SetWindowLong(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
			SetWindowLong(hWnd, GWL_EXSTYLE, 0);

			SetWindowPos(hWnd, HWND_TOP,
				mi.rcMonitor.left,
				mi.rcMonitor.top,
				mi.rcMonitor.right - mi.rcMonitor.left,
				mi.rcMonitor.bottom - mi.rcMonitor.top,
				SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else
	{
		// Restaurar estilo normal de ventana
		SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);

		// Restaurar posición original desde el placement guardado
		SetWindowPlacement(hWnd, &g_wpPrev);

		SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}

	ShowWindow(hWnd, SW_SHOW);
#else
	SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);
	SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

	ShowWindow(hWnd, SW_NORMAL);
	RECT winR = GetMaximizedAreaSize();
	SetWindowPos(hWnd, HWND_TOP, winR.left, winR.top, winR.right, winR.bottom, SWP_NOZORDER | SWP_FRAMECHANGED);
#endif
}

//CREATE
#if !defined(_EDITOR) && defined(_DEVELOPMENT)
int EngineConsoleMain()
{
	return EngineWinMain(GetModuleHandle(NULL), nullptr, nullptr, 0);
}
#endif

int APIENTRY EngineWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	SetThreadDescription(GetCurrentThread(), L"Main Thread");

	timer.SetFixedTimeStep(true);
	timer.SetTargetElapsedSeconds(gameUpdateFrequency);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	desktopHwnd = GetDesktopWindow();
	GetClientRect(desktopHwnd, &desktopRect);

	// Initialize global strings
	LoadStringW(hInstance, IDC_CULPEOENGINE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	//Get the Scripting's v8::Isolate*
	v8::Isolate* isolate = Scripting::GetIsolate();

	//create an isolate and a handle scope for the application lifetime
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);

	//Initialize the physics
	InitializePhysics();

#if !defined(_EDITOR)
	BootGame();
#endif

	// Main loop
	while (!appDone)
	{
		// Poll and handle messages (inputs, window resize, etc.)
		// See the WndProc() function below for our to dispatch events to the Win32 backend.
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				appDone = true;
		}
		if (appDone)
			break;

		AppStep();
	}
	DestroyInstance();

	return 0;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CULPEOENGINE));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	// 1. Inicializar la librería COM antes de cualquier llamada a mandos/audio
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		return FALSE;
	}

	hInst = hInstance;

#if defined(_EDITOR)
	RECT winR = GetMaximizedAreaSize();
	hWnd = CreateWindowW(szWindowClass, nostd::StringToWString(gameAppTitle).c_str(),
		WS_OVERLAPPEDWINDOW, winR.left, winR.top, winR.right, winR.bottom,
		nullptr, nullptr, hInstance, nullptr);
#else
	int initWidth = 1280;
	int initHeight = 720;
	int posX = (GetSystemMetrics(SM_CXSCREEN) - initWidth) / 2;
	int posY = (GetSystemMetrics(SM_CYSCREEN) - initHeight) / 2;

	hWnd = CreateWindowW(szWindowClass, nostd::StringToWString(gameAppTitle).c_str(),
		WS_OVERLAPPEDWINDOW, posX, posY, initWidth, initHeight,
		nullptr, nullptr, hInstance, nullptr);
#endif

	if (!hWnd) return FALSE;

	// 2. Mostrar la ventana brevemente para asegurar que los rectángulos nativos de Win32 estén construidos
	ShowWindow(hWnd, nCmdShow);

	// 3. Guardar la geometría Windowed de partida
	GetWindowPlacement(hWnd, &g_wpPrev);

	// 4. Aplicar el modo de ventana / pantalla completa
	ResetWindowStyle(inFullScreen);

	UpdateWindow(hWnd);
	GetClientRect(hWnd, &hWndRect);

	//initialize input helpers
	mouse = std::make_unique<Mouse>();
	mouse->SetWindow(hWnd);

	keyboard = std::make_unique<Keyboard>();

	gamePad = std::make_unique<GamePad>();
	buttons.Reset();

	BuildShaderCompiler();
#if defined(_DEVELOPMENT)
	MonitorShaderChanges(defaultShadersFolder);
#endif

	//Initialize the audio system
	InitAudio();

	//Initialize the v8 scripting
	InitScripting(std::filesystem::current_path().string().c_str());

	//Initialize the UI system
	InitUI(defaultUIFolder);

	//create the templates
	CreateSystemTemplates();
	CreateTemplates();

	//initialize the render and reset the commands
	renderer = std::make_unique<JRenderer>();
	renderer->Initialize(hWnd);
	//create the editor and a default scene
#if defined(_EDITOR)
	InitEditor();
#endif
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#if defined(_EDITOR)
	std::set<UINT> nonEditorMessages = { WM_QUIT };
	if (nonEditorMessages.contains(message)) DefWindowProc(hWnd, message, wParam, lParam);
	if (WndProcHandlerEditor(hWnd, message, wParam, lParam)) return true;
#endif

	switch (message)
	{
	case WM_ACTIVATE:
	case WM_ACTIVATEAPP:
	{
		Keyboard::ProcessMessage(message, wParam, lParam);
		Mouse::ProcessMessage(message, wParam, lParam);
	}
	break;
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
	{
		Mouse::ProcessMessage(message, wParam, lParam);
	}
	break;
	case WM_SYSKEYDOWN:
	{
		if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
		{
#if !defined(_EDITOR)
			inFullScreen = !inFullScreen;

			if (renderer)
			{
				renderer->Flush();
			}

			ResetWindowStyle(inFullScreen);
			resizeWindow = true;
			return 0;
#endif
		}
		else if (wParam == VK_F4 && (lParam & (1 << 29)))
		{
			appDone = true;
			return 0;
		}
		Keyboard::ProcessMessage(message, wParam, lParam);
	}
	break;
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		Keyboard::ProcessMessage(message, wParam, lParam);
	}
	break;
	case WM_PAINT:
	{
		if (inSizeMove && renderer)
		{
			AppStep();
		}
		else
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
	}
	break;
	case WM_DESTROY:
	{
		DestroyInstance();
		PostQuitMessage(0);
	}
	break;
	case WM_ENTERSIZEMOVE:
	{
		inSizeMove = true;
	}
	break;
	case WM_EXITSIZEMOVE:
	{
		inSizeMove = false;
		if (renderer)
		{
			GetClientRect(hWnd, &hWndRect);
			resizeWindow = true;
		}
	}
	break;
	case WM_SIZE:
	{
		if (renderer)
		{
			resizeWindow = true;
			minimized = (LOWORD(lParam) == 0U && HIWORD(lParam) == 0U);
		}
	}
	break;
	case WM_NCCALCSIZE:
	{
		if (wParam == TRUE)
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_SYSCOMMAND:
	{
		if (wParam == SC_MINIMIZE) {}
		if (wParam == SC_CLOSE) { appDone = true; return 0; }
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	break;
#if defined(_EDITOR)
	case WM_NCHITTEST:
	{
		POINTS pt = MAKEPOINTS(lParam);
		RECT& r = hWndRect;

		bool leftBorder = nostd::in_between(pt.x, r.left, r.left + 5);
		bool rightBorder = nostd::in_between(pt.x, r.right - 5, r.right);
		bool topBorder = nostd::in_between(pt.y, r.top, r.top + 5);
		bool bottomBorder = nostd::in_between(pt.y, r.bottom - 5, r.bottom);

		std::map<std::tuple<bool, bool, bool, bool>, LRESULT> htMap = {
			{ std::tuple(true,false,false,false), HTLEFT },
			{ std::tuple(false,true,false,false), HTRIGHT },
			{ std::tuple(false,false,true,false), HTTOP },
			{ std::tuple(false,false,false,true), HTBOTTOM },
			{ std::tuple(true,false,false,true), HTBOTTOMLEFT },
			{ std::tuple(false,true,false,true), HTBOTTOMRIGHT },
			{ std::tuple(true,false,false,true), HTTOPLEFT },
			{ std::tuple(false,true,false,true), HTTOPRIGHT },
		};

		std::tuple htTuple = std::tuple(leftBorder, rightBorder, topBorder, bottomBorder);
		return (htMap.contains(htTuple) ? htMap.at(htTuple) : HTCLIENT);
	}
	break;
#endif
	default:
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	break;
	}
	return 0;
}

static void CheckInputSwap()
{
	if (!canSwapInteractionMode) return;

	const float JOYSTICK_THRESHOLD = 0.2f; // Ignora movimientos menores al 20%
	const float MOUSE_THRESHOLD = 2.0f;    // Pixeles mínimos de movimiento del mouse

	auto gamepadState = gamePad->GetState(0);
	auto mouseState = mouse->GetState();
	auto kbState = keyboard->GetState();

	if (gameInteractionMode == GIM_KeyboardMouse && gamepadState.IsConnected())
	{
		// Verificar si se movieron los sticks o gatillos más allá del umbral
		bool stickMoved = abs(gamepadState.thumbSticks.leftX) > JOYSTICK_THRESHOLD ||
			abs(gamepadState.thumbSticks.leftY) > JOYSTICK_THRESHOLD ||
			abs(gamepadState.thumbSticks.rightX) > JOYSTICK_THRESHOLD ||
			abs(gamepadState.thumbSticks.rightY) > JOYSTICK_THRESHOLD;

		bool triggerPressed = gamepadState.triggers.left > JOYSTICK_THRESHOLD ||
			gamepadState.triggers.right > JOYSTICK_THRESHOLD;

		// Verificar si se presionó cualquier botón físico
		// Nota: Necesitas comparar el estado actual con el tracker o evaluar la máscara de botones
		bool buttonPressed = (
			gamepadState.buttons.a || gamepadState.buttons.b || gamepadState.buttons.x || gamepadState.buttons.y ||
			gamepadState.dpad.up || gamepadState.dpad.down || gamepadState.dpad.left || gamepadState.dpad.right ||
			gamepadState.buttons.leftShoulder || gamepadState.buttons.rightShoulder ||
			gamepadState.buttons.back || gamepadState.buttons.menu || gamepadState.buttons.start || gamepadState.buttons.view ||
			gamepadState.buttons.leftStick || gamepadState.buttons.rightStick
			);

		if (stickMoved || triggerPressed || buttonPressed)
		{
			gameInteractionMode = GIM_Gamepad;
			std::for_each(onGamepadInputDetected.begin(), onGamepadInputDetected.end(), [](auto& pair)
				{
					pair.second(pair.first);
				}
			);
		}
	}
	else if (gameInteractionMode == GIM_Gamepad)
	{
		// Verificar si el mouse se movió significativamente
		// (Asegúrate de limpiar o acumular el estado relativo X/Y del mouse)
		bool mouseMoved = false;// do later abs(mouseState.x) > MOUSE_THRESHOLD || abs(mouseState.y) > MOUSE_THRESHOLD;

		// Verificar botones del mouse o cualquier tecla presionada
		bool mouseClicked = mouseState.leftButton || mouseState.rightButton;

		auto IsKeyboardPressed = [](const DirectX::Keyboard::State& state)
			{
				static const DirectX::Keyboard::State emptyState = {};

				// Compara la memoria del estado actual con un estado vacío
				return memcmp(&state, &emptyState, sizeof(DirectX::Keyboard::State)) != 0;
			};

		// DirectXTK Keyboard permite verificar si hay alguna tecla activa de forma eficiente
		// O puedes usar los mensajes de Windows WM_KEYDOWN
		bool keyboardPressed = IsKeyboardPressed(kbState);
		// Una forma rápida con DirectXTK es revisar si el estado del teclado no está vacío:
		// (Puedes implementar una función auxiliar que revise el buffer de teclas)

		if (mouseMoved || mouseClicked || keyboardPressed)
		{
			gameInteractionMode = GIM_KeyboardMouse;
			std::for_each(onKeyboardMouseInputDetected.begin(), onKeyboardMouseInputDetected.end(), [](auto& pair)
				{
					pair.second(pair.first);
				}
			);
		}
	}
}

#if defined(_EDITOR)
extern bool restoringPlayMode;
#elif defined(_DEVELOPMENT)
bool resetAppStepTick = false;
#endif

void AppStep()
{
	std::locale::global(std::locale("C"));

	if (minimized) return;

	if (!inFullScreen)
	{
		GetClientRect(hWnd, &hWndRect);
	}

	if (resizeWindow && !inSizeMove) {
		return ResizeWindow();
	}

#if defined(_DEVELOPMENT) && !defined(_EDITOR)
	if (resetAppStepTick)
	{
		resetAppStepTick = false;
		timer.ResetElapsedTime();
	}
#endif

#if defined(_DEVELOPMENT)
	PIXScopedEvent(0, L"AppStep");
#endif

	SceneUnitsStep();
	UpdateAudio();
#if defined(_EDITOR)
	EditorStep();
#endif
	CheckInputSwap();
#if defined(_DEVELOPMENT)
	unsigned int stepCount = 0U;
#endif
	timer.Tick([&]()
		{
#if defined(_EDITOR)
			if (restoringPlayMode) return;
#endif
#if defined(_DEVELOPMENT)
			std::string event = std::string("timer.Tick:") + std::to_string(stepCount) + "\n";
			PIXScopedEvent(0, nostd::StringToWString(event).c_str());
#endif
			FetchPhysicsScenesResults(timer);
			TemplatesStep(timer);
			GameStep();
			SceneObjectsStep(timer);
			StepControllers(timer);
			SimulatePhysicScenes(timer);
#if defined(_DEVELOPMENT)
			stepCount++;
#endif
		}
	);
	UIStep();
	Render();
	LoadingProcessorsStep();
}

void Render()
{
	using namespace Scene;
	RunComputeShaders();
	SceneRender();
	renderer->Present();
	SolveComputeShaders();
	ScenePostRender();
}

void ResizeWindow()
{
	using namespace Scene;

	resizeWindow = false;

	if (!renderer) return;

	renderer->Flush();

	ResizeReleaseScenePasses();
	UI::ResizeReleaseUI();
	renderer->swapChainPass->ResizeRelease();

	GetClientRect(hWnd, &hWndRect);
	UINT newWidth = hWndRect.right - hWndRect.left;
	UINT newHeight = hWndRect.bottom - hWndRect.top;

	if (newWidth == 0 || newHeight == 0) return;

	renderer->Resize(newWidth, newHeight);
	renderer->swapChainPass->Resize(newWidth, newHeight);

	UI::ResizeUI(newWidth, newHeight);

	renderer->UpdateViewportPerspective();
	ResizeScenePasses(newWidth, newHeight);
}

//DESTROY
void DestroyInstance()
{
	using namespace Scene;
	if (destroyed) return;
	renderer->Flush();

#if defined(_EDITOR)
	DestroyEditor();
#endif

	DestroyScenes(true);
	DestroyControllers();
	DestroyPhysics();
	DestroyTemplatesInstances();
	DestroyTemplates();

	CoUninitialize();
	destroyed = true;
}