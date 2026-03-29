#pragma once
#include <UUID.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>

namespace Scene
{
	DEF_SCENEOBJECT_ID_DEP(Camera);
};

namespace Templates
{
	DEF_TEMPLATE_ID_DEP(RenderPassJson, GetRenderPassTemplate);
	DEF_TEMPLATE_ID_DEP(RenderPassInstance, GetRenderPassInstance);
	DEF_TEMPLATE_ID_DEP(MaterialInstance, GetMaterialInstance);

	struct OverridePass
	{
		OverridePass() { assert(!!!"do not use"); }
		explicit OverridePass(CameraID cam, unsigned int rpI, RenderPassJsonID rpT, RenderPassInstanceID rp) { camera = cam; renderPassIndex = rpI; renderPassTemplate = rpT; renderPassInstance = rp; };
		virtual ~OverridePass();
		virtual void Initialize() {};
		void CreateFsQuadResources(SceneUnitId id, std::string materialName, JUUID renderPassTemplate, std::function<void(std::string, ShaderConstantsBufferVariable&)> constantsBufferPusher = [](auto a, auto b) {});
		RenderPassInstanceID GetPrevRenderPass();
		RenderPassJsonID GetPrevRenderPassTemplate();
		JUUID GetPrevPassRenderToTexture(unsigned int index = 0U);
		virtual void CreatePrevPassDependentResources() = 0;
		virtual void Pass(SceneUnitId unit) = 0;

		//data from camera and renderpass
		CameraID camera;
		unsigned int renderPassIndex;
		RenderPassJsonID renderPassTemplate;
		RenderPassInstanceID renderPassInstance;

		//fsQuad
		JUUID fsQuad;
		MaterialInstanceID fsQuadMaterial;
		ConstantsBufferID fsQuadConstantsBuffer;
		CComPtr<ID3D12RootSignature> rootSignature;
		CComPtr<ID3D12PipelineState> pipelineState;
	};
}

#include "MinMaxChainPass.h"
#include "MinMaxChainResultPass.h"
#include "ResolvePass.h"
#include "ResolveUIPass.h"
#include "ToneMappingPass.h"