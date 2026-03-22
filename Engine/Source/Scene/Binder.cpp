#include "pch.h"
#include "Binder.h"
#include <Scene.h>
#include <SceneObject.h>

using namespace Scene;

#if defined(_DEBUG_UUID_NAMES)
std::map<SceneObjectType, std::function<JNAME(SceneUnitId, JUUID)>> NameFnc =
{
	{ SO_Renderables, [](SceneUnitId id, JUUID uuid)
		{
			auto& so = GetRenderableSceneObject(id, uuid);
			return so->name();
		}
	},
	{ SO_Cameras, [](SceneUnitId id, JUUID uuid)
		{
			auto& so = GetCameraSceneObject(id, uuid);
			return so->name();
		}
	},
	{ SO_Lights, [](SceneUnitId id, JUUID uuid)
		{
			auto& so = GetLightSceneObject(id, uuid);
			return so->name();
		}
	},
	{ SO_SoundEffects, [](SceneUnitId id, JUUID uuid)
		{
			auto& so = GetSoundFXSceneObject(id, uuid);
			return so->name();
		}
	},
	{ SO_PhysicScenes, [](SceneUnitId id, JUUID uuid)
		{
			auto& so = GetPhysicSceneSceneObject(id, uuid);
			return so->name();
		}
	},
	{ SO_Triggers, [](SceneUnitId id, JUUID uuid)
		{
			auto& so = GetTriggerSceneObject(id, uuid);
			return so->name();
		}
	},
	{ SO_Boundaries, [](SceneUnitId id, JUUID uuid)
		{
			auto& so = GetBoundarySceneObject(id, uuid);
			return so->name();
		}
	}
};
#endif

std::map<SceneObjectType, std::function<void(SceneUnitId, JUUID, JUUID)>> BindFnc =
{
	{ SO_Renderables, [](SceneUnitId id, JUUID uuid, JUUID uuidB)
		{
			auto& so = GetRenderableSceneObject(id, uuid);
			so->Bind(uuidB);
		}
	},
	{ SO_Cameras, [](SceneUnitId id, JUUID uuid, JUUID uuidB)
		{
			auto& so = GetCameraSceneObject(id, uuid);
			so->Bind(uuidB);
		}
	},
	{ SO_Lights, [](SceneUnitId id, JUUID uuid, JUUID uuidB)
		{
			auto& so = GetLightSceneObject(id, uuid);
			so->Bind(uuidB);
		}
	},
	{ SO_SoundEffects, [](SceneUnitId id, JUUID uuid, JUUID uuidB)
		{
			auto& so = GetSoundFXSceneObject(id, uuid);
			so->Bind(uuidB);
		}
	},
	{ SO_PhysicScenes, [](SceneUnitId id, JUUID uuid, JUUID uuidB)
		{
			auto& so = GetPhysicSceneSceneObject(id, uuid);
			so->Bind(uuidB);
		}
	},
	{ SO_Triggers, [](SceneUnitId id, JUUID uuid, JUUID uuidB)
		{
			auto& so = GetTriggerSceneObject(id, uuid);
			so->Bind(uuidB);
		}
	},
	{ SO_Boundaries, [](SceneUnitId id, JUUID uuid, JUUID uuidB)
		{
			auto& so = GetBoundarySceneObject(id, uuid);
			so->Bind(uuidB);
		}
	}
};

std::map<SceneObjectType, std::function<void(SceneUnitId, JUUID, JUUID)>> UnbindFnc =
{
	{ SO_Renderables, [](SceneUnitId id, JUUID uuid, JUUID uuidB)
		{
			auto& so = GetRenderableSceneObject(id, uuid);
			so->Unbind(uuidB);
		}
	},
	{ SO_Cameras, [](SceneUnitId id, JUUID uuid, JUUID uuidB)
		{
			auto& so = GetCameraSceneObject(id, uuid);
			so->Unbind(uuidB);
		}
	},
	{ SO_Lights, [](SceneUnitId id, JUUID uuid, JUUID uuidB)
		{
			auto& so = GetLightSceneObject(id, uuid);
			so->Unbind(uuidB);
		}
	},
	{ SO_SoundEffects, [](SceneUnitId id, JUUID uuid, JUUID uuidB)
		{
			auto& so = GetSoundFXSceneObject(id, uuid);
			so->Unbind(uuidB);
		}
	},
	{ SO_PhysicScenes, [](SceneUnitId id, JUUID uuid, JUUID uuidB)
		{
			auto& so = GetPhysicSceneSceneObject(id, uuid);
			so->Unbind(uuidB);
		}
	},
	{ SO_Triggers, [](SceneUnitId id, JUUID uuid, JUUID uuidB)
		{
			auto& so = GetTriggerSceneObject(id, uuid);
			so->Unbind(uuidB);
		}
	},
	{ SO_Boundaries, [](SceneUnitId id, JUUID uuid, JUUID uuidB)
		{
			auto& so = GetBoundarySceneObject(id, uuid);
			so->Unbind(uuidB);
		}
	}
};

