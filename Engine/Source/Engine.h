// Copyright (c) 2026 Vicente Conejeros
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
#pragma once
#include "pch.h"
#include "resource.h"

RECT GetMaximizedAreaSize();

//CREATE
#if !defined(_EDITOR) && defined(_DEVELOPMENT)
int EngineConsoleMain();
#endif
//#else
int APIENTRY EngineWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow);
//#endif
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow);

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);

//READ&GET

//UPDATE
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void AppStep();

//RENDER
void Render();
void ResizeWindow();

//DESTROY
void DestroyInstance();
