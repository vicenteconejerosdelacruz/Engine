#pragma once

#define SODECL_TUPLE(SOClass) typedef std::tuple<JNAME, std::unique_ptr<SOClass>> SOClass##Tuple;\
typedef SceneObjectsContainer<SOClass##Tuple> SOClass##SceneObjects;\
typedef std::unordered_map<SceneUnitId, SOClass##SceneObjects> SOClass##SUSceneObjects

#define SODECL_CREATESCENEOBJECTS(SOClass) void Create##SOClass##SceneObjects(SceneUnitId id)
#define SODECL_GETSCENEOBJECTS(SOClass) SOClass##SceneObjects& Get##SOClass##sSceneObjects(SceneUnitId id)
#define SODECL_CREATE(SOClass) void Create##SOClass(SceneUnitId id, nlohmann::json& json)
#define SODECL_GET(SOClass) std::unique_ptr<SOClass>& Get##SOClass##SceneObject(SceneUnitId id, JUUID uuid)
#define SODECL_GETIDNAMES(SOClass) std::vector<JUUIDName> Get##SOClass##sIDsNames(SceneUnitId id)
#define SODECL_GETNAMES(SOClass) std::vector<JNAME> Get##SOClass##sNames(SceneUnitId id)
#define SODECL_GETNAME(SOClass) JNAME Get##SOClass##Name(SOClass##ID so) 
#define SODECL_GETIDBYNAME(SOClass) SOClass##ID Get##SOClass##IDByName(SceneUnitId id, JNAME name)
#define SODECL_RELEASE(SOClass) void Release##SOClass##SceneObjects(SceneUnitId id)
#define SODECL_EXIST(SOClass) bool SOClass##SceneObjectExist(SOClass##ID so)
#define SODECL_RENAME(SOClass) void Rename##SOClass##SceneObject(SOClass##ID so, std::string newName)
#define SODECL_DELETE(SOClass) void Delete##SOClass##SceneObject(SOClass##ID so)

#define SODECL_FULL(SOClass) \
	SODECL_TUPLE(SOClass);\
	SODECL_CREATESCENEOBJECTS(SOClass);\
	SODECL_GETSCENEOBJECTS(SOClass);\
	SODECL_CREATE(SOClass);\
	SODECL_GET(SOClass);\
	SODECL_GETIDNAMES(SOClass);\
	SODECL_GETNAMES(SOClass);\
	SODECL_GETNAME(SOClass);\
	SODECL_GETIDBYNAME(SOClass);\
	SODECL_RELEASE(SOClass);\
	SODECL_EXIST(SOClass);\
	SODECL_RENAME(SOClass);\
	SODECL_DELETE(SOClass)