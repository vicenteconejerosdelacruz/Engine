#include "pch.h"
#include "Mold.h"

namespace Templates
{
#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <MoldAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <MoldAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <MoldAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <MoldAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <MoldAtt.h>
#include <JEnd.h>

#endif

	MoldJson::MoldJson(nlohmann::json& json) : JTemplate(json)
	{
#include <Attributes/JInit.h>
#include <MoldAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <MoldAtt.h>
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include <MoldAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void MoldJson::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <MoldAtt.h>
#include <JEnd.h>
	}
#endif

	TEMPDEF_FULL(Mold);
};