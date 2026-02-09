#include "pch.h"
#include "Light.h"
#include <DeviceUtils/RenderPass/RenderToTexturePass.h>
#include <Renderer.h>

#if defined(_EDITOR)
namespace Editor
{
	extern bool IsPlaying(SceneUnitId id);
	extern JUUID GetSceneUnitEditorCamera(SceneUnitId id);
}
#endif

extern std::unique_ptr<Renderer> renderer;

namespace Scene
{
	std::map<SceneUnitId, CD3DX12_CPU_DESCRIPTOR_HANDLE[MaxLights]>  shadowMapSrvCpuDescriptorHandle; //SRV CPU Handles for shadowmaps
	std::map<SceneUnitId, CD3DX12_GPU_DESCRIPTOR_HANDLE[MaxLights]>  shadowMapSrvGpuDescriptorHandle; //SRV GPU Handles for shadowmaps
	std::map<SceneUnitId, std::set<unsigned int>> usedShadowMapSlots;

	void DumpDescriptorHandles(SceneUnitId unit)
	{
		OutputDebugStringA(std::string(std::string(__FUNCTION__) + ":" + std::to_string(unit) + "\n").c_str());
		for (UINT i = 0; i < MaxLights; i++)
		{
			OutputDebugStringA(std::string(std::string("cpu") + ":" + std::to_string(shadowMapSrvCpuDescriptorHandle[unit][i].ptr) +
				std::string(", gpu") + ":" + std::to_string(shadowMapSrvGpuDescriptorHandle[unit][i].ptr) + "\n").c_str());
		}
	}

	void Light::BindRenderablesToShadowMapCamera()
	{
		for (JUUID caster : GetShadowCasts(unit))
		{
			RenderableID r = MAKESUUUID(unit, caster);
			auto rvCams = r->cameras();
			auto lvCams = cameras();
			std::set<CameraID> rCams;
			std::transform(rvCams.begin(), rvCams.end(), std::inserter(rCams, rCams.begin()), [&](JUUID c) { return MAKESUUUID(unit, c); });
			std::set<CameraID> lCams;
			std::transform(lvCams.begin(), lvCams.end(), std::inserter(lCams, lCams.begin()), [&](JUUID c) { return MAKESUUUID(unit, c); });
			bool mkbind = std::any_of(rCams.begin(), rCams.end(), [&lCams](auto uuid)
				{
					return lCams.contains(uuid);
				}
			);
			if (!mkbind)
				continue;

			for (auto& cuuid : shadowMapCameras)
			{
				Scene::BindToScene(unit, cuuid.uuid(), r.uuid());
			}
		}
	}

	void Light::UnbindRenderablesFromShadowMapCameras()
	{
		for (auto& r : GetShadowCasts(unit))
		{
			for (auto& cuuid : shadowMapCameras)
			{
				Scene::UnbindFromScene(unit, cuuid.uuid(), r);
			}
		}
	}

	void Light::UnbindRenderableFromShadowMapCamera(RenderableID r)
	{
		for (auto& cam : shadowMapCameras)
		{
			cam->UnbindRenderable(r);
			Scene::UnbindFromScene(unit, cam.uuid(), r.uuid());
		}
	}

	void Light::LoadShadowMap()
	{
		InsertLightIntoShadowMapLights(unit, uuid());
		auto& scene = GetSceneUnit(unit);
		scene->ResetLoadingCommandList();
		scene->SetLoading(true);
		scene->SetCanSubmitLoading(false);

		CreateShadowMap();
		BindSceneObjects(scene->Id());
		BindRenderablesToShadowMapCamera();
		auto cams = cameras();
		for (auto& uuid : cams)
		{
			CameraID cam = MAKESUUUID(unit, uuid);
			cam->BindLightWithShadowMap(SUuuid());
		}
		scene->SetCanSubmitLoading(true);

#if defined(_EDITOR)
		EditorPreview(1 << Light::Update_hasShadowMaps);
#endif

		std::set<Light::Light_UpdateFlags> smAttributes =
		{
			Light::Update_coneAngle, Light::Update_shadowMapWidth, Light::Update_shadowMapHeight,
			Light::Update_viewWidth, Light::Update_viewHeight, Light::Update_nearZ, Light::Update_farZ,
			Light::Update_position, Light::Update_rotation, Light::Update_dirDist
		};
		for (auto f : smAttributes)
		{
			dirty(1ULL << f);
		}
	}

