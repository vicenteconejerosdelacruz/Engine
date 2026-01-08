#include "pch.h"
#include "Binder.h"
#include <Scene.h>
#include <SceneObject.h>
//#include <Renderable/Renderable.h>
//#include <Camera/Camera.h>
//#include <Light/Light.h>
//#include <Sound/SoundFX.h>

using namespace Scene;

#if defined(_DEBUG_UUID_NAMES)
//std::map<SceneObjectType, std::function<JNAME(JUUID)>> NameFnc =
//{
//	{ SO_Renderables, [](JUUID uuid)
//		{
//			auto& so = GetRenderableSceneObject(uuid);
//			return so->name();
//		}
//	},
//	{ SO_Cameras, [](JUUID uuid)
//		{
//			auto& so = GetCameraSceneObject(uuid);
//			return so->name();
//		}
//	},
//	{ SO_Lights, [](JUUID uuid)
//		{
//			auto& so = GetLightSceneObject(uuid);
//			return so->name();
//		}
//	},
//	{ SO_SoundEffects, [](JUUID uuid)
//		{
//			auto& so = GetSoundFXSceneObject(uuid);
//			return so->name();
//		}
//	}
//};
#endif

std::map<SceneObjectType, std::function<void(SceneUnitId, JUUID, JUUID)>> BindFnc =
{
	{ SO_Renderables, [](SceneUnitId id, JUUID uuid, JUUID uuidB)
		{
			auto& so = GetRenderableSUSceneObject(id, uuid);
			so->Bind(uuidB);
		}
	},
	{ SO_Cameras, [](SceneUnitId id, JUUID uuid, JUUID uuidB)
		{
			auto& so = GetCameraSUSceneObject(id, uuid);
			so->Bind(uuidB);
		}
	},
	{ SO_Lights, [](SceneUnitId id, JUUID uuid, JUUID uuidB)
		{
			auto& so = GetLightSUSceneObject(id, uuid);
			so->Bind(uuidB);
		}
	},
	{ SO_SoundEffects, [](SceneUnitId id, JUUID uuid, JUUID uuidB)
		{
			auto& so = GetSoundFXSUSceneObject(id, uuid);
			so->Bind(uuidB);
		}
	}
};

//std::map<SceneObjectType, std::function<void(JUUID, JUUID)>> UnbindFnc =
//{
//	{ SO_Renderables, [](JUUID uuid, JUUID uuidB)
//		{
//			auto& so = GetRenderableSceneObject(uuid);
//			so->Unbind(uuidB);
//		}
//	},
//	{ SO_Cameras, [](JUUID uuid, JUUID uuidB)
//		{
//			auto& so = GetCameraSceneObject(uuid);
//			so->Unbind(uuidB);
//		}
//	},
//	{ SO_Lights, [](JUUID uuid, JUUID uuidB)
//		{
//			auto& so = GetLightSceneObject(uuid);
//			so->Unbind(uuidB);
//		}
//	},
//	{ SO_SoundEffects, [](JUUID uuid, JUUID uuidB)
//		{
//			auto& so = GetSoundFXSceneObject(uuid);
//			so->Unbind(uuidB);
//		}
//	}
//};

#if defined(_DEBUG_UUID_NAMES)
std::string Binder::name(JUUID uuid)
{
	//if (bindingName.contains(uuid))
	//	return bindingName.at(uuid);

	//std::string name = NameFnc.at(GetSceneObjectType(uuid))(uuid);
	//bindingName.insert_or_assign(uuid, name);
	//return name;
	return "";
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
		OutputDebugStringA(std::string(std::string("insert:") + name(soA) + " -> " + name(soB) + "\n").c_str());
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
		OutputDebugStringA(std::string(std::string("insert:") + name(soB) + " -> " + name(soA) + "\n").c_str());
#endif
		binding.insert({ soB,soA });
		BindFnc.at(GetSceneObjectType(unit, soB))(unit, soB, soA);
	}
}

void Binder::erase(JUUID soA)
{
	//	std::set<JUUID> soBs;
	//	auto rangeA = binding.equal_range(soA);
	//	for (auto it = rangeA.first; it != rangeA.second; it++)
	//	{
	//		soBs.insert(it->second);
	//	}
	//#if defined(_DEBUG_UUID_NAMES)
	//	OutputDebugStringA(std::string(std::string("erase:") + name(soA) + "\n").c_str());
	//#endif
	//
	//	binding.erase(soA);
	//	for (auto soB : soBs)
	//	{
	//		auto rangeB = binding.equal_range(soB);
	//		for (auto it = rangeB.first; it != rangeB.second; )
	//		{
	//			if (it->second == soA)
	//			{
	//				it = binding.erase(it);
	//#if defined(_DEBUG_UUID_NAMES)
	//				OutputDebugStringA(std::string(std::string("erase:") + name(soB) + " -> " + name(soA) + "\n").c_str());
	//#endif
	//				if (SceneObjectExists(soA) && SceneObjectExists(soB))
	//				{
	//					UnbindFnc.at(GetSceneObjectType(soA))(soA, soB);
	//					UnbindFnc.at(GetSceneObjectType(soB))(soB, soA);
	//				}
	//			}
	//			else
	//				it++;
	//		}
	//	}
}

void Binder::erase(JUUID soA, JUUID soB)
{
	//	UnbindFnc.at(GetSceneObjectType(soA))(soA, soB);
	//	UnbindFnc.at(GetSceneObjectType(soB))(soB, soA);
	//
	//	auto rangeA = binding.equal_range(soA);
	//	for (auto it = rangeA.first; it != rangeA.second; )
	//	{
	//		if (it->second == soB)
	//		{
	//#if defined(_DEBUG_UUID_NAMES)
	//			OutputDebugStringA(std::string(std::string("erase:") + name(soA) + " -> " + name(soB) + "\n").c_str());
	//#endif
	//			it = binding.erase(it);
	//		}
	//		else
	//			it++;
	//	}
	//	auto rangeB = binding.equal_range(soB);
	//	for (auto it = rangeB.first; it != rangeB.second; )
	//	{
	//		if (it->second == soA)
	//		{
	//#if defined(_DEBUG_UUID_NAMES)
	//			OutputDebugStringA(std::string(std::string("erase:") + name(soB) + " -> " + name(soA) + "\n").c_str());
	//#endif
	//			it = binding.erase(it);
	//		}
	//		else
	//			it++;
	//	}
}