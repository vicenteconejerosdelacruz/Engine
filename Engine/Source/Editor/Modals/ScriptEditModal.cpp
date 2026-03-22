#include "pch.h"
#include "ScriptEditModal.h"
#include <imgui.h>
#include <string>
#include <IconsFontAwesome5.h>

void ScriptEditModal::Init(JObject* j, std::string att)
{
	json = j;
	attribute = att;
	script = j->at(att);
	SimpleModal::Init(ImVec2(0.8f, 0.75f), attribute);
}

static float titleBarH = 19.0f;
void ScriptEditModal::Draw()
{
	SimpleModal::Draw([&](ImVec2 size)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
			ImGui::BeginChild("script-editor", size, 0);
			{
				ImGui::InputTextMultiline("Script Content", &script, ImVec2(-1, size.y));
			}
			ImGui::EndChild();
			ImGui::PopStyleVar(2);
		},
		true,
		[&]
		{
			showing = false;
			json->at(attribute) = script;
		}
	);
}

