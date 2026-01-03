#pragma once
#include <Scene.h>
#include <SceneObject.h>
//#include <nlohmann/json.hpp>
//#include <set>
#include <Model3D/Model3D.h>
#include <Material/MeshMaterial.h>
#include <RenderPass/PassMaterialOverride.h>
#include <Sequence/AnimationSequences.h>
#include <Sequence/SequencePlayer.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>

//#include <SceneObjectDecl.h>
//#include <SceneObject.h>

typedef std::vector<MeshInstanceUUID> RenderableMeshes;
typedef std::unordered_map<RenderPassInstanceUUID, std::vector<MaterialInstanceUUID>> RenderableMaterials; //RenderPassInstanceUUID -> MaterialInstanceUUID
typedef std::unordered_map<RenderPassInstanceUUID, std::vector<std::vector<ConstantsBufferUUID>>> RenderableConstantsBuffer; //RenderPassUUID -> vector:vector:ConstantsBufferUUID
typedef std::unordered_map<RenderPassInstanceUUID, std::vector<CComPtr<ID3D12RootSignature>>> RenderableRootSignatures; //RenderPassUUID -> vector:RootSignature
typedef std::unordered_map<RenderPassInstanceUUID, std::vector<CComPtr<ID3D12PipelineState>>> RenderablePipelineStates; //RenderPassUUID -> vector:PipelineState

static nlohmann::json defaultShadowMapShaderAttributes = { { "uniqueMaterialInstance", false }, { "castShadows",false }, { "ibl", false} };
#if defined(_EDITOR)
static nlohmann::json defaultPickingShaderAttributes = { { "uniqueMaterialInstance", true}, {"castShadows", false}, {"ibl" , false } };
#endif

using namespace Game;

