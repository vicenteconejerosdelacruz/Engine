#include "pch.h"
#include "Light.h"
//#include <Renderer.h>
#include <Scene.h>
//#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
//#include <SceneObjectDef.h>

//extern std::unique_ptr<Renderer> renderer;

#if defined(_EDITOR)
namespace Editor
{
	extern void SelectLight(SceneUnitId id, JUUID luuid);
	extern JUUID CreateBillboardFromMaterials(SceneUnitId id, CameraSUUUID camera, std::string name, std::string material, std::string pickingMaterial);
	extern void RegisterBillboard(SceneUnitId id, JUUID sceneObject);
	//extern JUUID GetBillboard(JUUID sceneObject);
	extern void DestroyBillboard(SceneUnitId id, JUUID sceneObject);
}
#endif

namespace Scene
{
	SODEF_FULL(Light);

#include <TrackUUID/JDef.h>
#include <LightAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <LightAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <LightAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <LightAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <LightAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <LightAtt.h>
#include <JEnd.h>

#endif

	using namespace DeviceUtils;

	Light::Light(SceneUnitId id, nlohmann::json& json) : SceneObject(id, json)
	{
#include <Attributes/JInit.h>
#include <LightAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <LightAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void Light::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <LightAtt.h>
#include <JEnd.h>
	}
#endif

	void Light::Initialize()
	{
#if defined(_EDITOR)
		using namespace Editor;
#endif

#include <TrackUUID/JInsert.h>
#include <LightAtt.h>
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
	}

