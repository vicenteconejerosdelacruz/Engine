#pragma once

#include <set>
#include <map>
#include <vector>
#include <memory>
#include <d3d12shader.h>
#include <wrl.h>
#include <wrl/client.h>
#include <dxcapi.h>
#include <string>
#include <Templates.h>
#include <JTemplate.h>

namespace Templates
{
	inline static const std::string ShadowMapLightsShaderResourceViewName = TextureShaderUsageToString.at(TextureShaderUsage_ShadowMaps);

#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#endif

#if defined(_DEVELOPMENT)
	void ShaderJsonStep();
	void MonitorShaderChanges(std::string folder);
#endif

	namespace Shader
	{
		inline static const std::string templateName = "shaders.json";
		inline static const TemplateType templateType = T_Shaders;
	};

	struct ShaderJson : public JTemplate
	{
		TEMPLATE_DECL(Shader);

#include <Attributes/JFlags.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <ShaderAtt.h>
#include <JEnd.h>

		DEF_STRING2FLAGS_FUNC(ShaderJson, JTemplate);

#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif
	};

	struct ShaderInstance;

	TEMPDECL_FULL(Shader);
	TEMPDECL_REFTRACKER(Shader);
	DEF_TEMPLATE_ID(ShaderJson, GetShaderTemplate);
	DEF_TEMPLATE_ID(ShaderInstance, GetShaderInstance);

	struct ShaderInstance
	{
		JUUID instanceUUID;
		JUUID shaderUUID;
		Source shaderSource;

		//vertex shader semantics(POSITION,TEXCOORD0, etc)
		std::vector<std::string> vsSemantics;

		//CBV
		ShaderConstantsBufferParametersMap constantsBuffersParameters;
		ShaderConstantsBufferVariablesMap constantsBuffersVariables;
		//UAV
		ShaderUAVParametersMap uavParameters;
		//SRV CS
		ShaderSRVCSParametersMap srvCSParameters;
		//SRV Tex
		ShaderSRVTexParametersMap srvTexParameters;
		//Samplers
		ShaderSamplerParametersMap samplersParameters;

		std::vector<size_t> cbufferSize;

		//Specific registers slots (c1,c2,c3,...) 
		struct
		{
			int camera = -1;
			int light = -1;
			int animation = -1;
			int lightsShadowMap = -1;
		} CBV;

		struct
		{
			int lightsShadowMap = -1;
			int iblIrradiance = -1;
			int iblPrefiteredEnv = -1;
			int iblBRDFLUT = -1;
		} SRV;

		//the bytecode(vector of bytes)
		ShaderByteCode byteCode;

		ShaderInstance(JUUID uuid) { assert(!!!"do not use"); }
		explicit ShaderInstance(
			JUUID instance_uuid,
			JUUID uuid, Source params,
			JUUID objectUUID = ""
		);
		~ShaderInstance() {}

		void CreateVSSemantics(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc);
		void CreateResourcesBinding(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc);
		void CreateConstantsBuffersVariables(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc);
		void CreateByteCode(const ComPtr<IDxcResult>& result);
	};
};

using namespace Templates;
DEF_TEMPLATE_ID_HASH(ShaderJson);
DEF_TEMPLATE_ID_HASH(ShaderInstance);