	void Light::CreateShadowMap()
	{
		switch (lightType())
		{
		case LT_Directional:
		{
			CreateDirectionalLightShadowMap();
		}
		break;
		case LT_Spot:
		{
			CreateSpotLightShadowMap();
		}
		break;
		case LT_Point:
		{
			CreatePointLightShadowMap();
		}
		break;
		default:
		{
			assert(lightType() != LT_Directional || lightType() != LT_Spot || lightType() != LT_Point);
		}
		break;
		}

		CreateShadowMapDepthStencilResource();
	}

	nlohmann::json Light::CreateDirectionalShadowMapCameraJson(unsigned camIndex)
	{
		XMFLOAT3 pos = position();
		XMFLOAT3 rot = rotation();
		nlohmann::json j = {
			{ "uuid", uuid() + "-cam-" + std::to_string(camIndex)},
			{ "hidden", true },
			{ "name", name() + ".cam." + std::to_string(camIndex)},
			{ "fitWindow", false },
			{ "projectionType", ProjectionsTypesToString.at(PROJ_Orthographic) },
			{ "orthographic", {
				{ "nearZ", nearZ() },
				{ "farZ", farZ() },
				{ "viewLeft", viewWidth() },
				{ "viewRight", viewRight() },
				{ "viewBottom", viewBottom() },
				{ "viewTop", viewHeight() },
			}},
			{ "position", { pos.x, pos.y, pos.z }},
			{ "rotation", { rot.x, rot.y, rot.z }},
			{ "shadowMapLight", uuid()},
			{ "renderPasses", { GetRenderPassUUIDByName("ShadowMap") }}
		};
		return j;
	}

	nlohmann::json Light::CreateSpotShadowMapCameraJson()
	{
		float spotDim = 2.0f * sinf(XMConvertToRadians(coneAngle())) * farZ();

		XMFLOAT3 pos = position();
		XMFLOAT3 rot = rotation();

		nlohmann::json j = {
			{ "uuid", uuid() + "-cam"},
			{ "hidden", true },
			{ "fitWindow", false },
			{ "name", name() + ".cam" },
			{ "projectionType", ProjectionsTypesToString.at(PROJ_Perspective) },
			{ "perspective", {
				{ "nearZ", nearZ() },
				{ "farZ", farZ() },
				{ "fovAngleY", coneAngle() * 2.0f },
				{ "width", spotDim },
				{ "height", spotDim }
			}},
			{ "position", { pos.x, pos.y, pos.z}},
			{ "rotation", { rot.x, rot.y, rot.z }},
			{ "shadowMapLight", uuid()},
			{ "renderPasses", { GetRenderPassUUIDByName("ShadowMap") }}
		};
		return j;
	}

	nlohmann::json Light::CreatePointShadowMapCameraJson(unsigned camIndex)
	{
		float fDim = static_cast<float>(shadowMapWidth());
		XMFLOAT3 pos = position();

		nlohmann::json j = {
			{ "uuid", uuid() + "-cam-" + std::to_string(camIndex)},
			{ "hidden", true },
			{ "name", name() + ".cam." + std::to_string(camIndex)},
			{ "fitWindow", false },
			{ "projectionType", ProjectionsTypesToString.at(PROJ_Perspective) },
			{ "perspective",
			{
				{ "nearZ", nearZ() },
				{ "farZ", farZ() },
				{ "fovAngleY", 90.0f },
				{ "width", fDim },
				{ "height", fDim },
			}
			},
			{ "position", { pos.x, pos.y, pos.z}},
			{ "shadowMapLight", uuid()},
			{ "renderPasses", { GetRenderPassUUIDByName("ShadowMap") }}
		};
		return j;
	}

