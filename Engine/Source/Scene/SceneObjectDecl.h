#pragma once

#define SODECL_TUPLE(SOClass) typedef std::tuple<JNAME, std::unique_ptr<SOClass>> SOClass##Tuple;\
typedef SceneObjectsContainer<SOClass##Tuple> SOClass##SceneObjects
#define SODECL_GETSCENEOBJECTS(SOClass) SOClass##SceneObjects& Get##SOClass##sSceneObjects()
#define SODECL_CREATE(SOClass) void Create##SOClass(nlohmann::json& json, SceneUnitId id = nostd::threadIdHash())
#define SODECL_GET(SOClass) std::unique_ptr<SOClass>& Get##SOClass##SceneObject(JUUID uuid)
#define SODECL_GETUUIDNAMES(SOClass) std::vector<JUUIDName> Get##SOClass##sUUIDsNames()
#define SODECL_GETNAMES(SOClass) std::vector<JNAME> Get##SOClass##sNames()
#define SODECL_GETNAME(SOClass) JNAME Get##SOClass##Name(SceneUnitId unit, JUUID uuid)
#define SODECL_GETUUIDBYNAME(SOClass) JUUID Get##SOClass##UUIDByName(JNAME name)
#define SODECL_WRITEJSON(SOClass) void Write##SOClass##SceneObject(nlohmann::json& json)
#define SODECL_RELEASE(SOClass) void Release##SOClass##SceneObjects()
#define SODECL_EXIST(SOClass) bool SOClass##SceneObjectExist(JUUID uuid)
#define SODECL_RENAME(SOClass) void Rename##SOClass##SceneObject(JUUID uuid, std::string newName)
#define SODECL_DELETE(SOClass) void Delete##SOClass##SceneObject(JUUID uuid)

#define SODECL_FULL(SOClass) \
	SODECL_TUPLE(SOClass);\
	SODECL_GETSCENEOBJECTS(SOClass);\
	SODECL_CREATE(SOClass);\
	SODECL_GET(SOClass);\
	SODECL_GETUUIDNAMES(SOClass);\
	SODECL_GETNAMES(SOClass);\
	SODECL_GETNAME(SOClass);\
	SODECL_GETUUIDBYNAME(SOClass);\
	SODECL_WRITEJSON(SOClass);\
	SODECL_RELEASE(SOClass);\
	SODECL_EXIST(SOClass);\
	SODECL_RENAME(SOClass);\
	SODECL_DELETE(SOClass)