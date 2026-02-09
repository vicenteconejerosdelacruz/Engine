#pragma once
#include <string>
#include <vector>
#include <Material/SamplerDesc.h>
#include <Shader/Shader.h>
#include <Shader/ShaderInstance.h>
#include "../Compiler/ShaderCompiler.h"

using namespace Templates;

namespace ComputeShader
{
	struct ComputeShader
	{
		ShaderJsonID shader;
		CComPtr<ID3D12RootSignature> rootSignature;
		CComPtr<ID3D12PipelineState> pipelineState;

		~ComputeShader() {
			DeleteShaderInstance(shader());
		}
		void Init(std::string shaderName, std::vector<MaterialSamplerDesc> samplers = {}, std::wstring target = ShaderCompiler::shaderTarget.at(COMPUTE_SHADER));
		void SetComputeState(SceneUnitId unit);
	};
}

