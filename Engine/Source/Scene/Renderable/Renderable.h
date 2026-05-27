#pragma once
#include <Scene.h>
#include <SceneObject.h>
#include <Model3D/Model3D.h>
#include <Material/MeshMaterial.h>
#include <RenderPass/PassMaterialOverride.h>
#include <Sequence/AnimationSequences.h>
#include <Sequence/SequencePlayer.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
//#include <Renderable/RenderableBoundingBox.h>
#include <PhysicObject.h>
#include <NoV8.h>

typedef std::vector<MeshInstanceID> RenderableMeshes;
typedef std::unordered_map<RenderPassInstanceID, std::vector<MaterialInstanceID>> RenderableMaterials; //RenderPassInstanceID -> MaterialInstanceID
typedef std::unordered_map<RenderPassInstanceID, std::vector<std::vector<ConstantsBufferID>>> RenderableConstantsBuffer; //RenderPassUUID -> vector:vector:ConstantsBufferID
typedef std::unordered_map<RenderPassInstanceID, std::vector<CComPtr<ID3D12RootSignature>>> RenderableRootSignatures; //RenderPassUUID -> vector:RootSignature
typedef std::unordered_map<RenderPassInstanceID, std::vector<CComPtr<ID3D12PipelineState>>> RenderablePipelineStates; //RenderPassUUID -> vector:PipelineState

static nlohmann::json defaultShadowMapShaderAttributes = { { "uniqueMaterialInstance", false }, { "castShadows",false }, { "ibl", false} };
#if defined(_EDITOR)
static nlohmann::json defaultPickingShaderAttributes = { { "uniqueMaterialInstance", true}, {"castShadows", false}, {"ibl" , false } };
#endif

using namespace Game;
using namespace Physics;
using namespace nov8;

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

#include <Attributes/JStr2Flag.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <RenderableAtt.h>
#include <JEnd.h>

		//lifecycle
		Renderable(SceneUnitId id, nlohmann::json& json);
		~Renderable() { Destroy(); }

#if defined(_EDITOR)
		void DropJsonMoldAttributes(nlohmann::json& j) override;
