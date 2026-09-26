cmake_minimum_required(VERSION 3.20)
project(Engine LANGUAGES CXX)

# ---------------------------------------------------------------------------
# 1. Definiciones del Preprocesador (Defines)
# ---------------------------------------------------------------------------
set(DEFINES_EDITOR_DEBUG
    _DEBUG _WINDOWS _DEVELOPMENT HAS_DIRECTXTK12 _EDITOR
    DIRECTX_TEX_VERSION=207 TOOL_VERSION=DIRECTX_TEX_VERSION NOMINMAX
    V8_COMPRESS_POINTERS V8_31BIT_SMIS_ON_64BIT_ARCH
)

set(DEFINES_EDITOR_DEVELOPMENT
    NDEBUG _WINDOWS _DEVELOPMENT HAS_DIRECTXTK12 _EDITOR
    DIRECTX_TEX_VERSION=207 TOOL_VERSION=DIRECTX_TEX_VERSION NOMINMAX
    V8_COMPRESS_POINTERS V8_31BIT_SMIS_ON_64BIT_ARCH PROFILE
)

set(DEFINES_EDITOR_RELEASE
    NDEBUG _WINDOWS _DEVELOPMENT HAS_DIRECTXTK12 _EDITOR
    DIRECTX_TEX_VERSION=207 TOOL_VERSION=DIRECTX_TEX_VERSION NOMINMAX
    V8_COMPRESS_POINTERS V8_31BIT_SMIS_ON_64BIT_ARCH
)

set(DEFINES_DEBUG
    _DEBUG _WINDOWS _DEVELOPMENT HAS_DIRECTXTK12
    DIRECTX_TEX_VERSION=207 TOOL_VERSION=DIRECTX_TEX_VERSION NOMINMAX
    V8_COMPRESS_POINTERS V8_31BIT_SMIS_ON_64BIT_ARCH PROFILE
)

set(DEFINES_DEVELOPMENT
    NDEBUG _WINDOWS _DEVELOPMENT HAS_DIRECTXTK12
    DIRECTX_TEX_VERSION=207 TOOL_VERSION=DIRECTX_TEX_VERSION NOMINMAX
    V8_COMPRESS_POINTERS V8_31BIT_SMIS_ON_64BIT_ARCH PROFILE
)

set(DEFINES_RELEASE
    NDEBUG _WINDOWS HAS_DIRECTXTK12 NOMINMAX
    V8_COMPRESS_POINTERS V8_31BIT_SMIS_ON_64BIT_ARCH
)

# ---------------------------------------------------------------------------
# 2. Directorios de Inclusión (Includes)
# ---------------------------------------------------------------------------
set(INCLUDES_SDK
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/dxc/dxc_2024_07_31/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/assimp/assimp-6.0.5/build/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/assimp/assimp-6.0.5/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/DirectXTK12/Inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/imgui/backends"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/imguizmo"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/tween"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/DirectXTex/DirectXTex"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/DirectXTex/Texconv"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/DirectXTex/Texdiag"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/DirectXTex/Texassemble"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/DirectXTex/Common"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/DirectXTex/CmdLineHelpers"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/v8pp-2.1.1/packages/v8-v143-x64.9.8.177.4/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/v8pp-2.1.1"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/v8pp-2.1.1/out"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/PhysX/physx/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/ultralight/ultralight-free-sdk-1.4.0-win-x64/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/yaml-cpp-yaml-cpp-0.9.0/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/nlohmann.json/build/native/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/WinPixEventRuntime/Include/WinPixEventRuntime"
)

set(INCLUDES_COMMON
    ${INCLUDES_SDK}
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/pch"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/JExpose"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/JExpose/Scene"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/JExpose/Templates"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/JExpose/Primitives"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/JExpose/Physics"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/JExpose/Controllers"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Effects"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Animation"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Audio"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Primitives"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Common"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Scene"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Templates"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Controllers"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Compiler"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Compute"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Scripting"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Physics"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/UI"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Utils"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/GameControllers"
)

