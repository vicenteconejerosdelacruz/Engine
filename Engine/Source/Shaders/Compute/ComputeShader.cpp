#include "pch.h"
#include "ComputeShader.h"
#include <Renderer.h>
#include "ComputeInterface.h"
#include <DeviceUtils/RootSignature/RootSignature.h>
#include <DeviceUtils/PipelineState/PipelineState.h>
#include <Scene.h>

extern std::unique_ptr<JRenderer> renderer;

namespace ComputeShader
{
	ComputeShader::~ComputeShader() {
		DeleteShaderInstance(shader());
	}

	void ComputeShader::Init(std::string shaderName, std::vector<MaterialSamplerDesc> samplers, std::wstring target)
	{
		using namespace DeviceUtils;

		//Get an instance of the BoundingBox Compute shader
		shader = GetShaderUUIDByName(shaderName);
		Source compCS = { .shaderType = COMPUTE_SHADER, .shaderTarget = target, .shaderTemplate = shader };
		auto& shaderIns = CreateShaderInstance(shader(), [compCS]
			{
				return std::make_unique<ShaderInstance>(compCS.shaderTemplate(), compCS.shaderTemplate(), compCS);
			}
		);

		//Build the shader's root signature
		auto& vsCBparams = shaderIns->constantsBuffersParameters;
		auto& psCBparams = shaderIns->constantsBuffersParameters;
		auto& uavParams = shaderIns->uavParameters;
		auto& psSRVCSparams = shaderIns->srvCSParameters;
		auto& psSRVTexparams = shaderIns->srvTexParameters;
		auto& psSamplersParams = shaderIns->samplersParameters;

		rootSignature = CreateRootSignature(std::string("rootSignature:" + shaderName), vsCBparams, psCBparams, uavParams, psSRVCSparams, psSRVTexparams, psSamplersParams, samplers);
		pipelineState = CreateComputePipelineState(std::string("pipelineState:" + shaderName), shaderIns->byteCode, rootSignature);
	}

	void ComputeShader::SetComputeState(SceneUnitId unit)
	{
		using namespace Scene;

		CComPtr<ID3D12GraphicsCommandList2>& commandList = GetSceneUnit(unit)->GetComputeCommandList();
		commandList->SetComputeRootSignature(rootSignature);
		commandList->SetPipelineState(pipelineState);
	}
}