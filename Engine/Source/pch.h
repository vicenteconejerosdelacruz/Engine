// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"

enum SceneObjectType {
	SO_None,
	SO_Renderables,
	SO_Lights,
	SO_Cameras,
	SO_SoundEffects,
	SO_PhysicScenes,
};

#include "pch/SceneUnitID.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <rpc.h>
// C RunTime Header Files
#include <stdlib.h>
#include <stdint.h>
#include <basetsd.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <wrl.h>
#include <wrl/client.h>
#include <atlbase.h>
#include <widemath.h>

//concurrency
#include <ppl.h>
#include <ppltasks.h>	// Para create_task
//#include <agile.h>

#if defined(_DEVELOPMENT)
#include <DirectXTex.h>
#endif
#include <DirectXMath.h>
#include <DirectXColors.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3d12shader.h>
#include "d3dx12.h"

#if defined(_DEBUG)
#include <dxgidebug.h>
#endif

#include <d3dcompiler.h>
#include <dxcapi.h>
#if defined(_DEVELOPMENT)
#include <pix3.h>
#endif

//DirectXTK stuff
#include <DDSTextureLoader.h>
#include <GamePad.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <Audio.h>
#include <SimpleMath.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <Effects.h>
#include <EffectPipelineStateDescription.h>

//std stuff
#include <string_view>
#include <memory>
#include <vector>
#include <map>
#include <concrt.h>
#include <string>
#include <unordered_set>
#include <filesystem>
#include <queue>
#include <stack>
#include <set>
#include <array>
#include <thread>
#include <chrono>
#include <ranges>
#include <any>
#include <ios>
#include <streambuf>
#include <fstream>
#include <iostream>
#include <ostream>
#include <istream>
#include <sstream>
#include <regex>
#include <limits>
#include <algorithm>
#include <random>
#include <iterator>
#include <cctype>
#include <locale>

#if defined(_EDITOR)
//#include <ShlObj.h>
#include <imgui.h>
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "Editor/IconsFontAwesome5.h"
#include "ImGuizmo.h"
#include "Editor/ImEditor.h"
#endif

//json
#include <nlohmann/json.hpp>
//tween
#include <tween.hpp>
//v8
#include <v8.h>
#include <libplatform/libplatform.h>
//v8pp
#include <v8pp/context.hpp>
#include <v8pp/module.hpp>

template<typename... Args> void whatis();
template<typename T> void whatis(T);

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::SimpleMath;
//using namespace Concurrency;

#include "pch/Application.h"
#include "pch/NoStd.h"
#include "pch/UUID.h"
#include "pch/Json.h"
#include "pch/DXTypes.h"
#include "pch/Debug.h"
#include "pch/ShaderMaterials.h"
#include "pch/RefTracker.h"
#include "pch/NoMath.h"
#if defined(_EDITOR)
#include "pch/JExposeEditor.h"
#endif
#include "pch/GameStateMachine.h"
#include "pch/GameEngineState.h"

template<>
struct std::hash<SUUUID>
{
	std::size_t operator()(const SUUUID& other) const
	{
		size_t hash = 0;
		nostd::hash_combine(hash, std::get<0>(other), std::get<1>(other));
		return hash;
	}
};