# ---------------------------------------------------------------------------
# 3. Listas de Código Fuente
# ---------------------------------------------------------------------------
set(ENGINE_SOURCES
    Source/Animation/Sequence/Channels/Elements/SequenceChannelElement.cpp
    Source/Animation/Sequence/Channels/Elements/SequenceChannelElement.h
    Source/Animation/Sequence/Channels/Elements/SequenceChannelElementAnimation.cpp
    Source/Animation/Sequence/Channels/Elements/SequenceChannelElementAnimation.h
    Source/Animation/Sequence/Channels/Elements/SequenceChannelElementBoneTransformation.cpp
    Source/Animation/Sequence/Channels/Elements/SequenceChannelElementBoneTransformation.h
    Source/Animation/Sequence/Channels/Elements/SequenceChannelElementScript.cpp
    Source/Animation/Sequence/Channels/Elements/SequenceChannelElementScript.h
    Source/Animation/Sequence/Channels/Elements/SequenceChannelElementSoundFX.cpp
    Source/Animation/Sequence/Channels/Elements/SequenceChannelElementSoundFX.h
    Source/Animation/Sequence/Channels/Elements/SequenceChannelElementTransformation.cpp
    Source/Animation/Sequence/Channels/Elements/SequenceChannelElementTransformation.h
    Source/Animation/Sequence/Channels/Elements/SequenceChannelElementTrigger.cpp
    Source/Animation/Sequence/Channels/Elements/SequenceChannelElementTrigger.h
    Source/Animation/Sequence/Channels/Elements/TransformationKeyFrame.cpp
    Source/Animation/Sequence/Channels/Elements/TransformationKeyFrame.h
    Source/Animation/Sequence/Channels/ChannelElement.cpp
    Source/Animation/Sequence/Channels/ChannelElement.h
    Source/Animation/Sequence/AnimationSequences.cpp
    Source/Animation/Sequence/AnimationSequences.h
    Source/Animation/Sequence/Sequence.cpp
    Source/Animation/Sequence/Sequence.h
    Source/Animation/Sequence/SequenceChannel.cpp
    Source/Animation/Sequence/SequenceChannel.h
    Source/Animation/Sequence/SequencePlayer.cpp
    Source/Animation/Sequence/SequencePlayer.h
    Source/Animation/Animated.cpp
    Source/Animation/Animated.h
    Source/Audio/AudioSystem.cpp
    Source/Audio/AudioSystem.h
    Source/Common/d3dx12.h
    Source/Common/DirectXHelper.h
    Source/Common/StepTimer.h
    Source/Controllers/Controller.cpp
    Source/Controllers/Controller.h
    Source/Controllers/SceneController.cpp
    Source/Controllers/SceneController.h
    Source/Controllers/SceneControllerAtt.h
    Source/Game/BootLevel/BootLevel.cpp
    Source/Game/BootLevel/BootLevel.h
    Source/Game/GameEngineStates/GameEngineBoot.cpp
    Source/Game/GameEngineStates/GameEngineBoot.h
    Source/Game/Game.cpp
    Source/Game/Game.h
    Source/JExpose/Attributes/JDecl.h
    Source/JExpose/Attributes/JDestroy.h
    Source/JExpose/Attributes/JExpose.h
    Source/JExpose/Attributes/JFlags.h
    Source/JExpose/Attributes/JInit.h
    Source/JExpose/Attributes/JOrder.h
    Source/JExpose/Attributes/JStr2Flag.h
    Source/JExpose/Attributes/JUpdate.h
    Source/JExpose/Attributes/JV8Att.h
    Source/JExpose/Controllers/ControllerAtt.h
    Source/JExpose/Creator/JDrawersDecl.h
    Source/JExpose/Creator/JDrawersDef.h
    Source/JExpose/Creator/JExpose.h
    Source/JExpose/Creator/JJsonDecl.h
    Source/JExpose/Creator/JJsonDef.h
    Source/JExpose/Creator/JRequired.h
    Source/JExpose/Creator/JValidatorDecl.h
    Source/JExpose/Creator/JValidatorDef.h
    Source/JExpose/Editor/JDrawersDecl.h
    Source/JExpose/Editor/JDrawersDef.h
    Source/JExpose/Editor/JExpose.h
    Source/JExpose/Editor/JPreviewDecl.h
    Source/JExpose/Editor/JPreviewDef.h
    Source/JExpose/Editor/JReleaseBuilder.h
    Source/JExpose/Editor/JSaveFile.h
    Source/JExpose/Editor/JWriteJson.h
    Source/JExpose/Primitives/CapsuleAtt.h
    Source/JExpose/Primitives/ConeAtt.h
    Source/JExpose/Primitives/CubeAtt.h
    Source/JExpose/Primitives/SphereAtt.h
    Source/JExpose/TrackUUID/JClear.h
    Source/JExpose/TrackUUID/JClearUnit.h
    Source/JExpose/TrackUUID/JDecl.h
    Source/JExpose/TrackUUID/JDef.h
    Source/JExpose/TrackUUID/JErase.h
    Source/JExpose/TrackUUID/JExpose.h
    Source/JExpose/TrackUUID/JInsert.h
    Source/JExpose/JEnd.h
    Source/JExpose/JExpose.h
    Source/JExpose/JObject.cpp
    Source/JExpose/JObject.h
    Source/JExpose/JTypes.h
    Source/pch/Application.h
    Source/pch/DDSTextures.h
    Source/pch/Debug.h
    Source/pch/DeleteHook.h
    Source/pch/DXTypes.h
    Source/pch/GameEngineState.h
    Source/pch/GameStateMachine.h
    Source/pch/JExposeEditor.h
    Source/pch/Json.h
    Source/pch/NoMath.h
    Source/pch/NoStd.h
    Source/pch/RefTracker.h
    Source/pch/SceneUnitId.h
    Source/pch/ShaderMaterials.h
    Source/pch/UUID.h
    Source/Physics/Boundary.cpp
    Source/Physics/Boundary.h
    Source/Physics/BoundaryAtt.h
    Source/Physics/CharacterHitCallback.cpp
    Source/Physics/CharacterHitCallback.h
    Source/Physics/ContactCallback.cpp
    Source/Physics/ContactCallback.h
    Source/Physics/PhysicGeometry.cpp
    Source/Physics/PhysicGeometry.h
    Source/Physics/PhysicGeometryAtt.h
    Source/Physics/PhysicObject.cpp
    Source/Physics/PhysicObject.h
    Source/Physics/PhysicObjectAtt.h
    Source/Physics/Physics.cpp
    Source/Physics/Physics.h
    Source/Physics/PhysicScene.cpp
    Source/Physics/PhysicScene.h
    Source/Physics/PhysicSceneAtt.h
    Source/Physics/Trigger.cpp
    Source/Physics/Trigger.h
    Source/Physics/TriggerAtt.h
    Source/Primitives/BoxLines.cpp
    Source/Primitives/BoxLines.h
    Source/Primitives/Capsule.cpp
    Source/Primitives/Capsule.h
    Source/Primitives/Cone.cpp
    Source/Primitives/Cone.h
    Source/Primitives/Cube.cpp
    Source/Primitives/Cube.h
    Source/Primitives/Decal.cpp
    Source/Primitives/Decal.h
    Source/Primitives/Floor.cpp
    Source/Primitives/Floor.h
    Source/Primitives/Pentahedron.cpp
    Source/Primitives/Pentahedron.h
    Source/Primitives/Primitive.h
    Source/Primitives/Primitives.h
    Source/Primitives/Sphere.cpp
    Source/Primitives/Sphere.h
    Source/Primitives/UtahTeapot.cpp
    Source/Primitives/UtahTeapot.h
    Source/Renderer/DeviceUtils/CommandsProcessor/CommandsProcessor.cpp
    Source/Renderer/DeviceUtils/CommandsProcessor/CommandsProcessor.h
    Source/Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.cpp
    Source/Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.h
    Source/Renderer/DeviceUtils/D3D12Device/Builder.cpp
    Source/Renderer/DeviceUtils/D3D12Device/Builder.h
    Source/Renderer/DeviceUtils/D3D12Device/Interop.cpp
    Source/Renderer/DeviceUtils/D3D12Device/Interop.h
    Source/Renderer/DeviceUtils/DescriptorHeap/DescriptorHeap.cpp
    Source/Renderer/DeviceUtils/DescriptorHeap/DescriptorHeap.h
    Source/Renderer/DeviceUtils/IndexBuffer/IndexBuffer.cpp
    Source/Renderer/DeviceUtils/IndexBuffer/IndexBuffer.h
    Source/Renderer/DeviceUtils/PipelineState/PipelineState.cpp
    Source/Renderer/DeviceUtils/PipelineState/PipelineState.h
    Source/Renderer/DeviceUtils/RenderPass/RenderToTexturePass.cpp
    Source/Renderer/DeviceUtils/RenderPass/RenderToTexturePass.h
    Source/Renderer/DeviceUtils/RenderPass/SwapChainPass.cpp
    Source/Renderer/DeviceUtils/RenderPass/SwapChainPass.h
    Source/Renderer/DeviceUtils/RenderTarget/RenderTarget.cpp
    Source/Renderer/DeviceUtils/RenderTarget/RenderTarget.h
    Source/Renderer/DeviceUtils/RenderToTexture/RenderToTexture.cpp
    Source/Renderer/DeviceUtils/RenderToTexture/RenderToTexture.h
    Source/Renderer/DeviceUtils/Resources/Resources.cpp
    Source/Renderer/DeviceUtils/Resources/Resources.h
    Source/Renderer/DeviceUtils/RootSignature/RootSignature.cpp
    Source/Renderer/DeviceUtils/RootSignature/RootSignature.h
    Source/Renderer/DeviceUtils/VertexBuffer/VertexBuffer.cpp
    Source/Renderer/DeviceUtils/VertexBuffer/VertexBuffer.h
    Source/Renderer/Renderer.cpp
    Source/Renderer/Renderer.h
    Source/Renderer/VertexFormats.cpp
    Source/Renderer/VertexFormats.h
    Source/Scene/Camera/Projections/Orthographic.h
    Source/Scene/Camera/Projections/Perspective.h
    Source/Scene/Camera/Camera.cpp
    Source/Scene/Camera/Camera.h
    Source/Scene/Camera/CameraAtt.h
    Source/Scene/Light/Light.cpp
    Source/Scene/Light/Light.h
    Source/Scene/Light/LightAtt.h
    Source/Scene/Light/ShadowMap.cpp
    Source/Scene/Light/ShadowMap.h
    Source/Scene/Renderable/Renderable.cpp
    Source/Scene/Renderable/Renderable.h
    Source/Scene/Renderable/RenderableAtt.h
    Source/Scene/Sound/SoundFX.cpp
    Source/Scene/Sound/SoundFX.h
    Source/Scene/Sound/SoundFXAtt.h
    Source/Scene/Binder.cpp
    Source/Scene/Binder.h
    Source/Scene/Level.cpp
    Source/Scene/Level.h
    Source/Scene/Scene.cpp
    Source/Scene/Scene.h
    Source/Scene/SceneObject.cpp
    Source/Scene/SceneObject.h
    Source/Scene/SceneObjectDecl.h
    Source/Scene/SceneObjectDef.h
    Source/Scene/SceneUnit.cpp
    Source/Scene/SceneUnit.h
    Source/Scripting/NoV8.cpp
    Source/Scripting/NoV8.h
    Source/Scripting/SceneUnitScripting.cpp
    Source/Scripting/SceneUnitScripting.h
    Source/Scripting/ScriptBinding.h
    Source/Scripting/Scripting.cpp
    Source/Scripting/Scripting.h
    Source/Shaders/Compiler/CompilerQueue.h
    Source/Shaders/Compiler/CustomIncludeHandler.cpp
    Source/Shaders/Compiler/CustomIncludeHandler.h
    Source/Shaders/Compiler/ShaderCompiler.cpp
    Source/Shaders/Compiler/ShaderCompiler.h
    Source/Shaders/Compute/HDR/LuminanceHistogram.cpp
    Source/Shaders/Compute/HDR/LuminanceHistogram.h
    Source/Shaders/Compute/HDR/LuminanceHistogramAverage.cpp
    Source/Shaders/Compute/HDR/LuminanceHistogramAverage.h
    Source/Shaders/Compute/Renderable/RenderableBoundingBox.cpp
    Source/Shaders/Compute/Renderable/RenderableBoundingBox.h
    Source/Shaders/Compute/ComputeInterface.h
    Source/Shaders/Compute/ComputeShader.cpp
    Source/Shaders/Compute/ComputeShader.h
    Source/Templates/HtmlUI/HtmlUI.cpp
    Source/Templates/HtmlUI/HtmlUI.h
    Source/Templates/HtmlUI/HtmlUIAtt.h
    Source/Templates/Material/BlendDesc.h
    Source/Templates/Material/DepthStencilDesc.h
    Source/Templates/Material/Material.cpp
    Source/Templates/Material/Material.h
    Source/Templates/Material/MaterialAtt.h
    Source/Templates/Material/MeshMaterial.h
    Source/Templates/Material/RasterizerDesc.h
    Source/Templates/Material/SamplerDesc.h
    Source/Templates/Material/Variables.cpp
    Source/Templates/Material/Variables.h
    Source/Templates/Mesh/Mesh.cpp
    Source/Templates/Mesh/Mesh.h
    Source/Templates/Model3D/Model3D.cpp
    Source/Templates/Model3D/Model3D.h
    Source/Templates/Model3D/Model3DAtt.h
    Source/Templates/Mold/Mold.cpp
    Source/Templates/Mold/Mold.h
    Source/Templates/Mold/MoldAtt.h
    Source/Templates/RenderPass/Override/MinMaxChainPass.cpp
    Source/Templates/RenderPass/Override/MinMaxChainPass.h
    Source/Templates/RenderPass/Override/MinMaxChainResultPass.cpp
    Source/Templates/RenderPass/Override/MinMaxChainResultPass.h
    Source/Templates/RenderPass/Override/OverridePass.cpp
    Source/Templates/RenderPass/Override/OverridePass.h
    Source/Templates/RenderPass/Override/ResolvePass.cpp
    Source/Templates/RenderPass/Override/ResolvePass.h
    Source/Templates/RenderPass/Override/ResolveUIPass.cpp
    Source/Templates/RenderPass/Override/ResolveUIPass.h
    Source/Templates/RenderPass/Override/ToneMappingPass.cpp
    Source/Templates/RenderPass/Override/ToneMappingPass.h
    Source/Templates/RenderPass/PassMaterialOverride.h
    Source/Templates/RenderPass/RenderPass.cpp
    Source/Templates/RenderPass/RenderPass.h
    Source/Templates/RenderPass/RenderPassAtt.h
    Source/Templates/Shader/Shader.cpp
    Source/Templates/Shader/Shader.h
    Source/Templates/Shader/ShaderAtt.h
    Source/Templates/Sound/Sound.cpp
    Source/Templates/Sound/Sound.h
    Source/Templates/Sound/SoundAtt.h
    Source/Templates/Textures/Texture.cpp
    Source/Templates/Textures/Texture.h
    Source/Templates/Textures/TextureAtt.h
    Source/Templates/JTemplate.h
    Source/Templates/TemplateDecl.h
    Source/Templates/TemplateDef.h
    Source/Templates/Templates.cpp
    Source/Templates/Templates.h
    Source/UI/UI.cpp
    Source/UI/UI.h
    Source/Utils/CommandLine.h
    Source/Utils/LoadingProcessor.cpp
    Source/Utils/LoadingProcessor.h
    Source/Utils/Yaml2Json.h
    Source/Engine.cpp
    Source/Engine.h
    Source/EngineInc.h
    Source/GameDecl.h
    Source/pch.cpp
    Source/pch.h
    Source/Resource.h
    Source/targetver.h
    Source/packages.config
)

