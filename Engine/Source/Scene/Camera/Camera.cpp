#include "pch.h"
#include "Camera.h"
#include <Application.h>
#include <Templates.h>
#include <Scene.h>
#include <Renderer.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <NoMath.h>

extern std::unique_ptr<JRenderer> renderer;

#if defined(_EDITOR)
namespace Editor
{
	extern void SelectCamera(CameraID camera);
	extern RenderableID CreateBillboardFromMaterials(SceneUnitId id, CameraID camera, std::string name, std::string material, std::string pickingMaterial);
	extern void RegisterBillboard(SceneUnitId id, JUUID sceneObject);
	extern void DestroyBillboard(SceneUnitId id, JUUID sceneObject);
	extern bool IsPlaying(SceneUnitId id);

	extern bool StaticBodiesSceneUnitRegistered(SceneUnitId id);
	extern bool DynamicBodiesSceneUnitRegistered(SceneUnitId id);
	extern bool CharactersSceneUnitRegistered(SceneUnitId id);
	extern bool TriggersSceneUnitRegistered(SceneUnitId id);

	//Should Draw
	extern bool StaticBodiesShouldDraw(SceneUnitId id);
	extern bool DynamicBodiesShouldDraw(SceneUnitId id);
	extern bool CharactersShouldDraw(SceneUnitId id);
	extern bool TriggersShouldDraw(SceneUnitId id);

	//Physics Objects list
	extern std::set<PhysicObjectID> GetStaticBodies(SceneUnitId id);
	extern std::set<PhysicObjectID> GetDynamicBodies(SceneUnitId id);
	extern std::set<PhysicObjectID> GetCharacters(SceneUnitId id);
	extern std::set<PhysicObjectID> GetTriggers(SceneUnitId id);
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
		RENAME_ON_DELETION(Camera);
	}

	void Camera::create_rotation(XMFLOAT3 v)
	{
		if (!contains("rotation"))
		{
			rotation(v);
		}
	}

	void Camera::rotation(XMFLOAT3 v)
	{
		(*this)["rotation"] = FromXMFLOAT3(v);
		updateRotationQ();
	}

#if defined(_EDITOR)
	void Camera::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <CameraAtt.h>
#include <JEnd.h>
	}

	std::map<std::string, ScriptBinding> Camera::GetScriptBindingOptions()
	{
		std::map<std::string, ScriptBinding> options = SceneObject::GetScriptBindingOptions();

		for (auto& [key, _] : at("controllers").items())
		{
			std::string name = std::string(at("name")) + "/" + std::string(key);
			options.insert_or_assign(name, ScriptBinding(at("uuid"), key));
		}

		return options;
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
		if (shadowMapLight().empty() && !SceneIsIsolated(unit))
			RegisterBillboard(unit, uuid());
#endif

		if (shadowMapLight().empty())
		{
			CreateLightsConstantsBuffer();
			CreateShadowMapsConstantsBuffer();
		}
		SetInitialConditions();
	}

	void Camera::SetInitialConditions()
	{
		updateRotationQ();
	}

#if defined(_EDITOR)
	void Camera::DropJsonMoldAttributes(nlohmann::json& j)
	{
		SceneObject::DropJsonMoldAttributes(j);
		j.at("shadowMapLight") = "";
	}
