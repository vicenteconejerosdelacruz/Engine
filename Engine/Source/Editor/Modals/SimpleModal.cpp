#include "pch.h"
#include "SimpleModal.h"
#include <IconsFontAwesome5.h>
#include <ImEditor.h>

void SimpleModal::Init(ImVec2 size, std::string title)
{
	showing = true;
	this->size = size;
	this->title = title;
}

static float titleBarH = 19.0f;
void SimpleModal::Draw(std::function<void(ImVec2)> innerDraw, bool canSaveAndExit, std::function<void()> saveAndExit)
{
	if (!showing) return;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImVec2 modalPos = ImVec2(viewport->WorkSize.x * (1.0f - size.x) * 0.5f, viewport->WorkSize.y * (1.0f - size.y) * 0.5f);
	ImVec2 modalSize = ImVec2(viewport->WorkSize.x * size.x, viewport->WorkSize.y * size.y);

	ImGui::OpenPopup("##");
	ImGui::SetNextWindowPos(modalPos);
	ImGui::SetNextWindowSize(modalSize);

	ImVec2 bottomSize = ImVec2(modalSize.x, 30.0f);

	bool exit = false;
	if (ImGui::BeginPopupModal("##", nullptr,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
	))
	{
		ImVec2 titlePos = modalPos;
		ImVec2 titleSize = ImVec2(modalSize.x, titleBarH);
		DrawTitleBar(title.c_str(), titlePos, titleSize, exit);

		ImVec2 contentSize = ImVec2(-1, modalSize.y - titleSize.y - bottomSize.y);

		innerDraw(contentSize);

		DrawBottomButtons(canSaveAndExit, [&] {exit = true; }, saveAndExit);

		ImGui::EndPopup();
	}
	if (exit)
	{
		showing = false;
	}
}

void SimpleModal::DrawTitleBar(const char* title, ImVec2 pos, ImVec2 size, bool& exit)
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

void SimpleModal::DrawBottomButtons(bool canSaveAndExit, std::function<void()> exit, std::function<void()> saveAndExit)
{
	ImGui::DrawItemWithEnabledState([&]
		{
			if (ImGui::Button("Save&Exit"))
			{
				saveAndExit();
			}
		}, canSaveAndExit
	);
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
	{
		exit();
	}
}
