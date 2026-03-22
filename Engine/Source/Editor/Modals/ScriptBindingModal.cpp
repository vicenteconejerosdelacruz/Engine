#include "pch.h"
#include <imgui.h>
#include "ScriptBindingModal.h"
#include <ImEditor.h>

void ScripBindingModal::Init(
	ScriptBinding sb,
	JObject* object,
	std::function<std::vector<JUUIDName>()> getSceneObjects,
	std::function<SceneObject* (JUUID)> getSceneObjectPtr,
	std::function<void(nlohmann::json)> selector
)
{
	binding = sb;
	this->object = object;
	assetsTree.BuildAssetsTree(getSceneObjects, getSceneObjectPtr, true);
	if (object == nullptr)
	{
		selectables = { {"",ScriptBinding()} };
	}
	else
	{
		selectables = object->GetScriptBindingOptions();
		std::for_each(selectables.begin(), selectables.end(), [&](auto& pair)
			{
				pair.second.bindingName = binding.bindingName;
			}
		);
	}
	this->selector = selector;
	SimpleModal::Init(ImVec2(0.4f, 0.6f), "Binding");
}

void ScripBindingModal::Draw()
{
	if (!showing) return;

	SimpleModal::Draw([&](ImVec2 size)
		{
			ImVec2 treeSize = ImVec2(size.x, size.y - 30.0f);
			DrawAssetTreeSelector(treeSize);

			ImVec2 selectorSize = ImVec2(ImGui::GetWindowSize().x, 30.0f);
			DrawAssetResourceSelector(selectorSize);
		},
		!binding.uuid.empty() && !binding.bindingName.empty(),
		[&]
		{
			selector(binding.ToJSON());
			showing = false;
		}
	);
}

void ScripBindingModal::DrawAssetTreeSelector(ImVec2 size)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	ImGui::BeginChild("script-binding", size, 0);
	{
		assetsTree.DrawAssetsTree([&](JUUID assetUUID, JObject* assetObject)
			{
				std::string assetName = assetObject->at("name");
				std::string nodeID = "node-" + assetUUID;
				ImGui::PushID(nodeID.c_str());
				{
					if (ImGui::TreeNodeEx(assetName.c_str(), ImGuiTreeNodeFlags_Leaf | ((assetObject == object) ? ImGuiTreeNodeFlags_Selected : 0)))
					{
						if (ImGui::IsItemClicked() && ImGui::IsMouseClicked(ImGuiPopupFlags_MouseButtonLeft))
						{
							object = assetObject;
							selectables = object->GetScriptBindingOptions();
							auto it = std::find_if(selectables.begin(), selectables.end(), [](auto& pair) { return pair.second.bindingType == BT_SceneObject; });
							if (it != selectables.end())
								binding = it->second;
							else
								binding.Reset();
						}
						ImGui::TreePop();
					}
				}
				ImGui::PopID();
			}, "");
	}
	ImGui::EndChild();
	ImGui::PopStyleVar(2);
}

void ScripBindingModal::DrawAssetResourceSelector(ImVec2 size)
{
	ImGui::SetNextItemWidth(size.x * 0.3f);
	if (ImGui::InputText("name", &binding.bindingName))
	{
		std::for_each(selectables.begin(), selectables.end(), [&](auto& pair)
			{
				pair.second.bindingName = binding.bindingName;
			}
		);
	}
	bool enabled = false;
	if (object != nullptr)
	{
		if (selectables.size() == 0UL || (selectables.size() == 1ULL && selectables.begin()->first == ""))
		{
			selectables = { {"",ScriptBinding()} };
		}
		else
		{
			enabled = true;
		}
	}

	ImGui::SameLine();
	ImGui::SetNextItemWidth(size.x * 0.6f);
	ImGui::DrawItemWithEnabledState([&]
		{
			ImGui::DrawComboSelection(binding, selectables, [&](std::string itemName)
				{
					binding = selectables.at(itemName);
				});
		}, enabled);
}
