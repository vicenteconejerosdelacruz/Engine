cmake_minimum_required(VERSION 3.20)
project(TestGame LANGUAGES CXX)

# ---------------------------------------------------------------------------
# 1. Archivos Fuente de TestGame
# ---------------------------------------------------------------------------
set(TESTGAME_SOURCES
    Source/GameControllers/BootScreen/BootScreen.cpp
    Source/GameControllers/BootScreen/BootScreen.h
    Source/GameControllers/BootScreen/BootScreenAtt.h
    Source/GameControllers/Brawler/Camera/BrawlerCamera.cpp
    Source/GameControllers/Brawler/Camera/BrawlerCamera.h
    Source/GameControllers/Brawler/Camera/BrawlerCameraAtt.h
    Source/GameControllers/Brawler/Characters/Bosses/GreenGoblin.cpp
    Source/GameControllers/Brawler/Characters/Bosses/GreenGoblin.h
    Source/GameControllers/Brawler/Characters/Bosses/GreenGoblinAtt.h
    Source/GameControllers/Brawler/Characters/Bosses/PumpkinBomb.cpp
    Source/GameControllers/Brawler/Characters/Bosses/PumpkinBomb.h
    Source/GameControllers/Brawler/Characters/Bosses/PumpkinBombAtt.h
    Source/GameControllers/Brawler/Characters/Enemies/Thug.cpp
    Source/GameControllers/Brawler/Characters/Enemies/Thug.h
    Source/GameControllers/Brawler/Characters/Enemies/ThugAtt.h
    Source/GameControllers/Brawler/Characters/Heroes/Hero.cpp
    Source/GameControllers/Brawler/Characters/Heroes/Hero.h
    Source/GameControllers/Brawler/Characters/Heroes/HeroAtt.h
    Source/GameControllers/Brawler/Characters/Heroes/Venom.cpp
    Source/GameControllers/Brawler/Characters/Heroes/Venom.h
    Source/GameControllers/Brawler/Characters/Heroes/VenomAtt.h
    Source/GameControllers/Brawler/Characters/BrawlerCharacter.cpp
    Source/GameControllers/Brawler/Characters/BrawlerCharacter.h
    Source/GameControllers/Brawler/Characters/BrawlerCharacterAtt.h
    Source/GameControllers/Brawler/Dialogs/BrawlerDialog.h
    Source/GameControllers/Brawler/Menus/BrawlerMenu.cpp
    Source/GameControllers/Brawler/Menus/BrawlerMenu.h
    Source/GameControllers/Brawler/Menus/BrawlerMenuAtt.h
    Source/GameControllers/Brawler/Rounds/BrawlerRound.h
    Source/GameControllers/Brawler/Scene/BrawlerScene.cpp
    Source/GameControllers/Brawler/Scene/BrawlerScene.h
    Source/GameControllers/Brawler/Scene/BrawlerSceneAtt.h
    Source/GameControllers/Effects/AnimatedDecal.cpp
    Source/GameControllers/Effects/AnimatedDecal.h
    Source/GameControllers/Effects/AnimatedDecalAtt.h
    Source/GameControllers/Effects/DelayedDeletion.cpp
    Source/GameControllers/Effects/DelayedDeletion.h
    Source/GameControllers/Effects/DelayedDeletionAtt.h
    Source/GameControllers/Test/SpinYaw.cpp
    Source/GameControllers/Test/SpinYaw.h
    Source/GameControllers/Test/SpinYawAtt.h
    Source/GameControllers/ThirdPerson/ThirdPersonCharacter.cpp
    Source/GameControllers/ThirdPerson/ThirdPersonCharacter.h
    Source/GameControllers/ThirdPerson/ThirdPersonCharacterAtt.h
    Source/GameControllers/GameControllers.cpp
    Source/GamePhysics/GamePhysics.cpp
    Source/GamePhysics/GamePhysics.h
    Source/pch.cpp
    Source/pch.h
    Source/Resource.h
    Source/targetver.h
    Source/TestGame.cpp
    Source/TestGame.h
)

