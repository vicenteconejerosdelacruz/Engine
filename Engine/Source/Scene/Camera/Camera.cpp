#include "pch.h"
#include "Camera.h"
#include <Application.h>
#include <Templates.h>
#include <Scene.h>
//#include <Light/Light.h>
//#include <Renderable/Renderable.h>
#include <Renderer.h>
//#include <Textures/Texture.h>
//#include <RenderPass/RenderPass.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
//#include <SceneObjectDef.h>

extern std::unique_ptr<Renderer> renderer;

#if defined(_EDITOR)
namespace Editor
{
	extern void SelectCamera(SceneUnitId id, JUUID camera);
	extern JUUID CreateBillboardFromMaterials(SceneUnitId id, CameraSUUUID camera, std::string name, std::string material, std::string pickingMaterial);
	extern void RegisterBillboard(SceneUnitId id, JUUID sceneObject);
	//extern JUUID GetBillboard(JUUID sceneObject);
	extern void DestroyBillboard(SceneUnitId id, JUUID sceneObject);
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

	Camera::Camera(SceneUnitId id, nlohmann::json& json) :SceneObject(id, json)
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
#if defined(_EDITOR)
		using namespace Editor;
#endif

#include <TrackUUID/JInsert.h>
#include <CameraAtt.h>
#include <JEnd.h>

