#pragma once
#include <map>
#include <string>
#include <any>
#include <JObject.h>
#include <set>
#include <UUID.h>

struct AssetsTree
{
	bool empty() const
	{
		return assets.empty();
	}

	void clear()
	{
		assetsJsons.clear();
	}

	void select(JUUID uuid)
	{
		selected.insert(uuid);
	}

	void clear_selection()
	{
		selected.clear();
	}

	bool is_selected(JUUID uuid) const
	{
		return selected.contains(uuid);
	}

	std::string get_name(JUUID uuid)
	{
		return assetsNames.at(uuid);
	}

	void BuildAssetsTree(auto GetObjects, auto GetPanelObject, bool refresh, std::string ignorePrefix = "")
	{
		if (assets.empty() || refresh)
		{
			assets.clear();
			assetsNames.clear();
			assetsJsons.clear();
			std::vector<JUUIDName> objects = GetObjects();
			for (JUUIDName uuidName : objects)
			{
				std::string uuid = std::get<0>(uuidName);
				std::string path = std::get<1>(uuidName);

				if (!ignorePrefix.empty())
				{
					path = std::regex_replace(path, std::regex(ignorePrefix), "");
				}

				std::vector<std::string> splitParts = nostd::split(path, "/");
				assetsNames.insert_or_assign(uuid, splitParts.back());
				assetsJsons.insert_or_assign(uuid, GetPanelObject(uuid));

				splitParts.pop_back();
				splitParts.push_back(uuid);
				std::tuple<std::string, std::vector<std::string>> uuidParts = std::make_tuple(uuid, splitParts);
				BuildAssetsTree(assets, uuidParts);
			}
		}
	}

	void BuildAssetsTree(std::map<std::string, std::any>& assets, std::tuple<std::string, std::vector<std::string>>& uuidParts);

	bool HasSelectedChildren(std::map<std::string, std::any>& dump, std::string path);

	void DrawAssetTreeNodes(std::map<std::string, std::any>& dump, std::string path, auto DrawItem)
	{
		std::map<std::string, std::any> nodes;
		std::copy_if(dump.begin(), dump.end(), std::inserter(nodes, nodes.begin()), [](auto& pair)
			{
				std::map<std::string, std::any>& child = std::any_cast<std::map<std::string, std::any>&>(pair.second);
				return !child.empty();
			}
		);

		for (auto& [first, second] : nodes)
		{
			std::map<std::string, std::any>& child = std::any_cast<std::map<std::string, std::any>&>(second);
			std::string p = path + (path.empty() ? "" : "/") + first;
			if (HasSelectedChildren(child, p))
				ImGui::SetNextItemOpen(true);
			if (ImGui::TreeNodeEx(first.c_str()))
			{
				DrawAssetTreeNodes(child, p, DrawItem);
				ImGui::TreePop();
			}
		}

		std::map<std::string, std::any> leafs;
		std::vector<std::string> orderedLeafs;
		std::copy_if(dump.begin(), dump.end(), std::inserter(leafs, leafs.begin()), [](auto& pair)
			{
				std::map<std::string, std::any>& child = std::any_cast<std::map<std::string, std::any>&>(pair.second);
				return child.empty();
			}
		);

		std::transform(leafs.begin(), leafs.end(), std::back_inserter(orderedLeafs), [this](auto& pair) { return pair.first; });

		std::sort(orderedLeafs.begin(), orderedLeafs.end(), [this](std::string uuidA, std::string uuidB)
			{
				JObject* assetA = assetsJsons.at(uuidA);
				JObject* assetB = assetsJsons.at(uuidB);
				return assetA->at("name") < assetB->at("name");
			}
		);

		for (std::string uuid : orderedLeafs)
		{
			JObject* object = assetsJsons.at(uuid);
			DrawItem(uuid, object);
		}
	}

	void DrawAssetsTree(auto DrawItem, std::string ignorePrefix)
	{
		DrawAssetTreeNodes(assets, "", DrawItem);
	}

	std::set<std::string> selected;
	std::map<std::string, std::any> assets;
	std::map<std::string, std::string> assetsNames;
	std::map<std::string, JObject*> assetsJsons;
};