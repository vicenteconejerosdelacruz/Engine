#include "pch.h"
#include "Camera.h"
#include <Scene.h>
#include <Light/Light.h>
#include <Renderable/Renderable.h>
#include <Renderer.h>
#include <Textures/Texture.h>
#include <RenderPass/RenderPass.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <SceneObjectDef.h>
#include <Application.h>

extern std::unique_ptr<Renderer> renderer;

#if defined(_EDITOR)
namespace Editor
{
	extern void SelectCamera(JUUID camera);
	extern JUUID CreateBillboardFromMaterials(CameraUUID camera, std::string name, std::string material, std::string pickingMaterial);
	extern void RegisterBillboard(JUUID sceneObject);
	extern JUUID GetBillboard(JUUID sceneObject);
	extern void DestroyBillboard(JUUID sceneObject);
}
#endif
namespace Scene
{

	SODEF_FULL(Camera);

#include <TrackUUID/JDef.h>
#include <CameraAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <CameraAtt.h>
#include <JEnd.h>

#endif

	void DestroyCameras()
	{
		auto uuids = nostd::GetUUIDS<CameraUUID>(CamerasceneObjects);
		for (auto cam : uuids)
		{
			if (cam->shadowMapLight().empty())
			{
				DeleteCameraSceneObject(cam());
			}
		}
#include <TrackUUID/JClear.h>
#include <CameraAtt.h>
#include <JEnd.h>
	}

	void DeleteCamera(std::string uuid)
	{
		CameraUUID cam = uuid;
#if defined(_EDITOR)
		Editor::DestroyBillboard(uuid);
#endif
		cam->markedForDelete = true;
	}

