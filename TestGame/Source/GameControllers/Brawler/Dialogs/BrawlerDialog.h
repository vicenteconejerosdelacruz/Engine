#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct BrawlerDialogLine
{
	std::string name;
	std::string text;
	std::string picture;
};

inline BrawlerDialogLine ToBrawlerDialogLine(nlohmann::json& j)
{
	BrawlerDialogLine l;
	if (j.contains("name")) l.name = j.at("name");
	if (j.contains("text")) l.text = j.at("text");
	if (j.contains("picture")) l.picture = j.at("picture");
	return l;
}

inline nlohmann::json FromBrawlerDialogLine(BrawlerDialogLine l)
{
	nlohmann::json j = {
		{ "name", l.name },
		{ "text", l.text },
		{ "picture", l.picture }
	};
	return j;
}

struct BrawlerDialog
{
	std::string name;
	std::vector<BrawlerDialogLine> lines;
	std::string startScript;
	std::string endScript;
};

inline BrawlerDialog ToBrawlerDialog(nlohmann::json& j)
{
	BrawlerDialog d;
	if (j.contains("name")) d.name = j.at("name");
	if (j.contains("lines"))
	{
		for (int i = 0; i < j.at("lines").size(); i++)
		{
			d.lines.push_back(ToBrawlerDialogLine(j.at("lines").at(i)));
		}
	}
	if (j.contains("startScript")) d.startScript = j.at("startScript");
	if (j.contains("endScript")) d.endScript = j.at("endScript");
	return d;
}

inline nlohmann::json FromBrawlerDialog(BrawlerDialog d)
{
	nlohmann::json j = {
		{ "name", d.name },
		{ "lines", nlohmann::json::array() },
		{ "startScript", d.startScript },
		{ "endScript", d.endScript }
	};
	for (auto& l : d.lines)
	{
		j["lines"].push_back(FromBrawlerDialogLine(l));
	}
	return j;
}