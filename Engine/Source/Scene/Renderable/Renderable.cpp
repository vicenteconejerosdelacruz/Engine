#include "pch.h"
#include <vector>
#include "Renderable.h"
#include <Scene.h>
#include <Camera/Camera.h>
#include <Light/Light.h>
#include <Model3D/Model3D.h>
#include <Mesh/Mesh.h>
#include <Material/MeshMaterial.h>
#include <Renderer.h>
#include <RenderPass/RenderPass.h>
#include <DeviceUtils/RootSignature/RootSignature.h>
#include <DeviceUtils/PipelineState/PipelineState.h>
#include <Renderable/RenderableBoundingBox.h>
#include <SceneObjectDef.h>

extern std::unique_ptr<Renderer> renderer;
extern DX::StepTimer timer;

#if defined(_EDITOR)
namespace Editor
{
	extern void SelectRenderable(JUUID ruuid);
	extern void BindRenderableToPickingPass(RenderableUUID r);
	extern void UnbindRenderableFromPickingPass(RenderableUUID r);
	extern bool IsPlaying();
	extern bool IsPaused();
};
#endif

namespace Scene
{
	SODEF_FULL(Renderable);

#include <TrackUUID/JDef.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#endif

	std::unordered_map<RenderableUUID, SequencePlayer*> animationPlayers;

	void RenderablesStep()
	{
#if defined(_EDITOR)
		using namespace ComputeShader;

		auto& Renderables = GetRenderables();
		std::set<RenderableUUID> r;
		std::transform(Renderables.begin(), Renderables.end(), std::inserter(r, r.begin()), [](auto o) { return o; });

		std::set<RenderableUUID> meshes;
		std::copy_if(r.begin(), r.end(), std::inserter(meshes, meshes.begin()), [](auto r)
			{
				return (r->dirty(Renderable::Update_meshMaterials));
			}
		);

		std::set<RenderableUUID> models;
		std::copy_if(r.begin(), r.end(), std::inserter(models, models.begin()), [](auto r)
			{
				return (r->dirty(Renderable::Update_model));
			}
		);

		std::set<RenderableUUID> bindToCam;
		std::copy_if(r.begin(), r.end(), std::inserter(bindToCam, bindToCam.end()), [](auto r)
			{
				if (r->dirty(Renderable::Update_cameras))
				{
					if (r->UpdatePrevValues.contains("cameras"))
					{
						nlohmann::json prevCams = r->UpdatePrevValues.at("cameras");
						std::set<std::string> prevCamUUIDs;
						for (auto& cam : prevCams) {
							if (cam != "") prevCamUUIDs.insert(cam);
						}
						std::vector<std::string> currCams = r->cameras();
						std::set<std::string> currCamUUIDs;
						for (auto& cam : currCams) {
							if (cam != "") currCamUUIDs.insert(cam);
						}
						bool isDifferent = prevCamUUIDs != currCamUUIDs;
						if (!isDifferent)
							r->clean(Renderable::Update_cameras);
						return isDifferent;
					}
					return true;
				}
				return false;
			}
		);

		std::set<RenderableUUID> todelete;
		std::copy_if(r.begin(), r.end(), std::inserter(todelete, todelete.begin()), [](auto r)
			{
				return r->markedForDelete;
			}
		);

		bool criticalFrame = meshes.size() > 0ULL || models.size() > 0ULL || bindToCam.size() > 0ULL || todelete.size() > 0ULL;

		if (criticalFrame)
		{
			renderer->Flush();
			renderer->RenderCriticalFrame([&meshes, &models, &bindToCam, &todelete]
				{
					for (auto r : meshes)
					{
						nlohmann::json patch = { {"model",""} };
						r->merge_patch(patch);
						r->RebuildMeshMaterials();
						r->BindToScene();
						r->clean(Renderable::Update_meshMaterials);
					}
					for (auto r : models)
					{
						EraseRenderableFromAnimables(r());
						nlohmann::json patch = { {"meshMaterials", nlohmann::json::array({})} };
						r->merge_patch(patch);
						r->RebuildMeshMaterials();
						if (!r->animable.empty())
						{
							AttachAnimation(r(), r->model3D->animations);
							r->StepAnimation(0.0f);
							r->boundingBoxCompute = CreateRenderableBoundingBox(r());
						}
						r->BindToScene();
						r->clean(Renderable::Update_model);
					}

					for (auto r : bindToCam)
					{
						std::set<std::string> currCamsUUIDs;
						std::vector<std::string> cameras = r->cameras();
						for (auto& cam : cameras) {
							if (cam != "") currCamsUUIDs.insert(cam);
						}

						std::set<std::string> prevCamsUUIDs;
						if (r->UpdatePrevValues.contains("cameras"))
						{
							nlohmann::json prevCams = r->UpdatePrevValues.at("cameras");
							for (auto& cam : prevCams) {
								if (cam != "") prevCamsUUIDs.insert(cam);
							}
						}

						//get cams present in the current set, but not present in the last set(this means adding cams)
						std::set<std::string> addCams;
						std::set_difference(
							currCamsUUIDs.begin(), currCamsUUIDs.end(),
							prevCamsUUIDs.begin(), prevCamsUUIDs.end(),
							std::inserter(addCams, addCams.begin())
						);

						//get cams present in the previous set, but not present in the current set(this means deleting cams)
						std::set<std::string> delCams;
						std::set_difference(
							prevCamsUUIDs.begin(), prevCamsUUIDs.end(),
							currCamsUUIDs.begin(), currCamsUUIDs.end(),
							std::inserter(delCams, delCams.begin())
						);

						//remove the camera from the renderable's binded camera set
						for (auto& uuid : delCams)
						{
							Scene::UnbindFromScene(r->Juuid(), uuid);
						}
						//add the camera to the renderable's binded camera set
						for (auto& uuid : addCams)
						{
							Scene::BindToScene(r->Juuid(), uuid);
						}

						r->clean(Renderable::Update_cameras);
					}

					for (auto r : todelete)
					{
						EraseRenderableFromRenderables(r());
						EraseRenderableFromAnimables(r());
						EraseRenderableFromShadowCasts(r());
						DeleteRenderableSceneObject(r());
					}
				}
			);
		}
#endif
		float dt = static_cast<float>(timer.GetElapsedSeconds());
		for (auto& [renderable, player] : animationPlayers)
		{
#if defined(_EDITOR)
			if (Editor::IsPlaying() && !Editor::IsPaused())
#endif
			{
				player->Step(dt * 1000.0f);
			}
			player->ApplyFrameValues(renderable);
		}
	}

