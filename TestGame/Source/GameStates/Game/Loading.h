#pragma once

#include "../GameStates.h"

//Loading
void LoadingScreenCreate(GameStates prevState);
void LoadingScreenLeave(GameStates nextState);
void LoadingScreenStep();
void LoadingScreenRender();
void LoadingScreenPostRender();