	void Light::CreateDirectionalLightShadowMap()
	{
		shadowMapScissorRect.clear();
		shadowMapViewport.clear();

		for (unsigned int i = 0; i < (ARRAYSIZE(cascadePartitionsZeroToOne) - 1ULL); i++)
		{
			nlohmann::json camJ = CreateDirectionalShadowMapCameraJson(i);
			JUUID uuid = camJ.at("uuid");
			CreateCamera(unit, camJ);
			CameraID cam = MAKESUUUID(unit, uuid);
			cam->BindToScene();
			shadowMapCameras.push_back(cam);
		}
		UpdateShadowMapCameraProperties();
	}

	void Light::CreateSpotLightShadowMap()
	{
		shadowMapScissorRect.clear();
		shadowMapViewport.clear();

		nlohmann::json camJ = CreateSpotShadowMapCameraJson();
		JUUID uuid = camJ.at("uuid");
		CreateCamera(unit, camJ);
		CameraID cam = MAKESUUUID(unit, uuid);
		cam->BindToScene();
		shadowMapCameras.push_back(cam);
		UpdateShadowMapCameraProperties();
	}

	void Light::CreatePointLightShadowMap()
	{
		shadowMapScissorRect.clear();
		shadowMapViewport.clear();

		for (unsigned int i = 0U; i < 6U; i++)
		{
			nlohmann::json camJ = CreatePointShadowMapCameraJson(i);
			JUUID uuid = camJ.at("uuid");
			CreateCamera(unit, camJ);
			CameraID cam = MAKESUUUID(unit, uuid);
			cam->BindToScene();
			shadowMapCameras.push_back(cam);
		}
		UpdateShadowMapCameraProperties();
	}

	void Light::CreateShadowMapDepthStencilResource()
	{
		unsigned int w = shadowMapWidth();
		unsigned int h = 0U;
		switch (lightType())
		{
		case LT_Directional:
		{
			h = (ARRAYSIZE(cascadePartitionsZeroToOne) - 1ULL) * shadowMapHeight();
		}
		break;
		case LT_Spot:
		{
			h = shadowMapHeight();
		}
		break;
		case LT_Point:
		{
			h = 6U * shadowMapWidth();
		}
		break;
		}
		shadowMapRenderPass = CreateRenderToTexturePass(name() + "->shadowMap", {}, DXGI_FORMAT_D32_FLOAT, w, h);

		shadowMapIndex = GetNextAvailableShadowMapSlot(unit);
		AllocShadowMapSlot(unit, shadowMapIndex);
		CreateShadowMapShaderResourceView();
	}

	void Light::CreateShadowMapShaderResourceView()
	{
		auto& smPass = shadowMapRenderPass;
		D3D12_SHADER_RESOURCE_VIEW_DESC shadowMapSrvDesc = {
			.Format = Renderer::depthFormatSRVConversion.contains(smPass->depthStencilFormat) ? Renderer::depthFormatSRVConversion.at(smPass->depthStencilFormat) : smPass->depthStencilFormat,
			.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			.Texture2D = {.MostDetailedMip = 0, .MipLevels = 1U, .ResourceMinLODClamp = 0.0f },
		};
		renderer->d3dDevice->CreateShaderResourceView(smPass->depthStencilTexture, &shadowMapSrvDesc, shadowMapSrvCpuDescriptorHandle[unit][shadowMapIndex]);
	}

	void Light::UnloadShadowMap()
	{
		EraseLightFromShadowMapLights(unit, uuid());
		DestroyShadowMap();
		UnbindRenderablesFromShadowMapCameras();
		auto cams = cameras();
		for (auto& uuid : cams)
		{
			CameraID cam = MAKESUUUID(unit, uuid);
			cam->UnbindLightWithShadowMap(SUuuid());
		}
#if defined(_EDITOR)
		DestroyEditorPreview();
#endif
	}