	void CamerasStep()
	{
		auto& Cameras = GetCameras();
		std::set<CameraUUID> cams;
		std::transform(Cameras.begin(), Cameras.end(), std::inserter(cams, cams.begin()), [](auto o) { return o; });

		//we construct the set of cameras with dirty render passes
		std::set<CameraUUID> camsRpi;
		std::copy_if(cams.begin(), cams.end(), std::inserter(camsRpi, camsRpi.begin()), [](auto cam)
			{
				return cam->dirty(Camera::Update_renderPasses);
			}
		);

		//build a set of cameras for which ibl settings changed
		std::set<CameraUUID> camsIBL;
		std::copy_if(cams.begin(), cams.end(), std::inserter(camsIBL, camsIBL.begin()), [](auto cam)
			{
				return cam->dirty(Camera::Update_IBLIrradiance) || cam->dirty(Camera::Update_IBLPreFilteredEnvironment) ||
					cam->dirty(Camera::Update_IBLBRDFLUT);

			}
		);

		//go through all cameras checking updates of the swapchain
		std::set<CameraUUID> allCams(Cameras.begin(), Cameras.end());
		for (auto cam : allCams)
		{
			//as a special case update the billboard
#if defined(_EDITOR)
			if (cam() != *GetMouseCameras().begin() && cam->shadowMapLight().empty())
			{
				JUUID bbuuid = Editor::GetBillboard(cam());
				if (!bbuuid.empty())
				{
					cam->UpdateBillboard(bbuuid);
				}

			}
#endif
			if (!cam->dirty(Camera::Update_useSwapChain)) continue;
			cam->clean(Camera::Update_useSwapChain);
			if (cam->useSwapChain())
			{
				InsertCameraIntoSwapChainCameras(cam());
			}
			else
			{
				EraseCameraFromSwapChainCameras(cam());
			}
			//as swapchain changed add this camera to the update renderpass set
			camsRpi.insert(cam);
		}

		//do the same for the camera controllers cameras
		for (auto cam : allCams)
		{
			if (!cam->dirty(Camera::Update_mouseController)) continue;
			cam->clean(Camera::Update_mouseController);

			if (cam->mouseController())
			{
				InsertCameraIntoMouseCameras(cam());
			}
			else
			{
				EraseCameraFromMouseCameras(cam());
			}
		}

		//update projection attributes
		for (auto cam : allCams)
		{
			if (
				!cam->dirty(Camera::Update_projectionType) &&
				!cam->dirty(Camera::Update_perspective) &&
				!cam->dirty(Camera::Update_orthographic) &&
				!cam->dirty(Camera::Update_fitWindow)
				) continue;
			cam->clean(Camera::Update_projectionType);
			cam->clean(Camera::Update_perspective);
			cam->clean(Camera::Update_orthographic);
			cam->clean(Camera::Update_fitWindow);
			cam->UpdateProjection();
		}

		//rebuild ibl attributes if needed
		if (camsIBL.size() > 0ULL)
		{
			renderer->Flush();
			renderer->RenderCriticalFrame([&camsIBL]
				{
					for (auto cam : camsIBL)
					{
						std::set<RenderableUUID> renderables(cam->renderables.begin(), cam->renderables.end());

						for (auto r : renderables)
						{
							cam->UnbindRenderable(r());
						}

						cam->DestroyIBLTextures();
						cam->DestroyRenderPasses();
						DestroyConstantsBuffer(cam->cameraCb());

						cam->CreateConstantsBuffer();
						cam->CreateRenderPasses();

						for (auto r : renderables)
						{
							cam->BindRenderable(r());
						}

						cam->clean(Camera::Update_IBLIrradiance);
						cam->clean(Camera::Update_IBLPreFilteredEnvironment);
						cam->clean(Camera::Update_IBLBRDFLUT);
					}
				});
		}

		std::set<CameraUUID> delCams;
		std::copy_if(cams.begin(), cams.end(), std::inserter(delCams, delCams.begin()), [](auto c) {return c->markedForDelete; });

		if (camsRpi.size() > 0ULL || delCams.size() > 0ULL)
		{
			renderer->Flush();
			renderer->RenderCriticalFrame([&camsRpi, &delCams]
				{
					for (auto c : camsRpi)
					{
						c->clean(Camera::Update_renderPasses);

						for (auto rpi : c->renderPassesUUID)
						{
							if (rpi->renderCallbackOverride == RenderPassRenderCallbackOverride_Resolve)
							{
								EraseCameraFromSwapChainCameras(c());
								break;
							}
						}

						std::set<RenderableUUID> renderables(c->renderables.begin(), c->renderables.end());

						for (auto renderable : renderables)
						{
							UnbindFromScene(c->Juuid(), renderable->Juuid());
						}
						c->DestroyRenderPasses();

						c->CreateRenderPasses();
						for (auto renderable : renderables)
						{
							BindToScene(c->Juuid(), renderable->Juuid());
						}

						for (auto rpi : c->renderPassesUUID)
						{
							if (rpi->renderCallbackOverride == RenderPassRenderCallbackOverride_Resolve)
							{
								InsertCameraIntoSwapChainCameras(c());
								break;
							}
						}
					}

					for (auto c : delCams)
					{
						EraseCameraFromCameras(c());
						EraseCameraFromWindowCameras(c());
						EraseCameraFromSwapChainCameras(c());
						EraseCameraFromMouseCameras(c());
						DeleteCameraSceneObject(c());
					}
				}
			);
		}
	}

#if defined(_EDITOR)
	void WriteCamerasJson(nlohmann::json& json)
	{
#include <Editor/JSaveFile.h>
#include <CameraAtt.h>
#include <JEnd.h>

	}
#endif

	Camera::Camera(nlohmann::json& json) :SceneObject(json)
	{
#include <Attributes/JInit.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <CameraAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void Camera::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <CameraAtt.h>
#include <JEnd.h>
	}
#endif

	void Camera::Initialize()
	{
#include <TrackUUID/JInsert.h>
#include <CameraAtt.h>
#include <JEnd.h>

		UpdateProjection();
		CreateConstantsBuffer();
		CreateRenderPasses();
#if defined(_EDITOR)
		if (GetCountFromMouseCameras() > 0 && uuid() != *GetMouseCameras().begin() && shadowMapLight().empty())
			Editor::RegisterBillboard(uuid());
#endif
		if (shadowMapLight().empty())
		{
			CreateLightsConstantsBuffer();
			CreateShadowMapsConstantsBuffer();
		}
	}

