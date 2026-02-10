#pragma once

#include <set>
#include <vector>
#include <string>
#include <map>
#include <UUID.h>

enum ShaderType {
	VERTEX_SHADER,
	PIXEL_SHADER,
	GEOMETRY_SHADER,
	COMPUTE_SHADER,
};

inline static std::map<ShaderType, std::string> ShaderTypeToStr = {
	{ VERTEX_SHADER, "VERTEX_SHADER" },
	{ PIXEL_SHADER, "PIXEL_SHADER" },
	{ GEOMETRY_SHADER, "GEOMETRY_SHADER" },
	{ COMPUTE_SHADER, "GEOMETRY_SHADER" },
};

inline static std::map<std::string, ShaderType> StrToShaderType = {
	{ "VERTEX_SHADER", VERTEX_SHADER },
	{ "PIXEL_SHADER", PIXEL_SHADER },
	{ "GEOMETRY_SHADER", GEOMETRY_SHADER },
	{ "COMPUTE_SHADER", COMPUTE_SHADER },
};

//shader compilation source (it's shader type, the hlsl path, the uuid and defines)
namespace Templates
{
	DEF_TEMPLATE_ID_DEP(ShaderJson, GetShaderTemplate);
};

using namespace Templates;
struct Source {
	ShaderType shaderType;
	std::wstring shaderTarget;
	ShaderJsonID shaderTemplate;
	std::vector<std::string> defines;

	bool operator<(const Source& other) const
	{
		return std::tie(shaderType, shaderTarget, shaderTemplate, defines) < std::tie(other.shaderType, other.shaderTarget, other.shaderTemplate, other.defines);
	}

	std::string to_string()
	{
		std::string defs;
		for (unsigned int i = 0; i < defines.size(); i++)
		{
			defs += defines[i];
			if (i < (defines.size() - 1))
				defs += ",";
		}
		return ShaderTypeToStr.at(shaderType) + ":" + shaderTemplate() + " (" + defs + ")";
	}
};

template <>
struct std::hash<Source>
{
	std::size_t operator()(const Source& src) const
	{
		using std::hash;
		std::string s = "";
		s += ShaderTypeToStr.at(src.shaderType);
		s += src.shaderTemplate();
		for (auto& v : src.defines) { s += v; }
		return hash<std::string>()(s);
	}
	std::size_t operator()(Source& src) const
	{
		using std::hash;
		std::string s = "";
		s += ShaderTypeToStr.at(src.shaderType);
		s += src.shaderTemplate();
		for (auto& v : src.defines) { s += v; }
		return hash<std::string>()(s);
	}
};

//define the byte code
typedef std::vector<unsigned char> ShaderByteCode;

//define the Constants Buffer binding
struct ShaderConstantsBufferParameter {
	unsigned int registerId;
	unsigned int numConstantsBuffers;
};
typedef std::unordered_map<std::string, ShaderConstantsBufferParameter> ShaderConstantsBufferParametersMap;
typedef std::pair<std::string, ShaderConstantsBufferParameter> ShaderConstantsBufferParametersPair;

template <>
struct std::hash<ShaderConstantsBufferParametersMap>
{
	std::size_t operator()(const ShaderConstantsBufferParametersMap& m) const
	{
		std::string s;
		for (auto& [k, v] : m)
		{
			s += k;
			s += std::to_string(v.registerId);
			s += std::to_string(v.numConstantsBuffers);
		}
		return hash<std::string>()(s);
	}
};

//define the UAV Buffer binding
struct ShaderUAVParameter {
	unsigned int registerId;
	unsigned int numUAV;
};
typedef std::map<std::string, ShaderUAVParameter> ShaderUAVParametersMap;
typedef std::pair<std::string, ShaderUAVParameter> ShaderUAVParametersPair;

template <>
struct std::hash<ShaderUAVParametersMap>
{
	std::size_t operator()(const ShaderUAVParametersMap& m) const
	{
		std::string s;
		for (auto& [k, v] : m)
		{
			s += k;
			s += std::to_string(v.registerId);
			s += std::to_string(v.numUAV);
		}
		return hash<std::string>()(s);
	}
};

