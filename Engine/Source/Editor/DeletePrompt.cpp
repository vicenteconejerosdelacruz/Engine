#include "pch.h"
#include "DeletePrompt.h"

void DeletePrompt::DrawPrompt(const char* prompTitle)
{
	const ImGuiTableFlags column_flags = ImGuiTableColumnFlags_None;
	bool deleteEnabled = false;
	bool allToDelete = true;
	std::string title = "delete-template";
	ImGui::OpenPopup(prompTitle);

	auto setAllToDelete = [this](bool value)
		{
			for (unsigned int i = 0; i < references.size(); i++)
			{
				auto& nav = references.at(i);
				std::string type = nav.at("type");
				if (type != "currentlevel")
				{
					nav.at("delete") = value;
				}
			}
		};

	if (ImGui::BeginPopupModal(prompTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		std::string tableName = "delete-template-table";
		if (ImGui::BeginTable(tableName.c_str(), 5, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_BordersV))
		{
			ImGui::TableSetupColumn("delete", column_flags);
			ImGui::TableSetupColumn("file", column_flags);
			ImGui::TableSetupColumn("type", column_flags);
			ImGui::TableSetupColumn("name", column_flags);
			ImGui::TableSetupColumn("path", column_flags);

			ImGui::TableHeadersRow();

			for (unsigned int i = 0; i < references.size(); i++)
			{
				ImGui::TableNextRow();
				auto& nav = references.at(i);
				std::string type = nav.at("type");

				ImGui::TableSetColumnIndex(0);
				bool value = nav.at("delete");
				deleteEnabled |= value;
				allToDelete &= value;

				ImGui::PushID(std::string(std::string("delete-") + std::to_string(i)).c_str());
				if (ImGui::Checkbox("##", &value))
				{
					if (type != "currentlevel")
					{
						nav.at("delete") = value;
					}
				}
				ImGui::PopID();

				if (type == "template")
				{
					ImGui::TableSetColumnIndex(2);
					std::string templ = nav.at("template");
					ImGui::Text(templ.c_str());
				}
				else if (type == "sceneobject")
				{
					ImGui::TableSetColumnIndex(1);
					std::string file = nav.at("filename");
					ImGui::Text(file.c_str());

					ImGui::TableSetColumnIndex(2);
					std::string sceneObject = nav.at("sceneObject");
					ImGui::Text(JsonContainerToString.at(sceneObject).c_str());
				}
				else
				{
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("scene");

					ImGui::TableSetColumnIndex(2);
					std::string sceneObject = nav.at("sceneObject");
					ImGui::Text(JsonContainerToString.at(sceneObject).c_str());
				}

				std::string name = nav.at("name");
				std::string path = nav.at("path");

				ImGui::TableSetColumnIndex(3);
				ImGui::Text(name.c_str());

				ImGui::TableSetColumnIndex(4);
				ImGui::Text(path.c_str());
			}

			ImGui::EndTable();
		}

		if (ImGui::Checkbox(allToDelete ? "Deselect all" : "Select all", &allToDelete))
		{
			setAllToDelete(allToDelete);
		}

		//ImGui::DrawItemWithEnabledState([this]
		//	{
		if (ImGui::Button("Delete"))
		{
			OnDelete(references);
		}
		//	}, deleteEnabled);

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			OnCancel();
		}
		ImGui::EndPopup();
	}
}