	XMVECTOR Camera::positionV()
	{
		XMFLOAT3 pos = position();
		return { pos.x,pos.y,pos.z,0.0f };
	}

	XMVECTOR Camera::rotationQ()
	{
		XMFLOAT3 rotV = rotation();
		float roll, pitch, yaw;
		pitch = rotV.x; yaw = rotV.y; roll = rotV.z;
		XMVECTOR rotQ = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
		return rotQ;
	}

	XMVECTOR Camera::forward()
	{
		if (!shadowMapLight().empty())
		{
			auto& lcam = GetLightSceneObject(shadowMapLight());
			if (lcam->lightType() == LT_Point)
			{
				unsigned int i = 0U;
				for (; i < 6U; i++) {
					if (lcam->shadowMapCameras[i] == uuid())
					{
						return Scene::PointLightDirection[i];
					}
				}
			}
		}

		FXMVECTOR dir = { 0.0f, 0.0f, 1.0f,0.0f };
		return XMVector3Normalize(XMVector3Rotate(dir, rotationQ()));
	}

	XMVECTOR Camera::up()
	{
		if (!shadowMapLight().empty())
		{
			auto& lcam = GetLightSceneObject(shadowMapLight());
			if (lcam->lightType() == LT_Point)
			{
				unsigned int i = 0U;
				for (; i < 6U; i++)
				{
					if (lcam->shadowMapCameras[i] == uuid())
					{
						return Scene::PointLightUp[i];
					}
				}
			}
		}

		FXMVECTOR up = { 0.0f, 1.0f, 0.0f,0.0f };
		return XMVector3Normalize(XMVector3Rotate(up, rotationQ()));
	}

	XMVECTOR Camera::right()
	{
		return XMVector3Cross(forward(), up());
	}

	XMMATRIX Camera::world()
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

	XMMATRIX Camera::view()
	{
		return XMMatrixLookToLH(positionV(), forward(), up());
	}

	XMMATRIX Camera::projection()
	{
		return (projectionType() == PROJ_Orthographic) ? orthographicProjection.projectionMatrix : perspectiveProjection.projectionMatrix;
	}

	float Camera::projectionWidth()
	{
		switch (projectionType())
		{
		case PROJ_Perspective:
		{
			return fitWindow() ? HWNDWIDTHF : perspective().width;
		}
		break;
		default:
			return fitWindow() ? HWNDWIDTHF : orthographic().viewLeft;
			break;
		}
	}

	float Camera::projectionRight()
	{
		return orthographic().viewRight;
	}

	float Camera::projectionBottom()
	{
		return orthographic().viewBottom;
	}

	float Camera::projectionHeight()
	{
		switch (projectionType())
		{
		case PROJ_Perspective:
			return  fitWindow() ? HWNDHEIGHTF : perspective().height;
			break;
		default:
			return  fitWindow() ? HWNDHEIGHTF : orthographic().viewTop;
			break;
		}
	}

	float Camera::projectionNearZ()
	{
		return (projectionType() == PROJ_Perspective) ? perspective().nearZ : orthographic().nearZ;
	}

	float Camera::projectionFarZ()
	{
		return (projectionType() == PROJ_Perspective) ? perspective().farZ : orthographic().farZ;
	}

	float Camera::projectionfovAngleY()
	{
		return (projectionType() == PROJ_Perspective) ? perspective().fovAngleY : 0.0f;
	}