namespace Scene
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#endif

	struct Renderable : SceneObject
	{
		inline static const SceneObjectType sceneObjectType = SO_Renderables;
		inline static const std::string fallbackMaterialName = "BaseLighting";

#include <Attributes/JFlags.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <RenderableAtt.h>
#include <JEnd.h>

		Renderable(nlohmann::json& json);
		~Renderable() { Destroy(); }

#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif

		XMVECTOR rotationQ();
		XMMATRIX world();

		virtual void Initialize();
		//Binding
		virtual void BindToScene();
		virtual void Bind(JUUID uuid);
		void BindCameras();
		void BindCamera(JUUID cuuid);
		void BindShadowMapCameras();
		//Unbinding
		virtual void UnbindFromScene();
		virtual void Unbind(JUUID uuid);
		void UnbindCameras();
		void UnbindCamera(JUUID cuuid);
		void UnbindMaterialsChangesCallback();
		void UnbindModelChangesCallback();

		//Render Passes
		std::vector<RenderPassInstanceUUID> GetCameraRenderPasses(CameraUUID cam);

		//Meshes
		void CreateMeshInstances();

		//Materials
		void CreateMaterialsInstances(CameraUUID cam);
		void CreateRenderPassMaterialsInstances(RenderPassInstanceUUID pass);
		void DestroyMaterialsInstances(CameraUUID cam);
		void DestroyRenderPassMaterialsInstances(RenderPassInstanceUUID pass);
		//Constants Buffers
		void CreateConstantsBuffersInstances(CameraUUID cam);
		void CreateRenderPassConstantsBuffersInstances(RenderPassInstanceUUID pass);
		void DestroyConstantsBuffersInstances(CameraUUID cam);
		void DestroyRenderPassConstantsBuffersInstances(RenderPassInstanceUUID pass);
		//Root Signatures
		void CreateRootSignatures(CameraUUID cam);
		void CreateRenderPassRootSignatures(RenderPassInstanceUUID rp);
		void DestroyRootSignatures(CameraUUID cam);
		void DestroyRenderPassRootSignatures(RenderPassInstanceUUID rp);
		//Pipeline States
		void CreatePipelineStates(CameraUUID cam);
		void CreateRenderPassPipelineStates(RenderPassInstanceUUID rp);
		void DestroyPipelineStates(CameraUUID cam);
		void DestroyRenderPassPipelineStates(RenderPassInstanceUUID rp);

		//void RebuildMeshMaterials();

		void CreateBoundingBox();
		BoundingBox GetBoundingBox();

		void WriteMaterialVariablesToConstantsBufferSpace(MaterialInstanceUUID material, ConstantsBufferUUID cbvData, unsigned int cbvFrameIndex);
		template<typename T>
		void WriteConstantsBuffer(std::string constantName, T& data, unsigned int backbufferIndex, unsigned int slot = 0U, size_t offset = 0ULL)
		{
			for (auto& [rp, meshMaterials] : materials)
			{
				for (unsigned int mesh = 0; mesh < meshMaterials.size(); mesh++)
				{
					auto& vsVars = meshMaterials.at(mesh)->vertexShaderInstanceUUID->constantsBuffersVariables;
					auto& psVars = meshMaterials.at(mesh)->pixelShaderInstanceUUID->constantsBuffersVariables;
					auto& cbuffers = constantsBuffers.at(rp).at(mesh);

					if (vsVars.contains(constantName)) {
						auto& vsVar = vsVars.at(constantName);
						if (cbuffers.size() > vsVar.bufferIndex)
						{
							cbuffers[vsVar.bufferIndex]->push<T>(data, backbufferIndex, vsVar.offset + vsVar.size * slot + offset);
						}
					}
					if (psVars.contains(constantName)) {
						auto& psVar = psVars.at(constantName);
						if (cbuffers.size() > psVar.bufferIndex)
						{
							cbuffers[psVar.bufferIndex]->push<T>(data, backbufferIndex, psVar.offset + psVar.size * slot + offset);
						}
					}
				}
			}
		};
		void WriteAnimationConstantsBuffer(unsigned int frame);
		void WriteConstantsBuffer(unsigned int frame);

		//Animation
		void CreateAnimationSequences();
		void RebuildAnimationSequences();
		void SetCurrentAnimation(SequencePlayer* sequencePlayer);
		void SetCurrentAnimation(std::string anim, float startTime = 0.0f, float timeFactor = 1.0f, bool play = true, bool loop = false);
		void StepAnimation(double elapsedSeconds);

		//DESTROY
		void Destroy();

		void Render(SceneUnitId unit, RenderPassInstanceUUID renderPass, CameraUUID camera);

#if defined(_EDITOR)
		std::function<void()> OnPick;
		//Gizmo
		virtual bool CanInteractWithGizmo(ImGuizmo::OPERATION operation) { return true; }
#endif

		//Destroy
		bool markedForDelete = false;
		//Render
		bool renderReady = false;
		bool renderException = false;
		//Model3D
		Model3DInstanceUUID model3D;
		//Meshes
		RenderableMeshes meshes;
		RenderableMaterials materials;
		RenderableConstantsBuffer constantsBuffers;
		RenderableRootSignatures rootSignatures;
		RenderablePipelineStates pipelineStates;
		//Animations
		Model3DInstanceUUID animable;
		Animation::BonesTransformations bonesTransformation;
		XMMATRIX animationTransformation;
		AnimationSequences animationsSequences;
		SequencePlayer sequencePlayer;
		//Cameras
		std::set<CameraUUID> bindedCameras;

		BoundingBox boundingBox;
		RenderableBoundingBoxUUID boundingBoxCompute; //used for animables
	};

	SODECL_FULL(Renderable);

#include <TrackUUID/JDecl.h>
#include <RenderableAtt.h>
#include <JEnd.h>

	void RenderablesStep(SceneUnitId unit, float dt);
	void DestroyRenderables();
	void DestroyRenderables(SceneUnitId unit);
	void DeleteRenderable(JUUID uuid);
	void RunBoundingBoxComputeShaders(SceneUnitId unit);
	void RunBoundingBoxComputeShadersSolution(SceneUnitId unit);
#if defined(_EDITOR)
	void WriteRenderablesJson(nlohmann::json& json);
#endif
}
