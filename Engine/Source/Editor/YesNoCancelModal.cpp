#include "pch.h"
#include "YesNoCancelModal.h"
#include <imgui.h>
#include <IconsFontAwesome5.h>

YesNoCancelModal::YesNoCancelModal()
{
	show = false;
	onYes = nullptr;
	onNo = nullptr;
	onCancel = nullptr;
}

void YesNoCancelModal::Init(std::function<void()> yes, std::function<void()> no, std::function<void()> cancel)
{
	onYes = yes;
	onNo = no;
	onCancel = cancel;
}

void YesNoCancelModal::Show()
{
	show = true;
}

void YesNoCancelModal::Hide()
{
	show = false;
}

bool YesNoCancelModal::Showing()
{
	return show;
}

void YesNoCancelModal::Draw(const char* title, const char* text)
{
	const ImGuiTableFlags column_flags = ImGuiTableColumnFlags_None;
	ImGui::OpenPopup(title);

	float yesButtonWidth = ImGui::CalcTextSize(ICON_FA_CHECK "Yes").x + ImGui::GetStyle().FramePadding.x * 2.0f;
	float noButtonWidth = ImGui::CalcTextSize(ICON_FA_TIMES "No").x + ImGui::GetStyle().FramePadding.x * 2.0f;
	float cancelButtonWidth = ImGui::CalcTextSize(ICON_FA_BAN "Cancel").x + ImGui::GetStyle().FramePadding.x * 2.0f;
	float buttonsWidth = yesButtonWidth + noButtonWidth + cancelButtonWidth;

	if (ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(text);

		float windowWidth = ImGui::GetContentRegionAvail().x;
		float curX = 0.5f * (windowWidth - buttonsWidth);
		ImGui::SetCursorPosX(curX);

		if (ImGui::Button(ICON_FA_CHECK "Yes"))
		{
			onYes();
		}
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_TIMES "No"))
		{
			onNo();
		}
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_BAN "Cancel"))
		{
			onCancel();
		}

		ImGui::EndPopup();
	}
}
