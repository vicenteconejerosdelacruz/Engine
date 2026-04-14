#pragma once

#define TEXTFLOATREGEXREPLACE std::regex("\\*\\/")

namespace Editor
{
	extern SceneUnitId currentSceneUnitId;
};

template<typename T, JsonToEditorValueType J>
inline JEdvEditorDrawerFunction DrawValue() { return nullptr; }

template<typename T, JsonToEditorValueType J>
inline JEdvEditorDrawerFunction DrawVector() { return nullptr; }

template<typename Ta, typename Tb>
inline JEdvEditorDrawerFunction DrawMap() { return nullptr; }

JEdvEditorDrawerFunction DrawFlags(std::vector<std::string> flags);

template<typename E, JsonToEditorValueType J>
inline JEdvEditorDrawerFunction DrawEnum(
	std::unordered_map<E, std::string>& EtoS,
	std::unordered_map<std::string, E>& StoE
) {
	if (J == jedv_t_hidden) return[](std::string attribute, std::vector<JObject*>& json) {};
	return [&EtoS, &StoE](std::string attribute, std::vector<JObject*>& json)
		{
			auto allSame = [attribute, &json]()
				{
					std::set<std::string> s;
					for (auto& j : json)
					{
						s.insert(j->at(attribute));
						if (s.size() > 1) return false;
					}
					return true;
				};
			auto update = [attribute, &json](auto value)
				{
					nlohmann::json patch = { {attribute,value} };
					for (auto& j : json)
					{
						j->JUpdate(patch);
					}
				};
			bool allEq = allSame();
			ImGui::PushID(attribute.c_str());

			std::vector<std::string> options{};
			std::string selected = "";
			if (!allEq)
			{
				options.push_back("");
			}
			else
			{
				selected = json.at(0)->at(attribute);
			}
			std::transform(StoE.begin(), StoE.end(), std::back_inserter(options), [](auto& p) { return p.first; });

			std::string tableName = "tables-" + attribute + "-table";
			if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(attribute.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::DrawComboSelection(selected, options, [update](std::string newOption)
					{
						update(newOption);
					}
				);
				ImGui::EndTable();
			}
			ImGui::PopID();
		};
}

template <JsonToEditorValueType J>
inline JEdvEditorDrawerFunction DrawVectorObject() { return nullptr; }

template <JsonToEditorValueType J>
inline JEdvEditorDrawerFunction DrawPreview() { return nullptr; }

void EditorDrawFloat(std::string attribute, JObject* json, const char* format = "%.3f", std::function<float(float)> cb = [](float v) {return v; });
void EditorDrawFloatArray(std::string attribute, std::vector<JObject*>& json, std::vector<std::string> labels, const char* format = "%.3f");
void EditorDrawFloatAngleArray(std::string attribute, std::vector<JObject*>& json, std::vector<std::string> labels, const char* format = "%.3f", std::function<void(JObject*)> onUpdate = [](JObject*) {});
void EditorDrawVector(
	std::string attribute,
	std::vector<JObject*>& json,
	const char* iconCode,
	std::function<std::vector<JUUIDName>()> GetSelectableItems,
	std::function<std::string(std::string)> GetNameFromUUID,
	std::function<void(const char*, JUUIDName)> OpenItem = [](const char* icon, JUUIDName item) {},
	std::function<bool(unsigned int, JUUIDName)> FilterItem = [](unsigned int index, JUUIDName item) {return true; },
	std::function<bool(unsigned int, unsigned int, unsigned int, JUUIDName, JUUIDName)> CanSwap = [](unsigned int index1, unsigned int index2, unsigned int numItems, JUUIDName item1, JUUIDName item2) { return true; }
);
void EditorDrawColor3(std::string attribute, JObject* json, std::vector<std::string> labels);
bool EditorDrawString(std::string attribute, JObject* json);
void EditorDrawCheckBox(std::string attribute, JObject* json);
void EditorDrawEnum(std::string attribute, auto strOptions, JObject* json);
void EditorDrawSelectableInt(std::string attribute, std::vector<std::string> selectables, JObject* json);

template<>
JEdvEditorDrawerFunction DrawEnum<LightType, jedv_t_lighttype>(
	std::unordered_map<LightType, std::string>& EtoS,
	std::unordered_map<std::string, LightType>& StoE
);

