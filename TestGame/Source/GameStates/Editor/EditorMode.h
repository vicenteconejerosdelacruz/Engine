#pragma once

#include "../GameStates.h"

//EditorMode
void EditorModeCreate(GameStates prevState);
void EditorModeStep();
void EditorModeRender();
void EditorModePostRender();
void EditorModeLeave(GameStates nextState);