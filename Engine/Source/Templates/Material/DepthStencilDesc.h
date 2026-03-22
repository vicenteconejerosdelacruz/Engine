#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <nlohmann/json.hpp>

struct DepthStencilOpDesc : D3D12_DEPTH_STENCILOP_DESC
{
	DepthStencilOpDesc() : D3D12_DEPTH_STENCILOP_DESC({
		D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS })
	{
	}

	DepthStencilOpDesc(const DepthStencilOpDesc& other)
	{
		StencilFailOp = other.StencilFailOp;
		StencilDepthFailOp = other.StencilDepthFailOp;
		StencilPassOp = other.StencilPassOp;
		StencilFunc = other.StencilFunc;
	}

	DepthStencilOpDesc(nlohmann::json& j)
	{
		nostd::ReplaceFromJsonUsingMap(StencilFailOp, StringToD3D12_STENCIL_OP, j, "StencilFailOp");
		nostd::ReplaceFromJsonUsingMap(StencilDepthFailOp, StringToD3D12_STENCIL_OP, j, "StencilDepthFailOp");
		nostd::ReplaceFromJsonUsingMap(StencilPassOp, StringToD3D12_STENCIL_OP, j, "StencilPassOp");
		nostd::ReplaceFromJsonUsingMap(StencilFunc, StringToD3D12_COMPARISON_FUNC, j, "StencilFunc");
	}

	DepthStencilOpDesc(const D3D12_DEPTH_STENCILOP_DESC& other)
	{
		StencilFailOp = other.StencilFailOp;
		StencilDepthFailOp = other.StencilDepthFailOp;
		StencilPassOp = other.StencilPassOp;
		StencilFunc = other.StencilFunc;
	}

	bool operator<(const DepthStencilOpDesc& other) const
	{
		return std::tie(
			StencilFailOp, StencilDepthFailOp,
			StencilPassOp, StencilFunc) < std::tie(
				other.StencilFailOp, other.StencilDepthFailOp,
				other.StencilPassOp, other.StencilFunc);
	}

	nlohmann::json json()
	{
		nlohmann::json j =
		{
			{ "StencilFailOp", D3D12_STENCIL_OPToString.at(StencilFailOp) },
			{ "StencilDepthFailOp", D3D12_STENCIL_OPToString.at(StencilDepthFailOp) },
			{ "StencilPassOp", D3D12_STENCIL_OPToString.at(StencilPassOp) },
			{ "StencilFunc", D3D12_COMPARISON_FUNCToString.at(StencilFunc) },
		};

		return j;
	}
};

struct DepthStencilDesc : D3D12_DEPTH_STENCIL_DESC
{
	DepthStencilDesc() : D3D12_DEPTH_STENCIL_DESC(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)) {}

	DepthStencilDesc(const DepthStencilDesc& other)
	{
		DepthEnable = other.DepthEnable;
		DepthWriteMask = other.DepthWriteMask;
		DepthFunc = other.DepthFunc;
		StencilEnable = other.StencilEnable;
		StencilReadMask = other.StencilReadMask;
		StencilWriteMask = other.StencilWriteMask;

		FrontFace.StencilFailOp = other.FrontFace.StencilFailOp;
		FrontFace.StencilDepthFailOp = other.FrontFace.StencilDepthFailOp;
		FrontFace.StencilPassOp = other.FrontFace.StencilPassOp;
		FrontFace.StencilFunc = other.FrontFace.StencilFunc;

		BackFace.StencilFailOp = other.BackFace.StencilFailOp;
		BackFace.StencilDepthFailOp = other.BackFace.StencilDepthFailOp;
		BackFace.StencilPassOp = other.BackFace.StencilPassOp;
		BackFace.StencilFunc = other.BackFace.StencilFunc;
	}

	DepthStencilDesc(nlohmann::json& j) : D3D12_DEPTH_STENCIL_DESC(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT))
	{
		ReplaceFromJson(DepthEnable, j, "DepthEnable");
		nostd::ReplaceFromJsonUsingMap(DepthWriteMask, StringToD3D12_DEPTH_WRITE_MASK, j, "DepthWriteMask");
		nostd::ReplaceFromJsonUsingMap(DepthFunc, StringToD3D12_COMPARISON_FUNC, j, "DepthFunc");
		ReplaceFromJson(StencilEnable, j, "StencilEnable");
		ReplaceFromJson(StencilReadMask, j, "StencilReadMask");
		ReplaceFromJson(StencilWriteMask, j, "StencilWriteMask");
		FrontFace = DepthStencilOpDesc(j["FrontFace"]);
		BackFace = DepthStencilOpDesc(j["BackFace"]);
	}

	DepthStencilDesc(const D3D12_DEPTH_STENCIL_DESC& other)
	{
		DepthEnable = other.DepthEnable;
		DepthWriteMask = other.DepthWriteMask;
		DepthFunc = other.DepthFunc;
		StencilEnable = other.StencilEnable;
		StencilReadMask = other.StencilReadMask;
		StencilWriteMask = other.StencilWriteMask;

		FrontFace.StencilFailOp = other.FrontFace.StencilFailOp;
		FrontFace.StencilDepthFailOp = other.FrontFace.StencilDepthFailOp;
		FrontFace.StencilPassOp = other.FrontFace.StencilPassOp;
		FrontFace.StencilFunc = other.FrontFace.StencilFunc;

		BackFace.StencilFailOp = other.BackFace.StencilFailOp;
		BackFace.StencilDepthFailOp = other.BackFace.StencilDepthFailOp;
		BackFace.StencilPassOp = other.BackFace.StencilPassOp;
		BackFace.StencilFunc = other.BackFace.StencilFunc;
	}

	bool operator<(const DepthStencilDesc& other) const {

		return std::tie(DepthEnable, DepthWriteMask, DepthFunc, StencilEnable, StencilReadMask,
			StencilWriteMask, FrontFace.StencilFailOp, FrontFace.StencilDepthFailOp, FrontFace.StencilPassOp,
			FrontFace.StencilFunc, BackFace.StencilFailOp, BackFace.StencilDepthFailOp, BackFace.StencilPassOp, BackFace.StencilFunc) <
			std::tie(other.DepthEnable, other.DepthWriteMask, other.DepthFunc, other.StencilEnable, other.StencilReadMask,
				other.StencilWriteMask, other.FrontFace.StencilFailOp, other.FrontFace.StencilDepthFailOp, other.FrontFace.StencilPassOp,
				other.FrontFace.StencilFunc, other.BackFace.StencilFailOp, other.BackFace.StencilDepthFailOp, other.BackFace.StencilPassOp,
				other.BackFace.StencilFunc);
	}

	nlohmann::json json()
	{
		return
		{
			{ "DepthEnable", static_cast<bool>(!!DepthEnable) },
			{ "DepthWriteMask", D3D12_DEPTH_WRITE_MASKToString.at(DepthWriteMask) },
			{ "DepthFunc", D3D12_COMPARISON_FUNCToString.at(DepthFunc) },
			{ "StencilEnable", static_cast<bool>(!!StencilEnable) },
			{ "StencilReadMask", StencilReadMask },
			{ "StencilWriteMask", StencilWriteMask },
			{ "FrontFace", DepthStencilOpDesc(FrontFace).json() },
			{ "BackFace", DepthStencilOpDesc(BackFace).json() },
		};
	}
};

inline DepthStencilDesc ToDepthStencilDesc(nlohmann::json j)
{
	return DepthStencilDesc(j);
}

inline nlohmann::json FromDepthStencilDesc(DepthStencilDesc r)
{
	return r.json();
}