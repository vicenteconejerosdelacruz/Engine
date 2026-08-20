// TestGame.cpp : Defines the entry point for the application.
#include "pch.h"
#include "TestGame.h"
#include <unordered_map>
#include <memory>
#include <functional>
#include <string>

std::string gameAppTitle = "Culpeo Test Game";
float gameUpdateFrequency = (1.0f / 60.0f);
GameInteractionMode gameInteractionMode = GIM_PC;

#if !defined(_EDITOR) && defined(_DEVELOPMENT)
int main()
{
	return EngineConsoleMain();
}
#else
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	return EngineWinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
#endif


