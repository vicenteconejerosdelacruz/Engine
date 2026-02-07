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

		//lifecycle
		Renderable(SceneUnitId id, nlohmann::json& json);
		~Renderable() { Destroy(); }
		virtual void Initialize();
		virtual void SetInitialConditions();
		virtual void BindToScene();
		virtual void Bind(JUUID uuid);
		void BindCameras();
		void BindCamera(JUUID cuuid);
		void BindShadowMapCameras();
		virtual void UnbindFromScene();
		virtual void Unbind(JUUID uuid);
		void UnbindCameras();
		void UnbindCamera(JUUID cuuid);

		//Render Passes
		std::vector<RenderPassInstanceUUID> GetCameraRenderPasses(CameraSUUUID cam);

		//Meshes
		void CreateMeshInstances();

		//Materials
		void CreateMaterialsInstances(CameraSUUUID cam);
		void CreateRenderPassMaterialsInstances(RenderPassInstanceUUID pass);
		void DestroyMaterialsInstances(CameraSUUUID cam);
		void DestroyRenderPassMaterialsInstances(RenderPassInstanceUUID pass);
		//Constants Buffers
		void CreateConstantsBuffersInstances(CameraSUUUID cam);
		void CreateRenderPassConstantsBuffersInstances(RenderPassInstanceUUID pass);
		void DestroyConstantsBuffersInstances(CameraSUUUID cam);
		void DestroyRenderPassConstantsBuffersInstances(RenderPassInstanceUUID pass);
		//Root Signatures
		void CreateRootSignatures(CameraSUUUID cam);
		void CreateRenderPassRootSignatures(RenderPassInstanceUUID rp);
		void DestroyRootSignatures(CameraSUUUID cam);
		void DestroyRenderPassRootSignatures(RenderPassInstanceUUID rp);
		//Pipeline States
		void CreatePipelineStates(CameraSUUUID cam);
		void CreateRenderPassPipelineStates(RenderPassInstanceUUID rp);
		void DestroyPipelineStates(CameraSUUUID cam);
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

		void Render(SceneUnitId unit, RenderPassInstanceUUID renderPass, CameraSUUUID camera);
		bool RenderReady();
		void RenderReady(bool value);

		XMVECTOR rotationQ();
		XMMATRIX world();

#if defined(_EDITOR)
		std::function<void()> OnPick;
		//Gizmo
		virtual bool CanInteractWithGizmo(ImGuizmo::OPERATION operation) { return true; }
		virtual void WriteJson(nlohmann::json& j);
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
		std::set<CameraSUUUID> bindedCameras;
		//RebuildMaterials
		//std::vector<std::tuple<unsigned int, JUUID>> rebuildMaterials;

		BoundingBox boundingBox;
		RenderableBoundingBoxUUID boundingBoxCompute; //used for animables
	};

	SODECL_FULL(Renderable);

#include <TrackUUID/JDecl.h>
#include <RenderableAtt.h>
#include <JEnd.h>

	void RenderablesStep(SceneUnitId id, float dt);
	void DestroyRenderables();
	void DestroyRenderables(SceneUnitId id);
	void DeleteRenderable(SceneUnitId id, JUUID uuid);
	void RunBoundingBoxComputeShaders(SceneUnitId id);
	void RunBoundingBoxComputeShadersSolution(SceneUnitId id);
#if defined(_EDITOR)
	void WriteRenderablesJson(SceneUnitId id, nlohmann::json& json);
#endif
}
