#pragma once

#include <Templates.h>
#include <JTemplate.h>

namespace Templates
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <MoldAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <MoldAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include <MoldAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <MoldAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <MoldAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <MoldAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <MoldAtt.h>
#include <JEnd.h>

#endif

	namespace Mold
	{
		inline static const std::string templateName = "molds.json";
		inline static const TemplateType templateType = T_Molds;
	};

	struct MoldJson : public JTemplate
	{
		TEMPLATE_DECL(Mold);

#include <Attributes/JFlags.h>
#include <MoldAtt.h>
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include <MoldAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <MoldAtt.h>
#include <JEnd.h>

		DEF_STRING2FLAGS_FUNC(MoldJson, JTemplate);

#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif
	};

	TEMPDECL_FULL(Mold);
	DEF_TEMPLATE_ID(MoldJson, GetMoldTemplate);
};

using namespace Templates;
DEF_TEMPLATE_ID_HASH(MoldJson);
