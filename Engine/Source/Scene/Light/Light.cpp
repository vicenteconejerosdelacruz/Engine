#include "pch.h"
#include "Light.h"
#include <Scene.h>
#include <NoMath.h>

#if defined(_EDITOR)
namespace Editor
{
	extern void SelectLight(LightID light);
	extern RenderableID CreateBillboardFromMaterials(SceneUnitId id, CameraID camera, std::string name, std::string material, std::string pickingMaterial);
	extern void RegisterBillboard(SceneUnitId id, JUUID sceneObject);
	extern void DestroyBillboard(SceneUnitId id, JUUID sceneObject);
}
#endif

namespace Scene
{
	SODEF_FULL(Light);

#include <TrackUUID/JDef.h>
#include "LightAtt.h"
#include <JEnd.h>

#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include "LightAtt.h"
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include "LightAtt.h"
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include "LightAtt.h"
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include "LightAtt.h"
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include "LightAtt.h"
#include <JEnd.h>

#endif

	using namespace DeviceUtils;

	Light::Light(SceneUnitId id, nlohmann::json& json) : SceneObject(id, json)
	{
#include <Attributes/JInit.h>
#include "LightAtt.h"
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include "LightAtt.h"
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include "LightAtt.h"
#include <JEnd.h>
		RENAME_ON_DELETION(Light);
	}

	void Light::create_rotation(XMFLOAT3 v)
	{
		if (!contains("rotation"))
		{
			rotation(v);
		}
	}

	void Light::rotation(XMFLOAT3 v)
	{
		(*this)["rotation"] = FromXMFLOAT3(v);
		updateRotationQ();
	}

#if defined(_EDITOR)
	void Light::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include "LightAtt.h"
#include <JEnd.h>
	}
#endif

	void Light::Initialize()
	{
#if defined(_EDITOR)
		using namespace Editor;
#endif

#include <TrackUUID/JInsert.h>
#include "LightAtt.h"
#include <JEnd.h>

		if (hasShadowMaps())
		{
			CreateShadowMap();
		}
#if defined(_EDITOR)
		if (lightType() != LT_Ambient && !SceneIsIsolated(unit))
		{
			RegisterBillboard(unit, uuid());
		}
#endif
		SetInitialConditions();
		SceneObject::Initialize();
	}

	void Light::SetInitialConditions()
	{
		updateRotationQ();
	}

	void Light::BindToScene()
	{
#include <TrackUUID/JInsert.h>
#include "LightAtt.h"
#include <JEnd.h>

		BindCameras();
		BindRenderablesToShadowMapCamera();
#if defined(_EDITOR)
		SceneObject::BindToScene();
#endif
	}

	void Light::BindCameras()
	{
		auto cams = cameras();
		for (auto& uuid : cams) {
			BindCamera(MAKESUUUID(unit, uuid));
		}
	}

	void Light::BindCamera(CameraID camera)
	{
		if (camera.empty()) return;
		Scene::BindToScene(unit, uuid(), camera.uuid());
	}

	void Light::UnbindFromScene()
	{
#include <TrackUUID/JErase.h>
#include "LightAtt.h"
#include <JEnd.h>

		UnbindCameras();

		if (!hasShadowMaps()) return;

#if defined(_EDITOR)
		DestroyShadowMapMinMaxChain();
#endif
		DestroyShadowMap();
	}

	void Light::UnbindCameras()
	{
		auto cams = cameras();
		for (auto& uuid : cams) {
			UnbindCamera(MAKESUUUID(unit, uuid));
		}
	}

	void Light::UnbindCamera(CameraID camera)
	{
		if (camera.empty()) return;
		camera->UnbindLight(MAKESUUUID(unit, uuid()));
		Scene::UnbindFromScene(unit, uuid(), camera.uuid());
	}

#if defined(_EDITOR)
	void Light::DropJsonMoldAttributes(nlohmann::json& j)
	{
		SceneObject::DropJsonMoldAttributes(j);
		j.at("cameras") = nlohmann::json::array({});
	}
