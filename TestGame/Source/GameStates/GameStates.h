#pragma once

enum GameStates {
	GS_None,
#if defined(_EDITOR)
	GS_EditorBooting,
	GS_EditorMode,
	GS_EditorPlaying,
	GS_EditorDestroy,
#else
	GS_Booting,
	GS_Loading,
	GS_Playing,
	GS_Destroy
#endif
};

#if defined(_EDITOR)
#include "Editor/EditorBooting.h"
#include "Editor/EditorMode.h"
#include "Editor/EditorPlaying.h"
#else
#include "Game/BootScreen.h"
#include "Game/Loading.h"
#include "Game/PlayMode.h"
#endif