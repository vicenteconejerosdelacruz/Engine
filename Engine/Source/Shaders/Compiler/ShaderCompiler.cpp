#include "pch.h"
#include "ShaderCompiler.h"
#include <DirectXHelper.h>
#include <string>
#include <set>
#include <map>
#include <NoStd.h>
#include <Application.h>
#include <ShaderMaterials.h>

using namespace Templates;

namespace ShaderCompiler {

	static std::mutex compileMutex;
	bool Compile(ShaderInstance& shaderInstance, Source params, ShaderIncludesDependencies& dependencies)
	{
		std::lock_guard<std::mutex> lock(compileMutex);
		//read the shader

		auto& json = GetShaderTemplate(params.shaderUUID);

		const std::string filename = defaultShadersFolder + json->path();
		ShaderByteCode shaderSource = DX::ReadDataAsync(filename.c_str()).get();

		//create a blob for the shader source code
		ComPtr<IDxcBlobEncoding> pSource;
		pUtils->CreateBlob(&shaderSource[0], (UINT32)shaderSource.size(), CP_UTF8, pSource.GetAddressOf());

		//build the arguments
		std::vector<LPCWSTR> arguments;
		arguments.push_back(L"-E");// -E for the entry point (eg. 'main')
		arguments.push_back(shaderEntryPoint[params.shaderType].c_str());
		arguments.push_back(L"-T");// -T for the target profile (eg. 'ps_6_6')
		arguments.push_back(params.shaderTarget.c_str());

#ifndef _DEBUG
		arguments.push_back(L"-Qstrip_debug"); //remove debug info(PDB)
		arguments.push_back(L"-Qstrip_reflect"); // Strip reflection data and pdbs (see later)
#else
		arguments.push_back(L"-Qembed_debug"); //add debug info(PDB)
#endif

		arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX
#ifdef _DEBUG
		arguments.push_back(DXC_ARG_DEBUG); //-Zi
#endif

		arguments.push_back(DXC_ARG_PACK_MATRIX_ROW_MAJOR);

		std::vector<std::wstring> wdefines;
		std::transform(params.defines.begin(), params.defines.end(), std::back_inserter(wdefines), [](std::string def) { return nostd::StringToWString(def); });
		wdefines.push_back(shaderDefine.at(params.shaderType));

		for (const std::wstring& def : wdefines) {
			arguments.push_back(L"-D");
			arguments.push_back(def.c_str());
		}

		DxcBuffer sourceBuffer{ .Ptr = pSource->GetBufferPointer(), .Size = pSource->GetBufferSize(), .Encoding = 0 };

		std::string compiler_arguments;
		for (int i = 0; i < arguments.size(); i++)
		{
			compiler_arguments += nostd::WStringToString(arguments[i]);
			if (i < (arguments.size() - 1))
				compiler_arguments += " ";
		}
		OutputDebugStringA(std::string("Compiling :" + nostd::WStringToString(shaderDefine.at(params.shaderType)) + " " + filename + " " + compiler_arguments + "\n").c_str());

		//compile the shader
		ComPtr<IDxcResult> pCompileResult;
		dependencies[params].clear();
		CustomIncludeHandler includeHandler(dependencies[params], defaultShadersFolder, pUtils);
		DX::ThrowIfFailed(pCompiler->Compile(&sourceBuffer, arguments.data(), (UINT32)arguments.size(), &includeHandler, IID_PPV_ARGS(pCompileResult.GetAddressOf())));

		//get the errors
		ComPtr<IDxcBlobUtf8> pErrors;
		pCompileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(pErrors.GetAddressOf()), nullptr);
		if (pErrors && pErrors->GetStringLength() > 0) {
			OutputDebugStringA(("compilation error:" + filename + "\n").c_str());
			OutputDebugStringA((char*)pErrors->GetBufferPointer());
			return false;
		}

		// Get shader reflection data.
		ComPtr<IDxcBlob> reflectionBlob{};
		DX::ThrowIfFailed(pCompileResult->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflectionBlob), nullptr));
		const DxcBuffer reflectionBuffer{ .Ptr = reflectionBlob->GetBufferPointer(), .Size = reflectionBlob->GetBufferSize(), .Encoding = 0, };

		//create the reflection
		ComPtr<ID3D12ShaderReflection> shaderReflection{};
		pUtils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&shaderReflection));

		//get a description of the shader
		D3D12_SHADER_DESC shaderDesc{};
		shaderReflection->GetDesc(&shaderDesc);

		shaderInstance.shaderSource = params;
		shaderInstance.CreateVSSemantics(shaderReflection, shaderDesc);
		shaderInstance.CreateResourcesBinding(shaderReflection, shaderDesc);
		shaderInstance.CreateConstantsBuffersVariables(shaderReflection, shaderDesc);
		shaderInstance.CreateByteCode(pCompileResult);
		return true;
	}

	void BuildShaderCompiler() {
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.GetAddressOf()));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));
	}

	void DestroyShaderCompiler()
	{
		pCompiler = nullptr;
		pUtils = nullptr;
	}
}