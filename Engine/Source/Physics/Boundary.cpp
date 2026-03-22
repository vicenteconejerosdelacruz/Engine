#include "pch.h"
#include "Boundary.h"
#include <Scene.h>
#include <Physics.h>
#include <NoMath.h>

namespace Editor
{
	extern void UnRegisterStaticBody(PhysicObjectID phO);
};

namespace Scene
{
	SODEF_FULL(Boundary);

#include <TrackUUID/JDef.h>
#include <BoundaryAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <BoundaryAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <BoundaryAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <BoundaryAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <BoundaryAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <BoundaryAtt.h>
#include <JEnd.h>

#endif

#if defined(_EDITOR)
	void WriteBoundariesJson(SceneUnitId id, nlohmann::json& json)
	{
#include <Editor/JSaveFile.h>
#include <BoundaryAtt.h>
#include <JEnd.h>
	}
#endif

	Boundary::Boundary(SceneUnitId id, nlohmann::json& json) : SceneObject(id, json)
	{
#include <Attributes/JInit.h>
#include <BoundaryAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <BoundaryAtt.h>
#include <JEnd.h>
		(*this)["behavior"] = PhysicsBehaviorToString.at(PB_Static);
		RENAME_ON_DELETION(Boundary);
	}

	void Boundary::create_rotation(XMFLOAT3 v)
	{
		if (!contains("rotation"))
		{
			rotation(v);
		}
	}

	void Boundary::rotation(XMFLOAT3 v)
	{
		(*this)["rotation"] = FromXMFLOAT3(v);
		updateRotationQ();
	}

#if defined(_EDITOR)
	void Boundary::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <BoundaryAtt.h>
#include <JEnd.h>
	}
#endif

	void Boundary::Initialize()
	{
#include <TrackUUID/JInsert.h>
#include <BoundaryAtt.h>
#include <JEnd.h>
		updateRotationQ();
	}

	void Boundary::BindToScene()
	{
#include <TrackUUID/JInsert.h>
#include <BoundaryAtt.h>
#include <JEnd.h>

		CreatePhysicObject();
	}

	void Boundary::UnbindFromScene()
	{
#include <TrackUUID/JErase.h>
#include <BoundaryAtt.h>
#include <JEnd.h>
	}

	void Boundary::Destroy()
	{
		if (!physicObject.empty())
		{
			DestroyPhysicObject(physicObject());
		}

#include <Attributes/JDestroy.h>
#include <BoundaryAtt.h>
#include <JEnd.h>

		SceneObject::Destroy();
	}

	void Boundary::CreatePhysicObject()
	{
		nlohmann::json data =
		{
			{ "behavior", "Static" },
			{ "geometry", geometry() },
			{ "color", FromXMFLOAT4(color()) },
			{ "overrideColor", overrideColor() }
		};

		std::string pOname = name() + "-physicObject";
		physicObject = Physics::CreatePhysicObject(pOname, SUuuid(), data);
		physicObject->CreatePhysicsBehavior();
#if defined(_EDITOR)
		physicObject->CreatePhysicsAvatar();
#endif
	}

#if defined(_EDITOR)
	BoundingBox Boundary::GetBoundingBox()
	{
		return BoundingBox(position(), { 0.1f,0.1f,0.1f });
	}
#endif

	XMVECTOR Boundary::positionV()
	{
		XMFLOAT3 pos = position();
		return XMLoadFloat3(&pos);
	}

	void Boundary::updateRotationQ()
	{
		XMFLOAT3 v = rotation();
		rotationQuaternion = XMQuaternionRotationRollPitchYaw(
			XMConvertToRadians(v.x),
			XMConvertToRadians(v.y),
			XMConvertToRadians(v.z)
		);
	}

	XMVECTOR Boundary::rotationQ()
	{
		return rotationQuaternion;
	}

	void Boundary::rotationQ(XMVECTOR Q)
	{
		rotationQuaternion = Q;
	}

	void BoundariesStep(SceneUnitId unit)
	{
#if defined(_EDITOR)
		using namespace Editor;
#endif
		auto& Boundaries = GetBoundaries(unit);
		std::set<BoundaryID> bs;
		std::transform(Boundaries.begin(), Boundaries.end(), std::inserter(bs, bs.begin()), [&](auto o) { return MAKESUUUID(unit, o); });

		auto checkForDelete = [](BoundaryID b)
			{
				if (!b->markedForDelete) return;

				PhysicObjectID phO = b->physicObject();
				phO->DestroyPhysicsBehavior();
#if defined(_EDITOR)
				phO->DestroyPhysicsAvatar();
#endif
				DestroyPhysicObject(phO());
				b->physicObject.clear();
				b->clear();
				EraseBoundaryFromBoundaries(FROMSUUUID(b()));
				DeleteBoundarySceneObject(b);
			};
		auto checkForPosRot = [](BoundaryID b)
			{
				if (!b->dirty({ Boundary::Update_position,Boundary::Update_rotation })) return;

				b->updateRotationQ();
				b->physicObject->UpdateGlobalPoseFromRenderable();
#if defined(_EDITOR)
				b->physicObject->UpdatePhysicsAvatarTransformation();
#endif
				b->clean({ Boundary::Update_position,Boundary::Update_rotation });
			};
		auto checkForScale = [](BoundaryID b)
			{
				if (!b->dirty(Boundary::Update_scale)) return;
				b->scale(XMClamp(b->scale(), 0.01f, 1000.0f));
				b->physicObject->DestroyPhysicsBehavior();
				b->physicObject->CreatePhysicsBehavior();
#if defined(_EDITOR)
				b->physicObject->UpdatePhysicsAvatarTransformation();
#endif
				b->clean(Boundary::Update_scale);
			};
#if defined(_EDITOR)
		auto checkForColor = [](BoundaryID b)
			{
				if (!b->dirty({ Boundary::Update_overrideColor, Boundary::Update_color })) return;

				b->physicObject->color(b->color());
				b->physicObject->overrideColor(b->overrideColor());

				b->clean({ Boundary::Update_overrideColor, Boundary::Update_color });
			};
#endif

		std::for_each(bs.begin(), bs.end(), checkForPosRot);
		std::for_each(bs.begin(), bs.end(), checkForScale);
#if defined(_EDITOR)
		std::for_each(bs.begin(), bs.end(), checkForColor);
#endif
		std::for_each(bs.begin(), bs.end(), checkForDelete);
	}

	void DestroyBoundaries()
	{
		for (auto& [id, container] : BoundarySUsceneObjects)
		{
			for (auto& [uuid, _] : container)
			{
				DeleteBoundarySceneObject(MAKESUUUID(id, uuid));
			}
		}
#include <TrackUUID/JClear.h>
#include <BoundaryAtt.h>
#include <JEnd.h>
	}

	void DestroyBoundaries(SceneUnitId id)
	{
		std::set<JUUID> uuids;
		std::transform(BoundarySUsceneObjects.at(id).begin(), BoundarySUsceneObjects.at(id).end(), std::inserter(uuids, uuids.begin()), [](auto& pair) { return pair.first; });
		for (auto& uuid : uuids)
		{
			DeleteBoundarySceneObject(MAKESUUUID(id, uuid));
		}
#include <TrackUUID/JClearUnit.h>
#include <BoundaryAtt.h>
#include <JEnd.h>
	}

	void DeleteBoundary(SceneUnitId id, JUUID uuid)
	{
		BoundaryID b = MAKESUUUID(id, uuid);
		b->markedForDelete = true;
	}
}