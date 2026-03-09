#pragma once

template<typename T, JsonToEditorValueType J>
JEdvCreatorDrawerFunction DrawCreatorValue() { return nullptr; }

template<typename T, JsonToEditorValueType J>
JEdvCreatorDrawerFunction DrawCreatorVector() { return nullptr; }

template<typename Ta, typename Tb>
JEdvCreatorDrawerFunction DrawCreatorMap() { return nullptr; }

template<typename E, JsonToEditorValueType J>
JEdvCreatorDrawerFunction DrawCreatorEnum(
	std::unordered_map<E, std::string>& EtoS,
	std::unordered_map<std::string, E>& StoE
) {
	if (J == jedv_t_hidden) return nullptr;
	return [&EtoS, &StoE](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			auto update = [attribute, &json](auto value)
				{
					nlohmann::json patch = { {attribute,value} };
					json.merge_patch(patch);
				};

			std::string selected = json.at(attribute);
			std::vector<std::string> options;
			std::transform(StoE.begin(), StoE.end(), std::back_inserter(options), [](auto& p) { return p.first; });

			ImGui::PushID(attribute.c_str());
			{
				ImGui::Text(attribute.c_str());
				ImGui::DrawComboSelection(selected, options, [update](std::string newOption)
					{
						update(newOption);
					}
				);
			}
			ImGui::PopID();
		};
}

template<>
JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_string>();

JEdvCreatorDrawerFunction DrawUniqueSUName(std::string objectName, std::function<std::vector<JNAME>(SceneUnitId)> getNames);

template<> JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_so_camera_name>();
template<> JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_so_light_name>();
template<> JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_so_renderable_name>();
template<> JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_so_soundeffect_name>();
template<> JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_so_physicscene_name>();
template<> JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_so_trigger_name>();
template<> JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_material_name>();
template<> JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_model3d_name>();
template<> JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_renderpass_name>();
template<> JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_shader_name>();
template<> JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_sound_name>();
template<> JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_texture_name>();
template<> JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_physycgeometry_name>();

template<>
JEdvCreatorDrawerFunction DrawCreatorValue<bool, jedv_t_boolean>();

template<>
JEdvCreatorDrawerFunction DrawCreatorValue<unsigned int, jedv_t_sound_instance_flags>();

void EditorCreatorDrawTemplateSelector(
	std::string attribute,
	nlohmann::json& json,
	std::function<JNAME(JUUID)> GetNameFromUUID,
	std::function<std::vector<JUUIDName>()> GetUUIDsNames
);

template<>
JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_shader>();

template<>
JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_sound>();

template<>
JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_material>();

template<>
JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_model3d>();

template<>
JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_renderpass>();

template<>
JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_texture>();

template<>
JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_physycgeometry>();

template<>
JEdvCreatorDrawerFunction DrawCreatorValue<MeshMaterial, jedv_t_mesh_material>();

template<>
JEdvCreatorDrawerFunction DrawCreatorVector<MeshMaterial, jedv_t_vector>();

template<>
JEdvCreatorDrawerFunction DrawCreatorVector<std::string, jedv_t_so_camera_vector>();

template<>
JEdvCreatorDrawerFunction DrawCreatorVector<DXGI_FORMAT, jedv_t_dxgi_format_vector>();

template<>
JEdvCreatorDrawerFunction DrawCreatorEnum<LightType, jedv_t_lighttype>(
	std::unordered_map<LightType, std::string>& EtoS,
	std::unordered_map<std::string, LightType>& StoE
);

bool EditorCreatorDrawFilePath(
	std::string attribute,
	nlohmann::json& json,
	std::string attText,
	const char* buttonIcon,
	nlohmann::json& modalProperties,
	std::vector<std::string> filterName,
	std::vector<std::string> filterPattern
);

bool EditorCreatorDrawFilePath(
	std::string attribute,
	nlohmann::json& json,
	unsigned int attIndex,
	std::string attText,
	const char* buttonIcon,
	nlohmann::json& modalProperties,
	std::vector<std::string> filterName,
	std::vector<std::string> filterPattern
);

template<>
JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_shaders_filepath>();

template<>
JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_sounds_filepath>();

template<>
JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_model3d_filepath>();

template<>
JEdvCreatorDrawerFunction DrawCreatorVector<std::string, jedv_t_filepath_vector_image>();