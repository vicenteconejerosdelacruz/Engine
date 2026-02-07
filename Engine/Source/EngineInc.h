#pragma once

#include "pch.h"
#include <algorithm>
#include "Engine.h"
#include "pch/NoStd.h"
#include "pch/NoMath.h"
#include "Common/StepTimer.h"
#include "Renderer/Renderer.h"
#include "Scene/Scene.h"
#include "Scene/Level.h"
//#include "Templates/Templates.h"
//#include "Templates/RenderPass/RenderPass.h"
//#include "Templates/Sound/Sound.h"
//#include "Scene/Camera/Camera.h"
//#include "Scene/Light/Light.h"
//#include "Scripting/Scripting.h"
#include "Controllers/Controller.h"
#if defined(_EDITOR)
#include "Editor/Editor.h"
#endif

extern std::unique_ptr<Renderer> renderer;
extern DX::StepTimer timer;
extern std::unique_ptr<DirectX::Mouse> mouse;
extern std::unique_ptr<DirectX::Keyboard> keyboard;
extern DirectX::Keyboard::KeyboardStateTracker keys;
extern std::unique_ptr<DirectX::GamePad> gamePad;
extern DirectX::GamePad::ButtonStateTracker buttons;