#endif

	XMVECTOR Camera::positionV()
	{
		XMFLOAT3 pos = position();
		return XMLoadFloat3(&pos);
	}

	void Camera::positionV(XMVECTOR v)
	{
		XMFLOAT3 pos;
		XMStoreFloat3(&pos, v);
		position(pos);
	}

	void Camera::updateRotationQ()
	{
		XMFLOAT3 v = rotation();
		rotationQuaternion = XMQuaternionRotationRollPitchYaw(
			XMConvertToRadians(v.x),
			XMConvertToRadians(v.y),
			XMConvertToRadians(v.z)
		);
	}

	XMVECTOR Camera::rotationQ()
	{
		return rotationQuaternion;
	}

	void Camera::rotationQ(XMVECTOR q)
	{
		rotationQuaternion = q;
	}

	XMVECTOR Camera::forward()
	{
		if (!shadowMapLight().empty())
		{
			LightID lcam = MAKESUUUID(unit, shadowMapLight());
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
			LightID lcam = MAKESUUUID(unit, shadowMapLight());
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
		XMMATRIX rotationM = XMMatrixRotationQuaternion(rotationQ());
		XMMATRIX positionM = XMMatrixTranslationFromVector(positionV());
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

	void Camera::CopyProjection(CameraID cam)
	{
		projectionType(cam->projectionType());
		fitWindow(cam->fitWindow());
		perspective(cam->perspective());
		orthographic(cam->orthographic());
		UpdateProjection();
	}

	std::tuple<unsigned int, unsigned int, bool> Camera::Project(XMVECTOR world_pos)
	{
		XMMATRIX viewM = view();
		XMMATRIX projM = projection();
		XMMATRIX viewProjection = XMMatrixMultiply(viewM, projM);
		float projWidth = projectionWidth();
		float projHeight = projectionHeight();

		world_pos = XMVectorSetW(world_pos, 1.0f);
		XMVECTOR clipSpacePos = XMVector4Transform(world_pos, viewProjection);
		float w = XMVectorGetW(clipSpacePos);
		XMVECTOR ndc = XMVectorScale(clipSpacePos, 1.0f / w);

		float screenX = (XMVectorGetX(ndc) + 1.0f) * 0.5f * projWidth;
		float screenY = (1.0f - XMVectorGetY(ndc)) * 0.5f * projHeight;

		return std::make_tuple(static_cast<unsigned int>(screenX), static_cast<unsigned int>(screenY), clipSpacePos.m128_f32[3] < 0);
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

		for (unsigned int i = 0; i < renderPasses().size(); i++)
		{
			JUUID passUUID = renderPasses().at(i);
			if (passUUID.empty()) continue;
			auto& rp = GetRenderPassTemplate(passUUID);
			if (rp->type() == RenderPassType_SwapChainPass && rp->renderCallbackOverride() != RenderPassRenderCallbackOverride_Resolve) continue;

			renderPassesUUID.push_back(CreateRenderPass(passUUID, static_cast<unsigned int>(renderPassesUUID.size())));
		}
	}

	RenderPassJsonID Camera::GetRenderPassTemplateFromInstanceIndex(unsigned int passIndex)
	{
		return renderPassesUUID.at(passIndex)->renderPassTemplate;
	}

	RenderPassInstanceID Camera::CreateRenderPass(RenderPassJsonID pass, unsigned int passIndex)
	{
		unsigned int projW = static_cast<unsigned int>(projectionWidth());
		unsigned int projH = static_cast<unsigned int>(projectionHeight());
		return CreateRenderPassInstance(SUuuid(), pass, passIndex, projW, projH);
	}

	void Camera::CreateRenderPassAtIndex(RenderPassJsonID pass, unsigned int passIndex)
	{
		renderPassesUUID.insert(renderPassesUUID.begin() + passIndex, CreateRenderPass(pass, passIndex));
		RearrangeRenderPassesAfter(passIndex);
	}

	void Camera::DeleteRenderPassAtIndex(unsigned int passIndex)
	{
		renderPassesUUID.at(passIndex)->MarkForDelete();
		renderPassesUUID.erase(renderPassesUUID.begin() + passIndex);
		RearrangeRenderPassesAfter(passIndex);
	}

	void Camera::SwapRenderPassAtIndex(RenderPassJsonID pass, unsigned int passIndex)
	{
		renderPassesUUID.at(passIndex)->MarkForDelete();
		renderPassesUUID[passIndex] = CreateRenderPass(pass, passIndex);
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
		for (auto pass : renderPassesUUID)
		{
			DestroyRenderPassInstance(pass);
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

	void Camera::BindRenderable(RenderableID renderable)
	{
		if (renderables.contains(renderable)) return;
		renderables.insert(renderable);

		renderable->CreateMaterialsInstances(SUuuid());
		renderable->CreateConstantsBuffersInstances(SUuuid());
		renderable->CreateRootSignatures(SUuuid());
		renderable->CreatePipelineStates(SUuuid());
	}

	void Camera::BindLight(LightID light)
	{
		lights.push_back(light);
		BindLightWithShadowMap(light);
	}

	void Camera::BindLightWithShadowMap(LightID light)
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
		DestroyConstantsBuffer(cameraCb());
		if (shadowMapLight().empty())
		{
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

	void Camera::UnbindRenderable(RenderableID renderable)
	{
		if (!renderables.contains(renderable)) return;
		renderables.erase(renderable);

		renderable->DestroyMaterialsInstances(SUuuid());
		renderable->DestroyConstantsBuffersInstances(SUuuid());
		renderable->DestroyRootSignatures(SUuuid());
		renderable->DestroyPipelineStates(SUuuid());
	}

	void Camera::UnbindLight(LightID light)
	{
		nostd::vector_erase(lights, light);
		UnbindLightWithShadowMap(light);
	}

	void Camera::UnbindLightWithShadowMap(LightID light)
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
			RenderPassJsonID pass = renderPasses().at(i);
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
		std::set<RenderableID> nonRoot;
		for (auto r : renderables)
		{
			std::vector<JUUID> uuids = r->renderNext();
			for (std::string& uuid : uuids)
			{
				if (!SceneObjectExists(unit, uuid)) continue;
				nonRoot.insert(MAKESUUUID(unit, uuid));
			}
		}

#if defined(_EDITOR)
		using namespace Editor;

		//Physics Objects list
		std::vector<std::tuple<bool, std::set<PhysicObjectID>>> drawPhysicsObjects;
		if (StaticBodiesSceneUnitRegistered(unit)) drawPhysicsObjects.push_back(std::make_tuple(StaticBodiesShouldDraw(unit), GetStaticBodies(unit)));
		if (DynamicBodiesSceneUnitRegistered(unit)) drawPhysicsObjects.push_back(std::make_tuple(DynamicBodiesShouldDraw(unit), GetDynamicBodies(unit)));
		if (CharactersSceneUnitRegistered(unit)) drawPhysicsObjects.push_back(std::make_tuple(CharactersShouldDraw(unit), GetCharacters(unit)));
		if (TriggersSceneUnitRegistered(unit)) drawPhysicsObjects.push_back(std::make_tuple(TriggersShouldDraw(unit), Editor::GetTriggers(unit)));

		for (auto& [draw, list] : drawPhysicsObjects)
		{
			for (auto phO : list)
			{
				phO->visible(false);
			}
		}
#endif

		//create the renderable set recursivelly
		nostd::VectorSet<RenderableID> renVecSet;
		std::function<void(RenderableID)> addToRenderablesVecSet;
		addToRenderablesVecSet = [&](RenderableID r)
			{
				if (!r->visible()) return;
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
					RenderableID renderable = *it;
					if (renderable->checkBoundingBox() && boundingFrustum.Contains(renderable->GetBoundingBox()) == ContainmentType::DISJOINT)
						continue;
					//OutputDebugStringA(std::string(rpi->renderPassTemplate->name() + ":" + renderable->name() + "\n").c_str());
					renderable->Render(unit, rpi, SUuuid());
				}

#if defined(_EDITOR)
				for (auto& [draw, list] : drawPhysicsObjects)
				{
					if (!draw) continue;
					for (auto phO : list)
					{

						RenderableID shape = phO->renderableShape;
						RenderableID lines = phO->renderableLines;
						if (!shape || (shape->checkBoundingBox() && boundingFrustum.Contains(shape->GetBoundingBox()) == ContainmentType::DISJOINT) || phO->skipRendering())
							continue;
						phO->visible(true);
						shape->Render(unit, rpi, SUuuid());
						lines->Render(unit, rpi, SUuuid());
						//triggers can be picked
						if (phO->behavior() != PB_Trigger && !phO->boundary) { phO->visible(false); }
					}
				}
#endif
			};

		std::vector<RenderPassInstanceID> rpiv = renderPassesUUID;
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

	void Camera::LookAt(XMVECTOR target)
	{
		XMVECTOR eyePos = positionV();
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		XMVECTOR dir = XMVector3Normalize(XMVectorSubtract(target, eyePos));

		if (XMVector3Equal(dir, up) || XMVector3Equal(dir, XMVectorNegate(up))) {
			up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // Cambiamos el up temporalmente
		}

		XMMATRIX viewMatrix = XMMatrixLookAtLH(eyePos, target, up);
		XMVECTOR det;
		XMMATRIX worldMatrix = XMMatrixInverse(&det, viewMatrix);

		XMVECTOR newRotationQ = XMQuaternionRotationMatrix(worldMatrix);

		rotation(Quaternion2Euler(newRotationQ));
		rotationQ(newRotationQ);
	}

	bool Camera::IsLookingAt(XMVECTOR targetPos, float epsilonDegrees)
	{
		XMVECTOR eyePos = positionV();
		XMVECTOR toTarget = XMVector3Normalize(XMVectorSubtract(targetPos, eyePos));

		XMVECTOR fw = forward();
		XMVECTOR dotVec = XMVector3Dot(fw, toTarget);
		float dot;
		XMStoreFloat(&dot, dotVec);

		float threshold = cosf(XMConvertToRadians(epsilonDegrees));

		return dot >= threshold;
	}

	void Camera::LookAtBoundingBox(BoundingBox bb, float scale)
	{
		XMVECTOR bbCenter = XMLoadFloat3(&bb.Center);
		XMVECTOR diff = bbCenter - positionV();

		if (IsLookingAt(bbCenter) && XMVectorGetX(XMVector3Length(diff)) <= 0.001f)
			return;

		LookAt(bbCenter);

		BoundingSphere bbs;
		BoundingSphere::CreateFromBoundingBox(bbs, bb);
		float fov = XMConvertToRadians(perspective().fovAngleY);
		float distance = scale * (bbs.Radius * 2.0f) / (XMScalarSin(fov) / XMScalarCos(fov));

		XMVECTOR camFwV = forward();
		XMVECTOR camPosV = XMVectorSubtract(bbCenter, XMVectorScale(camFwV, distance));
		XMFLOAT3 camPos;
		XMStoreFloat3(&camPos, camPosV);
		position(camPos);

	}

	void Camera::Destroy()
	{
#include <Attributes/JDestroy.h>
#include <CameraAtt.h>
#include <JEnd.h>

		SceneObject::Destroy();
	}

	void Camera::CreateConstantsBuffer()
	{
		cameraCb = DeviceUtils::CreateConstantsBuffer(sizeof(CameraAttributes), JRenderer::numFrames, name());
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
			TextureJsonID tex = iblTextures.at(TextureShaderUsage_IBLPreFilteredEnvironment);
			atts.IBLNumEnvLevels = static_cast<float>(tex->mipLevels());
		}
		else
		{
			atts.IBLNumEnvLevels = 0.0f;
		}

		cameraCb->push(atts, frame);
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
		XMVECTOR upV = { 0.0f,1.0f,0.0f,0.0f }; // we just use the up vector in order to avoid roll rotations
		XMVECTOR rightV = -right();
		XMVECTOR rotQYaw = XMQuaternionRotationAxis(upV, XMConvertToRadians(dx));
		XMVECTOR rotQPitch = XMQuaternionRotationAxis(rightV, XMConvertToRadians(dy));
		rotationQuaternion = XMQuaternionMultiply(rotationQuaternion, XMQuaternionMultiply(rotQPitch, rotQYaw));

		UpdateLightRotation();
	}

	void Camera::UpdateLightPosition()
	{
		using namespace Scene;

		if (!SceneObjectExists(unit, shadowMapLight()))
			return;

		LightID lcam = MAKESUUUID(unit, shadowMapLight());

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

	void Camera::UpdateLightRotation()
	{
		using namespace Scene;

		if (!SceneObjectExists(unit, shadowMapLight()))
			return;

		LightID lcam = MAKESUUUID(unit, shadowMapLight());

		switch (lcam->lightType())
		{
		case LT_Directional:
		{
			lcam->rotationQ(rotationQ());
			XMVECTOR camPos = XMVectorScale(XMVector3Normalize(forward()), lcam->dirDist());
			position(*(XMFLOAT3*)camPos.m128_f32);
		}
		break;
		case LT_Spot:
		{
			lcam->rotationQ(rotationQ());
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
		lightsCB = DeviceUtils::CreateConstantsBuffer(sizeof(LightPool), JRenderer::numFrames, std::string(name() + "-lightsCbv"));
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
		shadowMapsCB = DeviceUtils::CreateConstantsBuffer(sizeof(ShadowMapAttributes) * MaxLights, JRenderer::numFrames, std::string(name() + "-shadowMapsCbv"));
	}

	void Camera::DestroyShadowMapsConstantsBuffer()
	{
		DestroyConstantsBuffer(shadowMapsCB());
		shadowMapsCB.clear();
	}

	void Camera::WriteShadowMapsConstantsBuffer(unsigned int frame)
	{
		for (LightID light : lights)
		{
			for (CameraID cam : light->shadowMapCameras)
			{
				cam->WriteConstantsBuffer(frame);
			}
		}

		size_t offset = shadowMapsCB->alignedConstantBufferSize * frame;
		ShadowMapAttributes* atts = (ShadowMapAttributes*)(shadowMapsCB->mappedConstantBuffer + offset);
		for (LightID light : lights)
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

	RenderableID Camera::CreateBillboard(CameraID camera)
	{
		RenderableID bb = Editor::CreateBillboardFromMaterials(unit, camera, at("name"), "Camera", "CameraPicking");
		bb->OnPick = [&] {Editor::SelectCamera(SUuuid()); };
		UpdateBillboard(bb);
		return bb;
	}

	void Camera::UpdateBillboard(RenderableID renderable)
	{
		assert(!renderable.empty());
		if (renderable.empty()) return;

		auto& scene = GetSceneUnit(unit);

		XMFLOAT3 baseColor = { 1.0f,1.0f,1.0f };
		renderable->position(position());
		renderable->WriteConstantsBuffer<XMFLOAT3>("baseColor", baseColor, scene->Frame());
		renderable->WriteConstantsBuffer(scene->Frame());
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
		std::set<CameraID> cams;
		std::transform(Cameras.begin(), Cameras.end(), std::inserter(cams, cams.begin()), [&](auto o) { return MAKESUUUID(id, o); });

		//is this(hack) or fix the loading system. that's what i'm doing now
		//auto& scene = GetSceneUnit(id);
		//for (auto& c : cams)
		//{
		//	if (!c->RenderReady() && scene->IsBound(c.uuid()))
		//	{
		//		c->RenderReady(true);
		//		scene->EraseCameraFromLoadingPool(c);
		//	}
		//}

		//update rotation quaternion
		for (auto cam : cams)
		{
			if (cam->dirty(Camera::Update_rotation))
			{
				cam->updateRotationQ();
				cam->clean(Camera::Update_rotation);
			}
		}

		//we construct the set of cameras with dirty render passes
		std::set<CameraID> dirtyPassesCams;
		std::copy_if(cams.begin(), cams.end(), std::inserter(dirtyPassesCams, dirtyPassesCams.begin()), [](auto cam)
			{
				return cam->dirty(Camera::Update_renderPasses);
			}
		);

		for (auto& cam : dirtyPassesCams)
		{
			std::map<RenderPassJsonID, std::tuple<int, int>> passes;

			//prev pass fill
			for (unsigned int i = 0; i < cam->UpdatePrevValues["renderPasses"].size(); i++)
			{
				RenderPassJsonID pass = JUUID(cam->UpdatePrevValues["renderPasses"].at(i));
				if (pass.empty()) continue;
				passes[pass] = std::make_tuple(-1, -1);
			}
			//curr pass fill
			for (unsigned int i = 0; i < cam->at("renderPasses").size(); i++)
			{
				RenderPassJsonID pass = JUUID(cam->at("renderPasses").at(i));
				if (pass.empty()) continue;
				passes[pass] = std::make_tuple(-1, -1);
			}
			//set from indices
			int index = 0;
			for (unsigned int i = 0; i < cam->UpdatePrevValues["renderPasses"].size(); i++)
			{
				RenderPassJsonID pass = JUUID(cam->UpdatePrevValues["renderPasses"].at(i));
				if (pass.empty()) continue;
				std::get<0>(passes[pass]) = index;
				index++;
			}
			//set right indices
			index = 0;
			for (unsigned int i = 0; i < cam->at("renderPasses").size(); i++)
			{
				RenderPassJsonID pass = JUUID(cam->at("renderPasses").at(i));
				if (pass.empty()) continue;
				std::get<1>(passes[pass]) = index;
				index++;
			}

			//figure out the rearrange
			int deleteElementIndex = -1;
			int addElementIndex = -1;
			RenderPassJsonID addElementJsonUUID;
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
		std::set<CameraID> dirtyIBL;
		std::copy_if(cams.begin(), cams.end(), std::inserter(dirtyIBL, dirtyIBL.begin()), [](auto cam)
			{
				return cam->dirty(Camera::Update_IBLIrradiance) || cam->dirty(Camera::Update_IBLPreFilteredEnvironment) || cam->dirty(Camera::Update_IBLBRDFLUT);
			}
		);

		std::map<SceneUnitId, std::set<CameraID>> camerasToRebind;
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
			//auto& scene = GetSceneUnit(id);
			//scene->ResetLoadingCommandList();
			//scene->SetLoading(true);
			//scene->SetCanSubmitLoading(false);
			//for (auto& cam : cams)
			//{
			//	rebindCam(cam);
			//}
			//scene->CloseSubmitLoadingCommandList();
		}

		std::set<CameraID> dirtyProjectionCams;
		std::copy_if(cams.begin(), cams.end(), std::inserter(dirtyProjectionCams, dirtyProjectionCams.begin()), [](auto cam)
			{
				return cam->dirty(Camera::Update_projectionType) || cam->dirty(Camera::Update_perspective) ||
					cam->dirty(Camera::Update_orthographic) || cam->dirty(Camera::Update_fitWindow);
			}
		);

		for (auto& cam : dirtyProjectionCams)
		{
			cam->clean(Camera::Update_projectionType);
			cam->clean(Camera::Update_perspective);
			cam->clean(Camera::Update_orthographic);
			cam->clean(Camera::Update_fitWindow);
			cam->UpdateProjection();
		}

		std::set<CameraID> delCams;
		std::copy_if(cams.begin(), cams.end(), std::inserter(delCams, delCams.begin()), [](auto c) { return c->markedForDelete; });

		for (auto c : delCams)
		{
			EraseCameraFromCameras(c->unit, c.uuid());
			EraseCameraFromWindowCameras(c->unit, c.uuid());
			EraseCameraFromSwapChainCameras(c->unit, c.uuid());
			EraseCameraFromMouseCameras(c->unit, c.uuid());
			DeleteCameraSceneObject(c);
		}
	}

	void DestroyCameras()
	{
		for (auto& [id, container] : CameraSUsceneObjects)
		{
			for (auto& [uuid, _] : container)
			{
				CameraID cam = MAKESUUUID(id, uuid);
				if (cam->shadowMapLight().empty())
				{
					DeleteCameraSceneObject(cam);
				}
			}
		}
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
			DeleteCameraSceneObject(MAKESUUUID(id, uuid));
		}
#include <TrackUUID/JClearUnit.h>
#include <CameraAtt.h>
#include <JEnd.h>
	}

	void DeleteCamera(SceneUnitId id, JUUID uuid)
	{
		CameraID cam = MAKESUUUID(id, uuid);
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