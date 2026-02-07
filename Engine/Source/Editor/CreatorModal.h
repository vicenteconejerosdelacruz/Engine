#pragma once

template<typename T>
struct CreatorModal {
	bool creating = false;
	T type;
	SceneUnitId unit;
	nlohmann::json json;
	nlohmann::json modalProperties;
	std::vector<std::string> atts;
	std::map<std::string, JEdvCreatorDrawerFunction> drawers;
	std::map<std::string, JEdvCreatorValidatorFunction> validators;
	std::function<void(SceneUnitId unit, T type, nlohmann::json)> onCreate;

	void DrawCreationPopup(const char* title)
	{
		if (!creating) return;

		ImGui::OpenPopup(title);
		if (ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			for (auto& att : atts)
			{
				if (drawers.at(att)) drawers.at(att)(att, json, modalProperties);
			}
			bool valid = true;
			for (auto& att : atts)
			{
				if (validators.at(att)) valid &= validators.at(att)(att, json);
			}
			ImGui::DrawItemWithEnabledState([this]
				{
					if (ImGui::Button("Create"))
					{
						onCreate(unit, type, json);
						creating = false;
					}
				}, valid);
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				creating = false;
			}
			ImGui::EndPopup();
		}
	}
};