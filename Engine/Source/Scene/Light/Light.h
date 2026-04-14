#pragma once

#include <Scene.h>
#include <SceneObject.h>
#include "ShadowMap.h"
//#include <SceneObjectDecl.h>
//#include <ShaderMaterials.h>

//let's explain a little bit here
// 1.- One CVB of MaxLights Descriptors is created to write the data used
// by the shaders rendering(hence it needs to have NumFrames times the size)
// I also think that this should be moved to the "RenderPass" once this makes it here
// 2.- each light creates a DSV Heap + DepthStencil resource
// 3.- these DepthStencil resources for each light are then used to be mapped into
// a CBV descriptor as SRV unbounded slots

namespace Scene {

	static constexpr XMVECTOR PointLightDirection[] = {
	{ 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f,-1.0f, 0.0f, 0.0f },
	{ 1.0f, 0.0f, 0.0f, 0.0f }, {-1.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f,-1.0f, 0.0f },
	};
	static constexpr XMVECTOR PointLightUp[] = {
		{ 0.0f, 0.0f,-1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f },
	};

	inline static const unsigned int MaxLights = 100U;
	inline static const float cascadePartitionsZeroToOne[] = { 0.05f, 0.15f, 0.6f, 1.0f };

	enum LightType {
		LT_Ambient,
		LT_Directional,
		LT_Spot,
		LT_Point
	};

	static std::vector<std::string> LightTypesStr = {
		"Ambient",
		"Directional",
		"Spot",
		"Point"
	};

	static std::unordered_map<LightType, std::string> LightTypeToString = {
		{ LT_Ambient, "Ambient" },
		{ LT_Directional, "Directional" },
		{ LT_Spot, "Spot" },
		{ LT_Point, "Point" }
	};

	static std::unordered_map<std::string, LightType> StringToLightType = {
		{ "Ambient", LT_Ambient },
		{ "Directional", LT_Directional },
		{ "Spot", LT_Spot },
		{ "Point",	LT_Point }
	};

#if defined(_EDITOR)

	static const std::unordered_map<LightType, nlohmann::json> editorDefaultLightProperties = {
		{ LT_Ambient, {
			{ "color", { 0.3f, 0.3f, 0.3f} },
		}},
		{
			LT_Directional, {
			{ "color", { 1.0f, 1.0f, 1.0f } },
			{ "rotation", { -90.0f, 0.0f, 0.0 } },
			{ "shadowMapWidth", 1024 },
			{ "shadowMapHeight", 1024 },
			{ "zBias", 0.0002 },
			{ "hasShadowMaps", false },
		}},
		{	LT_Spot, {
			{ "color", { 1.0f, 1.0f, 1.0f } },
			{ "position", { 0.0f, 10.0f, 0.0f } },
			{ "rotation", { -90.0f, 0.0, 0.0 } },
			{ "coneAngle", 45.0f },
			{ "attenuation" , { 0.0f, 0.001f, 0.0001f } },
			{ "shadowMapWidth", 1024 },
			{ "shadowMapHeight", 1024 },
			{ "zBias", 0.000002 },
			{ "hasShadowMaps", false },
		}},
		{
			LT_Point, {
			{ "color", { 1.0f, 1.0f, 1.0f } },
			{ "position", { 0.0f, 10.0f, 0.0f } },
			{ "attenuation" , { 0.0f, 0.001f, 0.0001f } },
			{ "shadowMapWidth", 1024 },
			{ "shadowMapHeight", 1024 },
			{ "zBias", 0.000002 },
			{ "hasShadowMaps", false },
		}}
	};

	static inline std::unordered_map<LightType, std::vector<std::string>> shadowMapSizes =
	{
		{ LT_Directional, { "32","64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384" }},
		{ LT_Spot, { "32","64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384" }},
		{ LT_Point, { "32","64", "128", "256", "512", "1024", "2048" }}
	};

#endif

	struct LightAttributes {
		LightType lightType; //8
		XMFLOAT3 lightColor; //32
		XMFLOAT4 atts1; //64
		XMFLOAT4 atts2; //96
		XMFLOAT3 atts3; //120
		bool hasShadowMap; //128
		unsigned int shadowMapIndex; //136
		XMFLOAT3 _pad; //160
	};

	struct LightPool {
		unsigned int numLights;
		unsigned int _pad1;
		unsigned int _pad2;
		unsigned int _pad3;
		LightAttributes lights[MaxLights];
	};

#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <LightAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <LightAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include <LightAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <LightAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <LightAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <LightAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <LightAtt.h>
#include <JEnd.h>

#endif

