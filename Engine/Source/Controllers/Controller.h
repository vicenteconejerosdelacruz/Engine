#pragma once

#include <memory>
#include <string>
#include <vector>
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
#endif
		virtual void Map(SUUUID so);
		virtual void Unmap();
		virtual void Step(float delta) {};
		//Scripting
		virtual v8_templates_creators GetV8TemplatesCreators();
		virtual v8_context_creators GetV8ContextCreators();
		virtual v8_functions_creators GetV8FunctionsCreators() { return {}; }
		//Rendering
		virtual void Render(SceneUnitId id) {};

		SceneUnitId unit;
		SUUUID sceneObject;
	};

#if defined(_EDITOR)
	std::map<unsigned int, std::set<JUUID>> GetControllersPrioritySet(bool ignoreEditorPlay = false);
#else
	std::map<unsigned int, std::set<JUUID>> GetControllersPrioritySet();
#endif
	JUUID RegisterController(std::string controllerName, SUUUID sceneObject, std::unique_ptr<Controller>& controller);
	void MapControllers(SceneUnitId id);
	std::vector<JUUIDName> GetControllersInstancesInSceneUnit(SceneUnitId id);
	std::set<JUUID> GetControllersInSceneUnit(SceneUnitId id);
	std::unique_ptr<Controller>& GetController(JUUID uuid);
	std::set<JUUID> GetControllersBySceneObjectUUID(SUUUID uuid);
	void DestroyControllers();
	void DestroyController(JUUID uuid);
	void StepControllers(DX::StepTimer& timer);

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
};
