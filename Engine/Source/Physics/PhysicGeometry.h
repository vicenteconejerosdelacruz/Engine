#pragma once

#include <Templates.h>
#include <JTemplate.h>
#include <UUID.h>
#include <Trigger.h>
#include <PxPhysicsAPI.h>

enum PhysicsBehavior;

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

#if defined(_EDITOR)
	nlohmann::json GetCubeAttributes();
	nlohmann::json GetCapsuleAttributes();
	std::vector<JUUIDName> GetPhysicGeometrysTriggerUUIDsNames();
	std::vector<JUUIDName> GetPhysicGeometrysCharacterUUIDsNames();
#endif

	struct PhysicGeometryInstance;
	std::unique_ptr<PhysicGeometryInstance>& CreatePhysicGeometryInstance(JUUID instanceId, std::function<std::unique_ptr<PhysicGeometryInstance>()> newRefCallback);
	std::unique_ptr<PhysicGeometryInstance>& GetPhysicGeometryInstance(JUUID instanceId);
	void ClearPhysicGeometryInstances();

	TEMPDECL_FULL(PhysicGeometry);
	DEF_TEMPLATE_ID(PhysicGeometryJson, GetPhysicGeometryTemplate);
	DEF_TEMPLATE_ID(PhysicGeometryInstance, GetPhysicGeometryInstance);

	struct PhysicGeometryInstance
	{
		PhysicGeometryJsonID geometryTemplate;
		PhysicGeometryInstanceID instance;
		Model3DJsonID model3D;
		JUUID mesh;
		RenderableID renderable;
		TriggerID trigger;
		BoundaryID boundary;

		PxGeometryHolder geometry;

		PhysicGeometryInstance(JUUID uuid) { assert(!!!"do not use"); }
		PhysicGeometryInstance(PhysicGeometryJsonID geometryTemplate, RenderableID renderable, Model3DJsonID model3D, JUUID instance, PhysicsBehavior behavior);
		PhysicGeometryInstance(PhysicGeometryJsonID geometryTemplate, TriggerID trigger, Model3DJsonID model3D, JUUID instance, PhysicsBehavior behavior);
		PhysicGeometryInstance(PhysicGeometryJsonID geometryTemplate, RenderableID renderable, nlohmann::json& attributes, JUUID mesh, JUUID instance, PhysicsBehavior behavior);
		PhysicGeometryInstance(PhysicGeometryJsonID geometryTemplate, TriggerID trigger, nlohmann::json& attributes, JUUID mesh, JUUID instance, PhysicsBehavior behavior);
		PhysicGeometryInstance(PhysicGeometryJsonID geometryTemplate, BoundaryID boundary, nlohmann::json& attributes, JUUID mesh, JUUID instance, PhysicsBehavior behavior);
		~PhysicGeometryInstance();
	};
}

using namespace Templates;
DEF_TEMPLATE_ID_HASH(PhysicGeometryJson);
DEF_TEMPLATE_ID_HASH(PhysicGeometryInstance);

#if defined(_EDITOR)
static std::map<std::string, std::function<nlohmann::json()>> GetPxGeometryAttributes =
{
	{ "cube", GetCubeAttributes },
	{ "capsule", GetCapsuleAttributes },
};
#endif

static std::map<std::string, std::function<PxQuat(XMFLOAT3)>> ApplyGeometryLocalPoseTransformation =
{
	{ "capsule",[](XMFLOAT3 rot)
	{
		XMVECTOR rroll = XMQuaternionRotationAxis({ 0.0f,0.0f,1.0f,0.0f }, XM_PIDIV2);
		XMVECTOR rotQ = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(rot.x), XMConvertToRadians(rot.y), XMConvertToRadians(rot.z));
		rotQ = XMQuaternionMultiply(rotQ, rroll);
		return ToPxQuat(rotQ);
	}
	},
};