	struct Light : SceneObject
	{
		inline static const SceneObjectType sceneObjectType = SO_Lights;

#include <Attributes/JFlags.h>
#include <LightAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <LightAtt.h>
#include <JEnd.h>

		//lifecycle
		Light(SceneUnitId id, nlohmann::json& json);
		~Light() { Destroy(); }

#if defined(_EDITOR)
		void DropJsonMoldAttributes(nlohmann::json& j) override;
#endif

		void Destroy();

		XMVECTOR positionV();
		void updateRotationQ();
		XMVECTOR rotationQ();
		void rotationQ(XMVECTOR q);
		XMMATRIX world();
		XMVECTOR fw();

		//CREATE
		virtual void Initialize();
		virtual void SetInitialConditions();
		virtual void BindToScene();
		void BindCameras();
		void BindCamera(CameraID camera);
		void BindRenderablesToShadowMapCamera();
		virtual void UnbindFromScene();
		void UnbindCameras();
		void UnbindCamera(CameraID camera);
		void UnbindRenderablesFromShadowMapCameras();
		void UnbindRenderableFromShadowMapCamera(RenderableID r);

		void LoadShadowMap();
		void CreateShadowMap();
		nlohmann::json CreateDirectionalShadowMapCameraJson(unsigned camIndex);
		nlohmann::json CreateSpotShadowMapCameraJson();
		nlohmann::json CreatePointShadowMapCameraJson(unsigned camIndex);
		void CreateDirectionalLightShadowMap();
		void CreateSpotLightShadowMap();
		void CreatePointLightShadowMap();
		void CreateShadowMapDepthStencilResource();
		void CreateShadowMapShaderResourceView();
		void UnloadShadowMap();
		void DestroyShadowMap();
		void DestroyShadowMapCameras();

		void UpdateShadowMapCameraProperties();
		void UpdateDirectionalShadowMapCameraProperties();
		void CreateDirectionalCascadeShadowMapViewProjectionMatrices();
		void UpdateSpotShadowMapCameraProperties();
		void UpdatePointShadowMapCameraProperties();
		void UpdateShadowMapCameraTransformation();
		void UpdateDirectionalShadowMapCameraTransformation();
		void UpdateSpotShadowMapCameraTransformation();
		void UpdatePointShadowMapCameraTransformation();

		bool RenderReady();
		void RenderReady(bool value);

		void WriteConstantsBufferLightAttributes(LightAttributes& atts);
		void WriteConstantsBufferShadowMapAttributes(ShadowMapAttributes& atts);
		void RenderShadowMap(std::function<void(unsigned int)> renderScene);

#if defined(_EDITOR)
		virtual void EditorPreview(size_t flags);
		virtual void DestroyEditorPreview();
		void CreateShadowMapMinMaxChain();
		void DestroyShadowMapMinMaxChain();
		void RenderShadowMapMinMaxChain();

		//Billboard
		virtual RenderableID CreateBillboard(CameraID camera);
		virtual void UpdateBillboard(RenderableID renderable);
		BoundingBox GetBoundingBox();
		//Gizmo
		virtual bool CanInteractWithGizmo(ImGuizmo::OPERATION operation);
		virtual void WriteJson(nlohmann::json& j);
#endif

		DeleteHook markedForDelete;
		bool renderReady = false;
		//Transformation
		XMVECTOR rotationQuaternion;
		//Camera
		unsigned int shadowMapIndex = 0xFFFFFFFF;
		std::vector<CameraID> shadowMapCameras;
		std::vector<D3D12_RECT> shadowMapScissorRect;
		std::vector<D3D12_VIEWPORT> shadowMapViewport;
		RenderToTexturePassID shadowMapRenderPass;
		std::vector<std::tuple<float, float>> shadowMapNearFarPlanes;
		XMFLOAT2 shadowMapTexelInvSize;
#if defined(_EDITOR)
		unsigned int const destroyStepsCount = 2U;
		unsigned int destroySteps;
		bool destroySMChain = false;
		std::vector<RenderPassInstanceID> shadowMapMinMaxChainRenderPass;
		RenderPassInstanceID shadowMapMinMaxChainResultRenderPass;
#endif
	};

	SODECL_FULL(Light);

#include <TrackUUID/JDecl.h>
#include <LightAtt.h>
#include <JEnd.h>

	void LightsStep(SceneUnitId unit);
	void DestroyLights();
	void DestroyLights(SceneUnitId unit);
	void DeleteLight(SceneUnitId unit, JUUID uuid);
#if defined(_EDITOR)
	void WriteLightsJson(SceneUnitId id, nlohmann::json& json);
#endif
};

using namespace Scene;
DEF_SCENEOBJECT_ID_HASH(Light);