	void RunBoundingBoxComputeShaders()
	{
		for (RenderableUUID renderable : Renderables)
		{
			if (!renderable->boundingBoxCompute.empty())
			{
				renderable->boundingBoxCompute->Compute();
			}
		}
	}

	void RunBoundingBoxComputeShadersSolution()
	{
		for (RenderableUUID renderable : Renderables)
		{
			if (!renderable->boundingBoxCompute.empty())
			{
				renderable->boundingBoxCompute->Solution();
			}
		}
	}

	void DestroyRenderables()
	{
		auto uuids = nostd::GetUUIDS(RenderablesceneObjects);
		for (auto& uuid : uuids)
		{
			DeleteRenderableSceneObject(uuid);
		}

#include <TrackUUID/JClear.h>
#include <RenderableAtt.h>
#include <JEnd.h>
	}

	void DeleteRenderable(std::string uuid)
	{
		RenderableUUID r = uuid;
		r->markedForDelete = true;
	}

	//EDITOR
#if defined(_EDITOR)
	void WriteRenderablesJson(nlohmann::json& json)
	{
#include <Editor/JSaveFile.h>
#include <RenderableAtt.h>
#include <JEnd.h>
	}
#endif

	Renderable::Renderable(nlohmann::json& json) :SceneObject(json)
	{
#include <Attributes/JInit.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <RenderableAtt.h>
#include <JEnd.h>
	}

	void Renderable::Initialize()
	{
		using namespace Animation;
		using namespace ComputeShader;
		CreateMeshInstances(); //why here, this is a special case, as Animables depends of animables which is created in this function

#include <TrackUUID/JInsert.h>
#include <RenderableAtt.h>
#include <JEnd.h>

		animationTransformation = XMMatrixIdentity();

		if (!animable.empty())
		{
			AttachAnimation(uuid(), model3D->animations);
			StepAnimation(0.0f); //take an empty T-Pose step so the skinning can be performed
			boundingBoxCompute = CreateRenderableBoundingBox(uuid());
			WriteAnimationConstantsBuffer(renderer->backBufferIndex);
			sequencePlayer.renderable = Juuid();
		}
#if defined(_EDITOR)
		OnPick = [this] { Editor::SelectRenderable(uuid()); };
#endif
	}

