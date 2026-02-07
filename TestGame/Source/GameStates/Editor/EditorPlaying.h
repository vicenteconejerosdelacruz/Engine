#pragma once

#include "../GameStates.h"

//EditorPlaying
void EditorPlayingModeCreate(GameStates prevState);
void EditorPlayingModeStep();
void EditorPlayingModeRender();
void EditorPlayingModePostRender();
void EditorPlayingModeLeave(GameStates nextState);