#endif

	void Light::Destroy()
	{
		DestroyEditorPreview();
#include <Attributes/JDestroy.h>
#include "LightAtt.h"
#include <JEnd.h>

		SceneObject::Destroy();
	}

	XMVECTOR Light::positionV()
	{
		XMFLOAT3 pos = position();
		return XMLoadFloat3(&pos);
	}

	void Light::updateRotationQ()
	{
		XMFLOAT3 v = rotation();
		rotationQuaternion = XMQuaternionRotationRollPitchYaw(
			XMConvertToRadians(v.x),
			XMConvertToRadians(v.y),
			XMConvertToRadians(v.z)
		);
		for (auto c : shadowMapCameras)
		{
			c->rotationQ(rotationQuaternion);
		}
	}

	XMVECTOR Light::rotationQ()
	{
		return rotationQuaternion;
	}

	void Light::rotationQ(XMVECTOR q)
	{
		rotationQuaternion = q;
		for (auto c : shadowMapCameras)
		{
			c->rotationQ(rotationQuaternion);
		}
	}

	XMMATRIX Light::world()
	{
		XMMATRIX rotationM = XMMatrixRotationQuaternion(rotationQ());
		XMMATRIX positionM = XMMatrixTranslationFromVector(positionV());
		return XMMatrixMultiply(rotationM, positionM);
	}

	XMVECTOR Light::fw()
	{
		FXMVECTOR dir = { 0.0f, 0.0f, 1.0f,0.0f };
		XMVECTOR fw = XMVector3Normalize(XMVector3Rotate(dir, rotationQ()));
		return fw;
	}

	bool Light::RenderReady()
	{
		return renderReady;
	}

	void Light::RenderReady(bool value)
	{
		for (auto& c : shadowMapCameras)
		{
			c->RenderReady(value);
		}
		renderReady = value;
	}

	void Light::WriteConstantsBufferLightAttributes(LightAttributes& atts)
	{
		ZeroMemory(&atts, sizeof(atts));
		atts.lightType = lightType();

		switch (lightType())
		{
		case LT_Ambient:
		{
			atts.lightColor = color() * brightness();
		}
		break;
		case LT_Directional:
		{
			atts.lightColor = color() * brightness();
			XMVECTOR lightDir = fw();
			atts.atts1 = *((XMFLOAT4*)&lightDir.m128_f32[0]);
			atts.hasShadowMap = hasShadowMaps();
			atts.shadowMapIndex = shadowMapIndex;
		}
		break;
		case LT_Spot:
		{
			XMFLOAT3 pos = position();
			XMFLOAT3 atte = attenuation();
			atts.lightColor = color() * brightness();
			atts.atts1 = { pos.x, pos.y, pos.z, 0.0f };
			XMVECTOR lightDir = fw();
			atts.atts2 = *((XMFLOAT4*)&lightDir.m128_f32[0]);
			atts.atts2.w = cosf(XMConvertToRadians(coneAngle()));
			atts.atts3 = { atte.x, atte.y, atte.z };
			atts.hasShadowMap = hasShadowMaps();
			atts.shadowMapIndex = shadowMapIndex;
		}
		break;
		case LT_Point:
		{
			XMFLOAT3 pos = position();
			XMFLOAT3 atte = attenuation();
			atts.lightColor = color() * brightness();
			atts.atts1 = { pos.x, pos.y, pos.z, 0.0f };
			atts.atts2 = { atte.x, atte.y, atte.z, 0.0f };
			atts.hasShadowMap = hasShadowMaps();
			atts.shadowMapIndex = shadowMapIndex;
		}
		break;
		}
	}