	void Camera::CreateRenderPasses()
	{
		using namespace Templates;

		//do not create the render passes of the camera if one of the renderpasses is broken. as we want to avoid a broken chain of passes
		for (unsigned int i = 0; i < renderPasses().size(); i++)
		{
			JUUID passUUID = renderPasses().at(i);
			if (passUUID.empty()) continue;
			if (!RenderPassTemplateExist(passUUID)) return;
		}

		unsigned int projW = static_cast<unsigned int>(projectionWidth());
		unsigned int projH = static_cast<unsigned int>(projectionHeight());
		for (unsigned int i = 0; i < renderPasses().size(); i++)
		{
			std::string passUUID = renderPasses().at(i);
			if (passUUID == "") continue;
			auto& rp = GetRenderPassTemplate(passUUID);
			if (rp->type() == RenderPassType_SwapChainPass && rp->renderCallbackOverride() != RenderPassRenderCallbackOverride_Resolve) continue;

			renderPassesUUID.push_back(CreateRenderPassInstance(uuid(), passUUID, i, projW, projH));
		}
	}

	void Camera::DestroyRenderPasses()
	{
		for (auto uuid : renderPassesUUID)
		{
			DestroyRenderPassInstance(uuid());
		}
		renderPassesUUID.clear();
	}

	void Camera::ResizeReleasePasses()
	{
		for (auto& pass : renderPassesUUID)
		{
			pass->ResizeRelease();
		}
	}

	void Camera::ResizePasses(unsigned int width, unsigned int height)
	{
		for (auto& pass : renderPassesUUID)
		{
			pass->Resize(width, height);
		}
		switch (projectionType())
		{
		case PROJ_Perspective:
		{
			perspectiveProjection.updateProjectionMatrix(static_cast<float>(width), static_cast<float>(height));
		}
		break;
		case PROJ_Orthographic:
		{
			orthographicProjection.updateProjectionMatrix(static_cast<float>(width), 0.0f, 0.0f, static_cast<float>(height));
		}
		break;
		}
	}

	void Camera::UpdateProjection()
	{
		switch (projectionType())
		{
		case PROJ_Perspective:
		{
			perspectiveProjection = Perspective(projectionNearZ(), projectionFarZ(), projectionfovAngleY(), projectionWidth(), projectionHeight());
			perspectiveProjection.updateProjectionMatrix();
		}
		break;
		case PROJ_Orthographic:
		{
			orthographicProjection = Orthographic(projectionNearZ(), projectionFarZ(), projectionWidth(), projectionRight(), projectionHeight(), projectionBottom());
			orthographicProjection.updateProjectionMatrix();
		}
		break;
		default:
		{
			assert(true); //not implemented
		}
		break;
		}
	}

	void Camera::Bind(JUUID uuid)
	{
		switch (GetSceneObjectType(uuid))
		{
		case SO_Renderables:
		{
			BindRenderable(uuid);
		}
		break;
		case SO_Lights:
		{
			BindLight(uuid);
		}
		break;
		}
	}

	void Camera::Unbind(JUUID uuid)
	{
		if (!SceneObjectExists(uuid)) return;

		switch (GetSceneObjectType(uuid))
		{
		case SO_Renderables:
		{
			UnbindRenderable(uuid);
		}
		break;
		case SO_Lights:
		{
			UnbindLight(uuid);
		}
		break;
		}
		//DEBUGEAR USANDO grid.hlsl como punto de partida para debugear el shadowmap del previewer
	}

	void Camera::BindToScene()
	{
#include <TrackUUID/JInsert.h>
#include <CameraAtt.h>
#include <JEnd.h>
#if defined(_EDITOR)
		SceneObject::BindToScene();
#endif
	}

	void Camera::UnbindFromScene()
	{
#include <TrackUUID/JErase.h>
#include <CameraAtt.h>
#include <JEnd.h>

		Scene::UnbindFromScene(uuid());
		DestroyIBLTextures();
		DestroyRenderPasses();
		DestroyConstantsBuffer(cameraCb());
		if (shadowMapLight().empty())
		{
			DestroyLightsConstantsBuffer();
		}
	}

	void Camera::BindRenderable(RenderableUUID renderable)
	{
		if (renderables.contains(renderable)) return;
		renderables.insert(renderable);

		renderable->CreateMaterialsInstances(uuid());
		renderable->CreateConstantsBuffersInstances(uuid());
		renderable->CreateRootSignatures(uuid());
		renderable->CreatePipelineStates(uuid());
	}