	void Renderable::Bind(JUUID uuid)
	{
		switch (GetSceneObjectType(uuid))
		{
		case SO_Cameras:
		{
			bindedCameras.insert(uuid);
		}
		break;
		}
	}

	void Renderable::Unbind(JUUID uuid)
	{
		bindedCameras.erase(uuid);
	}

	void Renderable::BindToScene()
	{
#include <TrackUUID/JInsert.h>
#include <RenderableAtt.h>
#include <JEnd.h>

		BindCameras();
	}

	void Renderable::BindCameras()
	{
		auto cams = cameras();
		for (auto& uuid : cams) {
			if (bindedCameras.contains(uuid))
				continue;
			BindCamera(uuid);
		}
	}

	void Renderable::BindCamera(JUUID cuuid)
	{
		Scene::BindToScene(uuid(), cuuid);
	}

	void Renderable::UnbindFromScene()
	{
		using namespace ComputeShader;

#include <TrackUUID/JErase.h>
#include <RenderableAtt.h>
#include <JEnd.h>

		UnbindMaterialsChangesCallback();
		UnbindModelChangesCallback();
		Scene::UnbindFromScene(uuid());
		if (!boundingBoxCompute.empty())
		{
			DeleteRenderableBoundingBox(boundingBoxCompute());
			boundingBoxCompute.clear();
		}
		if (animationPlayers.contains(uuid()))
		{
			animationPlayers.erase(uuid());
		}
	}

	void Renderable::UnbindCameras()
	{
		auto cams = cameras();
		for (auto& uuid : cams) {
			if (!bindedCameras.contains(uuid))
				continue;
			UnbindCamera(uuid);
		}
	}

	void Renderable::UnbindCamera(JUUID cuuid)
	{
		Scene::UnbindFromScene(uuid(), cuuid);
	}

	void Renderable::BindShadowMapCameras()
	{
		if (!castShadows()) return;

		auto lights = GetShadowMapLights();
		for (LightUUID light : lights) {
			for (CameraUUID cam : light->shadowMapCameras)
			{
				BindCamera(cam());
			}
		}
	}

	void Renderable::UnbindMaterialsChangesCallback()
	{
		for (auto& [rp, vec0] : materials)
		{
			for (auto& mat : vec0)
			{
				mat->materialUUID->UnbindChangeCallback(uuid());
			}
		}
	}

	void Renderable::UnbindModelChangesCallback()
	{
		if (!model().empty())
		{
			Model3DJsonUUID mdl = model();
			mdl->UnbindChangeCallback(uuid());
		}
	}

	XMVECTOR Renderable::rotationQ()
	{
		XMFLOAT3 rotV = rotation();
		float roll, pitch, yaw;
		pitch = rotV.x; yaw = rotV.y; roll = rotV.z;
		XMVECTOR rotQ = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
		return rotQ;
	}

	XMMATRIX Renderable::world()
	{
		XMFLOAT3 posV = position();
		XMFLOAT3 scaleV = scale();
		XMMATRIX rotationM = XMMatrixRotationQuaternion(rotationQ());
		XMMATRIX scaleM = XMMatrixScalingFromVector({ scaleV.x, scaleV.y, scaleV.z });
		XMMATRIX positionM = XMMatrixTranslationFromVector({ posV.x, posV.y, posV.z });
		XMMATRIX worldM = XMMatrixMultiply(XMMatrixMultiply(scaleM, rotationM), positionM);
		return (!animationUseTransformation()) ? worldM : XMMatrixMultiply(animationTransformation, worldM);
	}

