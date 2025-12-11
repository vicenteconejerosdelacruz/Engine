#include "pch.h"
#include "Light.h"
#include <Renderer.h>
#include <Scene.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <SceneObjectDef.h>

extern std::unique_ptr<Renderer> renderer;

#if defined(_EDITOR)
namespace Editor
{
	extern void SelectLight(JUUID luuid);
	extern JUUID CreateBillboardFromMaterials(CameraUUID camera, std::string name, std::string material, std::string pickingMaterial);
	extern void RegisterBillboard(JUUID sceneObject);
	extern JUUID GetBillboard(JUUID sceneObject);
	extern void DestroyBillboard(JUUID sceneObject);
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

	//CREATE
	void DestroyLights()
	{
		auto uuids = nostd::GetUUIDS(LightsceneObjects);
		for (auto& uuid : uuids)
		{
			DeleteLightSceneObject(uuid);
		}
#include <TrackUUID/JClear.h>
#include <LightAtt.h>
#include <JEnd.h>
	}

	Light::Light(nlohmann::json& json) : SceneObject(json)
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
#include <TrackUUID/JInsert.h>
#include <LightAtt.h>
#include <JEnd.h>

		if (hasShadowMaps())
		{
			CreateShadowMap();
		}
#if defined(_EDITOR)
		if (lightType() != LT_Ambient)
			Editor::RegisterBillboard(uuid());
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
		Scene::BindToScene(uuid(), cuuid);
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
		Scene::UnbindFromScene(uuid(), cuuid);
	}

	void Light::UnbindFromScene()
	{
#include <TrackUUID/JErase.h>
#include <LightAtt.h>
#include <JEnd.h>

		if (!hasShadowMaps()) return;

		UnbindCameras();

#if defined(_EDITOR)
		DestroyShadowMapMinMaxChain();
#endif
		DestroyShadowMap();
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

	//READ&GET

	//UPDATE
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

	void LightsStep()
	{
#if defined(_EDITOR)
		std::set<LightUUID> lightsToDestroyShadowMaps;
		std::set<LightUUID> lightsToCreateShadowMaps;
		std::set<LightUUID> lightsToUpdateCamAttributes;
		std::set<LightUUID> lightsToUpdateTransformation;
		std::set<LightUUID> lightsToDestroySMChain;
		std::set<LightUUID> lightsToDelete;

		std::set<Light::Light_UpdateFlags> smCamAttributes =
		{
			Light::Update_coneAngle, Light::Update_shadowMapWidth, Light::Update_shadowMapHeight,
			Light::Update_viewWidth, Light::Update_viewHeight, Light::Update_nearZ, Light::Update_farZ
		};
		std::set<Light::Light_UpdateFlags> smCamTransformations =
		{
			Light::Update_position, Light::Update_rotation, Light::Update_dirDist
		};

		auto& Lights = GetLights();

		for (LightUUID l : Lights)
		{
			if (l->lightType() != LT_Ambient)
			{
				JUUID bbuuid = Editor::GetBillboard(l());
				if (!bbuuid.empty())
				{
					l->UpdateBillboard(bbuuid);
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
					lightsToDestroyShadowMaps.insert(l);
					l->hasShadowMaps(false);
				}

				l->clean(Light::Update_lightType);
			}

			if (l->dirty(Light::Update_hasShadowMaps))
			{
				if (l->hasShadowMaps())
				{
					lightsToCreateShadowMaps.insert(l);
				}
				else
				{
					lightsToDestroyShadowMaps.insert(l);
				}
				l->clean(Light::Update_hasShadowMaps);
			}

			//if destroying SMChain
			if (l->destroySMChain)
			{
				lightsToDestroySMChain.insert(l);
				l->destroySMChain = false;
			}

			//if resizing
			if (l->dirty(Light::Update_shadowMapWidth) || l->dirty(Light::Update_shadowMapHeight))
			{
				//verify first if the light has shadowmaps(it should)
				if (l->hasShadowMaps())
				{
					lightsToDestroyShadowMaps.insert(l);
					lightsToCreateShadowMaps.insert(l);
				}
				l->clean(Light::Update_shadowMapWidth);
				l->clean(Light::Update_shadowMapHeight);
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

			if (l->markedForDelete) {
				lightsToDelete.insert(l);
				if (l->hasShadowMaps())
				{
					lightsToDestroyShadowMaps.insert(l);
				}
			}
		}

		bool criticalFrame = !!lightsToDestroyShadowMaps.size() || !!lightsToCreateShadowMaps.size() || !!lightsToDestroySMChain.size() || !!lightsToDelete.size();

		if (criticalFrame)
		{
			renderer->Flush();
			renderer->RenderCriticalFrame([
				&lightsToDestroyShadowMaps,
				&lightsToCreateShadowMaps,
				&lightsToDestroySMChain,
				&lightsToDelete
			]
				{
					for (auto l : lightsToDestroyShadowMaps)
					{
						l->UnbindRenderablesFromShadowMapCameras();
						l->DestroyShadowMapMinMaxChain();
						l->DestroyShadowMap();
					}
					for (auto l : lightsToCreateShadowMaps)
					{
						l->CreateShadowMap();
						l->CreateShadowMapMinMaxChain();
						l->BindRenderablesToShadowMapCamera();
					}
					for (auto l : lightsToDestroySMChain)
					{
						l->DestroyShadowMapMinMaxChain();
					}
					for (auto l : lightsToDelete)
					{
						EraseLightFromLights(l());
						EraseLightFromShadowMapLights(l());
						DeleteLightSceneObject(l());
					}
				}
			);
		}

		for (auto l : lightsToUpdateCamAttributes)
		{
			l->UpdateShadowMapCameraProperties();
		}
		for (auto l : lightsToUpdateTransformation)
		{
			l->UpdateShadowMapCameraTransformation();
		}
#endif
	}

	//DESTROY
	void DeleteLight(JUUID uuid)
	{
		LightUUID  l = uuid;
#if defined(_EDITOR)
		Editor::DestroyBillboard(uuid);
#endif
		l->markedForDelete = true;
	}

	//EDITOR
#if defined(_EDITOR)
	static std::map<LightType, nlohmann::json> defaultShadowMapParameters = {
		{ LT_Directional, {{ "shadowMapWidth",1024}, {"shadowMapHeight",1024}, {"viewWidth", 32.0f}, {"viewHeight",32.0f},{"nearZ",0.01f}, {"farZ",1000.0f}}},
		{ LT_Spot, {{ "shadowMapWidth",1024}, {"shadowMapHeight",1024}, {"viewWidth", 32.0f}, {"viewHeight",32.0f},{"nearZ",0.01f}, {"farZ",100.0f}} },
		{ LT_Point, {{ "shadowMapWidth",1024}, {"shadowMapHeight",1024}, {"nearZ",0.01f}, {"farZ",20.0f}} },
	};

	static std::vector<std::string> shadowMapJsonAttributes = {
		"shadowMapWidth", "shadowMapHeight", "viewWidth", "viewHeight", "nearZ", "farZ",
	};

	void WriteLightsJson(nlohmann::json& json)
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

	JUUID Light::CreateBillboard(CameraUUID camera)
	{
		if (lightType() == LT_Ambient) return nullptr;

		JUUID uuid = Editor::CreateBillboardFromMaterials(camera, at("name"), "LightBulb", "LightBulbPicking");
		RenderableUUID bb = uuid;
		bb->OnPick = [this] { Editor::SelectLight(this->uuid()); };
		UpdateBillboard(uuid);
		return uuid;
	}

	void Light::UpdateBillboard(JUUID uuid)
	{
		assert(!uuid.empty());
		if (uuid.empty()) return;

		XMFLOAT3 baseColor = color();
		RenderableUUID bb = uuid;
		bb->position(position());
		bb->WriteConstantsBuffer<XMFLOAT3>("baseColor", baseColor, renderer->backBufferIndex);
		bb->WriteConstantsBuffer(renderer->backBufferIndex);
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
}