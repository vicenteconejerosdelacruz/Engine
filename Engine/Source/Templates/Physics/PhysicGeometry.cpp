#include "pch.h"
#include "PhysicGeometry.h"

#if defined(_EDITOR)
namespace Editor
{
	extern void MarkTemplatesPanelAssetsAsDirty();
};
#endif

namespace Templates
{
#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#endif

	PhysicGeometryJson::PhysicGeometryJson(nlohmann::json& json) : JTemplate(json)
	{
#include <Attributes/JInit.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void PhysicGeometryJson::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>
	}
#endif

	TEMPDEF_FULL(PhysicGeometry);
	TEMPDEF_REFTRACKER(PhysicGeometry);

	PhysicGeometryInstance::PhysicGeometryInstance(JUUID uuid)
	{

	}

	//PhysicGeometryInstance::PhysicGeometryInstance(SceneUnitId id, JUUID uuid, JUUID objectUUID)
	//{
	//}

	PhysicGeometryInstance::~PhysicGeometryInstance()
	{
	}

};