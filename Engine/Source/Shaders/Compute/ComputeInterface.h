#pragma once
#include "ComputeShader.h"

namespace Scene { struct Renderable; };

namespace ComputeShader
{
	struct ComputeInterface
	{
		ComputeShader shader;

		ComputeInterface(std::string shaderName, std::vector<MaterialSamplerDesc> samplers = {}, std::wstring target = ShaderCompiler::shaderTarget.at(COMPUTE_SHADER))
		{
			shader.Init(shaderName, samplers, target);
		}

		virtual void Compute(SceneUnitId unit) = 0;
		virtual void Solution(SceneUnitId unit) = 0;
	};
};