void EditorDrawFilePath(
	std::string attribute,
	std::vector<JObject*>& json,
	const char* buttonIcon,
	const std::string defaultFolder,
	std::vector<std::string> filterName,
	std::vector<std::string> filterPattern
);

template<>
JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_model3d_filepath>();
template<>
JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_shaders_filepath>();
template<>
JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_sounds_filepath>();
template<>
JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_htmls_filepath>();

template<>
JEdvEditorDrawerFunction DrawValue<int, jedv_t_integer>();
template<>
JEdvEditorDrawerFunction DrawValue<unsigned int, jedv_t_unsigned>();

template<>
JEdvEditorDrawerFunction DrawVector<std::string, jedv_t_filepath_vector_image>();

template<>
JEdvEditorDrawerFunction DrawValue<float, jedv_t_float>();
template<>
JEdvEditorDrawerFunction DrawValue<float, jedv_t_float_angle>();
template<>
JEdvEditorDrawerFunction DrawValue<float, jedv_t_float_coneangle>();
template<>
JEdvEditorDrawerFunction DrawValue<XMFLOAT2, jedv_t_float2>();
template<>
JEdvEditorDrawerFunction DrawValue<XMFLOAT2, jedv_t_float2_angle>();
template<>
JEdvEditorDrawerFunction DrawValue<XMFLOAT3, jedv_t_float3>();
template<>
JEdvEditorDrawerFunction DrawValue<XMFLOAT3, jedv_t_float3_angle>();
template<>
JEdvEditorDrawerFunction DrawValue<XMFLOAT3, jedv_t_color_float3>();
template<>
JEdvEditorDrawerFunction DrawValue<XMFLOAT4, jedv_t_float4>();
template<>
JEdvEditorDrawerFunction DrawValue<XMFLOAT4, jedv_t_color_float4>();

template<>
JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_string>();

JEdvEditorDrawerFunction DrawNonEmptyValue(std::function<void()> onChange);

template<> JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_so_camera_name>();
template<> JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_so_light_name>();
template<> JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_so_renderable_name>();
template<> JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_so_soundeffect_name>();
template<> JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_so_physicscene_name>();
template<> JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_so_trigger_name>();
template<> JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_so_controller_instance>();
template<> JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_te_material_name>();
template<> JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_te_model3d_name>();
template<> JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_te_renderpass_name>();
template<> JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_te_shader_name>();
template<> JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_te_sound_name>();
template<> JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_te_texture_name>();
template<> JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_te_physycgeometry_name>();
template<> JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_te_htmlui_name>();

void DrawResourceSelection(
	std::string attribute,
	std::vector<JObject*>& json,
	std::function<std::string(JUUID)> ResourceUUIDToName,
	std::function<std::vector<JUUIDName>()> GetResourcesUUIDsNames,
	const char* iconCode,
	std::function<void(const char*, JUUIDName)> OpenItem = [](const char* icon, JUUIDName resource) {},
	bool readOnly = false,
	std::function<void(JUUID)> updateCb = [](JUUID) {}
);

template<>
JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_so_renderable>();
template<>
JEdvEditorDrawerFunction DrawValue<ControllerBinding, jedv_t_so_controller_instance>();
template<>
JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_te_mesh>();
template<>
JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_te_shader>();
template<>
JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_te_model3d>();
template<>
JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_te_sound>();
template<>
JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_te_texture>();
template<>
JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_te_physycgeometry>();
template<>
JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_te_htmlui>();
template<>
JEdvEditorDrawerFunction DrawValue<bool, jedv_t_boolean>();
template<>
JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_animation_sequence>();
template<>
JEdvEditorDrawerFunction DrawValue<unsigned int, jedv_t_tex_dimension>();