	void Light::DestroyShadowMap()
	{
		DestroyShadowMapCameras();

		unsigned int numShadowMaps = static_cast<unsigned int>(usedShadowMapSlots[unit].size());
		FreeShadowMapSlot(unit, shadowMapIndex);

		//this is tricky hence worth of explaining, if there are more than one shadowmap we need to shift left the CPU/GPU handles representing the textures adresses
		if (numShadowMaps > 1)
		{
			//get how many lights are needed to shift left
			unsigned int numHandlesToShift = numShadowMaps - shadowMapIndex - 1;
			unsigned int shiftFrom = shadowMapIndex + 1;
			unsigned int shiftTo = shadowMapIndex;

			if (numHandlesToShift > 0)
			{
				//reasign the index of each light which shadow map index is greather than the this light shadow map index to a one less index value
				std::set<LightID> lightsToShift;
				for (JUUID c : cameras())
				{
					CameraID cam = MAKESUUUID(unit, c);
					for (LightID light : cam->lightsWithShadowMaps)
					{
						if (!LightSceneObjectExist(light) || light->shadowMapIndex <= shadowMapIndex || light->shadowMapIndex == 0 || light->shadowMapIndex == 0xFFFFFFFF) continue;
						lightsToShift.insert(light);
					}
				}
				for (LightID light : lightsToShift)
				{
					light->shadowMapIndex--;
					light->CreateShadowMapShaderResourceView();
				}

				AllocShadowMapSlot(unit, shadowMapIndex);
				FreeShadowMapSlot(unit, numShadowMaps - 1);
			}
		}

		//destroy the render pass
		DeleteRenderToTexturePass(shadowMapRenderPass());
		//and make the shadowmap index FFF...
		shadowMapIndex = 0xFFFFFFFF;
	}

	void Light::DestroyShadowMapCameras()
	{
		for (auto cam : shadowMapCameras)
		{
			cam->DestroyRenderPasses();
			DestroyConstantsBuffer(cam->cameraCb);
			DeleteCameraSceneObject(cam);
		}
		shadowMapCameras.clear();
	}

	void Light::UpdateShadowMapCameraProperties()
	{
		if (shadowMapCameras.empty()) return;

		switch (lightType())
		{
		case LT_Directional:
		{
			UpdateDirectionalShadowMapCameraProperties();
		}
		break;
		case LT_Spot:
		{
			UpdateSpotShadowMapCameraProperties();
		}
		break;
		case LT_Point:
		{
			UpdatePointShadowMapCameraProperties();
		}
		break;
		default:
		{
			assert(lightType() != LT_Directional || lightType() != LT_Spot || lightType() != LT_Point);
		}
		break;
		}
	}

	void Light::UpdateDirectionalShadowMapCameraProperties()
	{
		float fDim = static_cast<float>(shadowMapWidth());
		long lDim = static_cast<long>(shadowMapWidth());

		shadowMapScissorRect.clear();
		shadowMapViewport.clear();
		for (unsigned int i = 0U; i < (ARRAYSIZE(cascadePartitionsZeroToOne) - 1ULL); i++)
		{
			shadowMapScissorRect.push_back({ 0, static_cast<long>(i) * lDim, lDim, static_cast<long>(i + 1U) * lDim });
			shadowMapViewport.push_back({ 0.0f, static_cast<float>(i) * fDim, fDim, fDim, 0.0f, 1.0f });
		}
	}

	BoundingFrustum GetCascadeViewCameraBoundingFrustum(CameraID cam, unsigned int cascadeId, std::vector<std::tuple<float, float>>& shadowMapNearFarPlanes)
	{
		XMMATRIX camWorld = cam->world();
		float nearFarRange = cam->projectionFarZ() - cam->projectionNearZ();

		float nearPlane = cam->projectionNearZ();
		if (cascadeId > 0)
			nearPlane += (nearFarRange * cascadePartitionsZeroToOne[cascadeId - 1]);
		float farPlane = cam->projectionFarZ() * cascadePartitionsZeroToOne[cascadeId];

		Perspective cascadePerspectiveProjection = cam->perspectiveProjection;
		cascadePerspectiveProjection.nearZ = nearPlane;
		cascadePerspectiveProjection.farZ = farPlane;
		cascadePerspectiveProjection.updateProjectionMatrix();
		BoundingFrustum boundingFrustum;
		BoundingFrustum(cascadePerspectiveProjection.projectionMatrix).Transform(boundingFrustum, camWorld);

		//store the near and far plane in the vector
		shadowMapNearFarPlanes.push_back(std::make_tuple(nearPlane, farPlane));

		//the boundingFrustum is in world space
		return boundingFrustum;
	}

