#pragma once

//#define SODEF_TUPLE(SOClass) SOClass##SceneObjects SOClass##sceneObjects;\
//							 SOClass##SUSceneObjects SOClass##SUsceneObjects

#define SODEF_TUPLE(SOClass) SOClass##SUSceneObjects SOClass##SUsceneObjects

#define SODEF_CREATESCENEOBJECTS(SOClass) void Create##SOClass##SUSceneObjects(SceneUnitId id)\
{\
	SOClass##SUsceneObjects.insert_or_assign(id, SOClass##SceneObjects());\
}

//#define SODEF_GETSCENEOBJECTS(SOClass) SOClass##SceneObjects& Get##SOClass##sSceneObjects()\
//{\
//	return SOClass##sceneObjects;\
//}\
//SOClass##SceneObjects& Get##SOClass##sSUSceneObjects(SceneUnitId id)\
//{\
//	return SOClass##SUsceneObjects.at(id);\
//}

#define SODEF_GETSCENEOBJECTS(SOClass) SOClass##SceneObjects& Get##SOClass##sSUSceneObjects(SceneUnitId id)\
{\
	return SOClass##SUsceneObjects.at(id);\
}

/*
#define SODEF_CREATE(SOClass) void Create##SOClass(nlohmann::json& json, SceneUnitId id)\
{\
	CreateJsonSceneObject<SOClass::sceneObjectType, SOClass>(json, Get##SOClass##sSceneObjects, id);\
}\
void CreateSU##SOClass(SceneUnitId id, nlohmann::json& json)\
{\
	CreateJsonSUSceneObject<SOClass::sceneObjectType, SOClass>(id, json, Get##SOClass##sSUSceneObjects); \
}
*/

