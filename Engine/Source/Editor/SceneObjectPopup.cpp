#include "pch.h"
#include "SceneObjectPopup.h"
#include <imgui.h>
#include <Scene.h>

namespace Editor
{
	extern void MarkScenePanelAssetsAsDirty();
	extern void MarkSceneUnitAsModified(SceneUnitId unit);
};

static inline ImGuiWindowFlags popupChildFlag = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize |
ImGuiWindowFlags_NoMove;

void SceneObjectPopup::Draw()
{
	if (!show) return;

	ImGui::OpenPopup(SceneObjectTypeToString.at(type).c_str());

	ImVec2 size(300, 75);

	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	if (ImGui::BeginPopupModal(SceneObjectTypeToString.at(type).c_str(), nullptr, popupChildFlag))
	{
		ImGui::SetNextItemWidth(size.x);

		float button1_width = ImGui::CalcTextSize("Clone").x + ImGui::GetStyle().FramePadding.x * 2.0f;
		float button2_width = ImGui::CalcTextSize("Cancel").x + ImGui::GetStyle().FramePadding.x * 2.0f;
		float total_width = button1_width + button2_width + ImGui::GetStyle().ItemSpacing.x;

		float window_width = ImGui::GetContentRegionAvail().x;

		ImGui::InputText("name", &name);

		if (ImGui::Button("clone"))
		{
			//make a patch for the uuid and clone the scene object
			nlohmann::json parameters = {
				{ "name", name },
			};
			JUUID clone_uuid = CloneSceneObject(id, uuid, parameters);
			Editor::MarkScenePanelAssetsAsDirty();
			Editor::MarkSceneUnitAsModified(id);
			SceneObject* so = GetSceneObjectPointer(id, clone_uuid);
			so->BindToScene();
			CreatePhysicsObjectsBehaviors(id);
			MapControllers(id);
			show = false;
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			show = false;
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);
}