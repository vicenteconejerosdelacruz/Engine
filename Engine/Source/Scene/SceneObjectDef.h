#pragma once

#define SODEF_TUPLE(SOClass) SOClass##SceneObjects SOClass##sceneObjects

#define SODEF_GETSCENEOBJECTS(SOClass) SOClass##SceneObjects& Get##SOClass##sSceneObjects()\
{\
	return SOClass##sceneObjects;\
}

#define SODEF_CREATE(SOClass) void Create##SOClass(nlohmann::json& json, SceneUnitId id)\
{\
	CreateJsonSceneObject<SOClass::sceneObjectType, SOClass>(json, Get##SOClass##sSceneObjects, id);\
}

#define SODEF_GET(SOClass) std::unique_ptr<SOClass>& Get##SOClass##SceneObject(JUUID uuid)\
{\
	auto& tuple = SOClass##sceneObjects.at(uuid);\
	auto& ptr = std::get<1>(tuple);\
	return ptr;\
}

#define SODEF_GETUUIDNAMES(SOClass) std::vector<JUUIDName> Get##SOClass##sUUIDsNames()\
{\
	return GetUUIDsNames(SOClass##sceneObjects);\
}

#define SODEF_GETNAMES(SOClass) std::vector<JNAME> Get##SOClass##sNames()\
{\
	return GetNames(SOClass##sceneObjects);\
}

#define SODEF_GETNAME(SOClass) JNAME Get##SOClass##Name(SceneUnitId id, JUUID uuid)\
{\
	return GetName(uuid, Get##SOClass##sSceneObjects);\
}

#define SODEF_GETUUIDBYNAME(SOClass) JUUID Get##SOClass##UUIDByName(JNAME name)\
{\
	return GetUUIDByName(name, Get##SOClass##sSceneObjects);\
}

#define SODEF_WRITEJSON(SOClass) void Write##SOClass##SceneObjects(nlohmann::json& json)\
{\
	WriteSceneObjectsJson(json, SOClass##sceneObjects);\
}

#define SODEF_RELEASE(SOClass) void Release##SOClass##SceneObjects()\
{\
	SOClass##sceneObjects.clear();\
}

#define SODEF_EXIST(SOClass) bool SOClass##SceneObjectExist(JUUID uuid)\
{\
	return SOClass##sceneObjects.contains(uuid);\
}

#define SODEF_RENAME(SOClass) void Rename##SOClass##SceneObject(JUUID uuid, std::string newName)\
{\
	auto& tup = SOClass##sceneObjects.at(uuid);\
	auto& refName = std::get<0>(tup);\
	auto& ptr = std::get<1>(tup);\
	refName = newName;\
	ptr->name(newName);\
}

#define SODEF_DELETE(SOClass) void Delete##SOClass##SceneObject(JUUID uuid)\
{\
	DeleteJsonSceneObject<SOClass::sceneObjectType, SOClass>(uuid, Get##SOClass##sSceneObjects);\
}

#define SODEF_FULL(SOClass) \
	SODEF_TUPLE(SOClass);\
	SODEF_GETSCENEOBJECTS(SOClass);\
	SODEF_CREATE(SOClass);\
	SODEF_GET(SOClass);\
	SODEF_GETUUIDNAMES(SOClass);\
	SODEF_GETNAMES(SOClass);\
	SODEF_GETNAME(SOClass);\
	SODEF_GETUUIDBYNAME(SOClass);\
	SODEF_WRITEJSON(SOClass);\
	SODEF_RELEASE(SOClass);\
	SODEF_EXIST(SOClass);\
	SODEF_RENAME(SOClass);\
	SODEF_DELETE(SOClass)