set(ENGINE_EDITOR_SOURCES
    Source/Editor/Builder/BuildMaker.cpp
    Source/Editor/Builder/BuildMaker.h
    Source/Editor/Builder/ReleaseBuilder.h
    Source/Editor/Modals/BuildModal.cpp
    Source/Editor/Modals/BuildModal.h
    Source/Editor/Modals/ScriptBindingModal.cpp
    Source/Editor/Modals/ScriptBindingModal.h
    Source/Editor/Modals/ScriptEditModal.cpp
    Source/Editor/Modals/ScriptEditModal.h
    Source/Editor/Modals/SimpleModal.cpp
    Source/Editor/Modals/SimpleModal.h
    Source/Editor/Modals/ThreadSafeStream.h
    Source/Editor/Timeline/TimelineChannel.cpp
    Source/Editor/Timeline/TimelineChannel.h
    Source/Editor/Timeline/TimelineEditor.cpp
    Source/Editor/Timeline/TimelineEditor.h
    Source/Editor/Timeline/TimelinePopups.cpp
    Source/Editor/Timeline/TimelinePopups.h
    Source/Editor/AnimationSequencerModal.cpp
    Source/Editor/AnimationSequencerModal.h
    Source/Editor/AssetsTree.cpp
    Source/Editor/AssetsTree.h
    Source/Editor/BillboardRegistry.h
    Source/Editor/CreatorModal.h
    Source/Editor/DefaultLevel.cpp
    Source/Editor/DefaultLevel.h
    Source/Editor/DeletePrompt.cpp
    Source/Editor/DeletePrompt.h
    Source/Editor/Editor.cpp
    Source/Editor/Editor.h
    Source/Editor/EditorMouseCamera.cpp
    Source/Editor/EditorMouseCamera.h
    Source/Editor/GizmoInteraction.h
    Source/Editor/IconsFontAwesome5.h
    Source/Editor/ImEditor.cpp
    Source/Editor/ImEditor.h
    Source/Editor/LoadingProgress.h
    Source/Editor/MousePicking.cpp
    Source/Editor/MousePicking.h
    Source/Editor/PhysicsDrawState.h
    Source/Editor/RightPanelComponent.cpp
    Source/Editor/RightPanelComponent.h
    Source/Editor/SceneObjectPopup.cpp
    Source/Editor/SceneObjectPopup.h
    Source/Editor/YesNoCancelModal.cpp
    Source/Editor/YesNoCancelModal.h
    Source/JExpose/Functions/JEdvCreatorDrawer.cpp
    Source/JExpose/Functions/JEdvCreatorDrawer.h
    Source/JExpose/Functions/JEdvCreatorValidator.cpp
    Source/JExpose/Functions/JEdvCreatorValidator.h
    Source/JExpose/Functions/JEdvEditorDrawer.cpp
    Source/JExpose/Functions/JEdvEditorDrawer.h
    Source/Shaders/Compute/IBL/BRDFLUT.cpp
    Source/Shaders/Compute/IBL/BRDFLUT.h
    Source/Shaders/Compute/IBL/DiffuseIrradianceMap.cpp
    Source/Shaders/Compute/IBL/DiffuseIrradianceMap.h
    Source/Shaders/Compute/IBL/PreFilteredEnvironmentMap.cpp
    Source/Shaders/Compute/IBL/PreFilteredEnvironmentMap.h
)