template<>
JEdvEditorDrawerFunction DrawVector<std::string, jedv_t_te_material_vector>();
template<>
JEdvEditorDrawerFunction DrawVector<std::string, jedv_t_te_model3d_vector>();
template<>
JEdvEditorDrawerFunction DrawVector<std::string, jedv_t_te_renderpass_vector>();
template<>
JEdvEditorDrawerFunction DrawVector<std::string, jedv_t_te_shader_vector>();
template<>
JEdvEditorDrawerFunction DrawVector<std::string, jedv_t_te_sound_vector>();
template<>
JEdvEditorDrawerFunction DrawVector<std::string, jedv_t_te_texture_vector>();
template<>
JEdvEditorDrawerFunction DrawVector<std::string, jedv_t_so_camera_vector>();
template<>
JEdvEditorDrawerFunction DrawVector<std::string, jedv_t_so_light_vector>();
template<>
JEdvEditorDrawerFunction DrawVector<std::string, jedv_t_so_renderable_vector>();
template<>
JEdvEditorDrawerFunction DrawVector<std::string, jedv_t_so_soundeffect_vector>();
template<>
JEdvEditorDrawerFunction DrawVector<MeshMaterial, jedv_t_vector>();

struct MaterialSamplerDesc;
template<>
JEdvEditorDrawerFunction DrawVector<MaterialSamplerDesc, jedv_t_vector>();

bool MaterialVariableBooleanDrawer(unsigned int index, nlohmann::json& value);
bool MaterialVariableIntegerDrawer(unsigned int index, nlohmann::json& value);
bool MaterialVariableUnsignedIntegerDrawer(unsigned int index, nlohmann::json& value);
bool MaterialVariableRGBDrawer(unsigned int index, nlohmann::json& value);
bool MaterialVariableRGBADrawer(unsigned int index, nlohmann::json& value);
bool MaterialVariableFloatDrawer(unsigned int index, nlohmann::json& value);
bool MaterialVariableFloat2Drawer(unsigned int index, nlohmann::json& value);
bool MaterialVariableFloat3Drawer(unsigned int index, nlohmann::json& value);
bool MaterialVariableFloat4Drawer(unsigned int index, nlohmann::json& value);
bool MaterialVariableMatrix4x4Drawer(unsigned int index, nlohmann::json& value);

template<>
JEdvEditorDrawerFunction DrawVector<MaterialInitialValuePair, jedv_t_vector>();

template<>
JEdvEditorDrawerFunction DrawVector<DXGI_FORMAT, jedv_t_dxgi_format_vector>();

template<>
JEdvEditorDrawerFunction DrawEnum<DXGI_FORMAT, jedv_t_dxgi_depth_format>(
	std::unordered_map<DXGI_FORMAT, std::string>& EtoS,
	std::unordered_map<std::string, DXGI_FORMAT>& StoE
);

struct MeshMaterial;
template<>
JEdvEditorDrawerFunction DrawValue<MeshMaterial, jedv_t_mesh_material>();

struct RasterizerDesc;
template<>
JEdvEditorDrawerFunction DrawValue<RasterizerDesc, jedv_t_object>();

struct RenderTargetBlendDesc;
struct BlendDesc;
template<>
JEdvEditorDrawerFunction DrawValue<BlendDesc, jedv_t_object>();

struct DepthStencilDesc;
template<>
JEdvEditorDrawerFunction DrawValue<DepthStencilDesc, jedv_t_object>();

template<>
JEdvEditorDrawerFunction DrawMap<TextureShaderUsage, std::string>();

template<>
JEdvEditorDrawerFunction DrawValue<Perspective, jedv_t_object>();

template<>
JEdvEditorDrawerFunction DrawValue<Orthographic, jedv_t_object>();

template<>
JEdvEditorDrawerFunction DrawVectorObject<jedv_t_controller_vector>();

template<>
JEdvEditorDrawerFunction DrawVectorObject<jedv_t_physic_object_vector>();

template<>
JEdvEditorDrawerFunction DrawPreview<jedv_draw_renderpass_vector>();

template<>
JEdvEditorDrawerFunction DrawPreview<jedv_draw_animator_sequencer>();

template<>
JEdvEditorDrawerFunction DrawPreview<jedv_cook_physx_mesh>();

template<>
JEdvEditorDrawerFunction DrawPreview<jedv_create_from_mold>();

template<>
JEdvEditorDrawerFunction DrawEnum<PhysicsBehavior, jedv_t_physic_behavior>(
	std::unordered_map<PhysicsBehavior, std::string>& EtoS,
	std::unordered_map<std::string, PhysicsBehavior>& StoE
);

template<>
JEdvEditorDrawerFunction DrawValue<std::string, jedv_t_script>();

template<>
JEdvEditorDrawerFunction DrawVector<ScriptBinding, jedv_t_vector>();