#if defined(_EDITOR)
	static std::map<LightType, nlohmann::json> defaultShadowMapParameters = {
		{ LT_Directional, {{ "shadowMapWidth",1024}, {"shadowMapHeight",1024}, {"viewWidth", 32.0f}, {"viewHeight",32.0f},{"nearZ",0.01f}, {"farZ",1000.0f}}},
		{ LT_Spot, {{ "shadowMapWidth",1024}, {"shadowMapHeight",1024}, {"viewWidth", 32.0f}, {"viewHeight",32.0f},{"nearZ",0.01f}, {"farZ",100.0f}} },
		{ LT_Point, {{ "shadowMapWidth",1024}, {"shadowMapHeight",1024}, {"nearZ",0.01f}, {"farZ",20.0f}} },
	};

	static std::vector<std::string> shadowMapJsonAttributes = {
		"shadowMapWidth", "shadowMapHeight", "viewWidth", "viewHeight", "nearZ", "farZ",
	};

	void WriteLightsJson(SceneUnitId id, nlohmann::json& json)
	{
#include <Editor/JSaveFile.h>
#include "LightAtt.h"
#include <JEnd.h>
	}

	void Light::EditorPreview(size_t flags)
	{
		if (flags & (1 << Light::Update_hasShadowMaps))
		{
			if (hasShadowMaps())
				CreateShadowMapMinMaxChain();
		}
		switch (lightType())
		{
		case LT_Directional:
		case LT_Spot:
		case LT_Point:
		{
			//leave commented until i know why this was here initialy, not actual part of the convertion comments
			//Editor::SelectSceneObject(uuid());
		}
		break;
		}
	}

	void Light::DestroyEditorPreview()
	{
		destroySMChain = true;
		destroySteps = destroyStepsCount;
		switch (lightType())
		{
		case LT_Directional:
		{
			//leave commented until i know why this was here initialy, not actual part of the convertion comments
			//Editor::DeselectSceneObject(uuid());
		}
		break;
		}
	}

	RenderableID Light::CreateBillboard(CameraID camera)
	{
		using namespace Editor;

		if (lightType() == LT_Ambient) return RenderableID();

		RenderableID bb = Editor::CreateBillboardFromMaterials(unit, camera, at("name"), "LightBulb", "LightBulbPicking");
		bb->OnPick = [&] { SelectLight(SUuuid()); };
		return bb;
	}

	void Light::UpdateBillboard(RenderableID renderable)
	{
		assert(!renderable.empty());
		if (renderable.empty()) return;

		auto& scene = GetSceneUnit(unit);

		XMFLOAT3 baseColor = color();
		renderable->position(position());
		renderable->WriteConstantsBuffer("baseColor", &baseColor, scene->Frame());
		renderable->WriteConstantsBuffer(scene->Frame());
	}

	BoundingBox Light::GetBoundingBox()
	{
		return BoundingBox(position(), { 0.1f,0.1f,0.1f });
	}

	bool Light::CanInteractWithGizmo(ImGuizmo::OPERATION operation)
	{
		return lightType() != LT_Ambient;
	}
