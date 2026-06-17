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
			moldTreeSelection = false;
		};
	auto moldAvailable = [&]
		{
			return GetMoldUUIDByName(name).empty();
		};
	auto getUUIDMoldJson = [&](SceneUnitId id, JUUID uuid)
		{
			SceneObject* so = GetSceneObjectPointer(id, uuid);
			nlohmann::json j;
			so->WriteJson(j);
			so->DropJsonMoldAttributes(j);
			return j;
		};
	auto createMold = [&]()
		{
			bool rewrite = !moldAvailable();

			JUUID uuid = (!rewrite) ? getUUID() : GetMoldUUIDByName(name);

			nlohmann::json moldTemplate =
			{
				{ "uuid", uuid },
				{ "name", name },
			};
			if (!moldTreeSelection)
			{
				nlohmann::json j = getUUIDMoldJson(id, uuid);
				moldTemplate[SceneObjectTypeJsonContainer.at(type)] = nlohmann::json::array({ j });
			}
			else
			{
				for (auto& sel_uuid : selected_uuids)
				{
					SceneObjectType t = GetSceneObjectType(id, sel_uuid);
					std::string container = SceneObjectTypeJsonContainer.at(t);
					if (!moldTemplate.contains(container))
						moldTemplate[container] = nlohmann::json::array({});

					nlohmann::json j = getUUIDMoldJson(id, sel_uuid);
					moldTemplate[container].push_back(j);
				}
			}

			if (rewrite)
				DeleteMoldTemplate(uuid);
			CreateMold(moldTemplate);
			Editor::MarkTemplatesPanelAssetsAsDirty();
			show = false;
			moldTreeSelection = false;
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

			ImGui::Checkbox("use tree?", &moldTreeSelection);
			if (ImGui::Button(available ? "Create Mold" : "Replace Mold"))
			{
				createMold();
			}
		}
		else if (openedCollapsableItem == 1)
		{
			openedCollapsableItem = -1;
		}

		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		if (ImGui::Button("Cancel"))
		{
			show = false;
			moldTreeSelection = false;
		}

		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);
}