	void Light::CreateDirectionalCascadeShadowMapViewProjectionMatrices()
	{
		using namespace Scene::CameraProjections;

		shadowMapNearFarPlanes.clear();

		JUUID camUUID = *cameras().begin();
#if defined(_EDITOR)
		if (!Editor::IsPlaying(unit) && !SceneIsIsolated(unit))
		{
			camUUID = Editor::GetSceneUnitEditorCamera(unit);
		}
#endif

		CameraID viewCamera = MAKESUUUID(unit, camUUID);
		CameraID lightCamera = MAKESUUUID(unit, shadowMapCameras.at(0).uuid());
		XMMATRIX lightViewMatrix = lightCamera->view();

		for (unsigned int i = 0U; i < (ARRAYSIZE(cascadePartitionsZeroToOne) - 1ULL); i++)
		{
			BoundingFrustum worldSpaceBoundingFrustum = GetCascadeViewCameraBoundingFrustum(viewCamera, i, shadowMapNearFarPlanes);
			BoundingFrustum lightViewSpaceBoundingFrustum;
			worldSpaceBoundingFrustum.Transform(lightViewSpaceBoundingFrustum, lightViewMatrix);

			XMFLOAT3 cornersLightViewSpace[8];
			lightViewSpaceBoundingFrustum.GetCorners(cornersLightViewSpace);

			BoundingBox lightSpaceCascadeAABB;
			BoundingBox::CreateFromPoints(lightSpaceCascadeAABB, ARRAYSIZE(cornersLightViewSpace), cornersLightViewSpace, sizeof(cornersLightViewSpace[0]));

			CameraID cascadeLightCamera = MAKESUUUID(unit, shadowMapCameras.at(i).uuid());
			auto& csmProj = cascadeLightCamera->orthographicProjection;

			csmProj.viewLeft = lightSpaceCascadeAABB.Center.x - lightSpaceCascadeAABB.Extents.x;
			csmProj.viewRight = lightSpaceCascadeAABB.Center.x + lightSpaceCascadeAABB.Extents.x;
			csmProj.viewBottom = lightSpaceCascadeAABB.Center.y - lightSpaceCascadeAABB.Extents.y;
			csmProj.viewTop = lightSpaceCascadeAABB.Center.y + lightSpaceCascadeAABB.Extents.y;
			csmProj.nearZ = lightSpaceCascadeAABB.Center.z - lightSpaceCascadeAABB.Extents.z;
			csmProj.farZ = lightSpaceCascadeAABB.Center.z + lightSpaceCascadeAABB.Extents.z;
			csmProj.updateProjectionMatrix();
		}
	}

	void Light::UpdateSpotShadowMapCameraProperties()
	{
		float spotDim = 2.0f * sinf(XMConvertToRadians(coneAngle())) * farZ();
		shadowMapScissorRect.push_back({ 0, 0, static_cast<long>(shadowMapWidth()), static_cast<long>(shadowMapHeight()) });
		shadowMapViewport.push_back({ 0.0f , 0.0f, static_cast<float>(shadowMapWidth()), static_cast<float>(shadowMapHeight()), 0.0f, 1.0f });
		shadowMapTexelInvSize = { 1.0f / static_cast<float>(shadowMapWidth()), 1.0f / static_cast<float>(shadowMapHeight()) };
		auto& cam = shadowMapCameras.at(0);
		cam->perspectiveProjection.fovAngleY = coneAngle() * 2.0f;
		cam->perspectiveProjection.updateProjectionMatrix(spotDim, spotDim);
	}

	void Light::UpdatePointShadowMapCameraProperties()
	{
		float fDim = static_cast<float>(shadowMapWidth());
		long lDim = static_cast<long>(shadowMapWidth());

		shadowMapScissorRect.clear();
		shadowMapViewport.clear();
		for (unsigned int i = 0U; i < 6U; i++)
		{
			shadowMapScissorRect.push_back({ 0, static_cast<long>(i) * lDim, lDim, static_cast<long>(i + 1U) * lDim });
			shadowMapViewport.push_back({ 0.0f, static_cast<float>(i) * fDim, fDim, fDim, 0.0f, 1.0f });
		}

		for (unsigned int i = 0U; i < 6U; i++)
		{
			auto& cam = shadowMapCameras.at(i);
			cam->perspectiveProjection.updateProjectionMatrix(fDim, fDim);
		}
	}