#define SODEF_CREATE(SOClass) void CreateSU##SOClass(SceneUnitId id, nlohmann::json& json)\
{\
	CreateJsonSUSceneObject<SOClass::sceneObjectType, SOClass>(id, json, Get##SOClass##sSUSceneObjects); \
}

//#define SODEF_GET(SOClass) std::unique_ptr<SOClass>& Get##SOClass##SceneObject(JUUID uuid)\
//{\
//	auto& tuple = SOClass##sceneObjects.at(uuid);\
//	auto& ptr = std::get<1>(tuple);\
//	return ptr;\
//}\
//std::unique_ptr<SOClass>& Get##SOClass##SUSceneObject(SceneUnitId id, JUUID uuid)\
//{\
//	auto& tuple = SOClass##SUsceneObjects.at(id).at(uuid);\
//	auto& ptr = std::get<1>(tuple);\
//	return ptr;\
//}

#define SODEF_GET(SOClass) std::unique_ptr<SOClass>& Get##SOClass##SUSceneObject(SceneUnitId id, JUUID uuid)\
{\
	auto& tuple = SOClass##SUsceneObjects.at(id).at(uuid);\
	auto& ptr = std::get<1>(tuple);\
	return ptr;\
}

//#define SODEF_GETUUIDNAMES(SOClass) std::vector<JUUIDName> Get##SOClass##sUUIDsNames()\
//{\
//	return GetUUIDsNames(SOClass##sceneObjects);\
//}\
//std::vector<JUUIDName> Get##SOClass##sSUUUIDsNames(SceneUnitId id)\
//{\
//	return GetUUIDsNames(SOClass##SUsceneObjects.at(id));\
//}

#define SODEF_GETUUIDNAMES(SOClass) std::vector<JUUIDName> Get##SOClass##sSUUUIDsNames(SceneUnitId id)\
{\
	return GetUUIDsNames(SOClass##SUsceneObjects.at(id));\
}

//#define SODEF_GETNAMES(SOClass) std::vector<JNAME> Get##SOClass##sNames()\
//{\
//	return GetNames(SOClass##sceneObjects);\
//}\
//std::vector<JNAME> Get##SOClass##sSUNames(SceneUnitId id)\
//{\
//	return GetNames(SOClass##SUsceneObjects.at(id));\
//}

#define SODEF_GETNAMES(SOClass) std::vector<JNAME> Get##SOClass##sSUNames(SceneUnitId id)\
{\
	return GetNames(SOClass##SUsceneObjects.at(id));\
}

//#define SODEF_GETNAME(SOClass) JNAME Get##SOClass##Name(SceneUnitId id, JUUID uuid)\
//{\
//	return GetName(uuid, Get##SOClass##sSceneObjects);\
//}\
//JNAME Get##SOClass##SUName(SceneUnitId id, JUUID uuid)\
//{\
//	return GetSUName(id, uuid, Get##SOClass##sSUSceneObjects);\
//}

#define SODEF_GETNAME(SOClass) JNAME Get##SOClass##SUName(SceneUnitId id, JUUID uuid)\
{\
	return GetSUName(id, uuid, Get##SOClass##sSUSceneObjects);\
}

//#define SODEF_GETUUIDBYNAME(SOClass) JUUID Get##SOClass##UUIDByName(JNAME name)\
//{\
//	return GetUUIDByName(name, Get##SOClass##sSceneObjects);\
//}\
//JUUID Get##SOClass##SUUUIDByName(SceneUnitId id, JNAME name)\
//{\
//	return GetUUIDBySUName(id, name, Get##SOClass##sSUSceneObjects);\
//}

#define SODEF_GETUUIDBYNAME(SOClass) JUUID Get##SOClass##SUUUIDByName(SceneUnitId id, JNAME name)\
{\
	return GetUUIDBySUName(id, name, Get##SOClass##sSUSceneObjects);\
}

//#define SODEF_WRITEJSON(SOClass) void Write##SOClass##SceneObjects(nlohmann::json& json)\
//{\
//	WriteSceneObjectsJson(json, SOClass##sceneObjects);\
//}\
//void Write##SOClass##SUSceneObjects(SceneUnitId id, nlohmann::json& json)\
//{\
//	WriteSceneObjectsJson(json, SOClass##SUsceneObjects.at(id));\
//}

#define SODEF_WRITEJSON(SOClass) void Write##SOClass##SUSceneObjects(SceneUnitId id, nlohmann::json& json)\
{\
	WriteSceneObjectsJson(json, SOClass##SUsceneObjects.at(id));\
}

//#define SODEF_RELEASE(SOClass) void Release##SOClass##SceneObjects()\
//{\
//	SOClass##sceneObjects.clear();\
//}\
//void Release##SOClass##SUSceneObjects(SceneUnitId id)\
//{\
//	SOClass##SUsceneObjects.erase(id);\
//}

#define SODEF_RELEASE(SOClass) void Release##SOClass##SUSceneObjects(SceneUnitId id)\
{\
	SOClass##SUsceneObjects.erase(id);\
}

//#define SODEF_EXIST(SOClass) bool SOClass##SceneObjectExist(JUUID uuid)\
//{\
//	return SOClass##sceneObjects.contains(uuid);\
//}\
//bool SOClass##SUSceneObjectExist(SceneUnitId id, JUUID uuid)\
//{\
//	return SOClass##SUsceneObjects.at(id).contains(uuid);\
//}

#define SODEF_EXIST(SOClass) bool SOClass##SUSceneObjectExist(SceneUnitId id, JUUID uuid)\
{\
	return SOClass##SUsceneObjects.at(id).contains(uuid);\
}

//#define SODEF_RENAME(SOClass) void Rename##SOClass##SceneObject(JUUID uuid, std::string newName)\
//{\
//	auto& tup = SOClass##sceneObjects.at(uuid);\
//	auto& refName = std::get<0>(tup);\
//	auto& ptr = std::get<1>(tup);\
//	refName = newName;\
//	ptr->name(newName);\
//}\
//void Rename##SOClass##SUSceneObject(SceneUnitId id, JUUID uuid, std::string newName)\
//{\
//	auto& tup = SOClass##SUsceneObjects.at(id).at(uuid); \
//	auto& refName = std::get<0>(tup); \
//	auto& ptr = std::get<1>(tup); \
//	refName = newName; \
//	ptr->name(newName); \
//}

#define SODEF_RENAME(SOClass) void Rename##SOClass##SUSceneObject(SceneUnitId id, JUUID uuid, std::string newName)\
{\
	auto& tup = SOClass##SUsceneObjects.at(id).at(uuid); \
	auto& refName = std::get<0>(tup); \
	auto& ptr = std::get<1>(tup); \
	refName = newName; \
	ptr->name(newName); \
}

//#define SODEF_DELETE(SOClass) void Delete##SOClass##SceneObject(JUUID uuid)\
//{\
//	DeleteJsonSceneObject<SOClass::sceneObjectType, SOClass>(uuid, Get##SOClass##sSceneObjects);\
//}\
//void Delete##SOClass##SUSceneObject(SceneUnitId id, JUUID uuid)\
//{\
//	DeleteJsonSUSceneObject<SOClass::sceneObjectType, SOClass>(id, uuid, Get##SOClass##sSUSceneObjects);\
//}

#define SODEF_DELETE(SOClass) void Delete##SOClass##SUSceneObject(SceneUnitId id, JUUID uuid)\
{\
	DeleteJsonSUSceneObject<SOClass::sceneObjectType, SOClass>(id, uuid, Get##SOClass##sSUSceneObjects);\
}

#define SODEF_FULL(SOClass) \
	SODEF_TUPLE(SOClass);\
	SODEF_CREATESCENEOBJECTS(SOClass);\
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
