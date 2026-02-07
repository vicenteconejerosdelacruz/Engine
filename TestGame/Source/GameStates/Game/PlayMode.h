#pragma once

#include "../GameStates.h"

//Playing
void PlayModeCreate(GameStates prevState);
void PlayModeLeave(GameStates nextState);
void PlayModeStep();
void PlayModeRender();
