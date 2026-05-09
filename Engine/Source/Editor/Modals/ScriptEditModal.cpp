#include "pch.h"
#include "ScriptEditModal.h"
#include <imgui.h>
#include <string>
#include <IconsFontAwesome5.h>

namespace Editor
{
	extern SceneUnitId currentSceneUnitId;
	extern void MarkSceneUnitAsModified(SceneUnitId id);
};

void ScriptEditModal::Init(JObject* j, std::string att)
{
	mode = ScriptEditModalMode::Mode_JObject;
	json = j;
	attribute = att;
	script = j->at(att);
	write = nullptr;
	SimpleModal::Init(ImVec2(0.8f, 0.75f), attribute);
}

void ScriptEditModal::Init(std::string att, std::string script, std::function<void(std::string)> writer)
{
	mode = ScriptEditModalMode::Mode_Callback;
	json = nullptr;
	attribute = att;
	this->script = script;
	write = writer;
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
			if(mode== ScriptEditModalMode::Mode_JObject)
			{
				json->at(attribute) = script;
			}
			else
			{
				write(script);
				write = nullptr;
			}
			Editor::MarkSceneUnitAsModified(Editor::currentSceneUnitId);
		}
	);
}

