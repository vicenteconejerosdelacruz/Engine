#pragma once
#include <string>

struct LoadingProgress
{
	LoadingProgress() { Reset(); }
	void Reset() {
		loadSceneUnitModal = false;
		defaultLevel = false;
		loading = false;
		levelName = "";
		asset = "";
		count = 0U;
		total = 0U;
	}
	void LoadLevel(bool defLevel, std::string name)
	{
		asset = "";
		count = 0U;
		total = 0U;
		defaultLevel = defLevel;
		levelName = name;
		loading = true;
		loadSceneUnitModal = true;
	}
	bool loadSceneUnitModal = true;
	bool defaultLevel = false;
	bool loading = false;
	std::string levelName;
	std::string asset;
	unsigned int count = 0U;
	unsigned int total = 0U;
};