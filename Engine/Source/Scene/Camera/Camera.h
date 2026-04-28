#pragma once

#include <Scene.h>
#include <SceneObject.h>
#include "Projections/Perspective.h"
#include "Projections/Orthographic.h"

enum ProjectionsTypes {
	PROJ_Orthographic,
	PROJ_Perspective
};

inline static std::vector<std::string> ProjectionsTypesStr = {
	"Orthographic",
	"Perspective"
};

inline static std::unordered_map<ProjectionsTypes, std::string> ProjectionsTypesToString = {
	{ PROJ_Orthographic, "Orthographic" },
	{ PROJ_Perspective, "Perspective" }
};

inline static std::unordered_map<std::string, ProjectionsTypes> StringToProjectionsTypes = {
	{ "Orthographic", PROJ_Orthographic },
	{ "Perspective", PROJ_Perspective }
};

struct CameraAttributes {
	XMMATRIX view;
	XMMATRIX viewProjection;
	XMFLOAT4 eyePosition;
	XMFLOAT4 eyeForward;
	XMFLOAT4 eyeUp;
	XMFLOAT4 eyeRight;
	XMFLOAT4 widthHeight;
	float white;
	float IBLNumEnvLevels;
};

using namespace Scene::CameraProjections;

namespace Scene {
#if defined(_EDITOR)
#include <Attributes/JOrder.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <CameraAtt.h>
#include <JEnd.h>

#endif

	struct Camera : SceneObject
	{
		inline static const SceneObjectType sceneObjectType = SO_Cameras;

#include <Attributes/JFlags.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <CameraAtt.h>
#include <JEnd.h>

		Camera(SceneUnitId id, nlohmann::json& json);
		~Camera() { Destroy(); }

#if defined(_EDITOR)
		void DropJsonMoldAttributes(nlohmann::json& j) override;
#endif

		XMVECTOR positionV();
		void positionV(XMVECTOR v);
		void updateRotationQ();
		XMVECTOR rotationQ();
		void rotationQ(XMVECTOR q);
		XMVECTOR forward();
		XMVECTOR up();
		XMVECTOR right();
		XMMATRIX world();
		XMMATRIX view();
		XMMATRIX projection();
		void CopyProjection(CameraID cam);
		std::tuple<unsigned int, unsigned int, bool> Project(XMVECTOR world_pos);

		float projectionWidth();
		float projectionRight();
		float projectionBottom();
		float projectionHeight();
		float projectionNearZ();
		float projectionFarZ();
		float projectionfovAngleY();

		void CreateRenderPasses();
		RenderPassJsonID GetRenderPassTemplateFromInstanceIndex(unsigned int passIndex);
		RenderPassInstanceID CreateRenderPass(RenderPassJsonID pass, unsigned int passIndex);
		void CreateRenderPassAtIndex(RenderPassJsonID pass, unsigned int passIndex);
		void DeleteRenderPassAtIndex(unsigned int passIndex);
		void SwapRenderPassAtIndex(RenderPassJsonID pass, unsigned int passIndex);
		void RearrangeRenderPassesAfter(unsigned int passIndex);
		void DestroyRenderPasses();
		void ResizeReleasePasses();
		void ResizePasses(unsigned int width, unsigned int height);
		void UpdateProjection();

		void Initialize() override;
		virtual void SetInitialConditions();
		virtual void BindToScene();
		virtual void Bind(JUUID uuid);
		void BindRenderable(RenderableID renderable);
		void BindLight(LightID light);
		void BindLightWithShadowMap(LightID light);
		virtual void UnbindFromScene();
		virtual void Unbind(JUUID uuid);
		void UnbindRenderable(RenderableID renderable);
		void UnbindLight(LightID light);
		void UnbindLightWithShadowMap(LightID light);

		bool ResolvesToSwapChain();
		bool RenderReady();
		void RenderReady(bool value);
		void Render();

