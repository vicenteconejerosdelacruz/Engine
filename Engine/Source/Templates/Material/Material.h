#pragma once

#include <VertexFormats.h>
#include "Variables.h"
#include "SamplerDesc.h"
#include "RasterizerDesc.h"
#include "BlendDesc.h"
#include <wrl/client.h>
#include <atlbase.h>
#include <TemplateDecl.h>
#include <Json.h>
#include <DXTypes.h>
#include <NoStd.h>
#include <Textures/Texture.h>
#include <ShaderMaterials.h>
#include <JTemplate.h>
#include <JTypes.h>
#include <Shader/Shader.h>

namespace Templates
{
	struct ShaderInstance;
};

using namespace Templates;

enum TextureShaderUsage;

typedef std::unordered_map<TextureShaderUsage, JUUID> TextureUsageInstanceMap; //JUUID(TextureInstance)
typedef std::pair<TextureShaderUsage, JUUID> TextureUsageInstancePair;

inline MaterialInitialValuePair ToMaterialInitialValuePair(nlohmann::json j)
{
	return MaterialInitialValuePair(j.at("variable"), JsonToMaterialInitialValue(j));
}

inline nlohmann::json FromMaterialInitialValuePair(MaterialInitialValuePair p)
{
	nlohmann::json j = nlohmann::json({});

	j["variable"] = p.first;
	j["variableType"] = MaterialVariablesTypesToString.at(p.second.variableType);
	valueMappingToJson(p.second.variableType, j, p.second);
	return j;
}

inline TextureShaderUsagePair ToTextureShaderUsagePair(nlohmann::json::iterator it)
{
	return { StringToTextureShaderUsage.at(it.key()), it.value() };
}

inline nlohmann::json FromTextureShaderUsagePair(TextureShaderUsagePair m)
{
	nlohmann::json j = nlohmann::json::object({});
	j[TextureShaderUsageToString.at(m.first)] = m.second;
	return j;
}

namespace Templates
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <MaterialAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <MaterialAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include <MaterialAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <MaterialAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <MaterialAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <MaterialAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <MaterialAtt.h>
#include <JEnd.h>

#endif

	void MaterialJsonStep();

	namespace Material
	{
		inline static const std::string templateName = "materials.json";
		inline static const TemplateType templateType = T_Materials;
#if defined(_EDITOR)
		inline static const std::string fallbackShader_vs = "BaseLighting_vs";
		inline static const std::string fallbackShader_ps = "BaseLighting_ps";
#endif
	};

	struct MaterialJson : public JTemplate
	{
		TEMPLATE_DECL(Material);

#include <Attributes/JFlags.h>
#include <MaterialAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <MaterialAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif
	};

	TEMPDECL_FULL(Material);

	//DESTROY
	//void FreeGPUTexturesUploadIntermediateResources();

	struct MaterialInstance
	{
		MaterialInstance(JUUID uuid) { assert(!!!"do not use"); }
		explicit MaterialInstance(
			JUUID Instance_uuid,
			JUUID Template_uuid,
			VertexClass vClass,
			bool isShadowed,
			bool hasIBL,
			TextureShaderUsageMap overrideTextures = {},
			JUUID ObjectUUID = "",
			JObjectChangeCallback cb = nullptr,
			JObjectChangePostCallback postCb = nullptr
		);
		~MaterialInstance() { Destroy(); }

		MaterialJsonUUID materialUUID;
		JUUID instanceUUID;

		ShaderJsonUUID vertexShaderUUID;
		ShaderJsonUUID pixelShaderUUID;

		VertexClass vertexClass;
		bool shadowed;
		bool ibl;
		MaterialVariablesMapping variablesMapping;
		std::vector<size_t> variablesBufferSize;
		std::vector<std::vector<byte>> variablesBuffer;

		std::vector<std::string> defines;
		ShaderInstanceUUID vertexShaderInstanceUUID;
		ShaderInstanceUUID pixelShaderInstanceUUID;
		std::vector<MaterialSamplerDesc> samplers;
		TextureUsageInstanceMap textures;
		std::map<unsigned int, ::CD3DX12_GPU_DESCRIPTOR_HANDLE> uav;

		void CreateMaterialShaderDefines();
		void CreateShaderInstances();
		void Destroy();
		bool ShaderInstanceHasRegister(std::function<int(ShaderInstanceUUID)> getRegister);
		void LoadVariablesMapping();
		void SetUAVRootDescriptorTable(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& slot);
		void SetSRVRootDescriptorTable(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& slot);
	};

	void DestroyMaterialInstance(JUUID materialInstance);

	TEMPDECL_REFTRACKER(Material);
};
