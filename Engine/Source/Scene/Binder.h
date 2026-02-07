#pragma once
#include <functional>
#include <map>

//#define _DEBUG_UUID_NAMES
struct Binder {

	SceneUnitId unit;
	std::multimap<JUUID, JUUID> binding;

#if defined(_DEBUG_UUID_NAMES)
	std::map<JUUID, JNAME> bindingName;
	std::string name(JUUID uuid);
#endif

	void insert(JUUID soA, JUUID soB);
	void erase(JUUID soA);
	void erase(JUUID soA, JUUID soB);
};