	void Light::BindToScene()
	{
#include <TrackUUID/JInsert.h>
#include <LightAtt.h>
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
			BindCamera(uuid);
		}
	}

	void Light::BindCamera(JUUID cuuid)
	{
		if (cuuid.empty()) return;
		Scene::BindToScene(unit, uuid(), cuuid);
	}

	void Light::UnbindFromScene()
	{
#include <TrackUUID/JErase.h>
#include <LightAtt.h>
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
			UnbindCamera(uuid);
		}
	}

	void Light::UnbindCamera(JUUID cuuid)
	{
		if (cuuid.empty()) return;
		CameraSUUUID cam = MAKESUUUID(unit, cuuid);
		cam->UnbindLight(MAKESUUUID(unit, uuid()));
		Scene::UnbindFromScene(unit, uuid(), cuuid);
	}

	void Light::Destroy()
	{
		DestroyEditorPreview();
#include <Attributes/JDestroy.h>
#include <LightAtt.h>
#include <JEnd.h>
	}

	XMMATRIX Light::world()
	{
		XMFLOAT3 posV = position();
		XMFLOAT3 rotV = rotation();
		float roll, pitch, yaw;
		pitch = rotV.x; yaw = rotV.y; roll = rotV.z;
		XMVECTOR rotQ = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
		XMMATRIX rotationM = XMMatrixRotationQuaternion(rotQ);
		XMMATRIX positionM = XMMatrixTranslationFromVector({ posV.x, posV.y, posV.z });
		return XMMatrixMultiply(rotationM, positionM);
	}

	XMVECTOR Light::fw()
	{
		FXMVECTOR dir = { 0.0f, 0.0f, 1.0f,0.0f };
		XMFLOAT3 rotV = rotation();
		float roll, pitch, yaw;
		pitch = rotV.x; yaw = rotV.y; roll = rotV.z;
		XMVECTOR rotQ = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
		XMVECTOR fw = XMVector3Normalize(XMVector3Rotate(dir, rotQ));
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
#include <LightAtt.h>
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

	JUUID Light::CreateBillboard(CameraSUUUID camera)
	{
		using namespace Editor;

		if (lightType() == LT_Ambient) return "";

		JUUID uuid = Editor::CreateBillboardFromMaterials(unit, camera, at("name"), "LightBulb", "LightBulbPicking");
		RenderableSUUUID bb = MAKESUUUID(unit, uuid);
		bb->OnPick = [&] { SelectLight(unit, this->uuid()); };
		UpdateBillboard(uuid);
		return uuid;
	}

	void Light::UpdateBillboard(JUUID uuid)
	{
		assert(!uuid.empty());
		if (uuid.empty()) return;

		auto& scene = GetSceneUnit(unit);

		XMFLOAT3 baseColor = color();
		RenderableSUUUID bb = MAKESUUUID(unit, uuid);
		bb->position(position());
		bb->WriteConstantsBuffer<XMFLOAT3>("baseColor", baseColor, scene->Frame());
		bb->WriteConstantsBuffer(scene->Frame());
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
		//#if defined(_EDITOR)
		//std::set<LightSUUUID> lightsToDestroyShadowMaps;
		//std::set<LightSUUUID> lightsToCreateShadowMaps;
		std::set<LightSUUUID> lightsToUpdateCamAttributes;
		std::set<LightSUUUID> lightsToUpdateTransformation;
		std::set<LightSUUUID> lightsToDelete;
		std::set<LightSUUUID> lightToRecreateCameras;
#if defined(_EDITOR)
		//std::set<LightSUUUID> lightsToDestroySMChain;
#endif

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
		//auto lights = nostd::GetUUIDS(LightsceneObjects);

		for (auto& uuid : lights)
		{
			LightSUUUID l = MAKESUUUID(id, uuid);
			if (l->lightType() != LT_Ambient)
			{
				//JUUID bbuuid = Editor::GetBillboard(l());
				//if (!bbuuid.empty())
				//{
				//	l->UpdateBillboard(bbuuid);
				//}

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
					//lightsToDestroyShadowMaps.insert(l);
					l->hasShadowMaps(false);
				}

				l->clean(Light::Update_lightType);
			}

			if (l->dirty(Light::Update_hasShadowMaps))
			{
				if (l->hasShadowMaps())
				{
					//lightsToCreateShadowMaps.insert(l);
					l->LoadShadowMap();
				}
				else
				{
					l->UnloadShadowMap();
				}
				//else
				//{
				//	lightsToDestroyShadowMaps.insert(l);
				//}
				l->clean(Light::Update_hasShadowMaps);
			}

			//if destroying SMChain
			if (l->destroySMChain)
			{
				l->destroySteps--;
				if (l->destroySteps == 0)
				{
					//lightsToDestroySMChain.insert(l);
					l->destroySMChain = false;
					l->DestroyShadowMapMinMaxChain();
				}
			}

			//			//if resizing
			//			if (l->dirty(Light::Update_shadowMapWidth) || l->dirty(Light::Update_shadowMapHeight))
			//			{
			//				//verify first if the light has shadowmaps(it should)
			//				if (l->hasShadowMaps())
			//				{
			//					lightsToDestroyShadowMaps.insert(l);
			//					lightsToCreateShadowMaps.insert(l);
			//				}
			//				l->clean(Light::Update_shadowMapWidth);
			//				l->clean(Light::Update_shadowMapHeight);
			//			}

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
				//if (l->hasShadowMaps())
				//{
				//	lightsToDestroyShadowMaps.insert(l);
				//}
			}
			//		}
			//
			//		bool criticalFrame = !!lightsToDestroyShadowMaps.size() || !!lightsToCreateShadowMaps.size() || !!lightsToDestroySMChain.size() || !!lightsToDelete.size();
			//
			//		if (criticalFrame)
			//		{
			//			renderer->Flush();
			//			renderer->RenderCriticalFrame([
			//				&lightsToDestroyShadowMaps,
			//				&lightsToCreateShadowMaps,
			//				&lightsToDestroySMChain,
			//				&lightsToDelete
			//			]
			//				{
			//					for (auto l : lightsToDestroyShadowMaps)
			//					{
			//						l->UnbindRenderablesFromShadowMapCameras();
			//						l->DestroyShadowMapMinMaxChain();
			//						l->DestroyShadowMap();
			//					}
			//					for (auto l : lightsToCreateShadowMaps)
			//					{
			//						l->CreateShadowMap();
			//						l->CreateShadowMapMinMaxChain();
			//						l->BindRenderablesToShadowMapCamera();
			//					}
			//					for (auto l : lightsToDestroySMChain)
			//					{
			//						l->DestroyShadowMapMinMaxChain();
			//					}
			//for (auto l : lightsToDelete)
			//{
			//	EraseLightFromLights(l->unit, l.uuid());
			//	EraseLightFromShadowMapLights(l->unit, l.uuid());
			//	DeleteLightSUSceneObject(l->unit, l.uuid());
			//}
			//				}
			//			);
			//		}

			for (auto l : lightsToUpdateCamAttributes)
			{
				l->UpdateShadowMapCameraProperties();
			}
			for (auto l : lightsToUpdateTransformation)
			{
				l->UpdateShadowMapCameraTransformation();
			}
			//#endif
		}

		for (auto l : lightsToDelete)
		{
			EraseLightFromLights(l->unit, l.uuid());
			EraseLightFromShadowMapLights(l->unit, l.uuid());
			DeleteLightSUSceneObject(l->unit, l.uuid());
		}

		if (lightToRecreateCameras.size() > 0)
		{
			auto& scene = GetSceneUnit(id);
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
		}
	}

	void DestroyLights()
	{
		for (auto& [id, container] : LightSUsceneObjects)
		{
			for (auto& [uuid, _] : container)
			{
				LightSUUUID l = MAKESUUUID(id, uuid);
				DeleteLightSUSceneObject(l->unit, l->uuid());
			}
		}
#include <TrackUUID/JClear.h>
#include <LightAtt.h>
#include <JEnd.h>
	}

	void DestroyLights(SceneUnitId id)
	{
		for (auto& [uuid, _] : LightSUsceneObjects.at(id))
		{
			LightSUUUID l = MAKESUUUID(id, uuid);
			DeleteLightSUSceneObject(l->unit, l->uuid());
		}
		//auto uuids = nostd::GetUUIDS(LightsceneObjects);
		//for (LightUUID uuid : uuids)
		//{
		//	if (uuid->unit != unit) continue;
		//	DeleteLightSUSceneObject(uuid->unit, uuid());
		//}
#include <TrackUUID/JClearUnit.h>
#include <LightAtt.h>
#include <JEnd.h>
	}

	void DeleteLight(SceneUnitId id, JUUID uuid)
	{
#if defined(_EDITOR)
		using namespace Editor;
#endif
		LightSUUUID l = MAKESUUUID(id, uuid);
#if defined(_EDITOR)
		DestroyBillboard(l->unit, uuid);
#endif
		l->markedForDelete = true;
	}
}