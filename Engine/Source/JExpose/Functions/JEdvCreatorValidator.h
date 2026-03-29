#pragma once

template<typename T, JsonToEditorValueType J>
JEdvCreatorValidatorFunction CreatorValidValue() {
	return[](std::string attribute, nlohmann::json& json) {return true; };
}

template<typename T, JsonToEditorValueType J>
JEdvCreatorValidatorFunction CreatorValidVector() {
	return[](std::string attribute, nlohmann::json& json) {return true; };
}

template<typename Ta, typename Tb>
JEdvCreatorValidatorFunction CreatorValidMap() { return[](std::string attribute, nlohmann::json& json) {return true; }; }

template<typename E, JsonToEditorValueType J>
JEdvCreatorValidatorFunction CreatorValidEnum(
	std::unordered_map<E, std::string>& EtoS,
	std::unordered_map<std::string, E>& StoE
) {
	return [&EtoS, &StoE](std::string attribute, nlohmann::json& json)
		{
			return true;
		};
}

template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_so_camera_name>();
template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_so_physicscene_name>();
template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_so_trigger_name>();
template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_string>();
template<>
JEdvCreatorValidatorFunction CreatorValidVector<std::string, jedv_t_so_camera_vector>();
template<>
JEdvCreatorValidatorFunction CreatorValidVector<DXGI_FORMAT, jedv_t_dxgi_format_vector>();
template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_te_sound>();
template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_te_material>();
template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_te_shader>();
template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_te_texture>();
template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_te_renderpass>();
template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_te_model3d>();
template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_te_physycgeometry>();
template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_shaders_filepath>();
template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_sounds_filepath>();
template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_model3d_filepath>();
template<>
JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_htmls_filepath>();
template<>
JEdvCreatorValidatorFunction CreatorValidVector<std::string, jedv_t_filepath_vector_image>();
