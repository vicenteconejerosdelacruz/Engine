#pragma once

#include <d3dx12.h>
#include <Material/Material.h>

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
	);

	CComPtr<ID3D12PipelineState> CreateComputePipelineState(
		std::string name,
		ShaderByteCode& csCode,
		CComPtr<ID3D12RootSignature>& rootSignature
	);
};
