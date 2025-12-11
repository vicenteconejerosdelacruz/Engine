#pragma once
#include <nlohmann/json.hpp>
#include <set>
#include <Model3D/Model3D.h>
#include <Material/MeshMaterial.h>
#include <RenderPass/PassMaterialOverride.h>
#include <SceneObjectDecl.h>
#include <SceneObject.h>
#include <Sequence/AnimationSequences.h>
#include <Sequence/SequencePlayer.h>
#include <Controller.h>

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

	//UPDATE
	void RenderablesStep();
	void RunBoundingBoxComputeShaders();
	void RunBoundingBoxComputeShadersSolution();

	//DELETE
	void DestroyRenderables();
	void DeleteRenderable(std::string uuid);

	//EDITOR
#if defined(_EDITOR)
	void WriteRenderablesJson(nlohmann::json& json);
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

		//Model3D
		Model3DInstanceUUID model3D;
		//meshes
		RenderableMeshes meshes;
		RenderableMaterials materials;
		RenderableConstantsBuffer constantsBuffers;
		RenderableRootSignatures rootSignatures;
		RenderablePipelineStates pipelineStates;
		std::set<CameraUUID> bindedCameras;

		virtual void Initialize();
		virtual void Bind(JUUID uuid);
		virtual void Unbind(JUUID uuid);
		virtual void BindToScene();
		virtual void UnbindFromScene();
		void BindCameras();
		void BindCamera(JUUID cuuid);
		void UnbindCameras();
		void UnbindCamera(JUUID cuuid);
		void BindShadowMapCameras();
		void CreateMeshInstances();

		std::vector<RenderPassInstanceUUID> GetCameraRenderPasses(CameraUUID cam);
		//Materials
		void CreateMaterialsInstances(CameraUUID cam);
		void DestroyMaterialsInstances(CameraUUID cam);
		void CreateRenderPassMaterialsInstances(RenderPassInstanceUUID pass);
		void DestroyRenderPassMaterialsInstances(RenderPassInstanceUUID pass);
		//Constants Buffers
		void CreateConstantsBuffersInstances(CameraUUID cam);
		void DestroyConstantsBuffersInstances(CameraUUID cam);
		void CreateRenderPassConstantsBuffersInstances(RenderPassInstanceUUID pass);
		void DestroyRenderPassConstantsBuffersInstances(RenderPassInstanceUUID pass);
		//Root Signatures
		void CreateRootSignatures(CameraUUID cam);
		void DestroyRootSignatures(CameraUUID cam);
		void CreateRenderPassRootSignatures(RenderPassInstanceUUID rp);
		void DestroyRenderPassRootSignatures(RenderPassInstanceUUID rp);
		//Pipeline States
		void CreatePipelineStates(CameraUUID cam);
		void DestroyPipelineStates(CameraUUID cam);
		void CreateRenderPassPipelineStates(RenderPassInstanceUUID rp);
		void DestroyRenderPassPipelineStates(RenderPassInstanceUUID rp);
		void RebuildMeshMaterials();
		//ANIMATION
		Model3DInstanceUUID animable;
		Animation::BonesTransformations bonesTransformation;
		XMMATRIX animationTransformation;

		BoundingBox boundingBox;
		RenderableBoundingBoxUUID boundingBoxCompute; //used for animables
		void CreateBoundingBox();
		BoundingBox GetBoundingBox();

		//UPDATEs
#if defined(_EDITOR)
		std::function<void()> OnPick;
		//Gizmo
		virtual bool CanInteractWithGizmo(ImGuizmo::OPERATION operation) { return true; }
#endif
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
		void WriteAnimationConstantsBuffer();
		void WriteAnimationConstantsBuffer(unsigned int backbufferIndex);
		void WriteConstantsBuffer();
		void WriteConstantsBuffer(unsigned int backbufferIndex);

		//Animation
		AnimationSequences animationsSequences;
		SequencePlayer sequencePlayer;
		void CreateAnimationSequences();
		void RebuildAnimationSequences();
		void SetCurrentAnimation(SequencePlayer* sequencePlayer);
		void SetCurrentAnimation(std::string anim, float startTime = 0.0f, float timeFactor = 1.0f, bool play = true, bool loop = false);
		void StepAnimation(double elapsedSeconds);

		//DESTROY
		bool markedForDelete = false;
		void Destroy();

		bool renderException = false;
		void Render(RenderPassInstanceUUID renderPass, CameraUUID camera);
		void UnbindMaterialsChangesCallback();
		void UnbindModelChangesCallback();
	};

	SODECL_FULL(Renderable);

#include <TrackUUID/JDecl.h>
#include <RenderableAtt.h>
#include <JEnd.h>
}
