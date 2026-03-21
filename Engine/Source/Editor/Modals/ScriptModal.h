#pragma once
#include <JObject.h>

struct ScriptModal
{
	void Init(JObject* j, std::string att);
	void Draw();
	void DrawTitleBar(const char* title, ImVec2 pos, ImVec2 size, bool& exit);
	void DrawScriptEdition(std::string& content, ImVec2 pos, ImVec2 size, std::function<void()> onSave, std::function<void()> onCancel);

	bool showing = false;
	JObject* json;
	std::string attribute;
	std::string script;
};