		//Bounding Frustum
		void CalculateBoundingFrustum();
		//Adjust
		void LookAt(XMVECTOR target);
		void LookAtBoundingBox(BoundingBox bb, float scale = 1.0f);
		bool IsLookingAt(XMVECTOR targetPos, float epsilonDegrees = 1.0f);

		void Destroy();

		void CreateConstantsBuffer();
		void WriteConstantsBuffer(unsigned int frame);
		void MoveAlongFwAxis(float dz);
		void MovePerpendicularFwAxis(float dx, float dy);
		void Rotate(float dx, float dy);
		void UpdateLightPosition();
		void UpdateLightRotation();
		void MoveForward(float step);
		void MoveBack(float step);
		void MoveLeft(float step);
		void MoveRight(float step);

		//Lighting
		void CreateLightsConstantsBuffer();
		void DestroyLightsConstantsBuffer();
		ConstantsBufferID GetLightsConstantsBuffer() const { return lightsCB; }
		void WriteLightsConstantsBuffer(unsigned int frame);

		//ShadowMaps
		void CreateShadowMapsConstantsBuffer();
		void DestroyShadowMapsConstantsBuffer();
		ConstantsBufferID GetShadowMapsConstantsBuffer() const { return shadowMapsCB; }
		void WriteShadowMapsConstantsBuffer(unsigned int frame);
		bool SceneHasShadowMaps() const { return !lightsWithShadowMaps.empty(); }

		//IBL
		TextureUsageInstanceMap iblTextures;
		bool HasIBL();
		void CreateIBLTextures();
		void CreateIBLIrradianceTexture();
		void CreateIBLPreFilteredEnvironmentTexture();
		void CreateIBLBRDFLUTTexture();

		void DestroyIBLTextures();
		void SetIBLRootDescriptorTables(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& cbvSlot);
#if defined(_EDITOR)
		virtual void EditorPreview(size_t flags);
		virtual void DestroyEditorPreview();
		virtual RenderableID CreateBillboard(CameraID camera);
		virtual void UpdateBillboard(RenderableID renderable);
		BoundingBox GetBoundingBox();

		//Gizmo
		virtual bool CanInteractWithGizmo(ImGuizmo::OPERATION operation) { return true; }
		virtual void WriteJson(nlohmann::json& j);
		virtual std::map<std::string, ScriptBinding> GetScriptBindingOptions();
#endif

		//State
		DeleteHook markedForDelete;
		//Transformation
		XMVECTOR rotationQuaternion;
		//Render
		bool renderReady = false;
		//render passes instances
		std::vector<RenderPassInstanceID> renderPassesUUID;
		//this camera attributes
		ConstantsBufferID cameraCb;
		//renderables
		std::set<RenderableID> renderables;
		//lights
		ConstantsBufferID lightsCB;
		std::vector<LightID> lights;
		//lights shadowmaps
		ConstantsBufferID shadowMapsCB;
		std::set<LightID> lightsWithShadowMaps;
		//projection
		CameraProjections::Perspective perspectiveProjection;
		CameraProjections::Orthographic orthographicProjection;
		//Bounding Frustum
		BoundingFrustum boundingFrustum;
#if defined(_EDITOR)
		unsigned int previewRenderPassIndex = 0U;
		unsigned int previewRenderToTextureIndex = 0U;
#endif
	};

	SODECL_FULL(Camera);

#include <TrackUUID/JDecl.h>
#include <CameraAtt.h>
#include <JEnd.h>

	void CamerasStep(SceneUnitId id);
	void DestroyCameras();
	void DestroyCameras(SceneUnitId id);
	void DeleteCamera(SceneUnitId id, JUUID uuid);
#if defined(_EDITOR)
	void WriteCamerasJson(SceneUnitId id, nlohmann::json& json);
#endif
};

using namespace Scene;
DEF_SCENEOBJECT_ID_HASH(Camera);