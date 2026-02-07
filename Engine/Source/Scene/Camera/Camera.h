#pragma once

#include <Scene.h>
#include <SceneObject.h>
#include "Projections/Perspective.h"
#include "Projections/Orthographic.h"
//#include <SceneObjectDecl.h>
//#include <SceneObject.h>
//#include <JTypes.h>

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

//namespace Templates { struct TextureInstance; struct RenderPassInstance; };
//namespace DeviceUtils { struct RenderToTexturePass; };

using namespace Scene::CameraProjections;

//enum TextureShaderUsage;

//typedef std::unordered_map<TextureShaderUsage, JUUID> TextureUsageInstanceMap;

namespace Scene {
	//struct Light;
	//struct Renderable;
	//struct Camera;

	//using namespace DeviceUtils;
	//using namespace Templates;

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

		union {
			CameraProjections::Perspective perspectiveProjection;
			CameraProjections::Orthographic orthographicProjection;
		};

		Camera(SceneUnitId id, nlohmann::json& json);
		~Camera() { Destroy(); }
		XMVECTOR positionV();
		XMVECTOR rotationQ();
		XMVECTOR forward();
		XMVECTOR up();
		XMVECTOR right();
		XMMATRIX world();
		XMMATRIX view();
		XMMATRIX projection();

		float projectionWidth();
		float projectionRight();
		float projectionBottom();
		float projectionHeight();
		float projectionNearZ();
		float projectionFarZ();
		float projectionfovAngleY();

		void CreateRenderPasses();
		RenderPassJsonUUID GetRenderPassTemplateFromInstanceIndex(unsigned int passIndex);
		RenderPassInstanceUUID CreateRenderPass(JUUID passUUID, unsigned int passIndex);
		void CreateRenderPassAtIndex(JUUID passUUID, unsigned int passIndex);
		void DeleteRenderPassAtIndex(unsigned int passIndex);
		void SwapRenderPassAtIndex(JUUID passUUID, unsigned int passIndex);
		void RearrangeRenderPassesAfter(unsigned int passIndex);
		void DestroyRenderPasses();
		void ResizeReleasePasses();
		void ResizePasses(unsigned int width, unsigned int height);
		void UpdateProjection();

		virtual void Initialize();
		virtual void BindToScene();
		virtual void Bind(JUUID uuid);
		void BindRenderable(RenderableSUUUID renderable);
		void BindLight(LightSUUUID light);
		void BindLightWithShadowMap(LightSUUUID light);
		virtual void UnbindFromScene();
		virtual void Unbind(JUUID uuid);
		void UnbindRenderable(RenderableSUUUID renderable);
		void UnbindLight(LightSUUUID light);
		void UnbindLightWithShadowMap(LightSUUUID light);

		bool ResolvesToSwapChain();
		bool RenderReady();
		void RenderReady(bool value);
		void Render();

		//Bounding Frustum
		BoundingFrustum boundingFrustum;
		void CalculateBoundingFrustum();

		void Destroy();

		void CreateConstantsBuffer();
		void WriteConstantsBuffer(unsigned int frame);
		//void ProcessKeyboardInput(DirectX::Keyboard::KeyboardStateTracker& tracker, DirectX::Keyboard::State& state);
		void MoveAlongFwAxis(float dz);
		void MovePerpendicularFwAxis(float dx, float dy);
		void Rotate(float dx, float dy);
		//void ProcessGamepadInput(DirectX::GamePad::State& gamePadState, DirectX::SimpleMath::Vector2 gamePadCameraRotationSensitivity);
		//void ProcessCameraMouseRotation(DirectX::Mouse::State& mouseState, DirectX::SimpleMath::Vector2 mouseCameraRotationSensitivity, bool firstStep);
		void UpdateLightPosition();
		void UdateLightRotation();
		void MoveForward(float step);
		void MoveBack(float step);
		void MoveLeft(float step);
		void MoveRight(float step);

		//Lighting
		void CreateLightsConstantsBuffer();
		void DestroyLightsConstantsBuffer();
		ConstantsBufferUUID GetLightsConstantsBuffer() const { return lightsCB; }
		void WriteLightsConstantsBuffer(unsigned int frame);

		//ShadowMaps
		void CreateShadowMapsConstantsBuffer();
		void DestroyShadowMapsConstantsBuffer();
		ConstantsBufferUUID GetShadowMapsConstantsBuffer() const { return shadowMapsCB; }
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
		virtual JUUID CreateBillboard(CameraSUUUID camera);
		virtual void UpdateBillboard(JUUID uuid);
		BoundingBox GetBoundingBox();

		//Gizmo
		virtual bool CanInteractWithGizmo(ImGuizmo::OPERATION operation) { return true; }
		virtual void WriteJson(nlohmann::json& j);
#endif

		//Destroy
		bool markedForDelete = false;
		//Render
		bool renderReady = false;
		//render passes instances
		std::vector<RenderPassInstanceUUID> renderPassesUUID;
		//this camera attributes
		ConstantsBufferUUID cameraCb;
		//renderables
		std::set<RenderableSUUUID> renderables;
		//lights
		ConstantsBufferUUID lightsCB;
		std::vector<LightSUUUID> lights;
		//lights shadowmaps
		ConstantsBufferUUID shadowMapsCB;
		std::set<LightSUUUID> lightsWithShadowMaps;
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
