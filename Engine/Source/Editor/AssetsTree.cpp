#include "pch.h"
#include "AssetsTree.h"

void AssetsTree::BuildAssetsTree(std::map<std::string, std::any>& assets, std::tuple<std::string, std::vector<std::string>>& uuidParts)
{
	std::string& uuid = std::get<0>(uuidParts);
	std::vector<std::string>& parts = std::get<1>(uuidParts);

	std::string part = *parts.begin();
	if (!assets.contains(part))
	{
		assets.insert_or_assign(part, std::map<std::string, std::any>());
	}

	if (parts.begin() != parts.end() - 1)
	{
		std::map<std::string, std::any>& child = std::any_cast<std::map<std::string, std::any>&>(assets.at(part));
		std::tuple<std::string, std::vector<std::string>> nextParts = std::make_tuple(uuid, std::vector<std::string>());
		std::copy(parts.begin() + 1, parts.end(), std::back_inserter(std::get<1>(nextParts)));
		BuildAssetsTree(child, nextParts);
	}
}

bool AssetsTree::HasSelectedChildren(std::map<std::string, std::any>& dump, std::string path)
{
	bool ret = false;
	for (auto it = dump.begin(); it != dump.end(); it++)
	{
		std::map<std::string, std::any>& child = std::any_cast<std::map<std::string, std::any>&>(it->second);
		std::string p = path + (path.empty() ? "" : "/") + it->first;

		if (child.empty())
		{
			std::string uuid = it->first;
			if (selected.contains(uuid)) return true;
		}
		else
		{
			ret |= HasSelectedChildren(child, path);
		}
	}

	return ret;
}