#pragma once

#define SODEF_TUPLE(SOClass) SOClass##SUSceneObjects SOClass##SUsceneObjects

#define SODEF_CREATESCENEOBJECTS(SOClass) void Create##SOClass##SceneObjects(SceneUnitId id)\
{\
	if(SOClass##SUsceneObjects.contains(id)) return;\
	SOClass##SUsceneObjects.insert_or_assign(id, SOClass##SceneObjects());\
}

#define SODEF_GETSCENEOBJECTS(SOClass) SOClass##SceneObjects& Get##SOClass##sSceneObjects(SceneUnitId id)\
{\
	return SOClass##SUsceneObjects.at(id);\
}

#define SODEF_CREATE(SOClass) void Create##SOClass(SceneUnitId id, nlohmann::json& json)\
{\
	CreateJsonSceneObject<SOClass::sceneObjectType, SOClass>(id, json, Get##SOClass##sSceneObjects); \
}

#define SODEF_GET(SOClass) std::unique_ptr<SOClass>& Get##SOClass##SceneObject(SceneUnitId id, JUUID uuid)\
{\
	auto& tuple = SOClass##SUsceneObjects.at(id).at(uuid);\
	auto& ptr = std::get<1>(tuple);\
	return ptr;\
}

#define SODEF_GETIDNAMES(SOClass) std::vector<JUUIDName> Get##SOClass##sIDsNames(SceneUnitId id, bool getHidden)\
{\
	if(getHidden)\
		return GetUUIDsNames(SOClass##SUsceneObjects.at(id));\
	else\
		return GetNonHiddenUUIDsNames(SOClass##SUsceneObjects.at(id));\
}

#define SODEF_GETNAMES(SOClass) std::vector<JNAME> Get##SOClass##sNames(SceneUnitId id)\
{\
	return GetNames(SOClass##SUsceneObjects.at(id));\
}

#define SODEF_GETNAME(SOClass) JNAME Get##SOClass##Name(SOClass##ID so)\
{\
	return GetSUName(so.unit(), so.uuid(), Get##SOClass##sSceneObjects);\
}

#define SODEF_GETIDBYNAME(SOClass) SOClass##ID Get##SOClass##IDByName(SceneUnitId id, JNAME name)\
{\
	return MAKESUUUID(id,GetUUIDBySUName(id, name, Get##SOClass##sSceneObjects));\
}

#define SODEF_RELEASE(SOClass) void Release##SOClass##SceneObjects(SceneUnitId id)\
{\
	SOClass##SUsceneObjects.erase(id);\
}

#define SODEF_EXIST(SOClass) bool SOClass##SceneObjectExist(SOClass##ID so)\
{\
	return SOClass##SUsceneObjects.at(so.unit()).contains(so.uuid());\
}

#define SODEF_RENAME(SOClass) void Rename##SOClass##SceneObject(SOClass##ID so, std::string newName)\
{\
	auto& tup = SOClass##SUsceneObjects.at(so.unit()).at(so.uuid()); \
	auto& refName = std::get<0>(tup); \
	auto& ptr = std::get<1>(tup); \
	refName = newName; \
	ptr->name(newName); \
}

#define SODEF_DELETE(SOClass) void Delete##SOClass##SceneObject(SOClass##ID so)\
{\
	DeleteJsonSceneObject<SOClass::sceneObjectType, SOClass>(so.unit(), so.uuid(), Get##SOClass##sSceneObjects);\
}

#define SODEF_FULL(SOClass) \
	SODEF_TUPLE(SOClass);\
	SODEF_CREATESCENEOBJECTS(SOClass);\
	SODEF_GETSCENEOBJECTS(SOClass);\
	SODEF_CREATE(SOClass);\
	SODEF_GET(SOClass);\
	SODEF_GETIDNAMES(SOClass);\
	SODEF_GETNAMES(SOClass);\
	SODEF_GETNAME(SOClass);\
	SODEF_GETIDBYNAME(SOClass);\
	SODEF_RELEASE(SOClass);\
	SODEF_EXIST(SOClass);\
	SODEF_RENAME(SOClass);\
	SODEF_DELETE(SOClass)

#define RENAME_ON_DELETION(SOClass) \
markedForDelete.Hook([&]\
	{\
		lifecycleState->wait(false);\
		Rename##SOClass##SceneObject(SUuuid(), "delete-" + uuid());\
	}\
)