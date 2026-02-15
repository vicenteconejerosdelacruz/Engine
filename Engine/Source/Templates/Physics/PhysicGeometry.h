#pragma once

#include <Templates.h>
#include <JTemplate.h>
#include <UUID.h>

namespace Templates
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#endif

	namespace PhysicGeometry
	{
		inline static const std::string templateName = "physic_geometries.json";
		inline static const TemplateType templateType = T_PhysicGeometries;
	};

	struct PhysicGeometryJson : public JTemplate
	{
		TEMPLATE_DECL(PhysicGeometry);

#include <Attributes/JFlags.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif
	};

	struct PhysicGeometryInstance;

	TEMPDECL_FULL(PhysicGeometry);
	TEMPDECL_REFTRACKER(PhysicGeometry);
	DEF_TEMPLATE_ID(PhysicGeometryJson, GetPhysicGeometryTemplate);
	DEF_TEMPLATE_ID(PhysicGeometryInstance, GetPhysicGeometryInstance);

	struct PhysicGeometryInstance
	{
		PhysicGeometryJsonID model3D;

		PhysicGeometryInstance(JUUID uuid);// { assert(!!!"do not use"); }
		//explicit PhysicGeometryInstance(SceneUnitId id, JUUID uuid, JUUID objectUUID);
		~PhysicGeometryInstance();
	};
}

using namespace Templates;
DEF_TEMPLATE_ID_HASH(PhysicGeometryJson);
DEF_TEMPLATE_ID_HASH(PhysicGeometryInstance);