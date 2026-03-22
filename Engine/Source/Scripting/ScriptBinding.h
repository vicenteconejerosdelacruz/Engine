#pragma once
#include <string>
#include <nlohmann/json.hpp>

enum BindingType
{
	BT_SceneObject,
	BT_Controller,
	BT_PhysicObject,
};

static inline std::unordered_map<BindingType, std::string> BindingTypeToString =
{
	{ BT_SceneObject, "SceneObject" },
	{ BT_Controller, "Controller" },
	{ BT_PhysicObject, "PhysicObject" },
};

static inline std::unordered_map<std::string, BindingType> StringToBindingType =
{
	{ "SceneObject", BT_SceneObject },
	{ "Controller", BT_Controller },
	{ "PhysicObject", BT_PhysicObject },
};

struct ScriptBinding
{
	ScriptBinding(JUUID uuid = "")
	{
		Reset(uuid);
	}
	ScriptBinding(JUUID uuid, JNAME controllerName) : ScriptBinding(uuid, "", controllerName) {}
	ScriptBinding(JUUID uuid, JNAME bindingName, JNAME controllerName)
	{
		bindingType = BT_Controller;
		this->controllerName = controllerName;
		physicObjectIndex = 0;
		this->bindingName = bindingName;
		this->uuid = uuid;
	}
	ScriptBinding(JUUID uuid, unsigned int physicObjectIndex) : ScriptBinding(uuid, "", physicObjectIndex) {}
	ScriptBinding(JUUID uuid, JNAME bindingName, unsigned int physicObjectIndex)
	{
		bindingType = BT_PhysicObject;
		controllerName = "";
		this->physicObjectIndex = physicObjectIndex;
		this->bindingName = bindingName;
		this->uuid = uuid;
	}
	ScriptBinding(nlohmann::json j)
	{
		bindingType = StringToBindingType.at(j.at("bindingType"));
		controllerName = j.at("controllerName");
		physicObjectIndex = j.at("physicObjectIndex");
		bindingName = j.at("bindingName");
		uuid = j.at("uuid");
	}

	nlohmann::json ToJSON()
	{
		return {
			{ "bindingType", BindingTypeToString.at(bindingType) },
			{ "controllerName", controllerName },
			{ "physicObjectIndex", physicObjectIndex },
			{ "bindingName", bindingName },
			{ "uuid", uuid }
		};
	}

	void Reset(JUUID uuid = "", std::string bindingName = "")
	{
		bindingType = BT_SceneObject;
		controllerName = "";
		physicObjectIndex = 0;
		this->bindingName = bindingName;
		this->uuid = uuid;
	}

	bool operator<(const ScriptBinding& other) const
	{
		if (bindingType != other.bindingType) return bindingType < other.bindingType;
		if (uuid != other.uuid) return uuid < other.uuid;
		if (controllerName != other.controllerName) return controllerName != other.controllerName;
		if (bindingName != other.bindingName) return bindingName != other.bindingName;
		return physicObjectIndex < other.physicObjectIndex;
	}

	bool operator==(const ScriptBinding& other) const
	{
		return uuid == other.uuid &&
			bindingName == other.bindingName &&
			bindingType == other.bindingType &&
			controllerName == other.controllerName &&
			physicObjectIndex == other.physicObjectIndex;
	}

	ScriptBinding& operator=(const ScriptBinding& other)
	{
		if (this != &other) {
			bindingType = other.bindingType;
			controllerName = other.controllerName;
			physicObjectIndex = other.physicObjectIndex;
			bindingName = other.bindingName;
			uuid = other.uuid;
		}
		return *this;
	}

	//Binding Properties
	BindingType bindingType;
	JNAME controllerName;
	unsigned int physicObjectIndex;
	//Binding Resource
	JNAME bindingName;
	JUUID uuid;
};

inline ScriptBinding ToScriptBinding(nlohmann::json j)
{
	return ScriptBinding(j);
}
inline nlohmann::json FromScriptBinding(ScriptBinding sb)
{
	return sb.ToJSON();
}
