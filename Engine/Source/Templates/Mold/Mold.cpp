#include "pch.h"
#include "Mold.h"
#if defined(_EDITOR)
#include <Builder/ReleaseBuilder.h>
#include <Builder/BuildMaker.h>
#include <Modals/BuildModal.h>
#endif

namespace Templates
{
#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include "MoldAtt.h"
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include "MoldAtt.h"
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include "MoldAtt.h"
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include "MoldAtt.h"
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include "MoldAtt.h"
#include <JEnd.h>

#endif

	MoldJson::MoldJson(nlohmann::json& json) : JTemplate(json)
	{
#include <Attributes/JInit.h>
#include "MoldAtt.h"
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include "MoldAtt.h"
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include "MoldAtt.h"
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void MoldJson::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include "MoldAtt.h"
#include <JEnd.h>
	}
	void MoldJson::GatherFiles(std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		nlohmann::json json = this->json();
#include <Editor/JReleaseBuilder.h>
#include "MoldAtt.h"
#include <JEnd.h>
		GatherFilesFromLevel(json, filesToCopy, templates, logStream);
	}
#endif

	TEMPDEF_FULL(Mold);
};