	void Light::UpdateShadowMapCameraTransformation()
	{
		if (shadowMapCameras.empty()) return;

		switch (lightType())
		{
		case LT_Directional:
		{
			UpdateDirectionalShadowMapCameraTransformation();
		}
		break;
		case LT_Spot:
		{
			UpdateSpotShadowMapCameraTransformation();
		}
		break;
		case LT_Point:
		{
			UpdatePointShadowMapCameraTransformation();
		}
		break;
		default:
		{
			assert(lightType() != LT_Directional || lightType() != LT_Spot || lightType() != LT_Point);
		}
		break;
		}
	}

	void Light::UpdateDirectionalShadowMapCameraTransformation()
	{
		auto& cam = shadowMapCameras.at(0);
		cam->position(position());
		cam->rotation(rotation());
	}

	void Light::UpdateSpotShadowMapCameraTransformation()
	{
		auto& cam = shadowMapCameras.at(0);
		cam->rotation(rotation());
		cam->position(position());
	}

	void Light::UpdatePointShadowMapCameraTransformation()
	{
		XMFLOAT3 pos = position();
		std::for_each(shadowMapCameras.begin(), shadowMapCameras.end(), [&pos](CameraID cam)
			{
				cam->position(pos);
			}
		);
	}

#if defined(_EDITOR)

	void Light::CreateShadowMapMinMaxChain()
	{
		auto& scene = GetSceneUnit(unit);
		bool doSubmit = false;
		if (!scene->LoadingCommandListIsOpen())
		{
			scene->ResetLoadingCommandList();
			scene->SetLoading(true);
			scene->SetCanSubmitLoading(false);
			doSubmit = true;
		}

		//pick the gpu handles for the final shadowmap and copies for the min/max chain initial calculation
		CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapChainGpuHandle = GetShadowMapGpuDescriptorHandle(unit, shadowMapIndex);
		CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapChainGpuHandle1 = shadowMapChainGpuHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapChainGpuHandle2 = shadowMapChainGpuHandle;

		float texWidth = static_cast<float>(shadowMapWidth());
		float texHeight;
		switch (lightType())
		{
		case LT_Directional:
		{
			texHeight = static_cast<float>(shadowMapWidth()) * 3.0f;
		}
		case LT_Spot:
		{
			texHeight = static_cast<float>(shadowMapHeight());
		}
		case LT_Point:
		{
			texHeight = static_cast<float>(shadowMapWidth()) * 6.0f;
		}
		break;
		}

		//calculate the width/height of the texture and the TexelInvSize of the shadow map texture for the current pass
		unsigned int width = static_cast<unsigned int>(texWidth) >> 1;
		unsigned int height = static_cast<unsigned int>(texHeight) >> 1;

		unsigned int renderPassIndex = 0;
		unsigned int chainIndex = 0;
		do
		{
			shadowMapMinMaxChainRenderPass.push_back(
				CreateRenderPassInstance(CameraID(), GetRenderPassUUIDByName("ShadowMapMinMaxChainPass"), renderPassIndex, std::max(2U, width), std::max(2U, height))
			);
			auto& rpInstance = shadowMapMinMaxChainRenderPass.back();
			MinMaxChainPass* chainPass = static_cast<MinMaxChainPass*>(rpInstance->overridePass.get());
			chainPass->shadowMapChainGpuHandle1 = shadowMapChainGpuHandle1;
			chainPass->shadowMapChainGpuHandle2 = shadowMapChainGpuHandle2;

			auto& rtt0 = chainPass->renderPassInstance->renderToTexturePass->renderToTexture.at(0);
			auto& rtt1 = chainPass->renderPassInstance->renderToTexturePass->renderToTexture.at(1);

			rtt0->renderToTexture->SetName(nostd::StringToWString(std::string(name() + "->smC[" + std::to_string(chainIndex) + "][0]")).c_str());
			rtt1->renderToTexture->SetName(nostd::StringToWString(std::string(name() + "->smC[" + std::to_string(chainIndex) + "][1]")).c_str());

			shadowMapChainGpuHandle1 = rtt0->gpuTextureHandle;
			shadowMapChainGpuHandle2 = rtt1->gpuTextureHandle;

			//OutputDebugStringA(std::string(std::to_string(chainIndex) + "(" + std::to_string(width) + "," + std::to_string(height) + ")" + "\n").c_str());

			//calculate the next width and height
			width = std::max(1U, width >> 1);
			height = std::max(1U, height >> 1);
			chainIndex++;
		} while (width != 1U || height != 1U);

		unsigned int texUWidth = 512U;
		unsigned int texUHeight = 512U * ((lightType() == LT_Point) ? 6U : 1U);
		shadowMapMinMaxChainResultRenderPass = CreateRenderPassInstance(
			CameraID(), GetRenderPassUUIDByName("ShadowMapMinMaxChainResultPass"), 0, texUWidth, texUHeight);

		MinMaxChainResultPass* resultPass = static_cast<MinMaxChainResultPass*>(shadowMapMinMaxChainResultRenderPass->overridePass.get());

		RenderPassInstanceID last = shadowMapMinMaxChainRenderPass.back();
		resultPass->depthGpuHandle = shadowMapChainGpuHandle;
		resultPass->shadowMapChainGpuHandle1 = last->renderToTexturePass->renderToTexture.at(0)->gpuTextureHandle;
		resultPass->shadowMapChainGpuHandle2 = last->renderToTexturePass->renderToTexture.at(1)->gpuTextureHandle;
		resultPass->CreateFSQuad((lightType() != LT_Spot) ? "DepthMinMaxToRGBA" : "DepthMinMaxToRGBASpot");

		if (doSubmit)
		{
			scene->SetCanSubmitLoading(true);
		}
	}