		UpdateProjection();
		CreateConstantsBuffer();
		CreateRenderPasses();
		CreateIBLTextures();

#if defined(_EDITOR)
		//if (GetCountFromMouseCameras(unit) > 0 && uuid() != *GetMouseCameras(unit).begin() && shadowMapLight().empty())
		if (shadowMapLight().empty() && !SceneIsIsolated(unit))
			RegisterBillboard(unit, uuid());
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
			auto& lcam = GetLightSUSceneObject(unit, shadowMapLight());
			if (lcam->lightType() == LT_Point)
			{
				unsigned int i = 0U;
				for (; i < 6U; i++) {
					if (lcam->shadowMapCameras[i] == SUuuid())
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
			auto& lcam = GetLightSUSceneObject(unit, shadowMapLight());
			if (lcam->lightType() == LT_Point)
			{
				unsigned int i = 0U;
				for (; i < 6U; i++)
				{
					if (lcam->shadowMapCameras[i] == SUuuid())
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

		//unsigned int projW = static_cast<unsigned int>(projectionWidth());
		//unsigned int projH = static_cast<unsigned int>(projectionHeight());
		for (unsigned int i = 0; i < renderPasses().size(); i++)
		{
			JUUID passUUID = renderPasses().at(i);
			if (passUUID.empty()) continue;
			auto& rp = GetRenderPassTemplate(passUUID);
			if (rp->type() == RenderPassType_SwapChainPass && rp->renderCallbackOverride() != RenderPassRenderCallbackOverride_Resolve) continue;

			//renderPassesUUID.push_back(CreateRenderPassInstance(unit, uuid(), passUUID, static_cast<unsigned int>(renderPassesUUID.size()), projW, projH));
			renderPassesUUID.push_back(CreateRenderPass(passUUID, static_cast<unsigned int>(renderPassesUUID.size())));
		}
	}

	RenderPassJsonUUID Camera::GetRenderPassTemplateFromInstanceIndex(unsigned int passIndex)
	{
		return renderPassesUUID.at(passIndex)->renderPassTemplate;
	}

	RenderPassInstanceUUID Camera::CreateRenderPass(JUUID passUUID, unsigned int passIndex)
	{
		unsigned int projW = static_cast<unsigned int>(projectionWidth());
		unsigned int projH = static_cast<unsigned int>(projectionHeight());
		return CreateRenderPassInstance(unit, uuid(), passUUID, passIndex, projW, projH);
	}

	void Camera::CreateRenderPassAtIndex(JUUID passUUID, unsigned int passIndex)
	{
		renderPassesUUID.insert(renderPassesUUID.begin() + passIndex, CreateRenderPass(passUUID, passIndex));
		RearrangeRenderPassesAfter(passIndex);
	}

	void Camera::DeleteRenderPassAtIndex(unsigned int passIndex)
	{
		renderPassesUUID.at(passIndex)->MarkForDelete();
		renderPassesUUID.erase(renderPassesUUID.begin() + passIndex);
		RearrangeRenderPassesAfter(passIndex);
	}

	void Camera::SwapRenderPassAtIndex(JUUID passUUID, unsigned int passIndex)
	{
		renderPassesUUID.at(passIndex)->MarkForDelete();
		renderPassesUUID[passIndex] = CreateRenderPass(passUUID, passIndex);
		RearrangeRenderPassesAfter(passIndex);
	}

	void Camera::RearrangeRenderPassesAfter(unsigned int passIndex)
	{
		for (unsigned int i = passIndex; i < renderPassesUUID.size(); i++)
		{
			renderPassesUUID.at(i)->renderPassIndex = i;
			if (renderPassesUUID.at(i)->overridePass)
			{
				auto& opass = renderPassesUUID.at(i)->overridePass;
				opass->renderPassIndex = i;
				opass->CreatePrevPassDependentResources();
			}
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

	void Camera::BindToScene()
	{
#include <TrackUUID/JInsert.h>
#include <CameraAtt.h>
#include <JEnd.h>
#if defined(_EDITOR)
		SceneObject::BindToScene();
#endif
	}

	void Camera::Bind(JUUID uuid)
	{
		switch (GetSceneObjectType(unit, uuid))
		{
		case SO_Renderables:
		{
			BindRenderable(MAKESUUUID(unit, uuid));
		}
		break;
		case SO_Lights:
		{
			BindLight(MAKESUUUID(unit, uuid));
		}
		break;
		}
	}

	void Camera::BindRenderable(RenderableSUUUID renderable)
	{
		if (renderables.contains(renderable)) return;
		renderables.insert(renderable);

		renderable->CreateMaterialsInstances(SUuuid());
		renderable->CreateConstantsBuffersInstances(SUuuid());
		renderable->CreateRootSignatures(SUuuid());
		renderable->CreatePipelineStates(SUuuid());
	}

	void Camera::BindLight(LightSUUUID light)
	{
		lights.push_back(light);
		BindLightWithShadowMap(light);
	}

	void Camera::BindLightWithShadowMap(LightSUUUID light)
	{
		if (light->hasShadowMaps())
		{
			lightsWithShadowMaps.insert(light);
		}
	}

	void Camera::UnbindFromScene()
	{
#include <TrackUUID/JErase.h>
#include <CameraAtt.h>
#include <JEnd.h>

		Scene::UnbindFromScene(unit, uuid());
		//DestroyIBLTextures();
		//DestroyRenderPasses();
		DestroyConstantsBuffer(cameraCb());
		if (shadowMapLight().empty())
		{
			//DestroyLightsConstantsBuffer();
		}
	}

	void Camera::Unbind(JUUID uuid)
	{
		if (!SceneObjectExists(unit, uuid)) return;

		switch (GetSceneObjectType(unit, uuid))
		{
		case SO_Renderables:
		{
			UnbindRenderable(MAKESUUUID(unit, uuid));
		}
		break;
		case SO_Lights:
		{
			UnbindLight(MAKESUUUID(unit, uuid));
		}
		break;
		}
		//DEBUGEAR USANDO grid.hlsl como punto de partida para debugear el shadowmap del previewer
	}

	void Camera::UnbindRenderable(RenderableSUUUID renderable)
	{
		if (!renderables.contains(renderable)) return;
		renderables.erase(renderable);

		renderable->DestroyMaterialsInstances(SUuuid());
		renderable->DestroyConstantsBuffersInstances(SUuuid());
		renderable->DestroyRootSignatures(SUuuid());
		renderable->DestroyPipelineStates(SUuuid());
	}

	void Camera::UnbindLight(LightSUUUID light)
	{
		nostd::vector_erase(lights, light);
		UnbindLightWithShadowMap(light);
	}

	void Camera::UnbindLightWithShadowMap(LightSUUUID light)
	{
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

	bool Camera::RenderReady()
	{
		return renderReady;
	}

	void Camera::RenderReady(bool value)
	{
		renderReady = value;
	}

	void Camera::Render()
	{
		unsigned int frame = GetSceneUnit(unit)->Frame();
		WriteConstantsBuffer(frame);
		CalculateBoundingFrustum();

		//first make a set of objects which are not meant to be rendered first
		std::set<RenderableSUUUID> nonRoot;
		for (auto r : renderables)
		{
			std::vector<JUUID> uuids = r->renderNext();
			for (std::string& uuid : uuids)
			{
				if (!SceneObjectExists(unit, uuid)) continue;
				nonRoot.insert(MAKESUUUID(unit, uuid));
			}
		}

		//create the renderable set recursivelly
		nostd::VectorSet<RenderableSUUUID> renVecSet;
		std::function<void(RenderableSUUUID)> addToRenderablesVecSet;
		addToRenderablesVecSet = [&](RenderableSUUUID r)
			{
				renVecSet.insert(r);
				for (auto& uuid : r->renderNext())
				{
					if (!SceneObjectExists(unit, uuid)) continue;
					addToRenderablesVecSet(MAKESUUUID(unit, uuid));
				}
			};

		//add the objects to the vecset only if are root objects
		for (auto r : renderables)
		{
			if (nonRoot.contains(r())) continue;
			addToRenderablesVecSet(r());
		}

		auto draw = [&](SceneUnitId unit, auto& rpi)
			{
				for (auto it = renVecSet.begin(); it != renVecSet.end(); it++)
				{
					RenderableSUUUID renderable = *it;
					if (renderable->checkBoundingBox() && boundingFrustum.Contains(renderable->GetBoundingBox()) == ContainmentType::DISJOINT)
						continue;
					renderable->Render(unit, rpi, SUuuid());
				}
			};

		std::vector<RenderPassInstanceUUID> rpiv = renderPassesUUID;
		if (useSwapChain())
		{
			rpiv.push_back(renderer->swapChainPass);
		}

		for (auto& rp : rpiv)
		{
			rp->Pass(unit, [&rp, draw](SceneUnitId unit) { draw(unit, rp); });
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
		cameraCb = DeviceUtils::CreateConstantsBuffer(sizeof(CameraAttributes), Renderer::numFrames, name());
	}

	void Camera::WriteConstantsBuffer(unsigned int frame)
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

		cameraCb->push(atts, frame);
	}

	/*void Camera::ProcessKeyboardInput(DirectX::Keyboard::KeyboardStateTracker& tracker, DirectX::Keyboard::State& state)
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
	}*/

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

	/*void Camera::ProcessGamepadInput(DirectX::GamePad::State& gamePadState, DirectX::SimpleMath::Vector2 gamePadCameraRotationSensitivity)
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
	}*/

	XMFLOAT2 lastMousePos;
	/*XMFLOAT2 GetMouseDiff(DirectX::Mouse::State& mouseState)
	{
		XMFLOAT2 currentMousePos = { static_cast<float>(mouseState.x) , static_cast<float>(mouseState.y) };
		XMFLOAT2 diff = { 0.0f, 0.0f };
		if (mouseState.leftButton)
		{
			diff = { currentMousePos.x - lastMousePos.x , currentMousePos.y - lastMousePos.y };
		}
		lastMousePos = currentMousePos;
		return diff;
	}*/

	int lastWheelValue = 0;
	float wheelDiffFactor = 0.0001f;
	/*void Camera::ProcessCameraMouseRotation(DirectX::Mouse::State& mouseState, DirectX::SimpleMath::Vector2 rotationSensitivity, bool firstStep)
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
	}*/

	void Camera::UpdateLightPosition()
	{
		using namespace Scene;

		if (!SceneObjectExists(unit, shadowMapLight()))
			return;

		LightSUUUID lcam = MAKESUUUID(unit, shadowMapLight());

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

		if (!SceneObjectExists(unit, shadowMapLight()))
			return;

		LightSUUUID lcam = MAKESUUUID(unit, shadowMapLight());

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
		lightsCB = DeviceUtils::CreateConstantsBuffer(sizeof(LightPool), Renderer::numFrames, std::string(name() + "-lightsCbv"));
	}

	void Camera::DestroyLightsConstantsBuffer()
	{
		DestroyConstantsBuffer(lightsCB());
		lightsCB.clear();
	}

	void Camera::WriteLightsConstantsBuffer(unsigned int frame)
	{
		size_t offset = lightsCB->alignedConstantBufferSize * frame;
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
		shadowMapsCB = DeviceUtils::CreateConstantsBuffer(sizeof(ShadowMapAttributes) * MaxLights, Renderer::numFrames, std::string(name() + "-shadowMapsCbv"));
	}

	void Camera::DestroyShadowMapsConstantsBuffer()
	{
		DestroyConstantsBuffer(shadowMapsCB());
		shadowMapsCB.clear();
	}

	void Camera::WriteShadowMapsConstantsBuffer(unsigned int frame)
	{
		for (LightSUUUID light : lights)
		{
			for (CameraSUUUID cam : light->shadowMapCameras)
			{
				cam->WriteConstantsBuffer(frame);
			}
		}

		size_t offset = shadowMapsCB->alignedConstantBufferSize * frame;
		ShadowMapAttributes* atts = (ShadowMapAttributes*)(shadowMapsCB->mappedConstantBuffer + offset);
		for (LightSUUUID light : lights)
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
		//return !IBLIrradiance().empty() && !IBLPreFilteredEnvironment().empty() && !IBLBRDFLUT().empty();
		return iblTextures.size() == 3ULL;
	}

	void Camera::CreateIBLTextures()
	{
		CreateIBLIrradianceTexture();
		CreateIBLPreFilteredEnvironmentTexture();
		CreateIBLBRDFLUTTexture();
	}

	void Camera::CreateIBLIrradianceTexture()
	{
		using namespace Templates;

		if (iblTextures.contains(TextureShaderUsage_IBLIrradiance))
		{
			DeleteTextureInstance(iblTextures.at(TextureShaderUsage_IBLIrradiance));
			iblTextures.erase(TextureShaderUsage_IBLIrradiance);
		}

		if (IBLIrradiance().empty())
			return;

		CreateTextureInstance(IBLIrradiance(), [&]
			{
				return std::make_unique<TextureInstance>(unit, IBLIrradiance());
			}
		);

		iblTextures.insert_or_assign(TextureShaderUsage_IBLIrradiance, IBLIrradiance());
	}

	void Camera::CreateIBLPreFilteredEnvironmentTexture()
	{
		using namespace Templates;

		if (iblTextures.contains(TextureShaderUsage_IBLPreFilteredEnvironment))
		{
			DeleteTextureInstance(iblTextures.at(TextureShaderUsage_IBLPreFilteredEnvironment));
			iblTextures.erase(TextureShaderUsage_IBLPreFilteredEnvironment);
		}

		if (IBLPreFilteredEnvironment().empty())
			return;

		CreateTextureInstance(IBLPreFilteredEnvironment(), [&]
			{
				return std::make_unique<TextureInstance>(unit, IBLPreFilteredEnvironment());
			}
		);

		iblTextures.insert_or_assign(TextureShaderUsage_IBLPreFilteredEnvironment, IBLPreFilteredEnvironment());
	}

	void Camera::CreateIBLBRDFLUTTexture()
	{
		using namespace Templates;

		if (iblTextures.contains(TextureShaderUsage_IBLBRDFLUT))
		{
			DeleteTextureInstance(iblTextures.at(TextureShaderUsage_IBLBRDFLUT));
			iblTextures.erase(TextureShaderUsage_IBLBRDFLUT);
		}

		if (IBLBRDFLUT().empty())
			return;

		CreateTextureInstance(IBLBRDFLUT(), [&]
			{
				return std::make_unique<TextureInstance>(unit, IBLBRDFLUT());
			}
		);

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

	JUUID Camera::CreateBillboard(CameraSUUUID camera)
	{
		JUUID uuid = Editor::CreateBillboardFromMaterials(unit, camera, at("name"), "Camera", "CameraPicking");
		RenderableSUUUID bb = MAKESUUUID(unit, uuid);
		bb->OnPick = [this] {Editor::SelectCamera(unit, this->uuid()); };
		UpdateBillboard(uuid);
		return uuid;
	}

	void Camera::UpdateBillboard(JUUID uuid)
	{
		assert(!uuid.empty());
		if (uuid.empty()) return;

		auto& scene = GetSceneUnit(unit);

		XMFLOAT3 baseColor = { 1.0f,1.0f,1.0f };
		RenderableSUUUID bb = MAKESUUUID(unit, uuid);
		bb->position(position());
		bb->WriteConstantsBuffer<XMFLOAT3>("baseColor", baseColor, scene->Frame());
		bb->WriteConstantsBuffer(scene->Frame());
	}

	BoundingBox Camera::GetBoundingBox()
	{
		return BoundingBox(position(), { 0.1f,0.1f,0.1f });
	}

#endif

	void CamerasStep(SceneUnitId id)
	{
		//auto Cameras = nostd::GetUUIDS(CamerasceneObjects);
		auto& Cameras = GetCameras(id);
		std::set<CameraSUUUID> cams;
		std::transform(Cameras.begin(), Cameras.end(), std::inserter(cams, cams.begin()), [&](auto o) { return MAKESUUUID(id, o); });

		//we construct the set of cameras with dirty render passes
		std::set<CameraSUUUID> dirtyPassesCams;
		std::copy_if(cams.begin(), cams.end(), std::inserter(dirtyPassesCams, dirtyPassesCams.begin()), [](auto cam)
			{
				return cam->dirty(Camera::Update_renderPasses);
			}
		);

		for (auto& cam : dirtyPassesCams)
		{
			std::map<RenderPassJsonUUID, std::tuple<int, int>> passes;

			//prev pass fill
			for (unsigned int i = 0; i < cam->UpdatePrevValues["renderPasses"].size(); i++)
			{
				RenderPassJsonUUID pass = JUUID(cam->UpdatePrevValues["renderPasses"].at(i));
				if (pass.empty()) continue;
				passes[pass] = std::make_tuple(-1, -1);
			}
			//curr pass fill
			for (unsigned int i = 0; i < cam->at("renderPasses").size(); i++)
			{
				RenderPassJsonUUID pass = JUUID(cam->at("renderPasses").at(i));
				if (pass.empty()) continue;
				passes[pass] = std::make_tuple(-1, -1);
			}
			//set from indices
			int index = 0;
			for (unsigned int i = 0; i < cam->UpdatePrevValues["renderPasses"].size(); i++)
			{
				RenderPassJsonUUID pass = JUUID(cam->UpdatePrevValues["renderPasses"].at(i));
				if (pass.empty()) continue;
				std::get<0>(passes[pass]) = index;
				index++;
			}
			//set right indices
			index = 0;
			for (unsigned int i = 0; i < cam->at("renderPasses").size(); i++)
			{
				RenderPassJsonUUID pass = JUUID(cam->at("renderPasses").at(i));
				if (pass.empty()) continue;
				std::get<1>(passes[pass]) = index;
				index++;
			}

			//figure out the rearrange
			int deleteElementIndex = -1;
			int addElementIndex = -1;
			RenderPassJsonUUID addElementJsonUUID;
			for (auto& [pass, fromto] : passes)
			{
				auto& [from, to] = fromto;

				if (from == -1)
				{
					addElementIndex = to;
					addElementJsonUUID = pass;
				}

				if (to == -1)
				{
					deleteElementIndex = from;
				}
			}

			//dirty flag but nothing to do, example: adding a new empty slot
			if (deleteElementIndex == -1 && addElementIndex == -1)
			{
				cam->clean(Camera::Update_renderPasses);
				continue;
			}

			//pure delete case
			if (deleteElementIndex != -1 && addElementIndex == -1)
			{
				cam->DeleteRenderPassAtIndex(deleteElementIndex);
				cam->clean(Camera::Update_renderPasses);
				continue;
			}

			//pure add case
			if (addElementIndex != -1 && deleteElementIndex == -1)
			{
				cam->CreateRenderPassAtIndex(addElementJsonUUID(), addElementIndex);
				cam->clean(Camera::Update_renderPasses);
				continue;
			}

			//swap case
			assert(addElementIndex == deleteElementIndex);
			cam->SwapRenderPassAtIndex(addElementJsonUUID(), addElementIndex);
			cam->clean(Camera::Update_renderPasses);
		}

		//now get cameras with dirty ibl
		std::set<CameraSUUUID> dirtyIBL;
		std::copy_if(cams.begin(), cams.end(), std::inserter(dirtyIBL, dirtyIBL.begin()), [](auto cam)
			{
				return cam->dirty(Camera::Update_IBLIrradiance) || cam->dirty(Camera::Update_IBLPreFilteredEnvironment) || cam->dirty(Camera::Update_IBLBRDFLUT);
			}
		);

		std::map<SceneUnitId, std::set<CameraSUUUID>> camerasToRebind;
		for (auto& cam : dirtyIBL)
		{
			size_t prevTexSize = cam->iblTextures.size();
			if (cam->dirty(Camera::Update_IBLIrradiance))
			{
				cam->CreateIBLIrradianceTexture();
				cam->clean(Camera::Update_IBLIrradiance);
			}
			if (cam->dirty(Camera::Update_IBLPreFilteredEnvironment))
			{
				cam->CreateIBLPreFilteredEnvironmentTexture();
				cam->clean(Camera::Update_IBLPreFilteredEnvironment);
			}
			if (cam->dirty(Camera::Update_IBLBRDFLUT))
			{
				cam->CreateIBLBRDFLUTTexture();
				cam->clean(Camera::Update_IBLBRDFLUT);
			}
			size_t currTexSize = cam->iblTextures.size();

			if ((prevTexSize != 3ULL && currTexSize == 3ULL) || (prevTexSize == 3ULL && currTexSize != 3ULL))
			{
				camerasToRebind[cam.unit()].insert(cam);
			}
		}

		auto rebindCam = [](auto cam)
			{
				for (auto& renderable : cam->renderables)
				{
					renderable->DestroyMaterialsInstances(cam);
					renderable->DestroyConstantsBuffersInstances(cam);
					renderable->DestroyRootSignatures(cam);
					renderable->DestroyPipelineStates(cam);
				}
				for (auto& renderable : cam->renderables)
				{
					renderable->CreateMaterialsInstances(cam);
					renderable->CreateConstantsBuffersInstances(cam);
					renderable->CreateRootSignatures(cam);
					renderable->CreatePipelineStates(cam);
				}
			};

		for (auto& [id, cams] : camerasToRebind)
		{
			auto& scene = GetSceneUnit(id);
			scene->ResetLoadingCommandList();
			scene->SetLoading(true);
			scene->SetCanSubmitLoading(false);
			for (auto& cam : cams)
			{
				rebindCam(cam);
			}
			scene->CloseSubmitLoadingCommandList();
		}

		/*
		//build a set of cameras for which ibl settings changed
		std::set<CameraUUID> camsIBL;
		std::copy_if(cams.begin(), cams.end(), std::inserter(camsIBL, camsIBL.begin()), [](auto cam)
			{
				return cam->dirty(Camera::Update_IBLIrradiance) || cam->dirty(Camera::Update_IBLPreFilteredEnvironment) ||
					cam->dirty(Camera::Update_IBLBRDFLUT);

			}
		);
		*/
		/*
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
			*/

		std::set<CameraSUUUID> delCams;
		std::copy_if(cams.begin(), cams.end(), std::inserter(delCams, delCams.begin()), [](auto c) { return c->markedForDelete; });

		/*
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
					*/

		for (auto c : delCams)
		{
			EraseCameraFromCameras(c->unit, c.uuid());
			EraseCameraFromWindowCameras(c->unit, c.uuid());
			EraseCameraFromSwapChainCameras(c->unit, c.uuid());
			EraseCameraFromMouseCameras(c->unit, c.uuid());
			DeleteCameraSUSceneObject(c->unit, c.uuid());
		}
	}

	void DestroyCameras()
	{
		for (auto& [id, container] : CameraSUsceneObjects)
		{
			for (auto& [uuid, _] : container)
			{
				CameraSUUUID cam = MAKESUUUID(id, uuid);
				if (cam->shadowMapLight().empty())
				{
					DeleteCameraSUSceneObject(cam->unit, cam->uuid());
				}
			}
		}
		/*
		auto uuids = nostd::GetUUIDS<CameraUUID>(CamerasceneObjects);
		for (auto cam : uuids)
		{
			if (cam->shadowMapLight().empty())
			{
				DeleteCameraSUSceneObject(cam->unit, cam());
			}
		}
		*/
#include <TrackUUID/JClear.h>
#include <CameraAtt.h>
#include <JEnd.h>
	}

	void DestroyCameras(SceneUnitId id)
	{
		std::set<JUUID> uuids;
		std::transform(CameraSUsceneObjects.at(id).begin(), CameraSUsceneObjects.at(id).end(), std::inserter(uuids, uuids.begin()), [](auto& pair) { return pair.first; });
		for (auto& uuid : uuids)
		{
			DeleteCameraSUSceneObject(id, uuid);
		}
		//for (auto& [uuid, _] : CameraSUsceneObjects.at(id))
		//{
		//	CameraSUUUID cam = MAKESUUUID(id, uuid);
		//	if (cam->shadowMapLight().empty())
		//	{
		//		DeleteCameraSUSceneObject(cam->unit, cam->uuid());
		//	}
		//}

		/*auto uuids = nostd::GetUUIDS(CamerasceneObjects);
		for (CameraUUID uuid : uuids)
		{
			if (uuid->unit != unit || !uuid->shadowMapLight().empty()) continue;
			DeleteCameraSUSceneObject(uuid->unit, uuid());
		}*/
#include <TrackUUID/JClearUnit.h>
#include <CameraAtt.h>
#include <JEnd.h>
	}

	void DeleteCamera(SceneUnitId id, JUUID uuid)
	{
		CameraSUUUID cam = MAKESUUUID(id, uuid);
#if defined(_EDITOR)
		Editor::DestroyBillboard(cam->unit, uuid);
#endif
		cam->markedForDelete = true;
	}

#if defined(_EDITOR)
	void WriteCamerasJson(SceneUnitId id, nlohmann::json& json)
	{
#include <Editor/JSaveFile.h>
#include <CameraAtt.h>
#include <JEnd.h>
	}
#endif
}