set(IMAGE_CONVERT_SOURCES
    "Source/Utils/ImageConvert.cpp"
    "Source/Utils/ImageConvert.h"
)

# ---------------------------------------------------------------------------
# 4. Declaración del Target
# ---------------------------------------------------------------------------
add_library(Engine STATIC
    ${ENGINE_SOURCES}
    $<$<CONFIG:Editor_Debug,Editor_Development,Editor_Release>:${ENGINE_EDITOR_SOURCES}>
    $<$<NOT:$<CONFIG:Release>>:${IMAGE_CONVERT_SOURCES}>
)

# Configuración del Precompiled Header (PCH)
target_compile_options(Engine PRIVATE /Yupch.h)
set_source_files_properties(Source/pch.cpp PROPERTIES COMPILE_FLAGS "/Ycpch.h")

# Estructura de carpetas para el Solution Explorer de Visual Studio
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Source" 
             PREFIX "Source Files" 
             FILES ${ENGINE_SOURCES} ${ENGINE_EDITOR_SOURCES} ${IMAGE_CONVERT_SOURCES})

# ---------------------------------------------------------------------------
# 5. Preprocesador e Inclusiones
# ---------------------------------------------------------------------------
target_compile_definitions(Engine PUBLIC
    $<$<CONFIG:Editor_Debug>:${DEFINES_EDITOR_DEBUG}>
    $<$<CONFIG:Editor_Development>:${DEFINES_EDITOR_DEVELOPMENT}>
    $<$<CONFIG:Editor_Release>:${DEFINES_EDITOR_RELEASE}>
    $<$<CONFIG:Debug>:${DEFINES_DEBUG}>
    $<$<CONFIG:Development>:${DEFINES_DEVELOPMENT}>
    $<$<CONFIG:Release>:${DEFINES_RELEASE}>
)