#endif

		XMVECTOR positionV();
		void updateRotationQ();
		XMVECTOR rotationQ();
		void rotationQ(XMVECTOR Q);
		XMMATRIX world();

		void Initialize() override;
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
		std::vector<RenderPassInstanceID> GetCameraRenderPasses(CameraID cam);

		//Meshes
		void CreateMeshInstances();

		//Materials
		void CreateMaterialsInstances(CameraID cam);
		void CreateRenderPassMaterialsInstances(RenderPassInstanceID pass);
		void DestroyMaterialsInstances(CameraID cam);
		void DestroyRenderPassMaterialsInstances(RenderPassInstanceID pass);
		//Constants Buffers
		void CreateConstantsBuffersInstances(CameraID cam);
		void CreateRenderPassConstantsBuffersInstances(RenderPassInstanceID pass);
		void DestroyConstantsBuffersInstances(CameraID cam);
		void DestroyRenderPassConstantsBuffersInstances(RenderPassInstanceID pass);
		//Root Signatures
		void CreateRootSignatures(CameraID cam);
		void CreateRenderPassRootSignatures(RenderPassInstanceID rp);
		void DestroyRootSignatures(CameraID cam);
		void DestroyRenderPassRootSignatures(RenderPassInstanceID rp);
		//Pipeline States
		void CreatePipelineStates(CameraID cam);
		void CreateRenderPassPipelineStates(RenderPassInstanceID rp);
		void DestroyPipelineStates(CameraID cam);
		void DestroyRenderPassPipelineStates(RenderPassInstanceID rp);

		void CreateBoundingBox();
		bool HasBoundingBoxComputed();
		BoundingBox GetBoundingBox();

		void WriteMaterialVariablesToConstantsBufferSpace(MaterialInstanceID material, ConstantsBufferID cbvData, unsigned int cbvFrameIndex);
		template<typename T>
		void WriteConstantsBuffer(std::string constantName, T& data, unsigned int backbufferIndex, unsigned int slot = 0U, size_t offset = 0ULL)
		{
			if (constantsBuffersLock->load() == true)
				constantsBuffersLock->wait(true);

			constantsBuffersLock->store(true);

			for (auto& [rp, meshMaterials] : materials)
			{
				if (!constantsBuffers.contains(rp))
					continue;

				for (unsigned int mesh = 0; mesh < meshMaterials.size(); mesh++)
				{
					auto& vsVars = meshMaterials.at(mesh)->vertexShaderInstanceID->constantsBuffersVariables;
					auto& psVars = meshMaterials.at(mesh)->pixelShaderInstanceID->constantsBuffersVariables;
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

			constantsBuffersLock->store(false);
			constantsBuffersLock->notify_one();
		};
		void WriteAnimationConstantsBuffer(unsigned int frame);
		void WriteConstantsBuffer(unsigned int frame);

		//Animation
		void CreateAnimationSequences();
		void RebuildAnimationSequences();
		void SetCurrentAnimation(SequencePlayer* sequencePlayer);
		void SetCurrentAnimation(std::string anim, float startTime = 0.0f, float timeFactor = 1.0f, bool play = true, bool loop = false);
		void CreateAnimationThread();
		void StepAnimation(double elapsedSeconds);
		std::tuple<XMMATRIX, XMFLOAT3, XMFLOAT3, XMVECTOR, XMFLOAT3> GetBoneTransformation(std::string bone);
		std::vector<std::string> GetBones();

		//DESTROY
		void Destroy();

		void Render(SceneUnitId unit, RenderPassInstanceID renderPass, CameraID camera);
		bool RenderReady();
		void RenderReady(bool value);

		//Scripting
		std::vector<ScriptBinding> GetScriptBindings() override;
#if defined(_EDITOR)
		virtual std::map<std::string, ScriptBinding> GetScriptBindingOptions();
#endif

#if defined(_EDITOR)
		std::function<void()> OnPick;
		//Gizmo
		virtual bool CanInteractWithGizmo(ImGuizmo::OPERATION operation) { return true; }
		virtual void WriteJson(nlohmann::json& j);
#endif

		//State
		DeleteHook markedForDelete;
		unsigned int deleteFrames;
		bool renderReady = false;
		bool renderException = false;
		//Transformation
		XMVECTOR rotationQuaternion;
		//Model3D
		Model3DInstanceID model3D;
		//Meshes
		RenderableMeshes meshes;
		RenderableMaterials materials;
		RenderableConstantsBuffer constantsBuffers;
		std::unique_ptr<std::atomic_bool> constantsBuffersLock;
		RenderableRootSignatures rootSignatures;
		RenderablePipelineStates pipelineStates;
		//Animations
		float lastAnimationTime;
		bool forceAnimation;
		std::unique_ptr<std::atomic_bool> animationStepLock;
		std::unique_ptr<std::atomic_bool> animationThreadAlive;
		std::thread animationThread;
		Model3DInstanceID animable;
		Animation::BonesTransformations bonesTransformation;
		Animation::NodeTransformsMap globalNodeTransforms;
		XMMATRIX animationTransformation;
		AnimationSequences animationsSequences;
		SequencePlayer sequencePlayer;
		Animation::BonesTransformations sequenceBoneTransformations;
		//Cameras
		std::set<CameraID> bindedCameras;

		BoundingBox boundingBox;
		//RenderableBoundingBoxID boundingBoxCompute; //used for animables
	};

	SODECL_FULL(Renderable);

#include <TrackUUID/JDecl.h>
#include <RenderableAtt.h>
#include <JEnd.h>

	void RenderablesStep(SceneUnitId id, float dt);
	void DestroyRenderables();
	void DestroyRenderables(SceneUnitId id);
	void DeleteRenderable(SceneUnitId id, JUUID uuid);
	//void RunBoundingBoxComputeShaders(SceneUnitId id);
	//void RunBoundingBoxComputeShadersSolution(SceneUnitId id);
#if defined(_EDITOR)
	void WriteRenderablesJson(SceneUnitId id, nlohmann::json& json);
#endif
}

using namespace Scene;
DEF_SCENEOBJECT_ID_HASH(Renderable);