#include "pch.h"
#include "ScriptModal.h"
#include <imgui.h>
#include <string>
#include <IconsFontAwesome5.h>

void ScriptModal::Init(JObject* j, std::string att)
{
	showing = true;
	json = j;
	attribute = att;
	script = j->at(att);
}

static float titleBarH = 19.0f;
void ScriptModal::Draw()
{
	if (!showing) return;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	static ImVec2 modalAdj(0.2f, 0.2f);
	ImVec2 modalSize = ImVec2(viewport->WorkSize.x - viewport->WorkSize.x * modalAdj.x, viewport->WorkSize.y - viewport->WorkSize.y * modalAdj.y);
	ImVec2 modalPos = ImVec2(viewport->WorkSize.x * modalAdj.x * 0.5f, viewport->WorkSize.y * modalAdj.y * 0.5f);

	//ImGui::OpenPopup(attribute.c_str());
	ImGui::OpenPopup("##");
	ImGui::SetNextWindowPos(modalPos);
	ImGui::SetNextWindowSize(modalSize);

	bool exit = false;
	//if (ImGui::BeginPopupModal(attribute.c_str(), nullptr,
	if (ImGui::BeginPopupModal("##", nullptr,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
	))
	{
		ImVec2 titlePos = modalPos;
		ImVec2 titleSize = ImVec2(modalSize.x, titleBarH);
		DrawTitleBar(attribute.c_str(), titlePos, titleSize, exit);

		ImVec2 editPos = ImVec2(titlePos.x, titlePos.y + titleSize.y + 5.0f);
		ImVec2 editSize = ImVec2(titleSize.x, modalSize.y - titleSize.y - 35.0f);
		DrawScriptEdition(script, editPos, editSize,
			[&]
			{
				json->at(attribute) = script;
				showing = false;
			},
			[&]
			{
				showing = false;
			}
		);

		ImGui::EndPopup();
	}
	if (exit)
	{
		showing = false;
	}
}

void ScriptModal::DrawTitleBar(const char* title, ImVec2 pos, ImVec2 size, bool& exit)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(45.0f / 255.0f, 62.0f / 255.0f, 104.0f / 255.0f, 1.0f));

	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);
	ImGui::BeginChild("title-bar", size, 0);
	{
		float windowWidth = ImGui::GetWindowSize().x;
		float textWidth = ImGui::CalcTextSize(title).x;
		ImVec2 textScreenPos(pos.x + (windowWidth - textWidth) * 0.5f, pos.y + 4.0f);
		ImGui::SetCursorScreenPos(textScreenPos);
		ImGui::Text(title);

		ImVec2 closeButtonScreenPos(pos.x + windowWidth - 20.0f, pos.y);
		ImGui::SetCursorScreenPos(closeButtonScreenPos);
		if (ImGui::Button(ICON_FA_TIMES, ImVec2(19.0f, 19.0f)))
		{
			exit = true;
		}
	}
	ImGui::EndChild();

	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void ScriptModal::DrawScriptEdition(std::string& content, ImVec2 pos, ImVec2 size, std::function<void()> onSave, std::function<void()> onCancel)
{
	ImGui::SetCursorScreenPos(pos);
	ImVec2 editorPos(pos.x, pos.y);
	ImVec2 editorSize(size.x, size.y);
	ImGui::SetNextWindowSize(editorSize, 0);
	ImGui::SetNextWindowPos(editorPos, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	ImGui::BeginChild("script-editor", editorSize, 0);
	{
		ImGui::InputTextMultiline("Script Content", &content, ImVec2(-1, editorSize.y));
	}
	ImGui::EndChild();
	ImGui::PopStyleVar(2);

	ImVec2 buttonsPos(pos.x + 10, editorPos.y + editorSize.y + 5);
	ImGui::SetCursorScreenPos(buttonsPos);
	if (ImGui::Button("Save&Exit"))
	{
		onSave();
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
	{
		onCancel();
	}
}