	void Light::DestroyShadowMapMinMaxChain()
	{
		DestroyRenderPassInstance(shadowMapMinMaxChainResultRenderPass());
		shadowMapMinMaxChainResultRenderPass.clear();
		for (auto& rp : shadowMapMinMaxChainRenderPass)
		{
			DestroyRenderPassInstance(rp());
		}
		shadowMapMinMaxChainRenderPass.clear();
	}

	void Light::RenderShadowMapMinMaxChain()
	{
		for (auto& rpi : shadowMapMinMaxChainRenderPass)
		{
			rpi->Pass(unit);
		}
		shadowMapMinMaxChainResultRenderPass->Pass(unit);
	}

	/*std::function<bool(JObject*)> Light::GetAssetsConditioner()
	{
		return [this](JObject* j)
			{
				SceneObject* so = static_cast<SceneObject*>(j);
				switch (so->JType())
				{
				case SO_Lights:
				{
					std::set<std::tuple<LightType, LightType>> compatibility = {
						std::make_tuple(LightType::LT_Spot,LightType::LT_Point),
						std::make_tuple(LightType::LT_Point,LightType::LT_Spot)
					};
					Light* l = static_cast<Light*>(so);
					return compatibility.contains(std::make_tuple(l->lightType(), lightType()));
				}
				break;
				}
				return true;
			};
	}*/
#endif

