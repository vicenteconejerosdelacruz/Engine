// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"

//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
//// Windows Header Files
//#include <windows.h>
//// C RunTime Header Files
//#include <stdlib.h>
//#include <malloc.h>
//#include <memory.h>
//#include <tchar.h>

#include "EngineInc.h"
//do not move the drawers from here, otherwise the code will not be executed
#if defined(_EDITOR)
#include "JExpose/Editor/JEdvBrawlerDrawer.h"
#endif
