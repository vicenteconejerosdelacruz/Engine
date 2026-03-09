#include "pch.h"
#include "JEdvCreatorValidator.h"

template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_so_camera_name>() {
	return[](std::string attribute, nlohmann::json& json)
		{
			return json.at(attribute) != "";
		};
}

template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_so_physicscene_name>() {
	return[](std::string attribute, nlohmann::json& json)
		{
			return json.at(attribute) != "";
		};
}

template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_so_trigger_name>() {
	return[](std::string attribute, nlohmann::json& json)
		{
			return json.at(attribute) != "";
		};
}

template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_string>() {
	return[](std::string attribute, nlohmann::json& json)
		{
			return json.at(attribute) != "";
		};
}

template<>
JEdvCreatorValidatorFunction CreatorValidVector<std::string, jedv_t_so_camera_vector>()
{
	return [](std::string attribute, nlohmann::json& json)
		{
			return json.at(attribute).size() > 0 && json.at(attribute).at(0) != "";
		};
}

template<>
JEdvCreatorValidatorFunction CreatorValidVector<DXGI_FORMAT, jedv_t_dxgi_format_vector>()
{
	return [](std::string attribute, nlohmann::json& json)
		{
			for (unsigned int i = 0; i < json.at(attribute).size(); i++)
			{
				if (json.at(attribute).at(i) == "") return false;
			}
			return true;
		};
}

template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_te_sound>() {
	return[](std::string attribute, nlohmann::json& json)
		{
			return json.at(attribute) != "";
		};
}

template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_te_material>() {
	return[](std::string attribute, nlohmann::json& json)
		{
			return json.at(attribute) != "";
		};
}

template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_te_shader>() {
	return[](std::string attribute, nlohmann::json& json)
		{
			return json.at(attribute) != "";
		};
}

template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_te_texture>() {
	return[](std::string attribute, nlohmann::json& json)
		{
			return json.at(attribute) != "";
		};
}

template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_te_renderpass>() {
	return[](std::string attribute, nlohmann::json& json)
		{
			return json.at(attribute) != "";
		};
}

template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_te_model3d>() {
	return[](std::string attribute, nlohmann::json& json)
		{
			return json.at(attribute) != "";
		};
}

template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_te_physycgeometry>() {
	return[](std::string attribute, nlohmann::json& json)
		{
			return json.at(attribute) != "";
		};
}

template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_shaders_filepath>() {
	return[](std::string attribute, nlohmann::json& json)
		{
			return json.at(attribute) != "";
		};
}

template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_sounds_filepath>() {
	return[](std::string attribute, nlohmann::json& json)
		{
			return json.at(attribute) != "";
		};
}

template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_model3d_filepath>() {
	return[](std::string attribute, nlohmann::json& json)
		{
			return json.at(attribute) != "";
		};
}

template<>
JEdvCreatorValidatorFunction CreatorValidVector<std::string, jedv_t_filepath_vector_image>() {
	return[](std::string attribute, nlohmann::json& json)
		{
			if (json.at("name") == "") return false;

			unsigned int size = static_cast<unsigned int>(json.at(attribute).size());
			for (unsigned int i = 0; i < size; i++)
			{
				if (json.at(attribute).at(i) == "")
					return false;
			}
			return true;
		};
}