	void Camera::UnbindRenderable(RenderableUUID renderable)
	{
		if (!renderables.contains(renderable)) return;
		renderables.erase(renderable);

		renderable->DestroyMaterialsInstances(uuid());
		renderable->DestroyConstantsBuffersInstances(uuid());
		renderable->DestroyRootSignatures(uuid());
		renderable->DestroyPipelineStates(uuid());
	}

	void Camera::BindLight(LightUUID light)
	{
		lights.push_back(light);
		if (light->hasShadowMaps())
		{
			lightsWithShadowMaps.insert(light);
		}
	}

	void Camera::UnbindLight(LightUUID light)
	{
		nostd::vector_erase(lights, light);
		if (lightsWithShadowMaps.contains(light))
		{
			lightsWithShadowMaps.erase(light);
		}
	}

	bool Camera::ResolvesToSwapChain()
	{
		using namespace Templates;
		if (useSwapChain()) return true;
		for (auto i = 0; i < renderPasses().size(); i++)
		{
			RenderPassJsonUUID pass = renderPasses().at(i);
			if (pass.empty()) continue;

			if (pass->type() == RenderPassType_SwapChainPass) return true;
			if (pass->renderCallbackOverride() == RenderPassRenderCallbackOverride_Resolve) return true;
		}
		return false;
	}

	void Camera::Render()
	{
		WriteConstantsBuffer(renderer->backBufferIndex);
		CalculateBoundingFrustum();

		//first make a set of objects which are not meant to be rendered first
		std::set<RenderableUUID> nonRoot;
		for (auto r : renderables)
		{
			std::vector<JUUID> uuids = r->renderNext();
			for (std::string& uuid : uuids)
			{
				if (!SceneObjectExists(uuid)) continue;
				nonRoot.insert(uuid);
			}
		}

		//create the renderable set recursivelly
		nostd::VectorSet<RenderableUUID> renVecSet;
		std::function<void(RenderableUUID)> addToRenderablesVecSet;
		addToRenderablesVecSet = [&addToRenderablesVecSet, &renVecSet](RenderableUUID r)
			{
				renVecSet.insert(r);
				for (auto& uuid : r->renderNext())
				{
					if (!SceneObjectExists(uuid)) continue;
					addToRenderablesVecSet(uuid);
				}
			};

		//add the objects to the vecset only if are root objects
		for (auto r : renderables)
		{
			if (nonRoot.contains(r())) continue;
			addToRenderablesVecSet(r());
		}

		auto draw = [this, &renVecSet](auto& rpi)
			{
				for (auto it = renVecSet.begin(); it != renVecSet.end(); it++)
				{
					RenderableUUID renderable = *it;
					if (boundingFrustum.Contains(renderable->GetBoundingBox()) == ContainmentType::DISJOINT)
						continue;
					renderable->Render(rpi, uuid());
				}
			};

		std::vector<RenderPassInstanceUUID> rpiv = renderPassesUUID;
		if (useSwapChain())
		{
			rpiv.push_back(renderer->swapChainPass);
		}

		for (auto& rp : rpiv)
		{
			rp->Pass([&rp, draw]() {draw(rp); });
		}
	}

	void Camera::CalculateBoundingFrustum()
	{
		BoundingFrustum(projection()).Transform(boundingFrustum, world());
	}

	void Camera::Destroy()
	{
#include <Attributes/JDestroy.h>
#include <CameraAtt.h>
#include <JEnd.h>
	}

	void Camera::CreateConstantsBuffer()
	{
		cameraCb = DeviceUtils::CreateConstantsBuffer(sizeof(CameraAttributes) * renderer->numFrames, name());
	}

