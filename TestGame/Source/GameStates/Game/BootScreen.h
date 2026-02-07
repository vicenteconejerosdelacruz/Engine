#pragma once

#include "../GameStates.h"

//Booting
void BootScreenCreate(GameStates prevState);
void BootScreenLeave(GameStates nextState);
void BootScreenStep();
void BootScreenRender();
