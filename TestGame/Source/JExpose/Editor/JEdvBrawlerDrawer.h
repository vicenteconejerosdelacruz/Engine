#pragma once

#include <Brawler/Rounds/BrawlerRound.h>
#include <Brawler/Characters/Enemies/Thug.h>

inline std::map<std::string, ControllerBinding> GetSelectableThugsControllers()
{
	std::vector<JUUIDName> controllers = GetDerivedControllersInstancesInSceneUnit<Brawler::Thug>(Editor::currentSceneUnitId);
	std::map<std::string, ControllerBinding> selectables = { { "",ControllerBinding()} };
	std::transform(controllers.begin(), controllers.end(), std::inserter(selectables, selectables.begin()), [](JUUIDName& uuidName)
		{
			JUUID sceneObjectId = std::get<0>(uuidName);
			JNAME controllerName = std::get<1>(uuidName);
			SceneObjectType type = GetSceneObjectType(Editor::currentSceneUnitId, sceneObjectId);
			SceneObject* so = GetSceneObjectPointer(Editor::currentSceneUnitId, sceneObjectId);
			std::string option = SceneObjectTypeToString.at(type) + "/" + std::string(so->at("name")) + ":" + controllerName;
			std::pair<std::string, ControllerBinding> pair(option, ControllerBinding(uuidName));
			return pair;
		}
	);
	return selectables;
}

inline std::set<ControllerBinding> GetBindedControllersInRounds(nlohmann::json& json, std::string att)
{
	std::set<ControllerBinding> bset;
	if (!json.contains(att))
		return bset;

	unsigned int numRounds = static_cast<unsigned int>(json.at(att).size());
	for (unsigned int i = 0; i < numRounds; i++)
	{
		unsigned int numEnemies = static_cast<unsigned int>(json.at(att).at(i).at("enemies").size());
		for (unsigned int e = 0; e < numEnemies; e++)
		{
			bset.insert(json.at(att).at(i).at("enemies").at(e));
		}
	}

	return bset;
}

template<>
inline JEdvEditorDrawerFunction DrawVector<BrawlerRound, jedv_t_vector>()
{
	return[](std::string attribute, std::vector<JObject*>& json)
		{
			if (json.size() > 1ULL) return;
			
			std::map<std::string, ControllerBinding> selectables = GetSelectableThugsControllers();
			std::set<ControllerBinding> bindedControllers = GetBindedControllersInRounds(*json.at(0), attribute);

			int deleteRound = -1;

			ImGui::Text(attribute.c_str());

			auto drawRoundScripts = [&](unsigned int index)
				{
					ImGui::Text("onStart");
					ImGui::SameLine();
					std::string onStartButtonId = "onStartButton-" + std::to_string(index);
					ImGui::PushID(onStartButtonId.c_str());
					{
						if (ImGui::Button(ICON_FA_RUNNING))
						{
							std::string script = json.at(0)->at(attribute).at(index).at("onStart");
							Editor::StartScriptEdition(attribute, script, [json, index, attribute](std::string value)
								{
									json.at(0)->at(attribute).at(index).at("onStart") = value;
									Editor::MarkSceneUnitAsModified(Editor::currentSceneUnitId);
								}
							);
						}
					}
					ImGui::PopID();
					ImGui::Text("onEnd");
					ImGui::SameLine();
					std::string onEndButtonId = "onEndButton-" + std::to_string(index);
					ImGui::PushID(onEndButtonId.c_str());
					{
						if (ImGui::Button(ICON_FA_RUNNING))
						{
							std::string script = json.at(0)->at(attribute).at(index).at("onEnd");
							Editor::StartScriptEdition(attribute, script, [json, index, attribute](std::string value)
								{
									json.at(0)->at(attribute).at(index).at("onEnd") = value;
									Editor::MarkSceneUnitAsModified(Editor::currentSceneUnitId);
								}
							);
						}
					}
					ImGui::PopID();
				};
			auto drawEnemySlot = [&selectables,&bindedControllers](nlohmann::json& round, unsigned int slot)
				{
					ControllerBinding selected;
					if (slot < round.at("enemies").size())
					{
						selected = ControllerBinding(round.at("enemies").at(slot));
					}
					std::map<std::string, ControllerBinding> availableSelectables;
					std::copy_if(selectables.begin(), selectables.end(),
						std::inserter(availableSelectables, availableSelectables.end()),
						[&bindedControllers,&selected](auto& pair)
						{
							return !bindedControllers.contains(pair.second) || (selected && pair.second==selected);
						}
					);

					std::string enemySlotId = "enemy-slot-" + std::to_string(slot);
					ImGui::PushID(enemySlotId.c_str());
					{
						ImGui::DrawComboSelection(selected, availableSelectables, [&](std::string option)
							{
								if(slot<round.at("enemies").size())
									round.at("enemies").at(slot) = FromControllerBinding(selectables.at(option));
								else
									round.at("enemies").push_back(FromControllerBinding(selectables.at(option)));
								Editor::MarkSceneUnitAsModified(Editor::currentSceneUnitId);
							}
						);
					}
					ImGui::PopID();
				};
			auto drawRoundEnemies = [&](unsigned int index)
				{
					std::string roundId = "round-" + std::to_string(index);
					ImGui::PushID(roundId.c_str());
					{
						unsigned int size = static_cast<unsigned int>(json.at(0)->at(attribute).at(index).at("enemies").size());
						for (unsigned int i = 0; i <= size; i++)
						{
							drawEnemySlot(json.at(0)->at(attribute).at(index), i);
						}
					}
					ImGui::PopID();
				};
			auto drawBrawlerRound = [&](unsigned int index)
				{
					if (ImGui::Button(ICON_FA_TIMES))
					{
						deleteRound = index;
					}
					ImGui::SameLine();
					if (ImGui::Button(ICON_FA_ARROW_UP))
					{

					}
					ImGui::SameLine();
					if (ImGui::Button(ICON_FA_ARROW_DOWN))
					{

					}
					ImGui::SameLine();
					std::string roundStr = "Round #" + std::to_string(index + 1);
					ImGui::Text(roundStr.c_str());
					drawRoundEnemies(index);
					drawRoundScripts(index);
				};

			unsigned int numRounds = json.at(0)->contains(attribute) ? static_cast<unsigned int>(json.at(0)->at(attribute).size()) : 0U;
			for (unsigned int i = 0; i < numRounds; i++)
			{
				std::string roundN = "round-" + std::to_string(i);
				ImGui::PushID(roundN.c_str());
				{
					drawBrawlerRound(i);
				}
				ImGui::PopID();
			}
			if (ImGui::Button(ICON_FA_PLUS, ImVec2(ImGui::GetContentRegionAvail().x, 20.0f)))
			{
				json.at(0)->at(attribute).push_back(FromBrawlerRound(BrawlerRound()));
			}

			ImGui::Separator();

			if (deleteRound != -1)
			{

			}
		};
}