#endif

	void LightsStep(SceneUnitId id)
	{
		std::set<LightID> lightsToUpdateCamAttributes;
		std::set<LightID> lightsToUpdateTransformation;
		std::set<LightID> lightsToDelete;
		std::set<LightID> lightToRecreateCameras;

		std::set<Light::Light_UpdateFlags> smCamAttributes =
		{
			Light::Update_coneAngle, Light::Update_shadowMapWidth, Light::Update_shadowMapHeight,
			Light::Update_viewWidth, Light::Update_viewHeight, Light::Update_nearZ, Light::Update_farZ
		};
		std::set<Light::Light_UpdateFlags> smCamTransformations =
		{
			Light::Update_position, Light::Update_rotation, Light::Update_dirDist
		};

		auto lights = GetLights(id);

		auto& scene = GetSceneUnit(id);

		for (auto& uuid : lights)
		{
			LightID l = MAKESUUUID(id, uuid);

			//is this(hack) or fix the loading system
			//if (!l->RenderReady() && scene->IsBound(uuid))
			//{
			//	l->RenderReady(true);
			//	scene->EraseLightFromLoadingPool(l);
			//}

			if (l->lightType() != LT_Ambient)
			{
				if (l->dirty(Light::Update_cameras))
				{
					l->RenderReady(false);
					lightToRecreateCameras.insert(l);
				}
			}

			//if the light type changed
			if (l->dirty(Light::Update_lightType))
			{
				//use default attributes depending of the light type
				l->JUpdate(editorDefaultLightProperties.at(l->lightType()));

				//we deactivate shadowmaps always a light type is converted
				if (l->hasShadowMaps())
				{
					l->hasShadowMaps(false);
				}

				l->clean(Light::Update_lightType);
			}

			if (l->dirty(Light::Update_hasShadowMaps))
			{
				if (l->hasShadowMaps())
				{
					l->LoadShadowMap();
				}
				else
				{
					l->UnloadShadowMap();
				}
				l->clean(Light::Update_hasShadowMaps);
			}

			//if destroying SMChain
			if (l->destroySMChain)
			{
				l->destroySteps--;
				if (l->destroySteps == 0)
				{
					l->destroySMChain = false;
					l->DestroyShadowMapMinMaxChain();
				}
			}

			if (std::any_of(smCamAttributes.begin(), smCamAttributes.end(), [&l](auto flag) { return l->dirty(flag); }))
			{
				lightsToUpdateCamAttributes.insert(l);
				std::for_each(smCamAttributes.begin(), smCamAttributes.end(), [&l](auto flag) { l->clean(flag); });
			}
			if (std::any_of(smCamTransformations.begin(), smCamTransformations.end(), [&l](auto flag) { return l->dirty(flag); }))
			{
				lightsToUpdateTransformation.insert(l);
				std::for_each(smCamTransformations.begin(), smCamTransformations.end(), [&l](auto flag) { l->clean(flag); });
			}

			if (l->markedForDelete)
			{
				lightsToDelete.insert(l);
			}
			for (auto l : lightsToUpdateCamAttributes)
			{
				l->UpdateShadowMapCameraProperties();
			}
			for (auto l : lightsToUpdateTransformation)
			{
				l->updateRotationQ();
				l->UpdateShadowMapCameraTransformation();
			}
		}

		for (auto l : lightsToDelete)
		{
			EraseLightFromLights(l->unit, l.uuid());
			EraseLightFromShadowMapLights(l->unit, l.uuid());
			DeleteLightSceneObject(l);
		}

		if (lightToRecreateCameras.size() > 0)
		{
			auto& scene = GetSceneUnit(id);
			/*
			scene->PushLoadingExecutionCallback([=]
				{
					for (auto& l : lightToRecreateCameras)
					{
						l->RenderReady(true);
#if defined(_EDITOR)
						l->EditorPreview(1 << Light::Update_hasShadowMaps);
#endif
					}
				}
			);
			scene->SubmitForLoading([&]
				{
					for (auto& l : lightToRecreateCameras)
					{
						l->UnbindFromScene();
						if (l->hasShadowMaps())
						{
							l->CreateShadowMap();
						}
						l->BindToScene();
						l->clean(Light::Update_cameras);
					}
				}
			);
			*/
		}
	}

	void DestroyLights()
	{
		for (auto& [id, container] : LightSUsceneObjects)
		{
			for (auto& [uuid, _] : container)
			{
				LightID l = MAKESUUUID(id, uuid);
				DeleteLightSceneObject(l);
			}
		}
#include <TrackUUID/JClear.h>
#include "LightAtt.h"
#include <JEnd.h>
	}

	void DestroyLights(SceneUnitId id)
	{
		std::set<JUUID> uuids;
		std::transform(LightSUsceneObjects.at(id).begin(), LightSUsceneObjects.at(id).end(), std::inserter(uuids, uuids.begin()), [](auto& pair) { return pair.first; });
		for (auto& uuid : uuids)
		{
			DeleteLightSceneObject(MAKESUUUID(id, uuid));
		}
#include <TrackUUID/JClearUnit.h>
#include "LightAtt.h"
#include <JEnd.h>
	}

	void DeleteLight(SceneUnitId id, JUUID uuid)
	{
#if defined(_EDITOR)
		using namespace Editor;
#endif
		LightID l = MAKESUUUID(id, uuid);
#if defined(_EDITOR)
		DestroyBillboard(l->unit, uuid);
#endif
		l->markedForDelete = true;
	}
}