	void Camera::WriteConstantsBuffer(unsigned int backbufferIndex)
	{
		CameraAttributes atts{};

		atts.view = view();
		atts.viewProjection = XMMatrixMultiply(view(), projection());
		XMStoreFloat4(&atts.eyePosition, positionV());
		XMStoreFloat4(&atts.eyeForward, forward());
		XMStoreFloat4(&atts.eyeUp, up());
		XMStoreFloat4(&atts.eyeRight, right());
		atts.white = white();
		atts.widthHeight.x = projectionWidth();
		atts.widthHeight.y = projectionHeight();

		if (iblTextures.contains(TextureShaderUsage_IBLPreFilteredEnvironment))
		{
			TextureJsonUUID tex = iblTextures.at(TextureShaderUsage_IBLPreFilteredEnvironment);
			atts.IBLNumEnvLevels = static_cast<float>(tex->mipLevels());
		}
		else
		{
			atts.IBLNumEnvLevels = 0.0f;
		}

		cameraCb->push(atts, backbufferIndex);
	}

	void Camera::ProcessKeyboardInput(DirectX::Keyboard::KeyboardStateTracker& tracker, DirectX::Keyboard::State& state)
	{
		if (!SceneObjectExists(shadowMapLight()))
			return;

		LightUUID lcam = shadowMapLight();

		if (lcam->lightType() == LT_Spot || lcam->lightType() == LT_Point)
		{
			float moveSpeed = speed() * ((state.LeftShift || state.RightShift) ? 10.0f : 1.0f);
			if (state.Up) { MoveForward(moveSpeed); }
			if (state.Down) { MoveBack(moveSpeed); }
			if (state.Left) { MoveLeft(moveSpeed); }
			if (state.Right) { MoveRight(moveSpeed); }
		}
	}

	void Camera::MoveAlongFwAxis(float dz)
	{
		XMVECTOR newPos = positionV() + forward() * dz;
		position(*(XMFLOAT3*)newPos.m128_f32);
		UpdateLightPosition();
	}

	void Camera::MovePerpendicularFwAxis(float dx, float dy)
	{
		XMVECTOR newPos = positionV() + up() * dy + right() * dx;
		position(*(XMFLOAT3*)newPos.m128_f32);
	}

	void Camera::Rotate(float dx, float dy)
	{
		rotation(rotation() + XMFLOAT3{ dy, dx, 0.0f });
		UdateLightRotation();
	}

	void Camera::ProcessGamepadInput(DirectX::GamePad::State& gamePadState, DirectX::SimpleMath::Vector2 gamePadCameraRotationSensitivity)
	{
		if (!SceneObjectExists(shadowMapLight()))
			return;

		LightUUID lcam = shadowMapLight();

		if (lcam->lightType() == LT_Spot || lcam->lightType() == LT_Point)
		{
			if (gamePadState.thumbSticks.leftY > 0) { MoveForward(speed()); }
			if (gamePadState.thumbSticks.leftY < 0) { MoveBack(speed()); }
			if (gamePadState.thumbSticks.leftX < 0) { MoveLeft(speed()); }
			if (gamePadState.thumbSticks.leftX > 0) { MoveRight(speed()); }
		}
		if (lcam->lightType() == LT_Directional || lcam->lightType() == LT_Spot)
		{
			Vector2 stickDiff = { gamePadState.thumbSticks.rightX, gamePadState.thumbSticks.rightY };
			rotation(rotation() - XMFLOAT3{ stickDiff.x * gamePadCameraRotationSensitivity.x, stickDiff.y * gamePadCameraRotationSensitivity.y, 0.0f });
			UdateLightRotation();
		}
	}

	XMFLOAT2 lastMousePos;
	XMFLOAT2 GetMouseDiff(DirectX::Mouse::State& mouseState)
	{
		XMFLOAT2 currentMousePos = { static_cast<float>(mouseState.x) , static_cast<float>(mouseState.y) };
		XMFLOAT2 diff = { 0.0f, 0.0f };
		if (mouseState.leftButton)
		{
			diff = { currentMousePos.x - lastMousePos.x , currentMousePos.y - lastMousePos.y };
		}
		lastMousePos = currentMousePos;
		return diff;
	}

