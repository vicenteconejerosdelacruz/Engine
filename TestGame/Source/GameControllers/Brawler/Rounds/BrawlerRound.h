#pragma once
#include <vector>
#include <Controller.h>

struct BrawlerRound
{
	std::vector<ControllerBinding> enemies;
	std::string onStart;
	std::string onEnd;
};


inline BrawlerRound ToBrawlerRound(nlohmann::json& j)
{
	BrawlerRound p;
	if(j.contains("enemies"))
	{
		for (unsigned int i = 0; i < j.at("enemies"); i++)
		{
			p.enemies.push_back(ControllerBinding(j.at("enemies").at(i)));
		}
	}
	if (j.contains("onStart")) p.onStart = j.at("onStart");
	if (j.contains("onEnd")) p.onStart = j.at("onEnd");
	return p;
}

inline nlohmann::json FromBrawlerRound(BrawlerRound p)
{
	nlohmann::json round = { { "enemies", nlohmann::json::array() } };
	for (auto& cb : p.enemies)
	{
		round.at("enemies").push_back(FromControllerBinding(cb));
	}
	round["onStart"] = p.onStart;
	round["onEnd"] = p.onEnd;

	return round;
}