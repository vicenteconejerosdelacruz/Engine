#include "pch.h"
#include "SceneObjectPopup.h"
#include <imgui.h>
#include <Scene.h>
#include <ImEditor.h>

namespace Editor
{
	extern void MarkScenePanelAssetsAsDirty();
	extern void MarkSceneUnitAsModified(SceneUnitId unit);
	extern void MarkTemplatesPanelAssetsAsDirty();
};

static inline ImGuiWindowFlags popupChildFlag = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize |
ImGuiWindowFlags_NoMove;

void SceneObjectPopup::Draw()
{
	if (!show) return;

	auto cloneSceneObject = [&]
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
		};
	auto moldAvailable = [&]
		{
			return GetMoldUUIDByName(name).empty();
		};
	auto createMold = [&]
		{
			SceneObject* so = GetSceneObjectPointer(id, uuid);
			nlohmann::json j;
			so->WriteJson(j);
			so->DropJsonMoldAttributes(j);
			nlohmann::json mold = {};
			mold[SceneObjectTypeJsonContainer.at(type)] = nlohmann::json::array({ j });
			std::string content = mold.dump(4);
			std::string fileName = name + ".json";

			//first create the directory if needed
			std::filesystem::path directory(defaultMoldsFolder);
			std::filesystem::create_directory(directory);

			//then create the json level file
			const std::string finalFilename = defaultMoldsFolder + fileName;
			std::filesystem::path path(finalFilename);
			std::string pathStr = path.generic_string();
			std::ofstream file;
			file.open(pathStr);
			file.write(content.c_str(), content.size());
			file.close();

			nlohmann::json moldTemplate =
			{
				{ "uuid", getUUID() },
				{ "name", name },
				{ "path", fileName }
			};
			CreateMold(moldTemplate);
			Editor::MarkTemplatesPanelAssetsAsDirty();
			show = false;
		};

	ImGui::OpenPopup(SceneObjectTypeToString.at(type).c_str());

	ImVec2 size(300, 0);

	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	if (ImGui::BeginPopupModal(SceneObjectTypeToString.at(type).c_str(), nullptr,
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoMove))
	{
		ImGui::SetNextItemOpen(openedCollapsableItem == 0);
		if (ImGui::CollapsingHeader("Cloning"))
		{
			openedCollapsableItem = 0;
			ImGui::InputText("name", &name);
			if (ImGui::Button("Clone"))
			{
				cloneSceneObject();
			}
		}
		else if (openedCollapsableItem == 0)
		{
			openedCollapsableItem = -1;
		}

		ImGui::SetNextItemOpen(openedCollapsableItem == 1);
		if (ImGui::CollapsingHeader("Mold"))
		{
			openedCollapsableItem = 1;
			ImGui::InputText("name", &name);
			bool available = moldAvailable();
			if (!available)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, rgba(255, 0, 0, 1));
				ImGui::Text("*not available");
				ImGui::PopStyleColor();
			}
			ImGui::DrawItemWithEnabledState([&]
				{
					if (ImGui::Button("Create Mold"))
					{
						createMold();
					}
				}, available);
		}
		else if (openedCollapsableItem == 1)
		{
			openedCollapsableItem = -1;
		}

		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		if (ImGui::Button("Cancel"))
		{
			show = false;
		}

		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);
}