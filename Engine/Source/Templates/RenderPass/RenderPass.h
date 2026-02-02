#pragma once
#include <string>
#include <tuple>
#include <map>
#include <vector>
#include <dxgiformat.h>
//#include <RenderPass/SwapChainPass.h>
//#include <RenderPass/RenderToTexturePass.h>
#include <RenderPass/Override/OverridePass.h>
#include "PassMaterialOverride.h"
#include <Templates.h>
#include <JTemplate.h>
//#include <JTypes.h>
//#include <TemplateDecl.h>

enum RenderPassType
{
	RenderPassType_SwapChainPass,
	RenderPassType_RenderToTexturePass
};

inline static std::unordered_map<RenderPassType, std::string> RenderPassTypeToString =
{
	{ RenderPassType_SwapChainPass, "SwapChainPass"},
	{ RenderPassType_RenderToTexturePass, "RenderToTexturePass"},
};

inline static std::unordered_map<std::string, RenderPassType> StringToRenderPassType =
{
	{ "SwapChainPass", RenderPassType_SwapChainPass },
	{ "RenderToTexturePass", RenderPassType_RenderToTexturePass },
};

enum RenderPassMaterialOverride
{
	RenderPassMaterialOverride_None,
	RenderPassMaterialOverride_ShadowMap,
	RenderPassMaterialOverride_Picking
};

inline static std::unordered_map<RenderPassMaterialOverride, std::string> RenderPassMaterialOverrideToString =
{
	{ RenderPassMaterialOverride_None, "None"},
	{ RenderPassMaterialOverride_ShadowMap, "ShadowMap"},
	{ RenderPassMaterialOverride_Picking, "Picking" },
};

inline static std::unordered_map<std::string, RenderPassMaterialOverride> StringToRenderPassMaterialOverride =
{
	{ "None" , RenderPassMaterialOverride_None },
	{ "ShadowMap" , RenderPassMaterialOverride_ShadowMap },
	{ "Picking", RenderPassMaterialOverride_Picking },
};

enum RenderPassRenderCallbackOverride
{
	RenderPassRenderCallbackOverride_None,
	RenderPassRenderCallbackOverride_ToneMapping,
	RenderPassRenderCallbackOverride_Resolve,
	RenderPassRenderCallbackOverride_MinMaxChain,
	RenderPassRenderCallbackOverride_MinMaxChainResult,
};

inline static std::unordered_map<RenderPassRenderCallbackOverride, std::string> RenderPassRenderCallbackOverrideToString =
{
	{ RenderPassRenderCallbackOverride_None, "None" },
	{ RenderPassRenderCallbackOverride_ToneMapping,	"ToneMapping" },
	{ RenderPassRenderCallbackOverride_Resolve, "Resolve" },
	{ RenderPassRenderCallbackOverride_MinMaxChain, "MinMaxChain" },
	{ RenderPassRenderCallbackOverride_MinMaxChainResult, "MinMaxChainResult" },
};

inline static std::unordered_map<std::string, RenderPassRenderCallbackOverride> StringToRenderPassRenderCallbackOverride =
{
	{ "None", RenderPassRenderCallbackOverride_None },
	{ "ToneMapping", RenderPassRenderCallbackOverride_ToneMapping },
	{ "Resolve", RenderPassRenderCallbackOverride_Resolve },
	{ "MinMaxChain", RenderPassRenderCallbackOverride_MinMaxChain },
	{ "MinMaxChainResult", RenderPassRenderCallbackOverride_MinMaxChainResult },
};

namespace Templates
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <RenderPassAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <RenderPassAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include <RenderPassAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <RenderPassAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <RenderPassAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <RenderPassAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <RenderPassAtt.h>
#include <JEnd.h>

#endif

	void RenderPassJsonStep();
	void UpdateRenderPassInstances(std::unordered_map<RenderPassJsonUUID, std::set<RenderPassInstanceUUID>> changes);

	namespace RenderPass
	{
		inline static const std::string templateName = "renderpasses.json";
		inline static const TemplateType templateType = T_RenderPasses;
		inline static const std::string shadowMapMaterialName = "ShadowMap";
		inline static const std::string pickingMaterialName = "Picking";
		void CreateRenderPassMainHeap();
		void DestroyRenderPassMainHeap();
		void ResizeRelease();
		void Resize(unsigned int width, unsigned int height);
	};

	struct RenderPassJson : public JTemplate
	{
		TEMPLATE_DECL(RenderPass);

#include <Attributes/JFlags.h>
#include <RenderPassAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <RenderPassAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif
		void SetPipelineStateCallback(size_t hash, std::function<void()> callback);

		std::unordered_map<size_t, std::function<void()>> pipelineChangeCallbacks;
	};

	struct RenderPassInstance;

	TEMPDECL_FULL(RenderPass);
	TEMPDECL_REFTRACKER(RenderPass);

	JUUID CreateRenderPassInstance(SceneUnitId id, JUUID cameraUUID, JUUID renderPassTemplateUUID, unsigned int renderPassIndex, unsigned int width = 0U, unsigned int height = 0U);
	void DestroyRenderPassInstance(JUUID renderPassInstanceUUID);

	struct RenderPassInstance
	{
		RenderPassInstance(JUUID uuid) { assert(!!!"do not use"); }
		explicit RenderPassInstance(SceneUnitId id, JUUID cameraUUID, JUUID renderPassTemplateUUID, JUUID renderPassInstanceUUID, unsigned int renderPassIndex, unsigned int width, unsigned int height);
		~RenderPassInstance();
		void Pass(SceneUnitId unit, std::function<void(SceneUnitId)> renderCallback = [](SceneUnitId) {}, bool clearRTV = true, XMVECTORF32 clearColor = DirectX::Colors::Black);
		JUUID GetRenderPassMaterialInstance(
			SceneUnitId id,
			MaterialJsonUUID material,
			MeshInstanceUUID mesh,
			bool shadowed,
			std::vector<PassMaterialOverride> passMaterialOverride,
			JUUID bindingUUID = ""
		);
		void InitRenderPass();
		void ResizeRelease();
		void Resize(unsigned int width, unsigned int height);
		void CreateRenderTargets();
		std::vector<DXGI_FORMAT> GetRenderTargetsFormats();
		DXGI_FORMAT GetDepthStencilFormat();


		unsigned int renderPassIndex;
		unsigned int width;
		unsigned int height;
		CameraSUUUID camera;
		RenderPassJsonUUID renderPassTemplate;
		RenderPassInstanceUUID renderPassInstance;
		RenderPassType type = RenderPassType_SwapChainPass;
		RenderPassMaterialOverride materialOverride;
		RenderPassRenderCallbackOverride renderCallbackOverride;
		std::unique_ptr<OverridePass> overridePass;
		SwapChainPassUUID swapChainPass;
		RenderToTexturePassUUID renderToTexturePass;
	};
};