	void Renderable::CreateMeshInstances()
	{
		using namespace Templates;
		if (!meshMaterials().empty())
		{
			std::vector<MeshMaterial> rmm = meshMaterials();
			std::vector<MeshMaterial> mm;
			std::copy_if(rmm.begin(), rmm.end(), std::back_inserter(mm), [](const MeshMaterial& mm)
				{
					return mm.mesh != "" && mm.materialUUID != "";
				}
			);

			std::transform(mm.begin(), mm.end(), std::back_inserter(meshes), [](const MeshMaterial& m)
				{
					return GetMeshInstance(m.mesh)->uuid;
				}
			);
			CreateBoundingBox();
		}
		else if (Model3DTemplateExist(model()))
		{
			CreateModel3DInstance(model(), [this]
				{
					return std::make_unique<Model3DInstance>(model(), uuid(), [this](JUUID model3DTemplateUUID)
						{
							auto& model = GetModel3DTemplate(model3DTemplateUUID);
							if (model->dirty(Model3DJson::Update_animationSequences))
							{
								RebuildAnimationSequences();
							}
						}
					);
				}
			);
			model3D = model();
			meshes = model3D->meshes;
			if (model3D->animations)
				animable = model3D;

			if (animable.empty())
			{
				CreateBoundingBox();
			}
			else
			{
				CreateAnimationSequences();
			}
		}
	}

	std::vector<RenderPassInstanceUUID> Renderable::GetCameraRenderPasses(CameraUUID cam)
	{
		if (!cam->useSwapChain()) return cam->renderPassesUUID;

		std::vector<RenderPassInstanceUUID> rpiv = cam->renderPassesUUID;
		rpiv.push_back(renderer->swapChainPass);
		return rpiv;
	}

