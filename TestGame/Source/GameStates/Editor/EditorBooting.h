#pragma once

#include "../GameStates.h"

//EditorBooting
void EditorBootingCreate(GameStates prevState);
void EditorBootingStep();
void EditorBootingRender();
void EditorBootingPostRender();
void EditorBootingLeave(GameStates nextState);