//Texture Type
enum TextureShaderUsage
{
	TextureShaderUsage_None,
	TextureShaderUsage_IBLIrradiance,
	TextureShaderUsage_IBLPreFilteredEnvironment,
	TextureShaderUsage_IBLBRDFLUT,
	TextureShaderUsage_Base,
	TextureShaderUsage_NormalMap,
	TextureShaderUsage_MetallicRoughness,
	TextureShaderUsage_ShadowMaps,
	TextureShaderUsage_MinTexture,
	TextureShaderUsage_MaxTexture,
	TextureShaderUsage_DepthTexture,
	TextureShaderUsage_AverageLuminance,
};

typedef std::map<TextureShaderUsage, std::string> TextureShaderUsageMap;
typedef std::pair<TextureShaderUsage, std::string> TextureShaderUsagePair;

inline static std::map<TextureShaderUsage, std::string> TextureShaderUsageToString = {
	{ TextureShaderUsage_Base, "BaseTexture" },
	{ TextureShaderUsage_NormalMap, "NormalMapTexture" },
	{ TextureShaderUsage_MetallicRoughness, "MetallicRoughnessTexture" },
	{ TextureShaderUsage_ShadowMaps, "ShadowMapsTextures" },
	{ TextureShaderUsage_MinTexture, "MinTexture" },
	{ TextureShaderUsage_MaxTexture, "MaxTexture" },
	{ TextureShaderUsage_DepthTexture, "DepthTexture" },
	{ TextureShaderUsage_AverageLuminance, "AverageLuminance"},
	{ TextureShaderUsage_IBLIrradiance, "IBLIrradiance" },
	{ TextureShaderUsage_IBLPreFilteredEnvironment, "IBLPreFilteredEnvironment" },
	{ TextureShaderUsage_IBLBRDFLUT, "IBLBRDFLUT" },
};

inline static std::map<std::string, TextureShaderUsage> StringToTextureShaderUsage = {
	{ "BaseTexture", TextureShaderUsage_Base },
	{ "NormalMapTexture", TextureShaderUsage_NormalMap },
	{ "MetallicRoughnessTexture", TextureShaderUsage_MetallicRoughness },
	{ "ShadowMapsTextures", TextureShaderUsage_ShadowMaps },
	{ "MinTexture", TextureShaderUsage_MinTexture },
	{ "MaxTexture", TextureShaderUsage_MaxTexture },
	{ "DepthTexture", TextureShaderUsage_DepthTexture },
	{ "AverageLuminance", TextureShaderUsage_AverageLuminance},
	{ "IBLIrradiance", TextureShaderUsage_IBLIrradiance },
	{ "IBLPreFilteredEnvironment", TextureShaderUsage_IBLPreFilteredEnvironment },
	{ "IBLBRDFLUT", TextureShaderUsage_IBLBRDFLUT },
};

inline static std::map<TextureShaderUsage, std::string> textureShaderUsageToShaderDefine = {
	{ TextureShaderUsage_Base, "_HAS_BASE_TEXTURE" },
	{ TextureShaderUsage_NormalMap, "_HAS_NORMALMAP_TEXTURE" },
	{ TextureShaderUsage_MetallicRoughness, "_HAS_METALLIC_ROUGHNESS_TEXTURE" },
	{ TextureShaderUsage_ShadowMaps, "_HAS_SHADOWMAPS_TEXTURES" },
	{ TextureShaderUsage_MinTexture, "_HAS_MIN_TEXTURE" },
	{ TextureShaderUsage_MaxTexture, "_HAS_MAX_TEXTURE" },
	{ TextureShaderUsage_DepthTexture, "_HAS_DEPTH_TEXTURE" },
	{ TextureShaderUsage_AverageLuminance, "_HAS_AVERAGE_LUMINANCE" },
	{ TextureShaderUsage_IBLIrradiance, "_HAS_IBL_IRRADIANCE" },
	{ TextureShaderUsage_IBLPreFilteredEnvironment, "_HAS_IBL_PREFILTERED_ENVIRONMENT" },
	{ TextureShaderUsage_IBLBRDFLUT, "_HAS_IBL_BRDF_LUT" },
};

inline static std::vector<std::tuple<std::string, TextureShaderUsage>> iblJsonTextures = {
	{ "IBLIrradiance", TextureShaderUsage_IBLIrradiance },
	{ "IBLPreFilteredEnvironment", TextureShaderUsage_IBLPreFilteredEnvironment},
	{ "IBLBRDFLUT", TextureShaderUsage_IBLBRDFLUT},
};

