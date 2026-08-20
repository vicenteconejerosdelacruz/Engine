#pragma once
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

inline nlohmann::json yaml_to_json(const YAML::Node& node) {
	switch (node.Type()) {
	case YAML::NodeType::Null:
		return nullptr;

	case YAML::NodeType::Scalar: {
		bool bool_val;
		if (YAML::convert<bool>::decode(node, bool_val)) return bool_val;

		int64_t int_val;
		if (YAML::convert<int64_t>::decode(node, int_val)) return int_val;

		double double_val;
		if (YAML::convert<double>::decode(node, double_val)) return double_val;

		return node.as<std::string>();
	}

	case YAML::NodeType::Sequence: {
		nlohmann::json j = nlohmann::json::array();
		for (const auto& item : node) {
			j.push_back(yaml_to_json(item));
		}
		return j;
	}

	case YAML::NodeType::Map: {
		nlohmann::json j = nlohmann::json::object();
		for (const auto& it : node) {
			std::string key = it.first.as<std::string>();
			j[key] = yaml_to_json(it.second);
		}
		return j;
	}

	default:
		return nullptr;
	}
}

inline std::string yaml_to_string(const YAML::Node& node) {
	std::stringstream ss;
	ss << node;
	return ss.str();
}

inline nlohmann::json yaml_str_to_json(const std::string input)
{
	YAML::Node yaml = YAML::Load(input);
	return yaml_to_json(yaml);
}

inline YAML::Node json_to_yaml(const nlohmann::json& j) {
	YAML::Node node;

	if (j.is_null()) {
		return node; // Nodo nulo
	}
	if (j.is_boolean()) {
		node = j.get<bool>();
	}
	else if (j.is_number_integer()) {
		node = j.get<int64_t>();
	}
	else if (j.is_number_unsigned()) {
		node = j.get<uint64_t>();
	}
	else if (j.is_number_float()) {
		node = j.get<double>();
	}
	else if (j.is_string()) {
		node = j.get<std::string>();
	}
	else if (j.is_array()) {
		for (const auto& element : j) {
			node.push_back(json_to_yaml(element));
		}
	}
	else if (j.is_object()) {
		for (auto it = j.begin(); it != j.end(); ++it) {
			node[it.key()] = json_to_yaml(it.value());
		}
	}

	return node;
}

inline std::string sanitize_yaml_tabs(std::string str) {
	std::replace(str.begin(), str.end(), '\t', ' ');
	return str;
}
