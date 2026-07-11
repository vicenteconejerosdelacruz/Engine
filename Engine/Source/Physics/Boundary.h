#pragma once

#include <Scene.h>
#include <SceneObject.h>
#include <SceneUnitId.h>
#include <PxPhysicsAPI.h>

enum SceneObjectType;

namespace Physics
{
	DEF_TEMPLATE_ID_DEP(PhysicObject, GetPhysicObject);
	extern std::vector<std::string> GetCollisionMasks();
}

namespace Scene
{
	using namespace physx;
	using namespace Physics;

#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include "BoundaryAtt.h"
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include "BoundaryAtt.h"
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include "BoundaryAtt.h"
#include <JEnd.h>

#include <Creator/JRequired.h>
#include "BoundaryAtt.h"
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include "BoundaryAtt.h"
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include "BoundaryAtt.h"
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include "BoundaryAtt.h"
#include <JEnd.h>

#endif

	struct Boundary : SceneObject
	{
		inline static const SceneObjectType sceneObjectType = SO_Boundaries;

#include <Attributes/JFlags.h>
#include "BoundaryAtt.h"
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include "BoundaryAtt.h"
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include "BoundaryAtt.h"
#include <JEnd.h>

		Boundary(SceneUnitId id, nlohmann::json& json);
		~Boundary() { Destroy(); }
		void Initialize() override;
		virtual void BindToScene();
		virtual void UnbindFromScene();

		virtual void Destroy();

		XMVECTOR positionV();
		void updateRotationQ();
		XMVECTOR rotationQ();
		void rotationQ(XMVECTOR Q);

		void CreatePhysicObject();
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
		virtual bool CanInteractWithGizmo(ImGuizmo::OPERATION operation) { return true; }
		BoundingBox GetBoundingBox();
#endif

		DeleteHook markedForDelete;
		PhysicObjectID physicObject;
		//Transformation
		XMVECTOR rotationQuaternion;
	};

	SODECL_FULL(Boundary);

#include <TrackUUID/JDecl.h>
#include "BoundaryAtt.h"
#include <JEnd.h>

	void BoundariesStep(SceneUnitId id);
	void DestroyBoundaries();
	void DestroyBoundaries(SceneUnitId id);
	void DeleteBoundary(SceneUnitId id, JUUID uuid);
#if defined(_EDITOR)
	void WriteBoundariesJson(SceneUnitId id, nlohmann::json& json);
#endif
}

using namespace Scene;
DEF_SCENEOBJECT_ID_HASH(Boundary);