#if defined(_DEBUG_UUID_NAMES)
std::string Binder::name(SceneUnitId id, JUUID uuid)
{
	if (bindingName.contains(uuid))
		return bindingName.at(uuid);

	std::string name = NameFnc.at(GetSceneObjectType(unit, uuid))(uuid);
	bindingName.insert_or_assign(uuid, name);
	return name;
}
#endif

void Binder::insert(JUUID soA, JUUID soB)
{
	bool AtoB = false;
	auto rangeAtoB = binding.equal_range(soA);
	for (auto& it = rangeAtoB.first; it != rangeAtoB.second; it++)
	{
		if (it->second == soB)
		{
			AtoB = true;
			break;
		}
	}
	if (!AtoB)
	{
#if defined(_DEBUG_UUID_NAMES)
		OutputDebugStringA(std::string(std::string("insert:") + name(unit, soA) + " -> " + name(unit, soB) + "\n").c_str());
#endif
		binding.insert({ soA,soB });
		BindFnc.at(GetSceneObjectType(unit, soA))(unit, soA, soB);
	}

	bool BtoA = false;
	auto rangeBtoA = binding.equal_range(soB);
	for (auto& it = rangeBtoA.first; it != rangeBtoA.second; it++)
	{
		if (it->second == soA)
		{
			BtoA = true;
			break;
		}
	}
	if (!BtoA)
	{
#if defined(_DEBUG_UUID_NAMES)
		OutputDebugStringA(std::string(std::string("insert:") + name(unit, soB) + " -> " + name(unit, soA) + "\n").c_str());
#endif
		binding.insert({ soB,soA });
		BindFnc.at(GetSceneObjectType(unit, soB))(unit, soB, soA);
	}
}

void Binder::erase(JUUID soA)
{
	std::set<JUUID> soBs;
	auto rangeA = binding.equal_range(soA);
	for (auto it = rangeA.first; it != rangeA.second; it++)
	{
		soBs.insert(it->second);
	}
#if defined(_DEBUG_UUID_NAMES)
	OutputDebugStringA(std::string(std::string("erase:") + name(unit, soA) + "\n").c_str());
#endif

	binding.erase(soA);
	for (auto soB : soBs)
	{
		auto rangeB = binding.equal_range(soB);
		for (auto it = rangeB.first; it != rangeB.second; )
		{
			if (it->second == soA)
			{
				it = binding.erase(it);
#if defined(_DEBUG_UUID_NAMES)
				OutputDebugStringA(std::string(std::string("erase:") + name(unit, soB) + " -> " + name(unit, soA) + "\n").c_str());
#endif
				if (SceneObjectExists(unit, soA) && SceneObjectExists(unit, soB))
				{
					UnbindFnc.at(GetSceneObjectType(unit, soA))(unit, soA, soB);
					UnbindFnc.at(GetSceneObjectType(unit, soB))(unit, soB, soA);
				}
			}
			else
				it++;
		}
	}
}

void Binder::erase(JUUID soA, JUUID soB)
{
	UnbindFnc.at(GetSceneObjectType(unit, soA))(unit, soA, soB);
	UnbindFnc.at(GetSceneObjectType(unit, soB))(unit, soB, soA);

	auto rangeA = binding.equal_range(soA);
	for (auto it = rangeA.first; it != rangeA.second; )
	{
		if (it->second == soB)
		{
#if defined(_DEBUG_UUID_NAMES)
			OutputDebugStringA(std::string(std::string("erase:") + name(unit, soA) + " -> " + name(unit, soB) + "\n").c_str());
#endif
			it = binding.erase(it);
		}
		else
			it++;
	}
	auto rangeB = binding.equal_range(soB);
	for (auto it = rangeB.first; it != rangeB.second; )
	{
		if (it->second == soA)
		{
#if defined(_DEBUG_UUID_NAMES)
			OutputDebugStringA(std::string(std::string("erase:") + name(unit, soB) + " -> " + name(unit, soA) + "\n").c_str());
#endif
			it = binding.erase(it);
		}
		else
			it++;
	}
}