# Filtros visuales para el Solution Explorer de Visual Studio
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Source" 
             PREFIX "Source Files" 
             FILES ${TESTGAME_SOURCES})

set(RESOURCE_FILES
    "Source/TestGame.rc"
    "Source/small.ico"
    "Source/TestGame.ico"
)

# ---------------------------------------------------------------------------
# 2. Declaración del Ejecutable
# ---------------------------------------------------------------------------
add_executable(TestGame
	${TESTGAME_SOURCES}
	${RESOURCE_FILES}
	$<$<CONFIG:Editor_Debug,Editor_Development,Editor_Release>:Source/JExpose/Editor/JEdvBrawlerDrawer.cpp>
    $<$<CONFIG:Editor_Debug,Editor_Development,Editor_Release>:Source/JExpose/Editor/JEdvBrawlerDrawer.h>
)

# ---------------------------------------------------------------------------
# Directorio de salida del ejecutable
# Configura el directorio final como: ${CMAKE_CURRENT_SOURCE_DIR}/<Configuracion>
# ---------------------------------------------------------------------------
set_target_properties(TestGame PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/$<CONFIG>"
)

# ---------------------------------------------------------------------------
# Nombres de ejecutable dinámicos por configuración
# Formato general: TestGame_<Configuracion>.exe
# Excepción Release: TestGame.exe
# ---------------------------------------------------------------------------
set_target_properties(TestGame PROPERTIES
    OUTPUT_NAME "TestGame_$<CONFIG>"
)

# Sobrescribimos el nombre únicamente para la build final Release
set_target_properties(TestGame PROPERTIES
    OUTPUT_NAME_EDITOR_RELEASE "TestGame_Editor"
)

# Sobrescribimos el nombre únicamente para la build final Release
set_target_properties(TestGame PROPERTIES
    OUTPUT_NAME_RELEASE "TestGame"
)

# ---------------------------------------------------------------------------
# Directorio de trabajo del depurador (Working Directory)
# Apunta al directorio exacto donde se genera el ejecutable
# ---------------------------------------------------------------------------
set_target_properties(TestGame PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/$<CONFIG>"
)

# Configuración del Precompiled Header (PCH) para TestGame
target_compile_options(TestGame PRIVATE /Yupch.h)
set_source_files_properties(Source/pch.cpp PROPERTIES COMPILE_FLAGS "/Ycpch.h")

# ---------------------------------------------------------------------------
# 3. Inclusiones y Enlazado con Engine
# ---------------------------------------------------------------------------
target_include_directories(TestGame PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/pch"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/GameControllers"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/GamePhysics"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/JExpose/Editor"
)

# ---------------------------------------------------------------------------
# 4. Propiedades del Executable (Subsystem, Nombres y CRT)
# ---------------------------------------------------------------------------
set_target_properties(TestGame PROPERTIES
    OUTPUT_NAME "TestGame_$<CONFIG>"

    # Subsystem: Consola para Debug/Development y Windows GUI para el resto
    WIN32_EXECUTABLE "$<$<CONFIG:Release,Editor_Debug,Editor_Development,Editor_Release>:TRUE>"

    # Hereda la configuración exacta del Runtime Library (MDd vs MD) usada en Engine
    MSVC_RUNTIME_LIBRARY "$<$<CONFIG:Debug,Editor_Debug>:MultiThreadedDebugDLL>$<$<NOT:$<CONFIG:Debug,Editor_Debug>>:MultiThreadedDLL>"
)

# ---------------------------------------------------------------------------
# 5. Flags de Compilación y Preprocesador (Coincidentes con Engine)
# ---------------------------------------------------------------------------
target_compile_definitions(TestGame PRIVATE
    $<$<CONFIG:Editor_Debug,Editor_Development,Editor_Release>:_EDITOR>
)