	void Light::WriteConstantsBufferShadowMapAttributes(ShadowMapAttributes& atts)
	{
		ZeroMemory(&atts, sizeof(atts));

		switch (lightType()) {
		case LT_Directional:
		{
			XMMATRIX* attsN = &atts.atts0;
			int i = 0;
			for (auto& cam : shadowMapCameras) {
				XMMATRIX view = cam->view();
				XMMATRIX projection = cam->projection();
				*attsN = XMMatrixMultiply(view, projection);

				atts.atts3.r[0].m128_f32[i] = shadowMapNearFarPlanes.size() > i ? std::get<1>(shadowMapNearFarPlanes.at(i)) : 0.0f;

				attsN++;
				i++;
			}
			atts.atts7 = { zBias(), shadowMapTexelInvSize.x, shadowMapTexelInvSize.y, 0.0f }; //ZBias, TexelInvSize
		}
		break;
		case LT_Spot:
		{
			auto& cam = shadowMapCameras[0];
			XMMATRIX view = cam->view();
			XMMATRIX projection = cam->perspectiveProjection.projectionMatrix;
			atts.atts0 = XMMatrixMultiply(view, projection);
			atts.atts7 = { zBias(), shadowMapTexelInvSize.x, shadowMapTexelInvSize.y, 0.0f }; //ZBias, TexelInvSize
		}
		break;
		case LT_Point:
		{
			XMMATRIX* attsN = &atts.atts0;
			for (UINT i = 0U; i < 6U; i++) {
				auto& cam = shadowMapCameras[i];
				XMMATRIX view = cam->view();
				XMMATRIX projection = cam->perspectiveProjection.projectionMatrix;
				*attsN = XMMatrixMultiply(view, projection);
				attsN++;
			}
			atts.atts7 = { zBias(), 3.0f, 0.0f, 0.0f }; //ZBias, PartialDerivativeScale
		}
		break;
		default:
		{
			assert(lightType() != LT_Directional || lightType() != LT_Spot || lightType() != LT_Point);
		}
		break;
		}
	}

	//RENDER
	void Light::RenderShadowMap(std::function<void(unsigned int)> renderScene)
	{
		if (cameras().empty()) return;
		auto& scene = GetSceneUnit(unit);
		auto& commandList = scene->GetCommandList();
		auto& smPass = shadowMapRenderPass;

		if (lightType() == LT_Directional)
		{
			CreateDirectionalCascadeShadowMapViewProjectionMatrices();
		}

		smPass->Pass(unit, [&](SceneUnitId unit)
			{
				std::unordered_map<LightType, std::function<void()>> LightTypePass =
				{
					{ LT_Directional, [&]()
					{
						for (unsigned int i = 0; i < (ARRAYSIZE(cascadePartitionsZeroToOne) - 1ULL); i++)
						{
							commandList->RSSetViewports(1, &shadowMapViewport.at(i));
							commandList->RSSetScissorRects(1, &shadowMapScissorRect.at(i));
							renderScene(i);
						}
					}
					},
					{ LT_Spot, [&]()
					{
						renderScene(0);
					}
					},
					{ LT_Point, [&]()
					{
						for (unsigned int i = 0; i < 6; i++)
						{
							commandList->RSSetViewports(1, &shadowMapViewport.at(i));
							commandList->RSSetScissorRects(1, &shadowMapScissorRect.at(i));
							renderScene(i);
						}
					}
					}
				};

				LightTypePass.at(lightType())();
			}
		);
	}

	void CreateShadowMapResources(SceneUnitId unit)
	{
		for (UINT i = 0; i < MaxLights; i++)
		{
			AllocCSUDescriptor(shadowMapSrvCpuDescriptorHandle[unit][i], shadowMapSrvGpuDescriptorHandle[unit][i]);
		}
	}

	void DestroyShadowMapResources(SceneUnitId unit)
	{
		for (UINT i = 0; i < MaxLights; i++)
		{
			FreeCSUDescriptor(shadowMapSrvCpuDescriptorHandle[unit][i], shadowMapSrvGpuDescriptorHandle[unit][i]);
		}
	}

	void AllocShadowMapSlot(SceneUnitId unit, unsigned int slot)
	{
		usedShadowMapSlots[unit].insert(slot);
	}

	void FreeShadowMapSlot(SceneUnitId unit, unsigned int slot)
	{
		usedShadowMapSlots[unit].erase(slot);
	}

	unsigned int GetNextAvailableShadowMapSlot(SceneUnitId unit)
	{
		for (unsigned int i = 0; i < MaxLights; i++)
		{
			if (usedShadowMapSlots[unit].contains(i)) continue;
			return i;
		}
		assert(MaxLights != MaxLights);
		return 0xFFFFFFFF;
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetShadowMapGpuDescriptorHandleStart(SceneUnitId unit)
	{
		return shadowMapSrvGpuDescriptorHandle[unit][0];
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetShadowMapGpuDescriptorHandle(SceneUnitId unit, unsigned int index)
	{
		return shadowMapSrvGpuDescriptorHandle[unit][index];
	}
}