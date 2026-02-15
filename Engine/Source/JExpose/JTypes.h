#pragma once

#include <nlohmann/json.hpp>
#include <JObject.h>

enum JEXPOSE_TYPE
{
	JEXPOSE_TYPE_VALUE,
	JEXPOSE_TYPE_TRANSFORM,
	JEXPOSE_TYPE_ENUM,
	JEXPOSE_TYPE_VECTOR,
	JEXPOSE_TYPE_VECTOR_TRANSFORM,
	JEXPOSE_TYPE_SET,
	JEXPOSE_TYPE_MAP_TRANSFORM,
	JEXPOSE_TYPE_OBJECT,
	JEXPOSE_TYPE_VECTOR_OBJECT
};

enum JsonToEditorValueType
{
	jedv_t_hidden,
	jedv_t_object,
	jedv_t_vector,
	jedv_t_set,
	jedv_t_map,
	jedv_t_filepath,
	jedv_t_model3d_filepath,
	jedv_t_shaders_filepath,
	jedv_t_sounds_filepath,
	jedv_t_sound_instance_flags,
	jedv_t_filepath_vector,
	jedv_t_filepath_vector_image,
	jedv_t_enum,
	jedv_t_bitmask,
	jedv_t_integer,
	jedv_t_unsigned,
	jedv_t_float,
	jedv_t_float_angle,
	jedv_t_float_coneangle,
	jedv_t_float2,
	jedv_t_float2_angle,
	jedv_t_float2_dimension,
	jedv_t_float3,
	jedv_t_float3_angle,
	jedv_t_float4,
	jedv_t_color_float3,
	jedv_t_color_float4,
	jedv_t_string,
	jedv_t_boolean,
	jedv_t_animation_sequence,
	jedv_t_animation,
	jedv_t_tex_dimension,
	jedv_t_lighttype,
	jedv_t_dxgi_format,
	jedv_t_dxgi_format_vector,
	jedv_t_dxgi_depth_format,
	jedv_t_so_camera,
	jedv_t_so_camera_name,
	jedv_t_so_camera_vector,
	jedv_t_so_light,
	jedv_t_so_light_name,
	jedv_t_so_light_vector,
	jedv_t_so_renderable,
	jedv_t_so_renderable_name,
	jedv_t_so_renderable_vector,
	jedv_t_so_soundeffect,
	jedv_t_so_soundeffect_name,
	jedv_t_so_soundeffect_vector,
	jedv_t_so_physicscene_name,
	jedv_t_te_mesh,
	jedv_t_te_material,
	jedv_t_te_material_name,
	jedv_t_te_material_vector,
	jedv_t_te_model3d,
	jedv_t_te_model3d_name,
	jedv_t_te_model3d_vector,
	jedv_t_te_renderpass,
	jedv_t_te_renderpass_name,
	jedv_t_te_renderpass_vector,
	jedv_t_te_shader,
	jedv_t_te_shader_name,
	jedv_t_te_shader_vector,
	jedv_t_te_sound,
	jedv_t_te_sound_name,
	jedv_t_te_sound_vector,
	jedv_t_te_texture,
	jedv_t_te_texture_name,
	jedv_t_te_texture_vector,
	jedv_t_te_physycgeometry,
	jedv_t_te_physycgeometry_name,
	jedv_t_controller_vector,
	jedv_t_physic_object_vector,
	jedv_t_physic_behavior,
	jedv_draw_renderpass_vector,
	jedv_draw_animator_sequencer
};

typedef std::function<void(std::string attribute, std::vector<JObject*>& json)> JEdvEditorDrawerFunction;

typedef std::function<void(std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)> JEdvCreatorDrawerFunction;

typedef std::function<bool(std::string attribute, nlohmann::json& json)> JEdvCreatorValidatorFunction;