target_compile_options(TestGame PRIVATE
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

target_link_options(TestGame PRIVATE
    /NOLOGO
    $<$<CONFIG:Release,Editor_Release>:
        /LTCG
    >
	$<$<CONFIG:Debug,Editor_Debug,Development,Editor_Development>:/DEBUG>
)

set(COMMON_LIBRARIES
    d3d12.lib
    dxgi.lib
    dxguid.lib
    dxcompiler.lib
    rpcrt4.lib
    version.lib
)	

set(DEBUG_ASSIMP_LIBRARIES
    assimp-vc143-mtd.lib
    zlibstaticd.lib
)

set(RELEASE_ASSIMP_LIBRARIES
    assimp-vc143-mt.lib
    zlibstatic.lib
)

set(DIRECTXTEX_LIBRARIES
    CmdLineHelpers.lib
    DirectXTex.lib
    texconv.lib
    texassemble.lib
    texdiag.lib
)

set(DIRECTXTK12_LIBRARIES
    DirectXTK12.lib
)

set(IMGUI_LIBRARIES
    imgui.lib
    ImGuizmo.lib
)

set(V8_LIBRARIES
    v8.dll.lib
    v8_libplatform.dll.lib
    v8pp.lib
)

set(PHYSX_LIBRARIES
    PhysX_64.lib
    PhysXCooking_64.lib
    PhysXExtensions_static_64.lib
    PhysXCharacterKinematic_static_64.lib
    PhysXPvdSDK_static_64.lib
    PhysXCommon_64.lib
    PhysXFoundation_64.lib
    PhysXGpu_64.lib
)

set(ULTRALIGHT_LIBRARIES
    Ultralight.lib
    UltralightCore.lib
    WebCore.lib
    AppCore.lib
)

set(DEBUG_YAMLCPP_LIBRARIES
    yaml-cppd.lib
)

set(RELEASE_YAMLCPP_LIBRARIES
    yaml-cpp.lib
)

set(WINPIX_LIBRARIES
	WinPixEventRuntime.lib
)

target_link_libraries(TestGame PRIVATE
    # Enlace de Engine (si aplica)
    Engine

    # Librerías comunes independientes de la configuración
    ${COMMON_LIBRARIES}
    ${DIRECTXTK12_LIBRARIES}
    ${V8_LIBRARIES}
    ${PHYSX_LIBRARIES}
    ${ULTRALIGHT_LIBRARIES}

	$<$<CONFIG:Editor_Debug,Editor_Development,Editor_Release>:
        ${IMGUI_LIBRARIES}
    >
	
    # Librerías específicas para Builds Debug / Editor_Debug
    $<$<CONFIG:Debug,Editor_Debug>:
        ${DEBUG_ASSIMP_LIBRARIES}
        ${DEBUG_YAMLCPP_LIBRARIES}
    >

    # Librerías específicas para Builds Release / Development / Editor_Release / etc.
    $<$<NOT:$<CONFIG:Debug,Editor_Debug>>:
        ${RELEASE_ASSIMP_LIBRARIES}
        ${RELEASE_YAMLCPP_LIBRARIES}
    >
	
	$<$<NOT:$<CONFIG:Release>>:
        ${DIRECTXTEX_LIBRARIES}
		${WINPIX_LIBRARIES}
    >
)

set(COMMON_LIBFOLDER
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/dxc/dxc_2024_07_31/lib/x64"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/v8pp-2.1.1/packages/v8-v143-x64.9.8.177.4/lib/Release"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/ultralight/ultralight-free-sdk-1.4.0-win-x64/lib"
)

set(DEBUG_PHYSX_LIBFOLDER
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/PhysX/physx/bin/win.x86_64.vc143.md/debug"
)

set(DEVELOPMENT_PHYSX_LIBFOLDER
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/PhysX/physx/bin/win.x86_64.vc143.md/profile"
)

set(RELEASE_PHYSX_LIBFOLDER
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/PhysX/physx/bin/win.x86_64.vc143.md/release"
)

set(DEBUG_ASSIMP_LIBFOLDER
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/assimp/assimp-6.0.5/build/lib/Debug"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/assimp/assimp-6.0.5/build/contrib/zlib/Debug"
)

set(RELEASE_ASSIMP_LIBFOLDER
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/assimp/assimp-6.0.5/build/lib/Release"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/assimp/assimp-6.0.5/build/contrib/zlib/Release"
)

set(DEBUG_DIRECTXTK12_LIBFOLDER
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/DirectXTK12/Bin/Desktop_2026/x64/Debug"
)

set(RELEASE_DIRECTXTK12_LIBFOLDER
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/DirectXTK12/Bin/Desktop_2026/x64/Release"
)

set(DEBUG_DIRECTXTEX_LIBFOLDER
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/DirectXTex/DirectXTex/Bin/Desktop_2022/x64/DebugLib"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/DirectXTex/Texconv/Bin/Desktop_2022/x64/DebugLib"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/DirectXTex/Texassemble/Bin/Desktop_2022/x64/DebugLib"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/DirectXTex/Texdiag/Bin/Desktop_2022/x64/DebugLib"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/DirectXTex/CmdLineHelpers/Bin/Desktop_2022/x64/Debug"
)

set(RELEASE_DIRECTXTEX_LIBFOLDER
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/DirectXTex/DirectXTex/Bin/Desktop_2022/x64/ReleaseLib"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/DirectXTex/Texconv/Bin/Desktop_2022/x64/ReleaseLib"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/DirectXTex/Texassemble/Bin/Desktop_2022/x64/ReleaseLib"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/DirectXTex/Texdiag/Bin/Desktop_2022/x64/ReleaseLib"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/DirectXTex/CmdLineHelpers/Bin/Desktop_2022/x64/Release"
)

set(DEBUG_IMGUI_LIBFOLDER
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/imgui/x64/Debug"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/imguizmo/x64/Debug"
)

set(RELEASE_IMGUI_LIBFOLDER
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/imgui/x64/Release"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/imguizmo/x64/Release"
)

set(DEBUG_V8PP_LIBFOLDER
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/v8pp-2.1.1/out/v8pp/Debug"
)

set(RELEASE_V8PP_LIBFOLDER
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/v8pp-2.1.1/out/v8pp/Release"
)

set(DEBUG_YAMLCPP_LIBFOLDER
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/yaml-cpp-yaml-cpp-0.9.0/build/Debug"
)

set(RELEASE_YAMLCPP_LIBFOLDER
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/yaml-cpp-yaml-cpp-0.9.0/build/Release"
)

set(WINPIX_LIBFOLDER
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/WinPixEventRuntime/bin/x64"
)

target_link_directories(TestGame PRIVATE 
	${COMMON_LIBFOLDER}

	$<$<CONFIG:Debug,Editor_Debug>:
		${DEBUG_ASSIMP_LIBFOLDER}
		${DEBUG_DIRECTXTK12_LIBFOLDER}
		${DEBUG_V8PP_LIBFOLDER}
		${DEBUG_YAMLCPP_LIBFOLDER}
		${DEBUG_PHYSX_LIBFOLDER}
	>

	$<$<NOT:$<CONFIG:Debug,Editor_Debug>>:
		${RELEASE_ASSIMP_LIBFOLDER}
		${RELEASE_DIRECTXTK12_LIBFOLDER}
		${RELEASE_V8PP_LIBFOLDER}
		${RELEASE_YAMLCPP_LIBFOLDER}
	>

	$<$<CONFIG:Development,Editor_Development>:
		${DEVELOPMENT_PHYSX_LIBFOLDER}
	>

	$<$<CONFIG:Release,Editor_Release>:
		${RELEASE_PHYSX_LIBFOLDER}
	>

	$<$<CONFIG:Debug,Editor_Debug>:
		${DEBUG_DIRECTXTEX_LIBFOLDER}
		${WINPIX_LIBFOLDER}
	>

	$<$<CONFIG:Development,Editor_Development,Editor_Release>:
		${RELEASE_DIRECTXTEX_LIBFOLDER}
		${WINPIX_LIBFOLDER}
	>

	$<$<CONFIG:Editor_Debug>:
		${DEBUG_IMGUI_LIBFOLDER}
	>
	$<$<CONFIG:Editor_Release,Editor_Development>:
		${RELEASE_IMGUI_LIBFOLDER}
	>
)

if (MSVC)
    target_compile_options(TestGame PRIVATE 
        /utf-8
        /bigobj
    )
endif()