	void Renderable::CreateMaterialsInstances(CameraUUID cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			CreateRenderPassMaterialsInstances(rp);
		}
	}

	void Renderable::DestroyMaterialsInstances(CameraUUID cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			DestroyRenderPassMaterialsInstances(rp);
		}
	}

	void Renderable::CreateRenderPassMaterialsInstances(RenderPassInstanceUUID pass)
	{
		auto onPostMaterialChange = [this](unsigned int index, unsigned int total)
			{
				if (index == 0U)
				{
					renderer->Flush();
					renderer->ResetCommands();
					renderer->SetCSUDescriptorHeap();
				}

				RebuildMeshMaterials();

				if (index >= (total - 1))
				{
					renderer->CloseCommandsAndFlush();
				}
			};

		std::vector<PassMaterialOverride> pmo = passMaterialOverrides();

		if (materials.contains(pass))
		{
			return;
		}

		if (!meshMaterials().empty())
		{
			std::vector<MeshMaterial> rmm = meshMaterials();
			std::vector<MeshMaterial> mm;
			std::copy_if(rmm.begin(), rmm.end(), std::back_inserter(mm), [](const MeshMaterial& mm)
				{
					return mm.mesh != "" && mm.materialUUID != "";
				}
			);

			for (unsigned i = 0; i < mm.size(); i++)
			{
				std::vector<PassMaterialOverride> mpmo;
				std::copy_if(pmo.begin(), pmo.end(), std::back_inserter(mpmo), [i](PassMaterialOverride& o)
					{
						return o.meshIndex == i;
					}
				);
				auto& mesh = meshes.at(i);
				JUUID matUUID = mm.at(i).materialUUID;
				if (!MaterialTemplateExist(matUUID))
				{
					matUUID = GetMaterialUUIDByName(fallbackMaterialName); //fallback
				}
				materials[pass].push_back(pass->GetRenderPassMaterialInstance(
					matUUID, mesh, shadowed(),
					mpmo, uuid(), nullptr, onPostMaterialChange));
			}
		}
		else if (!model3D.empty())
		{
			for (unsigned int i = 0; i < meshes.size(); i++)
			{
				std::vector<PassMaterialOverride> mpmo;
				std::copy_if(pmo.begin(), pmo.end(), std::back_inserter(mpmo), [i](PassMaterialOverride& o)
					{
						return o.meshIndex == i;
					}
				);
				auto& mesh = meshes.at(i);
				JUUID matUUID = model3D->materialUUIDs.at(i);
				materials[pass].push_back(pass->GetRenderPassMaterialInstance(matUUID, mesh, shadowed(),
					mpmo, uuid(), nullptr, onPostMaterialChange));
			}
		}
	}

	void Renderable::DestroyRenderPassMaterialsInstances(RenderPassInstanceUUID rp)
	{
		if (!materials.contains(rp))
		{
			return;
		}
		for (auto& m : materials.at(rp))
		{
			DeleteMaterialInstance(m());
		}
		materials.erase(rp);
	}

	void Renderable::CreateConstantsBuffersInstances(CameraUUID cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			CreateRenderPassConstantsBuffersInstances(rp);
		}
	}

	void Renderable::DestroyConstantsBuffersInstances(CameraUUID cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			DestroyRenderPassConstantsBuffersInstances(rp);
		}
	}

	void Renderable::CreateRenderPassConstantsBuffersInstances(RenderPassInstanceUUID pass)
	{
		if (constantsBuffers.contains(pass))
			return;

		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			constantsBuffers[pass].push_back({});
			auto& mi = materials[pass].at(i);
			auto& mesh = meshes.at(i);
			for (unsigned int j = 0; j < mi->variablesBufferSize.size(); j++)
			{
				size_t size = mi->variablesBufferSize[j];
				ConstantsBufferUUID cbuffer = CreateConstantsBuffer(size, name() + "." + std::to_string(j) + "." + mesh->uuid);
				for (unsigned int n = 0; n < renderer->numFrames; n++)
				{
					WriteMaterialVariablesToConstantsBufferSpace(mi, cbuffer, n);
				}
				constantsBuffers[pass][i].push_back(cbuffer);
			}
		}
	}

	void Renderable::DestroyRenderPassConstantsBuffersInstances(RenderPassInstanceUUID pass)
	{
		if (!constantsBuffers.contains(pass)) return;

		for (unsigned int i = 0; i < constantsBuffers.at(pass).size(); i++)
		{
			for (unsigned int c = 0; c < constantsBuffers.at(pass).at(i).size(); c++)
			{
				DestroyConstantsBuffer(constantsBuffers.at(pass).at(i)[c]());
			}
		}
		constantsBuffers.erase(pass);
	}

	void Renderable::CreateRootSignatures(CameraUUID cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			if (rootSignatures.contains(rp)) rootSignatures.erase(rp);
			CreateRenderPassRootSignatures(rp);
		}
	}

	void Renderable::DestroyRootSignatures(CameraUUID cam)
	{
		if (cam.empty()) return;
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			DestroyRenderPassRootSignatures(rp);
		}
	}

	void Renderable::CreateRenderPassRootSignatures(RenderPassInstanceUUID rp)
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			auto& mi = materials[rp].at(i);

			auto& vsCBparams = mi->vertexShaderInstanceUUID->constantsBuffersParameters;
			auto& psCBparams = mi->pixelShaderInstanceUUID->constantsBuffersParameters;
			auto& uavParams = mi->pixelShaderInstanceUUID->uavParameters;
			auto& psSRVCSparams = mi->pixelShaderInstanceUUID->srvCSParameters;
			auto& psSRVTexparams = mi->pixelShaderInstanceUUID->srvTexParameters;
			auto& psSamplersParams = mi->pixelShaderInstanceUUID->samplersParameters;
			auto& samplers = mi->samplers;

			std::string rsName = "rootSignature:" + name() + ":" + std::to_string(i);
			rootSignatures[rp].push_back(
				CreateRootSignature(rsName, vsCBparams, psCBparams, uavParams, psSRVCSparams, psSRVTexparams, psSamplersParams, samplers)
			);
		}
	}

	void Renderable::DestroyRenderPassRootSignatures(RenderPassInstanceUUID rp)
	{
		if (rootSignatures.contains(rp))
			rootSignatures.erase(rp);
	}

	void Renderable::CreatePipelineStates(CameraUUID cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			if (pipelineStates.contains(rp)) pipelineStates.erase(rp);
			CreateRenderPassPipelineStates(rp);
		}
	}

	void Renderable::DestroyPipelineStates(CameraUUID cam)
	{
		if (cam.empty()) return;
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			DestroyRenderPassPipelineStates(rp);
		}
	}

	void Renderable::CreateRenderPassPipelineStates(RenderPassInstanceUUID rp)
	{
		auto rtFormats = rp->GetRenderTargetsFormats();
		auto depthFormat = rp->GetDepthStencilFormat();

		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			auto& mesh = meshes.at(i);
			auto& mi = materials[rp].at(i);
			auto& vsLayout = vertexInputLayoutsMap[mesh->vertexClass];
			auto& rootSignature = rootSignatures[rp].at(i);
			auto& vsByteCode = mi->vertexShaderInstanceUUID->byteCode;
			auto& psByteCode = mi->pixelShaderInstanceUUID->byteCode;

			auto& material = mi->materialUUID;
			BlendDesc blendDesc = material->blendState();
			RasterizerDesc rasterizerDesc = material->rasterizerState();

			D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType = D3D_PRIMITIVE_TOPOLOGYToD3D12_PRIMITIVE_TOPOLOGY_TYPE.at(topology());

			std::string plName = "pipelineState:" + name() + ":" + std::to_string(i);
			pipelineStates[rp].push_back(
				CreateGraphicsPipelineState(plName, vsLayout, vsByteCode, psByteCode, rootSignature, blendDesc, rasterizerDesc, primitiveTopologyType, rtFormats, depthFormat)
			);
		}
	}

	void Renderable::DestroyRenderPassPipelineStates(RenderPassInstanceUUID rp)
	{
		if (pipelineStates.contains(rp))
		{
			for (auto& ps : pipelineStates.at(rp))
			{
				ps = nullptr;
			}
			pipelineStates.erase(rp);
		}
	}

	void Renderable::RebuildMeshMaterials()
	{
		using namespace ComputeShader;

		renderException = false;
#if defined(_EDITOR)
		Editor::UnbindRenderableFromPickingPass(uuid());
#endif
		Destroy();
		try
		{
			CreateMeshInstances();
			if (!animable.empty())
			{
				boundingBoxCompute = CreateRenderableBoundingBox(uuid());
				WriteAnimationConstantsBuffer(renderer->backBufferIndex);
			}
			for (auto& cam : bindedCameras)
			{
				CreateMaterialsInstances(cam);
				CreateConstantsBuffersInstances(cam);
				CreateRootSignatures(cam);
				CreatePipelineStates(cam);
			}
#if defined(_EDITOR)
			Editor::BindRenderableToPickingPass(uuid());
#endif
		}
		catch (...)
		{
			renderException = true;
		}
	}

	void Renderable::CreateBoundingBox()
	{
		using namespace Templates;

		bool extend = false;
		for (auto& mesh : meshes)
		{
			mesh->ExtendBoundingBox(boundingBox, extend);
			extend = true;
		}
	}

	BoundingBox Renderable::GetBoundingBox()
	{
		BoundingBox& bb = animable.empty() ? boundingBox : boundingBoxCompute->boundingBox;
		BoundingBox bbw;
		bb.Transform(bbw, world());
		return bbw;
	}

	void Renderable::WriteMaterialVariablesToConstantsBufferSpace(MaterialInstanceUUID material, ConstantsBufferUUID cbvData, unsigned int cbvFrameIndex)
	{
		for (auto& [varName, varMapping] : material->variablesMapping)
		{
			UINT8* source = material->variablesBuffer[varMapping.mapping.bufferIndex].data() + varMapping.mapping.offset;
			UINT8* destination = cbvData->mappedConstantBuffer + (cbvFrameIndex * cbvData->alignedConstantBufferSize) + varMapping.mapping.offset;
			memcpy(destination, source, varMapping.mapping.size);
		}
	}

	void Renderable::WriteAnimationConstantsBuffer()
	{
		WriteAnimationConstantsBuffer(renderer->backBufferIndex);
	}

	void Renderable::WriteAnimationConstantsBuffer(unsigned int backbufferIndex)
	{
		if (animable.empty()) return;

		using namespace Animation;
		WriteBoneTransformationsToConstantsBuffer(uuid(), bonesTransformation, backbufferIndex);
	}

	void Renderable::WriteConstantsBuffer()
	{
		WriteConstantsBuffer(renderer->backBufferIndex);
	}

	void Renderable::WriteConstantsBuffer(unsigned int backbufferIndex)
	{
		XMMATRIX w = world();
		WriteConstantsBuffer("world", w, backbufferIndex);
	}

	void Renderable::CreateAnimationSequences()
	{
		std::unique_ptr<Model3DJson>& mdl = GetModel3DTemplate(model());
		animationsSequences = mdl->animationSequences();
		for (auto& [name, time] : animable->animations->animationsLength)
		{
			int totalFrames = static_cast<int>(time * 60);
			totalFrames /= 1000;
			SequenceChannel seqChannel;
			seqChannel.name = "animation";
			ChannelElement channelElement;
			channelElement.type = SCET_Animation;
			SequenceChannelElementAnimation& animation = channelElement.animation;
			animation.animation = name;
			animation.startTime = 0.0f;
			animation.endTime = time;
			animation.frameStart = 0;
			animation.frameEnd = totalFrames;
			seqChannel.elements.push_back(channelElement);
			Sequence sequence;
			sequence.framesPerSecond = 60;
			sequence.totalFrames = totalFrames;
			sequence.sequenceChannels.push_back(seqChannel);
			animationsSequences.sequences.insert_or_assign(name, sequence);
		}
	}

	void Renderable::RebuildAnimationSequences()
	{
		CreateAnimationSequences();
		/*
		if (currentSequence == nullptr)
			return CreateAnimationSequences();

		std::string currentSequenceStr;
		for (auto it = animationsSequences.sequences.begin(); it != animationsSequences.sequences.end(); it++)
		{
			if (currentSequence == &(it->second))
			{
				currentSequenceStr = it->first;
				break;
			}
		}

		CreateAnimationSequences();
		if (currentSequenceStr.empty()) return;

		for (auto it = animationsSequences.sequences.begin(); it != animationsSequences.sequences.end(); it++)
		{
			if (currentSequenceStr != it->first) continue;
			currentSequence = &(it->second);
			return;
		}
		*/
	}

	void Renderable::SetCurrentAnimation(SequencePlayer* seqPlayer)
	{
		if (seqPlayer == nullptr)
		{
			if (animationPlayers.contains(uuid()))
			{
				animationPlayers.erase(uuid());
			}
		}
		else
		{
			animationPlayers.insert_or_assign(uuid(), seqPlayer);
		}
	}

	void Renderable::SetCurrentAnimation(std::string anim, float startTime, float timeFactor, bool play, bool loop)
	{
		if (!animationPlayers.contains(uuid()) || animationPlayers.at(uuid()) != &sequencePlayer)
			SetCurrentAnimation(&sequencePlayer);

		sequencePlayer.sequence = &animationsSequences.sequences.at(anim);
		sequencePlayer.loop = loop;
		sequencePlayer.newSequence = true;
		sequencePlayer.ResetFrames();
	}

	void Renderable::StepAnimation(double elapsedSeconds)
	{
		if (animable.empty()) return;

		using namespace Animation;
		auto& animations = animable->animations;

		//OutputDebugStringA(std::string(std::string("anim:") + animation() + "\n").c_str());
		TraverseMultiplycationQueue(animationTime(), animation(), animations, bonesTransformation);
	}

	void Renderable::Destroy()
	{
		using namespace ComputeShader;

		for (auto& [rp, vec0] : materials)
		{
			for (auto& mat : vec0)
			{
				DeleteMaterialInstance(mat());
			}
		}
		materials.clear();

		for (auto& [rp, vec0] : constantsBuffers)
		{
			for (auto& vec1 : vec0)
			{
				for (auto& cbuffer : vec1)
				{
					DestroyConstantsBuffer(cbuffer());
				}
			}
		}
		constantsBuffers.clear();

		auto destroyMeshInstance = [](auto& vec) { for (auto& mesh : vec) { DestroyMeshInstance(mesh()); } };

		if (model3D.empty())
		{
			destroyMeshInstance(meshes);
		}
		meshes.clear();

		if (!model3D.empty())
		{
			DeleteModel3DInstance(model3D());
			if (!boundingBoxCompute.empty())
			{
				DeleteRenderableBoundingBox(boundingBoxCompute());
				boundingBoxCompute.clear();
			}
		}
	}

	//RENDER
	void Renderable::Render(RenderPassInstanceUUID renderPass, CameraUUID camera)
	{
		using namespace Animation;

		if (!visible() || !materials.contains(renderPass) || renderException) return;

		auto& commandList = renderer->commandList;

#if defined(_DEVELOPMENT)
		PIXBeginEvent(commandList.p, 0, name().c_str());
#endif
		auto& meshesMaterials = materials.at(renderPass);
		auto& meshesRootSignatures = rootSignatures.at(renderPass);
		auto& meshesPipelineStates = pipelineStates.at(renderPass);

		auto setConstantsBuffersDescriptorTables = [&commandList](auto& cbuffers, unsigned int& slot)
			{
				for (auto& cbuffer : cbuffers) {
					cbuffer->SetRootDescriptorTable(commandList, slot, renderer->backBufferIndex);
				}
			};
		auto setCameraConstantsBufferDescriptorTable = [&commandList, &camera](auto& material, unsigned int& slot)
			{
				if (!camera.empty() && material->ShaderInstanceHasRegister([](ShaderInstanceUUID binary) { return binary->CBV.camera; })) {
					camera->cameraCb->SetRootDescriptorTable(commandList, slot, renderer->backBufferIndex);
				}
			};
		auto setLightsConstantsBufferDescriptorTable = [&commandList, &camera](MaterialInstanceUUID material, unsigned int& slot)
			{
				if (material->ShaderInstanceHasRegister([](ShaderInstanceUUID binary) { return binary->CBV.light; })) {
					camera->GetLightsConstantsBuffer()->SetRootDescriptorTable(commandList, slot, renderer->backBufferIndex);
				}
			};
		auto setShadowMapsConstantsBufferDescriptorTable = [&commandList, &camera](auto& material, unsigned int& slot)
			{
				if (material->ShaderInstanceHasRegister([](ShaderInstanceUUID binary) { return binary->CBV.lightsShadowMap; })) {
					if (camera->SceneHasShadowMaps())
						return camera->GetShadowMapsConstantsBuffer()->SetRootDescriptorTable(commandList, slot, renderer->backBufferIndex);
					slot++;
				}
			};
		auto setSkinningConstantsBufferDescriptorTable = [&commandList, this](auto& material, unsigned int& slot)
			{
				if (material->ShaderInstanceHasRegister([this](ShaderInstanceUUID binary) { return binary->CBV.animation; })) {
					if (!animable.empty())
						return GetAnimatedConstantsBuffer(uuid())->SetRootDescriptorTable(commandList, slot, renderer->backBufferIndex);
					slot++;
				}
			};
		auto setUAVRootDescriptorTable = [&commandList](auto& material, unsigned int& slot)
			{
				material->SetUAVRootDescriptorTable(commandList, slot);
			};
		auto setIBLRootDescriptorTable = [&commandList, &camera](auto& material, unsigned int& slot)
			{
				if (material->ShaderInstanceHasRegister([](ShaderInstanceUUID binary) { return
					(binary->SRV.iblIrradiance == -1 || binary->SRV.iblPrefiteredEnv == -1 || binary->SRV.iblBRDFLUT == -1) ? -1 : 1; })
					)
				{
					camera->SetIBLRootDescriptorTables(commandList, slot);
				}
			};
		auto setSRVRootDescriptorTable = [&commandList](auto& material, unsigned int& slot)
			{
				material->SetSRVRootDescriptorTable(commandList, slot);
			};
		auto setShadowMapsSRVDescriptorTable = [&commandList, &camera](auto& material, unsigned int& slot)
			{
				if (material->ShaderInstanceHasRegister([](ShaderInstanceUUID binary) { return binary->SRV.lightsShadowMap; })) {
					if (camera->SceneHasShadowMaps())
						return commandList->SetGraphicsRootDescriptorTable(slot, GetShadowMapGpuDescriptorHandleStart());
					slot++;
				}
			};

		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			if (skipMeshes_contains(i)) continue;

			commandList->IASetPrimitiveTopology(topology());
			commandList->SetGraphicsRootSignature(meshesRootSignatures.at(i));
			commandList->SetPipelineState(meshesPipelineStates.at(i));

			auto& material = meshesMaterials.at(i);
			auto& cbuffers = constantsBuffers.at(renderPass).at(i);
			unsigned int slot = 0U;

			setConstantsBuffersDescriptorTables(cbuffers, slot);
			setCameraConstantsBufferDescriptorTable(material, slot);
			setLightsConstantsBufferDescriptorTable(material, slot);
			setShadowMapsConstantsBufferDescriptorTable(material, slot);
			setSkinningConstantsBufferDescriptorTable(material, slot);
			setUAVRootDescriptorTable(material, slot);
			setIBLRootDescriptorTable(material, slot);
			setSRVRootDescriptorTable(material, slot);
			setShadowMapsSRVDescriptorTable(material, slot);

			auto& mesh = meshes.at(i);
			commandList->IASetVertexBuffers(0, 1, &mesh->vbvData.vertexBufferView);
			commandList->IASetIndexBuffer(&mesh->ibvData.indexBufferView);
			commandList->DrawIndexedInstanced(mesh->ibvData.indexBufferView.SizeInBytes / sizeof(unsigned int), 1, 0, 0, 0);
		}
#if defined(_DEVELOPMENT)
		PIXEndEvent(commandList.p);
#endif
	}
}
