#pragma once
#include <functional>
#include <string>
#include <imgui.h>

struct SimpleModal
{
	void Init(ImVec2 size, std::string title);
	void Draw(std::function<void(ImVec2)> innerDraw, bool canSaveAndExit, std::function<void()> saveAndExit, std::string buttonTitle = "Save&Exit");
	void DrawTitleBar(const char* title, ImVec2 pos, ImVec2 size, bool& exit);
	void DrawBottomButtons(bool canSaveAndExit, std::function<void()> exit, std::function<void()> saveAndExit, std::string buttonTitle);

	bool showing = false;
	ImVec2 size;
	std::string title;
	bool cancelable = true;
};