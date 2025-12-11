#pragma once

#include "Projections/Perspective.h"
#include "Projections/Orthographic.h"
#include <SceneObjectDecl.h>
#include <SceneObject.h>
#include <JTypes.h>

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

namespace Templates { struct TextureInstance; struct RenderPassInstance; };
namespace DeviceUtils { struct RenderToTexturePass; };

using namespace Scene::CameraProjections;

enum TextureShaderUsage;

typedef std::unordered_map<TextureShaderUsage, JUUID> TextureUsageInstanceMap;

namespace Scene {
	struct Light;
	struct Renderable;
	struct Camera;

	using namespace DeviceUtils;
	using namespace Templates;

	inline static const std::string CameraConstantBufferName = "camera";

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

	void CamerasStep();
	void DestroyCameras();
	void DeleteCamera(std::string uuid);

#if defined(_EDITOR)
	void WriteCamerasJson(nlohmann::json& json);
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

		Camera(nlohmann::json& json);
		~Camera() { Destroy(); }
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif
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

		std::vector<RenderPassInstanceUUID> renderPassesUUID;
		void CreateRenderPasses();
		void DestroyRenderPasses();
		void ResizeReleasePasses();
		void ResizePasses(unsigned int width, unsigned int height);
		void UpdateProjection();

		std::set<RenderableUUID> renderables;
		virtual void Initialize();
		virtual void BindToScene();
		virtual void UnbindFromScene();
		virtual void Bind(JUUID uuid);
		virtual void Unbind(JUUID uuid);
		void BindRenderable(RenderableUUID renderable);
		void UnbindRenderable(RenderableUUID renderable);
		void BindLight(LightUUID light);
		void UnbindLight(LightUUID light);
		bool ResolvesToSwapChain();
		void Render();

		//Bounding Frustum
		BoundingFrustum boundingFrustum;
		void CalculateBoundingFrustum();

		bool markedForDelete = false;
		void Destroy();

		ConstantsBufferUUID cameraCb;
		void CreateConstantsBuffer();
		void WriteConstantsBuffer(unsigned int backbufferIndex);
		void ProcessKeyboardInput(DirectX::Keyboard::KeyboardStateTracker& tracker, DirectX::Keyboard::State& state);
		void MoveAlongFwAxis(float dz);
		void MovePerpendicularFwAxis(float dx, float dy);
		void Rotate(float dx, float dy);
		void ProcessGamepadInput(DirectX::GamePad::State& gamePadState, DirectX::SimpleMath::Vector2 gamePadCameraRotationSensitivity);
		void ProcessCameraMouseRotation(DirectX::Mouse::State& mouseState, DirectX::SimpleMath::Vector2 mouseCameraRotationSensitivity, bool firstStep);
		void UpdateLightPosition();
		void UdateLightRotation();
		void MoveForward(float step);
		void MoveBack(float step);
		void MoveLeft(float step);
		void MoveRight(float step);

		//Lighting
		ConstantsBufferUUID lightsCB;
		std::vector<LightUUID> lights;
		void CreateLightsConstantsBuffer();
		void DestroyLightsConstantsBuffer();
		ConstantsBufferUUID GetLightsConstantsBuffer() { return lightsCB; }
		void WriteLightsConstantsBuffer();

		//ShadowMaps
		ConstantsBufferUUID shadowMapsCB;
		std::set<LightUUID> lightsWithShadowMaps;
		void CreateShadowMapsConstantsBuffer();
		void DestroyShadowMapsConstantsBuffer();
		ConstantsBufferUUID GetShadowMapsConstantsBuffer() { return shadowMapsCB; }
		void WriteShadowMapsConstantsBuffer();
		bool SceneHasShadowMaps() { return !lightsWithShadowMaps.empty(); }

		//IBL
		TextureUsageInstanceMap iblTextures;
		bool HasIBL();
		void CreateIBLTextures();
		void DestroyIBLTextures();
		void SetIBLRootDescriptorTables(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& cbvSlot);
#if defined(_EDITOR)
		unsigned int previewRenderPassIndex = 0U;
		unsigned int previewRenderToTextureIndex = 0U;
		virtual void EditorPreview(size_t flags);
		virtual void DestroyEditorPreview();
		virtual JUUID CreateBillboard(CameraUUID camera);
		virtual void UpdateBillboard(JUUID uuid);
		BoundingBox GetBoundingBox();

		//Gizmo
		virtual bool CanInteractWithGizmo(ImGuizmo::OPERATION operation) { return true; }
#endif
	};


	SODECL_FULL(Camera);

#include <TrackUUID/JDecl.h>
#include <CameraAtt.h>
#include <JEnd.h>
};