	int lastWheelValue = 0;
	float wheelDiffFactor = 0.0001f;
	void Camera::ProcessCameraMouseRotation(DirectX::Mouse::State& mouseState, DirectX::SimpleMath::Vector2 rotationSensitivity, bool firstStep)
	{
		if (!SceneObjectExists(shadowMapLight()))
			return;

		LightUUID lcam = shadowMapLight();

		if (lcam->lightType() != LT_Directional && lcam->lightType() != LT_Spot)
			return;

		Vector2 mouseDiff = GetMouseDiff(mouseState);
		mouseDiff = firstStep ? Vector2(0.0f, 0.0f) : mouseDiff;
		rotation(rotation() - XMFLOAT3{ mouseDiff.x * rotationSensitivity.x, mouseDiff.y * rotationSensitivity.y, 0.0f });
		UdateLightRotation();
		if (lcam->lightType() == LT_Directional)
		{
			float diff = static_cast<float>(mouseState.scrollWheelValue - lastWheelValue) * wheelDiffFactor;
			orthographicProjection.expandView(diff);
		}
	}

	void Camera::UpdateLightPosition()
	{
		using namespace Scene;

		if (!SceneObjectExists(shadowMapLight()))
			return;

		LightUUID lcam = shadowMapLight();

		switch (lcam->lightType())
		{
		case LT_Spot:
		{
			lcam->position(position());
		}
		break;
		case LT_Point:
		{
			lcam->position(position());
			for (auto cam : lcam->shadowMapCameras)
			{
				cam->position(position());
			}
		}
		break;
		}
	}

	void Camera::UdateLightRotation()
	{
		using namespace Scene;

		if (!SceneObjectExists(shadowMapLight()))
			return;

		LightUUID lcam = shadowMapLight();

		XMFLOAT3 rot = rotation();

		switch (lcam->lightType())
		{
		case LT_Directional:
		{
			lcam->rotation(rot);
			XMVECTOR camPos = XMVectorScale(XMVector3Normalize(forward()), lcam->dirDist());
			position(*(XMFLOAT3*)camPos.m128_f32);
		}
		break;
		case LT_Spot:
		{
			lcam->rotation(rot);
		}
		break;
		}
	}

	void Camera::MoveForward(float step)
	{
		XMVECTOR newPos = XMVectorAdd(positionV(), forward() * step);
		position(*(XMFLOAT3*)newPos.m128_f32);
		UpdateLightPosition();
	}

	void Camera::MoveBack(float step)
	{
		XMVECTOR newPos = XMVectorAdd(positionV(), forward() * -step);
		position(*(XMFLOAT3*)newPos.m128_f32);
		UpdateLightPosition();
	}

	void Camera::MoveLeft(float step)
	{
		XMVECTOR newPos = XMVectorAdd(positionV(), XMVector3Cross(forward(), up()) * -step);
		position(*(XMFLOAT3*)newPos.m128_f32);
		UpdateLightPosition();
	}

	void Camera::MoveRight(float step)
	{
		XMVECTOR newPos = XMVectorAdd(positionV(), XMVector3Cross(forward(), up()) * step);
		position(*(XMFLOAT3*)newPos.m128_f32);
		UpdateLightPosition();
	}

	//Lights
	void Camera::CreateLightsConstantsBuffer()
	{
		lightsCB = DeviceUtils::CreateConstantsBuffer(sizeof(LightPool), std::string(name() + "-lightsCbv"));
	}

	void Camera::DestroyLightsConstantsBuffer()
	{
		DestroyConstantsBuffer(lightsCB());
		lightsCB.clear();
	}

	void Camera::WriteLightsConstantsBuffer()
	{
		size_t offset = lightsCB->alignedConstantBufferSize * renderer->backBufferIndex;
		LightPool* lpool = (LightPool*)(lightsCB->mappedConstantBuffer + offset);
		unsigned int numLights = static_cast<unsigned int>(lights.size());
		lpool->numLights = numLights;
		for (unsigned int i = 0; i < numLights; i++)
		{
			lights[i]->WriteConstantsBufferLightAttributes(lpool->lights[i]);
		}
	}