inline static std::map<TextureShaderUsage, std::string> iblUsageTexture = {
	{ TextureShaderUsage_IBLIrradiance, "IBLIrradiance" },
	{ TextureShaderUsage_IBLPreFilteredEnvironment, "IBLPreFilteredEnvironment"},
	{ TextureShaderUsage_IBLBRDFLUT, "IBLBRDFLUT"},
};

inline static std::map<std::string, TextureShaderUsage> iblTextureUsage = {
	{ "IBLIrradiance", TextureShaderUsage_IBLIrradiance },
	{ "IBLPreFilteredEnvironment", TextureShaderUsage_IBLPreFilteredEnvironment},
	{ "IBLBRDFLUT", TextureShaderUsage_IBLBRDFLUT},
};

inline static std::map<TextureShaderUsage, std::string> textureShaderUsageInGammaSpaceToShaderDefine = {
	{ TextureShaderUsage_Base, "_BASE_TEXTURE_IN_GAMMA_SPACE" },
	{ TextureShaderUsage_NormalMap, "_NORMALMAP_TEXTURE_IN_GAMMA_SPACE" },
	{ TextureShaderUsage_MetallicRoughness, "_METALLIC_ROUGHNESS_TEXTURE_IN_GAMMA_SPACE" },
};

inline static std::set<TextureShaderUsage> materialTexturesShaderUsage = {
	TextureShaderUsage_Base,
	TextureShaderUsage_NormalMap,
	TextureShaderUsage_MetallicRoughness
};

inline void TransformJsonToMaterialTextures(std::map<TextureShaderUsage, std::string>& textures, nlohmann::json& object, const std::string& key)
{
	if (!object.contains(key)) return;

	nlohmann::json jtextures = object[key];

	for (nlohmann::json::iterator it = jtextures.begin(); it != jtextures.end(); it++)
	{
		textures.insert_or_assign(StringToTextureShaderUsage.at(it.key()), it.value());
	}
}

enum TextureType
{
	TextureType_2D,
	TextureType_Array,
	TextureType_Cube,
	TextureType_Skybox,
};

inline static std::unordered_map<TextureType, std::string> TextureTypeToString =
{
	{ TextureType_2D, "2D" },
	{ TextureType_Array, "Array" },
	{ TextureType_Cube, "Cube" },
	{ TextureType_Skybox, "Skybox" },
};

inline static std::unordered_map<std::string, TextureType> StringToTextureType =
{
	{ "2D", TextureType_2D },
	{ "Array", TextureType_Array },
	{ "Cube", TextureType_Cube },
	{ "Skybox", TextureType_Skybox},
};

//define the Textures binding
struct ShaderSRVParameter {
	unsigned int registerId;
	unsigned int numSRV;
};
typedef std::unordered_map<TextureShaderUsage, ShaderSRVParameter> ShaderSRVTexParametersMap;
typedef std::pair<TextureShaderUsage, ShaderSRVParameter> ShaderSRVTexParametersPair;
typedef std::unordered_map<std::string, ShaderSRVParameter> ShaderSRVCSParametersMap;
typedef std::pair<std::string, ShaderSRVParameter> ShaderSRVCSParametersPair;

template <>
struct std::hash<ShaderSRVCSParametersMap>
{
	std::size_t operator()(const ShaderSRVCSParametersMap& m) const
	{
		std::string s;
		for (auto& [k, v] : m)
		{
			s += k;
			s += std::to_string(v.registerId);
			s += std::to_string(v.numSRV);
		}
		return hash<std::string>()(s);
	}
};

template <>
struct std::hash<ShaderSRVTexParametersMap>
{
	std::size_t operator()(const ShaderSRVTexParametersMap& m) const
	{
		std::string s;
		for (auto& [k, v] : m)
		{
			s += k;
			s += std::to_string(v.registerId);
			s += std::to_string(v.numSRV);
		}
		return hash<std::string>()(s);
	}
};

//define the Samplers binding
struct ShaderSamplerParameter {
	unsigned int registerId;
	unsigned int numSamplers;
};
typedef std::unordered_map<std::string, ShaderSamplerParameter> ShaderSamplerParametersMap;
typedef std::pair<std::string, ShaderSamplerParameter> ShaderSamplerParametersPair;

template <>
struct std::hash<ShaderSamplerParametersMap>
{
	std::size_t operator()(const ShaderSamplerParametersMap& m) const
	{
		std::string s;
		for (auto& [k, v] : m)
		{
			s += k;
			s += std::to_string(v.registerId);
			s += std::to_string(v.numSamplers);
		}
		return hash<std::string>()(s);
	}
};

