#pragma once
#include <ShaderMaterials.h>
#include <nlohmann/json.hpp>
#include <SimpleMath.h>

template<MaterialVariablesTypes T, typename V>
MaterialVariableInitialValue TransformJsonToMaterialVariableInitialValue(nlohmann::json json) {
	V value = json.at("value");
	return MaterialVariableInitialValue({ .variableType = T, .value = value });
}

MaterialVariableInitialValue JsonToMaterialInitialValue(nlohmann::json json);

template<typename T>
void WriteMappedValueToDestination(MaterialVariableInitialValue& def, void* dst, size_t size)
{
	auto src = std::any_cast<T>(def.value);
	memcpy(dst, &src, size);
}

void WriteMappedInitialValuesToDestination(MaterialVariableInitialValue& def, void* dst, size_t size);

template<typename T>
void WriteMaterialVariableInitialValueToJson(nlohmann::json& matInitialValue, MaterialVariableInitialValue& varValue)
{
	auto value = std::any_cast<T>(varValue.value);
	matInitialValue["value"] = value;
}

nlohmann::json TransformMaterialValueMappingToJson(MaterialInitialValueMap mappedValues);

void TransformJsonToMaterialValueMapping(MaterialInitialValueMap& map, nlohmann::json object, std::string key);

void valueMappingToJson(MaterialVariablesTypes type, nlohmann::json& matInitialValue, MaterialVariableInitialValue& varValue);