	//ShadowMaps
	void Camera::CreateShadowMapsConstantsBuffer()
	{
		shadowMapsCB = DeviceUtils::CreateConstantsBuffer(sizeof(ShadowMapAttributes) * MaxLights, std::string(name() + "-shadowMapsCbv"));
	}

	void Camera::DestroyShadowMapsConstantsBuffer()
	{
		DestroyConstantsBuffer(shadowMapsCB());
		shadowMapsCB.clear();
	}

	void Camera::WriteShadowMapsConstantsBuffer()
	{
		for (LightUUID light : lights)
		{
			for (CameraUUID cam : light->shadowMapCameras)
			{
				cam->WriteConstantsBuffer(renderer->backBufferIndex);
			}
		}

		size_t offset = shadowMapsCB->alignedConstantBufferSize * renderer->backBufferIndex;
		ShadowMapAttributes* atts = (ShadowMapAttributes*)(shadowMapsCB->mappedConstantBuffer + offset);
		for (LightUUID light : lights)
		{
			if (!lightsWithShadowMaps.contains(light))
				continue;

			light->WriteConstantsBufferShadowMapAttributes(*atts);
			atts++;
		}
	}

	//IBL
	bool Camera::HasIBL()
	{
		return !IBLIrradiance().empty() && !IBLPreFilteredEnvironment().empty() && !IBLBRDFLUT().empty();
	}

	void Camera::CreateIBLTextures()
	{
		iblTextures.insert_or_assign(TextureShaderUsage_IBLIrradiance, IBLIrradiance());
		iblTextures.insert_or_assign(TextureShaderUsage_IBLPreFilteredEnvironment, IBLPreFilteredEnvironment());
		iblTextures.insert_or_assign(TextureShaderUsage_IBLBRDFLUT, IBLBRDFLUT());
	}

	void Camera::DestroyIBLTextures()
	{
		for (auto& [_, t] : iblTextures)
		{
			DeleteTextureInstance(t);
		}
		iblTextures.clear();
	}

	void Camera::SetIBLRootDescriptorTables(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& cbvSlot)
	{
		commandList->SetGraphicsRootDescriptorTable(cbvSlot++, GetTextureInstance(iblTextures.at(TextureShaderUsage_IBLIrradiance))->gpuHandle);
		commandList->SetGraphicsRootDescriptorTable(cbvSlot++, GetTextureInstance(iblTextures.at(TextureShaderUsage_IBLPreFilteredEnvironment))->gpuHandle);
		commandList->SetGraphicsRootDescriptorTable(cbvSlot++, GetTextureInstance(iblTextures.at(TextureShaderUsage_IBLBRDFLUT))->gpuHandle);
	}

#if defined(_EDITOR)
	void Camera::EditorPreview(size_t flags)
	{
		previewRenderPassIndex = 0U;
		previewRenderToTextureIndex = 0U;
	}

	void Camera::DestroyEditorPreview()
	{
	}

	JUUID Camera::CreateBillboard(CameraUUID camera)
	{
		JUUID uuid = Editor::CreateBillboardFromMaterials(camera, at("name"), "Camera", "CameraPicking");
		RenderableUUID bb = uuid;
		bb->OnPick = [this] {Editor::SelectCamera(this->uuid()); };
		UpdateBillboard(uuid);
		return uuid;
	}

	void Camera::UpdateBillboard(JUUID uuid)
	{
		assert(!uuid.empty());
		if (uuid.empty()) return;

		XMFLOAT3 baseColor = { 1.0f,1.0f,1.0f };
		RenderableUUID bb = uuid;
		bb->position(position());
		bb->WriteConstantsBuffer<XMFLOAT3>("baseColor", baseColor, renderer->backBufferIndex);
		bb->WriteConstantsBuffer();
	}

	BoundingBox Camera::GetBoundingBox()
	{
		return BoundingBox(position(), { 0.1f,0.1f,0.1f });
	}

#endif
}