//define the layout of the cbuffer definition
struct ShaderConstantsBufferVariable {
	size_t bufferIndex;
	size_t size;
	size_t offset;
};
typedef std::unordered_map<std::string, ShaderConstantsBufferVariable> ShaderConstantsBufferVariablesMap;
typedef std::pair<std::string, ShaderConstantsBufferVariable> ShaderConstantsBufferVariablesPair;

//Collection of header(.h) files
typedef std::set<std::string> ShaderDependencies;

//Dependencies of hlsl+shaderType(Source) file to many .h files
typedef std::map<Source, ShaderDependencies> ShaderIncludesDependencies;

enum MaterialVariablesTypes {
	MAT_VAR_NONE,
	MAT_VAR_BOOLEAN,
	MAT_VAR_INTEGER,
	MAT_VAR_UNSIGNED_INTEGER,
	MAT_VAR_RGB,
	MAT_VAR_RGBA,
	MAT_VAR_FLOAT,
	MAT_VAR_FLOAT2,
	MAT_VAR_FLOAT3,
	MAT_VAR_FLOAT4,
	MAT_VAR_MATRIX4X4
};

static std::unordered_map<MaterialVariablesTypes, std::string> MaterialVariablesTypesToString = {
	{ MAT_VAR_BOOLEAN, "BOOLEAN"},
	{ MAT_VAR_INTEGER, "INTEGER"},
	{ MAT_VAR_UNSIGNED_INTEGER, "UNSIGNED_INTEGER"},
	{ MAT_VAR_RGB, "RGB"},
	{ MAT_VAR_RGBA, "RGBA"},
	{ MAT_VAR_FLOAT, "FLOAT"},
	{ MAT_VAR_FLOAT2, "FLOAT2"},
	{ MAT_VAR_FLOAT3, "FLOAT3"},
	{ MAT_VAR_FLOAT4, "FLOAT4"},
	{ MAT_VAR_MATRIX4X4, "MATRIX4X4"},
};

static std::unordered_map<std::string, MaterialVariablesTypes> StringToMaterialVariablesTypes = {
	{ "BOOLEAN", MAT_VAR_BOOLEAN },
	{ "INTEGER", MAT_VAR_INTEGER },
	{ "UNSIGNED_INTEGER", MAT_VAR_UNSIGNED_INTEGER },
	{ "RGB", MAT_VAR_RGB },
	{ "RGBA", MAT_VAR_RGBA },
	{ "FLOAT", MAT_VAR_FLOAT },
	{ "FLOAT2", MAT_VAR_FLOAT2 },
	{ "FLOAT3", MAT_VAR_FLOAT3 },
	{ "FLOAT4", MAT_VAR_FLOAT4 },
	{ "MATRIX4X4", MAT_VAR_MATRIX4X4 }
};

static std::unordered_map<MaterialVariablesTypes, size_t> MaterialVariablesTypesSizes = {
	{ MAT_VAR_BOOLEAN, sizeof(bool)},
	{ MAT_VAR_INTEGER, sizeof(int)},
	{ MAT_VAR_UNSIGNED_INTEGER, sizeof(unsigned int)},
	{ MAT_VAR_RGB, sizeof(float[3])},
	{ MAT_VAR_RGBA, sizeof(float[4])},
	{ MAT_VAR_FLOAT, sizeof(float)},
	{ MAT_VAR_FLOAT2, sizeof(float[2])},
	{ MAT_VAR_FLOAT3, sizeof(float[3])},
	{ MAT_VAR_FLOAT4, sizeof(float[4])},
	{ MAT_VAR_MATRIX4X4, sizeof(float[16])},
};

static std::unordered_map<std::string, MaterialVariablesTypes> HLSLVariableClassToMaterialVariableTypes = {
	{	"bool", MAT_VAR_BOOLEAN },
	{	"int", MAT_VAR_INTEGER },
	{	"uint", MAT_VAR_UNSIGNED_INTEGER },
	{	"unsigned int", MAT_VAR_UNSIGNED_INTEGER },
	{	"dword", MAT_VAR_UNSIGNED_INTEGER },
	{	"float", MAT_VAR_FLOAT },
	{	"float2", MAT_VAR_FLOAT2 },
	{	"float3", MAT_VAR_FLOAT3 },
	{	"float4", MAT_VAR_FLOAT4 },
	{	"float4x4", MAT_VAR_MATRIX4X4 },
	{	"matrix", MAT_VAR_MATRIX4X4 },
};

