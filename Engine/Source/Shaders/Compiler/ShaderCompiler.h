#pragma once

#include "CustomIncludeHandler.h"
#include <Shader/Shader.h>

using namespace Templates;

namespace ShaderCompiler {

	inline static std::unordered_map<ShaderType, std::wstring> shaderEntryPoint =
	{
		{ VERTEX_SHADER, L"main_vs"	},
		{ PIXEL_SHADER, L"main_ps" },
		{ COMPUTE_SHADER, L"main_cs" },
	};

	inline static std::unordered_map<ShaderType, std::wstring> shaderTarget =
	{
		{ VERTEX_SHADER, L"vs_6_5" },
		{ PIXEL_SHADER, L"ps_6_5" },
		{ COMPUTE_SHADER, L"cs_6_5" },
	};

	inline static std::unordered_map<ShaderType, std::wstring> shaderDefine =
	{
		{ VERTEX_SHADER, L"_VERTEX_SHADER" },
		{ PIXEL_SHADER, L"_PIXEL_SHADER" },
		{ COMPUTE_SHADER, L"_COMPUTE_SHADER" },
	};

	void BuildShaderCompiler();
	bool Compile(ShaderInstance& shaderInstance, Source params, ShaderIncludesDependencies& dependencies);
	void DestroyShaderCompiler();

	//compiler and utils
	static ComPtr<IDxcCompiler3> pCompiler;
	static ComPtr<IDxcUtils> pUtils;
};