target_include_directories(Engine PUBLIC
    ${INCLUDES_COMMON}
	"${CMAKE_CURRENT_SOURCE_DIR}/Source"
    $<$<CONFIG:Editor_Debug,Editor_Development,Editor_Release>:${CMAKE_CURRENT_SOURCE_DIR}/Source/Editor>
)

# ---------------------------------------------------------------------------
# 6. Propiedades del Target y Rutas de Salida
# ---------------------------------------------------------------------------
set(ENGINE_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Lib")

set_target_properties(Engine PROPERTIES
    # C Runtime (MD vs MDd)
    MSVC_RUNTIME_LIBRARY "$<$<CONFIG:Debug,Editor_Debug>:MultiThreadedDebugDLL>$<$<NOT:$<CONFIG:Debug,Editor_Debug>>:MultiThreadedDLL>"

    # Nombre dinámico: Engine_Debug.lib, Engine_Release.lib, etc.
    OUTPUT_NAME "Engine_$<CONFIG>"

    # Subsystem (Console para Debug/Dev, Windows para el resto)
    WIN32_EXECUTABLE "$<$<CONFIG:Release,Editor_Debug,Editor_Development,Editor_Release>:TRUE>"

    # Directo a Engine/Lib/ sin subcarpetas
    ARCHIVE_OUTPUT_DIRECTORY "${ENGINE_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${ENGINE_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_DEVELOPMENT "${ENGINE_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${ENGINE_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_EDITOR_DEBUG "${ENGINE_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_EDITOR_DEVELOPMENT "${ENGINE_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_EDITOR_RELEASE "${ENGINE_LIB_DIR}"
)

# ---------------------------------------------------------------------------
# 7. Flags del Compilador y Linker (MSVC)
# ---------------------------------------------------------------------------
target_compile_options(Engine PRIVATE
    $<$<CONFIG:Debug,Editor_Debug>:
        /sdl
        /Zi
    >
    $<$<CONFIG:Development,Editor_Development>:
        /sdl
        /O2
        /Oi
        /Zi
    >
    $<$<CONFIG:Release,Editor_Release>:
        /sdl
        /O2
        /Oi
        /Ot
        /GL
    >
)

target_link_options(Engine PRIVATE
    /NOLOGO
    $<$<CONFIG:Release,Editor_Release>:
        /LTCG
    >
)

if (MSVC)
    target_compile_options(Engine PRIVATE 
        /utf-8
        /bigobj
    )
endif()
