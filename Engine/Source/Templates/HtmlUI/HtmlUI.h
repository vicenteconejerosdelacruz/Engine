#pragma once

#include <Templates.h>
#include <JTemplate.h>
#include <Ultralight/Ultralight.h>

using namespace ultralight;
namespace Templates
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <HtmlUIAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <HtmlUIAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include <HtmlUIAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <HtmlUIAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <HtmlUIAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <HtmlUIAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <HtmlUIAtt.h>
#include <JEnd.h>

#endif

	namespace HtmlUI
	{
		inline static const std::string templateName = "htmluis.json";
		inline static const TemplateType templateType = T_HtmlUIs;
	};

	struct HtmlUIJson : public JTemplate
	{
		TEMPLATE_DECL(HtmlUI);

#include <Attributes/JFlags.h>
#include <HtmlUIAtt.h>
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include <HtmlUIAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <HtmlUIAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif
	};

	struct HtmlUIInstance;

	TEMPDECL_FULL(HtmlUI);
	TEMPDECL_REFTRACKER(HtmlUI);
	DEF_TEMPLATE_ID(HtmlUIJson, GetHtmlUITemplate);
	DEF_TEMPLATE_ID(HtmlUIInstance, GetHtmlUIInstance);

	struct HtmlUIInstance
	{
		HtmlUIInstance(JUUID uuid) { assert(!!!"do not use"); }
		explicit HtmlUIInstance(SceneUnitId id, JUUID instance_uuid, JUUID template_uuid);
		~HtmlUIInstance() { Destroy(); }
		void Destroy();
		void UpdateTexture(SceneUnitId id);
		void Resolve(SceneUnitId id);
		void EvaluateScript(std::string js);

		JUUID instanceUUID;
		RefPtr<View> view;
		RenderToTextureID rt_texture;
		RenderPassInstanceID resolvePass;
		unsigned int rowPitch;
		CComPtr<ID3D12Resource> uploadBuffer;
	};
};

using namespace Templates;
DEF_TEMPLATE_ID_HASH(HtmlUIJson);
DEF_TEMPLATE_ID_HASH(HtmlUIInstance);
