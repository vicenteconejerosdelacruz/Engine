#pragma once

struct OverridePass
{
	OverridePass() { assert(!!!"do not use"); }
	explicit OverridePass(SceneUnitId id, JUUID cam, unsigned int rpI, JUUID rpT, JUUID rp) { camera = MAKESUUUID(id, cam); renderPassIndex = rpI; renderPassTemplate = rpT; renderPassInstance = rp; };
	virtual ~OverridePass();
	virtual void Initialize() {};
	void CreateFsQuadResources(SceneUnitId id, std::string materialName, JUUID renderPassTemplate, std::function<void(std::string, ShaderConstantsBufferVariable&)> constantsBufferPusher = [](auto a, auto b) {});
	RenderPassInstanceUUID GetPrevRenderPass();
	RenderPassJsonUUID GetPrevRenderPassTemplate();
	JUUID GetPrevPassRenderToTexture(unsigned int index = 0U);
	virtual void CreatePrevPassDependentResources() = 0;
	virtual void Pass(SceneUnitId unit) = 0;

	//data from camera and renderpass
	CameraSUUUID camera;
	unsigned int renderPassIndex;
	RenderPassJsonUUID renderPassTemplate;
	RenderPassInstanceUUID renderPassInstance;

	//fsQuad
	JUUID fsQuad;
	MaterialInstanceUUID fsQuadMaterial;
	ConstantsBufferUUID fsQuadConstantsBuffer;
	CComPtr<ID3D12RootSignature> rootSignature;
	CComPtr<ID3D12PipelineState> pipelineState;
};

#include "MinMaxChainPass.h"
#include "MinMaxChainResultPass.h"
#include "ResolvePass.h"
#include "ToneMappingPass.h"