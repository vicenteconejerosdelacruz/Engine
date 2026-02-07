#pragma once

#define SODECL_TUPLE(SOClass) typedef std::tuple<JNAME, std::unique_ptr<SOClass>> SOClass##Tuple;\
typedef SceneObjectsContainer<SOClass##Tuple> SOClass##SceneObjects;\
typedef std::unordered_map<SceneUnitId, SOClass##SceneObjects> SOClass##SUSceneObjects

#define SODECL_CREATESCENEOBJECTS(SOClass) void Create##SOClass##SUSceneObjects(SceneUnitId id)
#define SODECL_GETSCENEOBJECTS(SOClass) SOClass##SceneObjects& Get##SOClass##sSUSceneObjects(SceneUnitId id)
#define SODECL_CREATE(SOClass) void CreateSU##SOClass(SceneUnitId id, nlohmann::json& json)
#define SODECL_GET(SOClass) std::unique_ptr<SOClass>& Get##SOClass##SUSceneObject(SceneUnitId id, JUUID uuid)
#define SODECL_GETUUIDNAMES(SOClass) std::vector<JUUIDName> Get##SOClass##sSUUUIDsNames(SceneUnitId id)
#define SODECL_GETNAMES(SOClass) std::vector<JNAME> Get##SOClass##sSUNames(SceneUnitId id)
#define SODECL_GETNAME(SOClass) JNAME Get##SOClass##SUName(SceneUnitId unit, JUUID uuid) 
#define SODECL_GETUUIDBYNAME(SOClass) JUUID Get##SOClass##SUUUIDByName(SceneUnitId id, JNAME name)
#define SODECL_WRITEJSON(SOClass) void Write##SOClass##SUSceneObject(SceneUnitId id, nlohmann::json& json)
#define SODECL_RELEASE(SOClass) void Release##SOClass##SUSceneObjects(SceneUnitId id)
#define SODECL_EXIST(SOClass) bool SOClass##SUSceneObjectExist(SceneUnitId id, JUUID uuid)
#define SODECL_RENAME(SOClass) void Rename##SOClass##SUSceneObject(SceneUnitId id, JUUID uuid, std::string newName)
#define SODECL_DELETE(SOClass) void Delete##SOClass##SUSceneObject(SceneUnitId id, JUUID uuid)

#define SODECL_FULL(SOClass) \
	SODECL_TUPLE(SOClass);\
	SODECL_CREATESCENEOBJECTS(SOClass);\
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