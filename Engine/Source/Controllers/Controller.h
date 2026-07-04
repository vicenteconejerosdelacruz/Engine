#pragma once

#include <memory>
#include <string>
#include <vector>
#include <set>
#include <UUID.h>
#include <JObject.h>
#include <JTypes.h>
#include <nlohmann/json.hpp>
#include <StepTimer.h>
#include <NoV8.h>

#define DECL_CONTROLLER_DRAWER(JClass,ParentJClass)\
std::map<std::string, JEdvEditorDrawerFunction> GetControllerDrawers() override\
{\
	std::map<std::string, JEdvEditorDrawerFunction> drawers = ParentJClass::GetControllerDrawers();\
	drawers.merge(Get##JClass##Drawers());\
	return drawers;\
}\
std::vector<std::pair<std::string, JsonToEditorValueType>> GetControllerAttributes() override\
{\
	std::vector<std::pair<std::string, JsonToEditorValueType>> attributes = ParentJClass::GetControllerAttributes();\
	auto atts = Get##JClass##Attributes();\
	attributes.insert(attributes.end(), atts.begin(), atts.end());\
	return attributes;\
}

namespace Scene
{
	struct SceneObject;
};

struct ControllerBinding
{
	ControllerBinding() {}
	ControllerBinding(nlohmann::json json)
	{
		uuid = json.at("uuid");
		name = json.at("name");
	}
	ControllerBinding(JUUIDName uuidName)
	{
		uuid = std::get<0>(uuidName);
		name = std::get<1>(uuidName);
	}

	nlohmann::json ToJSON()
	{
		return {
			{ "uuid", uuid },
			{ "name", name }
		};
	}
	bool operator<(const ControllerBinding& other) const
	{
		if (uuid != other.uuid) return uuid < other.uuid;
		return name < other.name;
	}

	bool operator==(const ControllerBinding& other) const
	{
		return uuid == other.uuid && name == other.name;
	}
	bool operator!() const
	{
		return uuid.empty();
	}
	bool operator!()
	{
		return uuid.empty();
	}
	explicit operator bool() const
	{
		return !uuid.empty();
	}
	explicit operator bool()
	{
		return !uuid.empty();
	}

	ControllerBinding& operator=(const ControllerBinding& other)
	{
		if (this != &other) {
			name = other.name;
			uuid = other.uuid;
		}
		return *this;
	}

	JUUID uuid;
	JNAME name;
};

inline ControllerBinding ToControllerBinding(nlohmann::json j)
{
	return ControllerBinding(j);
}
inline nlohmann::json FromControllerBinding(ControllerBinding sb)
{
	return sb.ToJSON();
}

inline void to_json(nlohmann::json& j, const ControllerBinding& d) {
	j = const_cast<ControllerBinding&>(d).ToJSON();
}

inline void from_json(const nlohmann::json& j, ControllerBinding& d) {
	d = ControllerBinding(const_cast<nlohmann::json&>(j));
}

using namespace nov8;
namespace Game
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <ControllerAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <ControllerAtt.h>
#include <JEnd.h>

#endif
	struct Controller : JObject
	{
#include <Attributes/JFlags.h>
#include <ControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include <ControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <ControllerAtt.h>
#include <JEnd.h>

		virtual ~Controller() = default;
		Controller(nlohmann::json& json);
		virtual void JUpdate(nlohmann::json p);
		virtual void JPatch(nlohmann::json p);
		virtual void SetInitialConditions() {};
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
		virtual std::map<std::string, JEdvEditorDrawerFunction> GetControllerDrawers() { return Game::GetControllerDrawers(); }
		virtual std::vector<std::pair<std::string, JsonToEditorValueType>> GetControllerAttributes() { return Game::GetControllerAttributes(); }
		virtual void SwitchToPlayMode() {};
#endif
		virtual void Map(SUUUID so);
		virtual void Unmap();
		virtual void Step(float delta) {};
		virtual void Render(SceneUnitId id) {};
		//Scripting
		virtual std::set<std::string> GetControllerAliases() { return {}; }
		virtual void BindNestedControllers(Local<Context> context, Isolate* isolate, std::unique_ptr<SceneUnitScripting>& scriptData) {}

		SceneUnitId unit;
		SUUUID sceneObject;
	};

	std::map<JUUID, std::unique_ptr<Controller>>& GetControllersUUIDs();
	std::map<SUUUID, std::set<JUUIDName>>& GetControllerUUIDNameBySUUUID();
#if defined(_EDITOR)
	std::map<unsigned int, std::set<JUUID>> GetControllersPrioritySet(bool ignoreEditorPlay = false);
	std::map<unsigned int, std::set<JUUID>> GetControllersPrioritySet(SceneUnitId id, bool ignoreEditorPlay = false);
#else
	std::map<unsigned int, std::set<JUUID>> GetControllersPrioritySet();
	std::map<unsigned int, std::set<JUUID>> GetControllersPrioritySet(SceneUnitId id);
#endif
	JUUID RegisterController(std::string controllerName, SUUUID sceneObject, std::unique_ptr<Controller>& controller);
	void MapControllers(SceneUnitId id);
	std::vector<JUUIDName> GetControllersInstancesInSceneUnit(SceneUnitId id);
	template<typename T>
	std::vector<JUUIDName> GetDerivedControllersInstancesInSceneUnit(SceneUnitId id)
	{
		std::vector<JUUIDName> controllers;
		auto& uuid2ctrl = GetControllersUUIDs();
		auto& suuid2ctrlsuuidName = GetControllerUUIDNameBySUUUID();
		for (auto& [uuid, ctrl] : uuid2ctrl)
		{
			if (ctrl->unit != id || !dynamic_cast<T*>(ctrl.get())) continue;
			auto& uuidNameSet = suuid2ctrlsuuidName.at(ctrl->sceneObject);
			for (auto [uuidInSet, nameInSet] : uuidNameSet)
			{
				if (uuidInSet == uuid)
				{
					controllers.push_back(std::make_tuple(std::get<1>(ctrl->sceneObject), nameInSet));
					break;
				}
			}
		}
		SortUUIDByName(controllers);
		return controllers;
	}
	std::set<JUUID> GetControllersInSceneUnit(SceneUnitId id);
	std::unique_ptr<Controller>& GetController(JUUID uuid);
	std::set<JUUID> GetControllersBySceneObjectUUID(SUUUID uuid);
	std::set<JUUIDName> GetControllersUUIDNamesBySceneObjectUUID(SUUUID uuid);
	void DestroyControllers();
	void DestroyController(JUUID uuid);
	void StepControllers(DX::StepTimer& timer);
	SUUUID GetControllerSUUUID(JUUID uuid);

	template<typename T>
	T* GetController(JUUID uuid)
	{
		return static_cast<T*>(GetController(uuid).get());
	}

	Controller* GetController(SceneUnitId id, ControllerBinding cb);
	template<typename T>
	T* GetController(SceneUnitId id, ControllerBinding cb)
	{
		return static_cast<T*>(GetController(id, cb));
	};

	JUUID GetControllerUUID(SceneUnitId id, ControllerBinding cb);

	extern std::vector<std::string> GetControllers();
	extern JUUID CreateController(std::string name, SUUUID sceneObject, nlohmann::json& json);
	extern void CreateControllersMemberFunctionTemplates(Isolate* isolate, SceneUnitId id);
};
