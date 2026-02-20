#include "pch.h"
#include "PipelineState.h"
#include <Renderer.h>
#include <VertexFormats.h>
#include <DirectXHelper.h>
#include <NoStd.h>
#include <ios>
#include <DeviceUtils/RootSignature/RootSignature.h>

extern std::unique_ptr<Renderer> renderer;

namespace DeviceUtils
{
	CComPtr<ID3D12PipelineState> CreateGraphicsPipelineState(
		std::string name,
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout,
		ShaderByteCode& vsCode,
		ShaderByteCode& psCode,
		CComPtr<ID3D12RootSignature>& rootSignature,
		D3D12_BLEND_DESC& BlendState,
		D3D12_RASTERIZER_DESC& RasterizerState,
		D3D12_DEPTH_STENCIL_DESC& DepthStencilState,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE& PrimitiveTopologyType,
		std::vector<DXGI_FORMAT>& renderTargetsFormats,
		DXGI_FORMAT& depthStencilFormat
	)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC state{};

		//input layout
		state.InputLayout = { inputLayout.data(), static_cast<UINT>(inputLayout.size()) };

		//root signature
		state.pRootSignature = rootSignature;

		//shader based state
		state.VS = CD3DX12_SHADER_BYTECODE(vsCode.data(), vsCode.size());
		state.PS = CD3DX12_SHADER_BYTECODE(psCode.data(), psCode.size());

		//material based state
		state.RasterizerState = RasterizerState;
		state.BlendState = BlendState;
		state.SampleDesc.Count = 1;

		//depth stencil
		state.DepthStencilState = DepthStencilState;

		//render target based
		//state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		//if (depthStencilFormat == DXGI_FORMAT_UNKNOWN) { state.DepthStencilState.DepthEnable = false; }
		state.SampleMask = UINT_MAX;
		state.PrimitiveTopologyType = PrimitiveTopologyType;
		state.NumRenderTargets = static_cast<UINT>(std::max(1ULL, renderTargetsFormats.size()));
		state.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		for (unsigned int i = 0; i < renderTargetsFormats.size(); i++)
		{
			state.RTVFormats[i] = renderTargetsFormats[i];
		}
		state.DSVFormat = depthStencilFormat;

		CComPtr<ID3D12PipelineState> pipelineState;
		DX::ThrowIfFailed(renderer->d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&pipelineState)));

		CCNAME_D3D12_OBJECT(pipelineState);
		LogCComPtrAddress(name, pipelineState);

		return pipelineState;
	}

	CComPtr<ID3D12PipelineState> CreateComputePipelineState(std::string name, ShaderByteCode& csCode, CComPtr<ID3D12RootSignature>& rootSignature)
	{
		struct PipelineStateStream
		{
			CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE RootSignature;
			CD3DX12_PIPELINE_STATE_STREAM_CS CS;
		} PSS;

		PSS.RootSignature = rootSignature;
		PSS.CS = CD3DX12_SHADER_BYTECODE(csCode.data(), csCode.size());

		D3D12_PIPELINE_STATE_STREAM_DESC pssDescription = { sizeof(PSS), &PSS };
		CComPtr<ID3D12PipelineState> pipelineState;
		DX::ThrowIfFailed(renderer->d3dDevice->CreatePipelineState(&pssDescription, IID_PPV_ARGS(&pipelineState)));

		CCNAME_D3D12_OBJECT(pipelineState);
		LogCComPtrAddress(name, pipelineState);

		return pipelineState;
	}
};