template <>
struct std::hash<std::tuple<std::string, std::string>>
{
	std::size_t operator()(const std::tuple<std::string, std::string> t) const
	{
		std::string s1 = std::get<0>(t);
		std::string s2 = std::get<1>(t);
		std::string s = s1 + s2;
		return hash<std::string>()(s);
	}
};

static std::unordered_map<std::tuple<std::string, std::string>, MaterialVariablesTypes, std::hash<std::tuple<std::string, std::string>>> HLSLVariablePatternToMaterialVariableTypes = {
	{ { "float3","color" }, MAT_VAR_RGB },
	{ { "float4","color" }, MAT_VAR_RGBA },
};

static std::set<D3D_SHADER_VARIABLE_CLASS> HLSLVariableClassAllowedTypes =
{
	D3D_SVC_SCALAR,
	D3D_SVC_VECTOR,
	D3D_SVC_MATRIX_ROWS,
	D3D_SVC_MATRIX_COLUMNS,
};

//ok this is ugly, for some reason dxil thinks that this StructuredBuffer<float> AverageLuminance : register(t1); it's a constant buffer
static std::set<std::string> HLSLNonConstantsBuffers = {
	"AverageLuminance"
};

static std::unordered_map<MaterialVariablesTypes, std::function<void(nlohmann::json&, std::string)>> MaterialVariablesMappedJsonInitializer = {
	{ MAT_VAR_BOOLEAN, [](nlohmann::json& mappedValues, std::string variable)
	{
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesToString.at(MAT_VAR_BOOLEAN)},
			{"value",false}
		});
	}},
	{ MAT_VAR_INTEGER, [](nlohmann::json& mappedValues, std::string variable) {
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesToString.at(MAT_VAR_INTEGER)},
			{"value",0}
		});
	}},
	{ MAT_VAR_UNSIGNED_INTEGER, [](nlohmann::json& mappedValues, std::string variable) {
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesToString.at(MAT_VAR_UNSIGNED_INTEGER)},
			{"value",0U}
		});
	}},
	{ MAT_VAR_RGB, [](nlohmann::json& mappedValues, std::string variable) {
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesToString.at(MAT_VAR_RGB)},
			{"value", {1.0f, 1.0f, 1.0f } }
		});
	}},
	{ MAT_VAR_RGBA, [](nlohmann::json& mappedValues, std::string variable) {
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesToString.at(MAT_VAR_RGBA)},
			{"value", {1.0f, 1.0f, 1.0f, 1.0f } }
		});
	}},
	{ MAT_VAR_FLOAT, [](nlohmann::json& mappedValues, std::string variable) {
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesToString.at(MAT_VAR_FLOAT)},
			{"value", 0.0f }
		});
	}},
	{ MAT_VAR_FLOAT2, [](nlohmann::json& mappedValues, std::string variable) {
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesToString.at(MAT_VAR_FLOAT2)},
			{"value", { 0.0f, 0.0f } }
		});
	}},
	{ MAT_VAR_FLOAT3, [](nlohmann::json& mappedValues, std::string variable) {
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesToString.at(MAT_VAR_FLOAT3)},
			{"value", { 0.0f, 0.0f, 0.0f }}
		});
	}},
	{ MAT_VAR_FLOAT4, [](nlohmann::json& mappedValues, std::string variable) {
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesToString.at(MAT_VAR_FLOAT4)},
			{"value", { 0.0f, 0.0f, 0.0f, 0.0f } }
		});
	}},
	{ MAT_VAR_MATRIX4X4, [](nlohmann::json& mappedValues, std::string variable) {
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesToString.at(MAT_VAR_MATRIX4X4)},
			{"value", {
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f,
			}}
		});
	}},
};

struct MaterialVariableInitialValue
{
	MaterialVariablesTypes variableType;
	std::any value;
};
typedef std::pair<std::string, MaterialVariableInitialValue> MaterialInitialValuePair;
typedef std::unordered_map<std::string, MaterialVariableInitialValue> MaterialInitialValueMap;

struct MaterialVariableMapping
{
	MaterialVariablesTypes variableType;
	ShaderConstantsBufferVariable mapping;
};
typedef std::unordered_map<std::string, MaterialVariableMapping> MaterialVariablesMapping;
typedef std::pair<std::string, MaterialVariableMapping> MaterialVariablesPair;