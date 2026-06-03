#include "pch.h"
#include "Renderable.h"
#include <Scene.h>
#include <Renderer.h>
#include <DeviceUtils/RootSignature/RootSignature.h>
#include <DeviceUtils/PipelineState/PipelineState.h>
#include <Renderable/RenderableBoundingBox.h>
#include <NoMath.h>

extern std::unique_ptr<JRenderer> renderer;

#if defined(_EDITOR)
namespace Editor
{
	extern void SelectRenderable(RenderableID renderable);
	extern bool IsPlaying(SceneUnitId unit);
	extern bool IsPaused(SceneUnitId unit);
	extern std::unordered_map<SceneUnitId, CameraID> levelCameraUUID;
	extern std::unordered_map<SceneUnitId, CameraID> editorCameraUUID;
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

	std::unordered_map<RenderableID, SequencePlayer*> animationPlayers;

	Renderable::Renderable(SceneUnitId id, nlohmann::json& json) :SceneObject(id, json)
	{
#include <Attributes/JInit.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include <RenderableAtt.h>
#include <JEnd.h>

		lifecycleState->store(false);
		RENAME_ON_DELETION(Renderable);
		animationStepLock = std::make_unique<std::atomic_bool>(false);
		for (unsigned int i = 0; i < JRenderer::numFrames; i++)
		{
			constantsBuffersLock[i] = std::make_unique<std::atomic_bool>(false);
		}
	}

	void Renderable::create_rotation(XMFLOAT3 v)
	{
		if (!contains("rotation"))
		{
			rotation(v);
		}
	}

	void Renderable::rotation(XMFLOAT3 v)
	{
		(*this)["rotation"] = FromXMFLOAT3(v);
		updateRotationQ();
	}

#if defined(_EDITOR)
	void Renderable::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <RenderableAtt.h>
#include <JEnd.h>
	}
#endif

	void Renderable::Initialize()
	{
		using namespace Animation;
		using namespace ComputeShader;

		CreateMeshInstances(); //why here, this is a special case, as Animables depends of animables which is created in this function

#include <TrackUUID/JInsert.h>
#include <RenderableAtt.h>
#include <JEnd.h>

		if (!animable.empty())
		{
			AttachAnimation(SUuuid(), model3D->animations);
			//boundingBoxCompute = CreateRenderableBoundingBox(MAKESUUUID(unit, uuid()));
			sequencePlayer.renderable = SUuuid();
			CreateAnimationThread();
		}

		SetInitialConditions();

#if defined(_EDITOR)
		OnPick = [&] { Editor::SelectRenderable(SUuuid()); };
#endif
		SceneObject::Initialize();
	}

	void Renderable::SetInitialConditions()
	{
		lastAnimationTime = 0.0f;
		forceAnimation = true;
		animationTransformation = XMMatrixIdentity();

		if (!animable.empty())
		{
			animationTime(0.0f);
			SetCurrentAnimation(animationSequence(), animationTime(), animationTimeFactor(), animationPlay(), animationLoop());
			StepAnimation(0.0f); //take an empty T-Pose step so the skinning can be performed
		}

		updateRotationQ();
		deleteFrames = JRenderer::numFrames;
	}

	void Renderable::BindToScene()
	{
#include <TrackUUID/JInsert.h>
#include <RenderableAtt.h>
#include <JEnd.h>

		BindCameras();
		BindShadowMapCameras();
		lifecycleState->store(true);
		lifecycleState->notify_all();
	}

	void Renderable::Bind(JUUID uuid)
	{
		switch (GetSceneObjectType(unit, uuid))
		{
		case SO_Cameras:
		{
			bindedCameras.insert(MAKESUUUID(unit, uuid));
		}
		break;
		}
	}

	void Renderable::BindCameras()
	{
		auto cams = cameras();
		for (auto& uuid : cams)
		{
			if (bindedCameras.contains(MAKESUUUID(unit, uuid)))
				continue;
			BindCamera(uuid);
		}
	}

	void Renderable::BindCamera(JUUID cuuid)
	{
		Scene::BindToScene(unit, uuid(), cuuid);
	}

	void Renderable::BindShadowMapCameras()
	{
		if (!castShadows()) return;

		auto lights = GetShadowMapLights(unit);
		for (auto uuid : lights)
		{
			LightID light = MAKESUUUID(unit, uuid);
			for (CameraID cam : light->shadowMapCameras)
			{
				BindCamera(cam.uuid());
			}
		}
	}

	void Renderable::UnbindFromScene()
	{
		using namespace ComputeShader;

#include <TrackUUID/JErase.h>
#include <RenderableAtt.h>
#include <JEnd.h>

		UnbindCameras();
		Scene::UnbindFromScene(unit, uuid());
		//if (!boundingBoxCompute.empty())
		//{
		//	DeleteRenderableBoundingBox(boundingBoxCompute());
		//	boundingBoxCompute.clear();
		//}
		if (animationPlayers.contains(MAKESUUUID(unit, uuid())))
		{
			animationPlayers.erase(MAKESUUUID(unit, uuid()));
		}
	}

	void Renderable::Unbind(JUUID uuid)
	{
		bindedCameras.erase(MAKESUUUID(unit, uuid));
	}

	void Renderable::UnbindCameras()
	{
		auto cams = cameras();
		for (auto& uuid : cams)
		{
			if (!bindedCameras.contains(MAKESUUUID(unit, uuid)))
				continue;
			UnbindCamera(uuid);
		}

		if (castShadows())
		{
			auto smlights = GetShadowMapLights(unit);
			for (auto& uuid : smlights)
			{
				LightID l = MAKESUUUID(unit, uuid);
				l->UnbindRenderableFromShadowMapCamera(SUuuid());
			}
		}
	}

	void Renderable::UnbindCamera(JUUID cuuid)
	{
		CameraID cam = MAKESUUUID(unit, cuuid);
		cam->UnbindRenderable(SUuuid());
		Scene::UnbindFromScene(unit, uuid(), cuuid);
	}

#if defined(_EDITOR)
	void Renderable::DropJsonMoldAttributes(nlohmann::json& j)
	{
		SceneObject::DropJsonMoldAttributes(j);
		j.at("cameras") = nlohmann::json::array({});
	}
#endif

	XMVECTOR Renderable::positionV()
	{
		XMFLOAT3 pos = position();
		return XMLoadFloat3(&pos);
	}

	void Renderable::updateRotationQ()
	{
		XMFLOAT3 v = rotation();
		rotationQuaternion = XMQuaternionRotationRollPitchYaw(
			XMConvertToRadians(v.x),
			XMConvertToRadians(v.y),
			XMConvertToRadians(v.z)
		);
	}

	XMVECTOR Renderable::rotationQ()
	{
		return rotationQuaternion;
	}

	void Renderable::rotationQ(XMVECTOR Q)
	{
		rotationQuaternion = Q;
	}

	XMMATRIX Renderable::world()
	{
		XMFLOAT3 posV = position();
		XMFLOAT3 scaleV = scale();
		XMMATRIX rotationM = XMMatrixRotationQuaternion(rotationQuaternion);
		XMMATRIX scaleM = XMMatrixScalingFromVector(XMLoadFloat3(&scaleV));
		XMMATRIX positionM = XMMatrixTranslationFromVector(XMLoadFloat3(&posV));
		XMMATRIX worldM = XMMatrixMultiply(XMMatrixMultiply(scaleM, rotationM), positionM);
		XMMATRIX animT = (!animationUseTransformation()) ? worldM : XMMatrixMultiply(animationTransformation, worldM);
		if (attachedTo().empty())
		{
			return animT;
		}
		if (attachedBone().empty())
		{
			return XMMatrixMultiply(animT, RenderableID(unit, attachedTo())->world());
		}
		else
		{
			RenderableID parent(unit, attachedTo());
			auto [mm, pos, a, b, c] = parent->GetBoneTransformation(attachedBone());
			return XMMatrixMultiply(animT, mm);
		}
	}

	void Renderable::CreateMeshInstances()
	{
		using namespace Templates;

		if (Model3DTemplateExist(model()))
		{
			CreateModel3DInstance(model(), [this]
				{
					return std::make_unique<Model3DInstance>(unit, model(), uuid());
				}
			);
			model3D = model();
			meshes = model3D->meshes;
			if (model3D->animations)
			{
#if defined(_EDITOR)
				Model3DJsonID model3dJson = model();
				model3dJson->ListenUpdate(Model3DJson::Update_animationSequences, SUuuid(), [&]()
					{
						RebuildAnimationSequences();
					}
				);
#endif
				animable = model3D;
			}

			if (!animable)
			{
				CreateBoundingBox();
			}
			else
			{
				CreateAnimationSequences();
			}
		}
		else if (!meshMaterial().mesh.empty() && meshMaterial().mesh.contains("primitive") && !meshMaterial().mesh.at("primitive").empty() && !meshMaterial().materialUUID.empty())
		{
			nlohmann::json mesh = meshMaterial().mesh;
			meshes.push_back(GetMeshInstance(unit, mesh)->uuid);
			CreateBoundingBox();
		}
	}

	std::vector<RenderPassInstanceID> Renderable::GetCameraRenderPasses(CameraID cam)
	{
		if (!cam->useSwapChain()) return cam->renderPassesUUID;

		std::vector<RenderPassInstanceID> rpiv = cam->renderPassesUUID;
		rpiv.push_back(renderer->swapChainPass);
		return rpiv;
	}

	void Renderable::CreateMaterialsInstances(CameraID cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			CreateRenderPassMaterialsInstances(rp);
		}
	}

	void Renderable::CreateRenderPassMaterialsInstances(RenderPassInstanceID pass)
	{
		std::vector<PassMaterialOverride> pmo = passMaterialOverrides();

		if (materials.contains(pass))
		{
			return;
		}

		if (!model3D.empty())
		{
			for (unsigned int i = 0; i < meshes.size(); i++)
			{
				std::vector<PassMaterialOverride> mpmo;
				std::copy_if(pmo.begin(), pmo.end(), std::back_inserter(mpmo), [i](PassMaterialOverride& o) { return o.meshIndex == i; });
				auto& mesh = meshes.at(i);
				JUUID matUUID = model3D->materials.at(i)();
				materials[pass].push_back(pass->GetRenderPassMaterialInstance(unit, matUUID, mesh, shadowed(), mpmo, uuid()));
			}
		}
		else if (!meshMaterial().mesh.empty() && meshMaterial().mesh.contains("primitive") && !meshMaterial().mesh.at("primitive").empty() && !meshMaterial().materialUUID.empty())
		{
			std::vector<PassMaterialOverride> mpmo;
			std::copy_if(pmo.begin(), pmo.end(), std::back_inserter(mpmo), [](PassMaterialOverride& o) { return o.meshIndex == 0; });
			auto& mesh = meshes.at(0);
			JUUID matUUID = meshMaterial().materialUUID;
			if (!MaterialTemplateExist(matUUID))
			{
				matUUID = GetMaterialUUIDByName(fallbackMaterialName); //fallback
			}
			materials[pass].push_back(pass->GetRenderPassMaterialInstance(unit, matUUID, mesh, shadowed(), mpmo, uuid()));
		}
	}

	void Renderable::DestroyMaterialsInstances(CameraID cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			DestroyRenderPassMaterialsInstances(rp);
		}
	}

	void Renderable::DestroyRenderPassMaterialsInstances(RenderPassInstanceID rp)
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

	void Renderable::CreateConstantsBuffersInstances(CameraID cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			CreateRenderPassConstantsBuffersInstances(rp);
		}
	}

	void Renderable::CreateRenderPassConstantsBuffersInstances(RenderPassInstanceID pass)
	{
		//wait until every lock is free
		for (unsigned int i = 0; i < JRenderer::numFrames; i++)
		{
			if (constantsBuffersLock[i]->load() == true)
				constantsBuffersLock[i]->wait(true);
		}

		//mark every lock as being worked
		for (unsigned int i = 0; i < JRenderer::numFrames; i++)
		{
			constantsBuffersLock[i]->store(true);
		}

		//if the constants buffer exists wew restore the non-working state and notify each of the locks and return
		if (constantsBuffers.contains(pass))
		{
			for (unsigned int i = 0; i < JRenderer::numFrames; i++)
			{
				constantsBuffersLock[i]->store(false);
				constantsBuffersLock[i]->notify_one();
			}
			return;
		}

		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			constantsBuffers[pass].push_back({});
			auto& mi = materials[pass].at(i);
			auto& mesh = meshes.at(i);
			for (unsigned int j = 0; j < mi->variablesBufferSize.size(); j++)
			{
				size_t size = mi->variablesBufferSize[j];
				ConstantsBufferID cbuffer = CreateConstantsBuffer(size, JRenderer::numFrames, name() + "." + std::to_string(j) + "." + mesh->uuid);
				for (unsigned int n = 0; n < JRenderer::numFrames; n++)
				{
					WriteMaterialVariablesToConstantsBufferSpace(mi, cbuffer, n);
				}
				constantsBuffers[pass][i].push_back(cbuffer);
			}
		}

		auto createConstantWriter = [this](DeviceUtils::ConstantsBufferID& cbuffer, size_t cbOffset, size_t cbSize)
			{
				auto* cbuffer_ptr = (*cbuffer).get();
				unsigned int alignedConstantBufferSize = cbuffer_ptr->alignedConstantBufferSize;
				byte* mappedConstantBuffer = cbuffer_ptr->mappedConstantBuffer;

				return [this, cbuffer, cbOffset, cbSize, alignedConstantBufferSize, mappedConstantBuffer
				](void* data, unsigned int backbufferIndex, size_t offset, size_t slot)
					{
						std::unique_ptr<std::atomic_bool>& lock = constantsBuffersLock[backbufferIndex];

						//if current value is true(work is in progress), wait until the change from true notification happens
						if (lock->load() == true)
							lock->wait(true);

						//store as true(work is in progress) without triggering a notification
						lock->store(true);

						size_t offsetInMemory = alignedConstantBufferSize * backbufferIndex;
						offsetInMemory += cbOffset + cbSize * slot + offset;
						memcpy(mappedConstantBuffer + offsetInMemory, data, cbSize);

						//store a false freeing the work queue and notify other locks with notify_one to process just one
						lock->store(false);
						lock->notify_one();

						return cbOffset + cbSize;
					};
			};


		if (constantsBuffers.contains(pass))
		{
			auto& cbufferPass = constantsBuffers.at(pass);
			auto& meshMaterials = materials.at(pass);
			for (unsigned int mesh = 0; mesh < meshMaterials.size(); mesh++)
			{
				auto& vsVars = meshMaterials.at(mesh)->vertexShaderInstanceID->constantsBuffersVariables;
				auto& psVars = meshMaterials.at(mesh)->pixelShaderInstanceID->constantsBuffersVariables;
				auto& cbuffers = cbufferPass.at(mesh);
				std::vector<std::string> vsConstants = nostd::GetKeysFromMap(vsVars);
				std::vector<std::string> psConstants = nostd::GetKeysFromMap(psVars);

				for (auto& constantName : vsConstants)
				{
					auto& vsVar = vsVars.at(constantName);
					if (cbuffers.size() <= vsVar.bufferIndex)
						continue;

					constantsWriter[constantName].push_back(createConstantWriter(cbuffers[vsVar.bufferIndex], vsVar.offset, vsVar.size));
				}
				for (auto& constantName : psConstants)
				{
					auto& psVar = psVars.at(constantName);
					if (cbuffers.size() <= psVar.bufferIndex)
						continue;

					constantsWriter[constantName].push_back(createConstantWriter(cbuffers[psVar.bufferIndex], psVar.offset, psVar.size));
				}
			}
		}

		//same here but now the constant buffer has already been created
		for (unsigned int i = 0; i < JRenderer::numFrames; i++)
		{
			constantsBuffersLock[i]->store(false);
			constantsBuffersLock[i]->notify_one();
		}
	}

	void Renderable::DestroyConstantsBuffersInstances(CameraID cam)
	{
		for (unsigned int i = 0; i < JRenderer::numFrames; i++)
		{
			if (constantsBuffersLock[i]->load() == true)
				constantsBuffersLock[i]->wait(true);
		}

		for (unsigned int i = 0; i < JRenderer::numFrames; i++)
		{
			constantsBuffersLock[i]->store(true);
		}

		for (auto& rp : GetCameraRenderPasses(cam))
		{
			DestroyRenderPassConstantsBuffersInstances(rp);
		}

		for (unsigned int i = 0; i < JRenderer::numFrames; i++)
		{
			constantsBuffersLock[i]->store(false);
			constantsBuffersLock[i]->notify_one();
		}
	}

	void Renderable::DestroyRenderPassConstantsBuffersInstances(RenderPassInstanceID pass)
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

	void Renderable::CreateRootSignatures(CameraID cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			if (rootSignatures.contains(rp)) rootSignatures.erase(rp);
			CreateRenderPassRootSignatures(rp);
		}
	}

	void Renderable::CreateRenderPassRootSignatures(RenderPassInstanceID rp)
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			auto& mi = materials[rp].at(i);

			auto& vsCBparams = mi->vertexShaderInstanceID->constantsBuffersParameters;
			auto& psCBparams = mi->pixelShaderInstanceID->constantsBuffersParameters;
			auto& uavParams = mi->pixelShaderInstanceID->uavParameters;
			auto& psSRVCSparams = mi->pixelShaderInstanceID->srvCSParameters;
			auto& psSRVTexparams = mi->pixelShaderInstanceID->srvTexParameters;
			auto& psSamplersParams = mi->pixelShaderInstanceID->samplersParameters;
			auto& samplers = mi->samplers;

			std::string rsName = "rootSignature:" + name() + ":" + std::to_string(i);
			rootSignatures[rp].push_back(
				CreateRootSignature(rsName, vsCBparams, psCBparams, uavParams, psSRVCSparams, psSRVTexparams, psSamplersParams, samplers)
			);
		}
	}

	void Renderable::DestroyRootSignatures(CameraID cam)
	{
		if (cam.empty()) return;
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			DestroyRenderPassRootSignatures(rp);
		}
	}

	void Renderable::DestroyRenderPassRootSignatures(RenderPassInstanceID rp)
	{
		if (rootSignatures.contains(rp))
			rootSignatures.erase(rp);
	}

	void Renderable::CreatePipelineStates(CameraID cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			if (pipelineStates.contains(rp)) pipelineStates.erase(rp);
			CreateRenderPassPipelineStates(rp);
		}
	}

	void Renderable::CreateRenderPassPipelineStates(RenderPassInstanceID rp)
	{
		auto setPipelineStateAt = [this](RenderPassInstanceID rp, unsigned int i)
			{
				auto rtFormats = rp->GetRenderTargetsFormats();
				auto depthFormat = rp->GetDepthStencilFormat();
				auto& mesh = meshes.at(i);
				auto& mi = materials[rp].at(i);
				auto& vsLayout = vertexInputLayoutsMap[mesh->vertexClass];
				auto& rootSignature = rootSignatures[rp].at(i);
				auto& vsByteCode = mi->vertexShaderInstanceID->byteCode;
				auto& psByteCode = mi->pixelShaderInstanceID->byteCode;

				auto& material = mi->materialUUID;
				BlendDesc blendDesc = material->blendState();
				RasterizerDesc rasterizerDesc = material->rasterizerState();
				DepthStencilDesc depthStencilDesc = material->overrideDepthStencil() ? material->depthStencil() : depthStencil();
				if (depthFormat == DXGI_FORMAT_UNKNOWN)
				{
					depthStencilDesc.DepthEnable = false;
				}

				D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType = D3D_PRIMITIVE_TOPOLOGYToD3D12_PRIMITIVE_TOPOLOGY_TYPE.at(topology());

				std::string plName = "pipelineState:" + name() + ":" + std::to_string(i);

				pipelineStates[rp][i] = CreateGraphicsPipelineState(plName, vsLayout, vsByteCode, psByteCode, rootSignature, blendDesc, rasterizerDesc, depthStencilDesc, primitiveTopologyType, rtFormats, depthFormat);
			};

		pipelineStates[rp].resize(meshes.size(), nullptr);

		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			setPipelineStateAt(rp, i);
			auto& mi = materials[rp].at(i);
			auto& material = mi->materialUUID;
			auto& renderPass = rp->renderPassTemplate;
			size_t hash = 0;
			nostd::hash_combine(hash, std::get<0>(SUuuid()), std::get<1>(SUuuid()), rp(), i);

			material->SetPipelineStateCallback(hash, [=] { setPipelineStateAt(rp, i); });
			renderPass->SetPipelineStateCallback(hash, [=] { setPipelineStateAt(rp, i); });
		}
	}

	void Renderable::DestroyPipelineStates(CameraID cam)
	{
		if (cam.empty()) return;
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			DestroyRenderPassPipelineStates(rp);
		}
	}

	void Renderable::DestroyRenderPassPipelineStates(RenderPassInstanceID rp)
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

	void Renderable::CreateRenderMethods(CameraID cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			std::tuple<CameraID, RenderPassInstanceID> key = std::make_tuple(cam, rp);
			std::vector<std::vector<DescriptorTableSetter>> setters = CreateRenderPassRenderMethods(cam, rp);
			descriptorsRenders[key] = setters;
		}
#if defined(_EDITOR)
		//this is a complete hack, but what the hell
		using namespace Editor;
		if (levelCameraUUID.contains(unit) && cam == levelCameraUUID.at(unit))
		{
			CameraID edCam = editorCameraUUID.at(unit);
			for (auto& rp : GetCameraRenderPasses(cam))
			{
				std::tuple<CameraID, RenderPassInstanceID> key = std::make_tuple(edCam, rp);
				std::vector<std::vector<DescriptorTableSetter>> setters = CreateEditorCameraRenderPassRenderMethods(edCam, cam, rp);
				descriptorsRenders[key] = setters;
			}
		}
#endif
	}

	std::vector<std::vector<DescriptorTableSetter>> Renderable::CreateRenderPassRenderMethods(CameraID cam, RenderPassInstanceID rp)
	{
		using namespace Animation;

		Camera* pcamera = (!cam.empty()) ? (*cam).get() : nullptr;
		auto& meshesMaterials = materials.at(rp);

		std::vector<std::vector<DescriptorTableSetter>> meshSetters;

		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			auto& material = meshesMaterials.at(i);
			MaterialInstance* material_ins = (*material).get();
			ShaderInstance* vertexShader_ins = (*material_ins->vertexShaderInstanceID).get();
			ShaderInstance* pixelShader_ins = (*material_ins->pixelShaderInstanceID).get();

			std::vector<DeviceUtils::ConstantsBufferID>& cbuffers = constantsBuffers.at(rp).at(i);
			std::vector<DeviceUtils::ConstantsBuffer*> pcbuffers;
			std::transform(cbuffers.begin(), cbuffers.end(), std::back_inserter(pcbuffers), [](auto& c)
				{
					return (*c).get();
				}
			);
			DeviceUtils::ConstantsBuffer* pcam_cbuffer = (pcamera) ? (*pcamera->cameraCb).get() : nullptr;
			DeviceUtils::ConstantsBuffer* pcam_light_cbuffer = (pcamera && pcamera->lightsCB) ? (*pcamera->lightsCB).get() : nullptr;
			DeviceUtils::ConstantsBuffer* pcam_shadowmap_cbuffer = (pcamera && pcamera->shadowMapsCB) ? (*pcamera->shadowMapsCB).get() : nullptr;
			DeviceUtils::ConstantsBuffer* animated_cbuffer = (!animable.empty()) ? (*GetAnimatedConstantsBuffer(SUuuid())).get() : nullptr;

			std::vector<std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE>> gpu_handles_cbuffers;
			std::transform(pcbuffers.begin(), pcbuffers.end(), std::back_inserter(gpu_handles_cbuffers), [](auto& c)
				{
					return c->gpu_xhandle;
				}
			);
			std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> gpu_handles_camera;
			if (pcam_cbuffer)
			{
				gpu_handles_camera = pcam_cbuffer->gpu_xhandle;
			}
			std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> gpu_handles_lights;
			if (pcam_light_cbuffer)
			{
				gpu_handles_lights = pcam_light_cbuffer->gpu_xhandle;
			}
			std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> gpu_handles_shadowMaps;
			if (pcam_shadowmap_cbuffer)
			{
				gpu_handles_shadowMaps = pcam_shadowmap_cbuffer->gpu_xhandle;
			}
			std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> gpu_handles_skinning;
			if (animated_cbuffer)
			{
				gpu_handles_skinning = animated_cbuffer->gpu_xhandle;
			}
			std::map<unsigned int, ::CD3DX12_GPU_DESCRIPTOR_HANDLE> mat_uav = material_ins->uav;

			CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_handle_iblIrradiance = (pcamera && pcamera->iblTextures.contains(TextureShaderUsage_IBLIrradiance)) ? GetTextureInstance(pcamera->iblTextures.at(TextureShaderUsage_IBLIrradiance))->gpuHandle : CD3DX12_GPU_DESCRIPTOR_HANDLE();

			CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_handle_iblPrefiteredEnv = (pcamera && pcamera->iblTextures.contains(TextureShaderUsage_IBLPreFilteredEnvironment)) ? GetTextureInstance(pcamera->iblTextures.at(TextureShaderUsage_IBLPreFilteredEnvironment))->gpuHandle : CD3DX12_GPU_DESCRIPTOR_HANDLE();

			CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_handle_iblBRDFLUT = (pcamera && pcamera->iblTextures.contains(TextureShaderUsage_IBLBRDFLUT)) ? GetTextureInstance(pcamera->iblTextures.at(TextureShaderUsage_IBLBRDFLUT))->gpuHandle : CD3DX12_GPU_DESCRIPTOR_HANDLE();

			std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> gpu_handle_srv;
			for (auto& [textureType, texParam] : pixelShader_ins->srvTexParameters)
			{
				if (texParam.numSRV == 0xFFFFFFFF || iblUsageTexture.contains(textureType)) continue;
				auto& texInstance = GetTextureInstance(material_ins->textures.at(textureType));
				gpu_handle_srv.push_back(texInstance->gpuHandle);
			}

			CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_handle_srv_shadowMap = (pcamera && pcamera->SceneHasShadowMaps()) ? GetShadowMapGpuDescriptorHandleStart(unit) : CD3DX12_GPU_DESCRIPTOR_HANDLE();

			std::vector<DescriptorTableSetter> descriptor_table_setters;

			//f2f
			SetConstantsBuffersDescriptorTables(descriptor_table_setters, gpu_handles_cbuffers);
			SetCameraConstantsBufferDescriptorTable(descriptor_table_setters, gpu_handles_camera, vertexShader_ins, pixelShader_ins);
			SetLightsConstantsBufferDescriptorTable(descriptor_table_setters, gpu_handles_lights, vertexShader_ins, pixelShader_ins);
			SetShadowMapsConstantsBufferDescriptorTable(descriptor_table_setters, pcamera, gpu_handles_shadowMaps, vertexShader_ins, pixelShader_ins);
			SetSkinningConstantsBufferDescriptorTable(descriptor_table_setters, gpu_handles_skinning, vertexShader_ins, pixelShader_ins);
			//not f2f
			SetUAVRootDescriptorTable(descriptor_table_setters, mat_uav, pixelShader_ins);
			SetIBLRootDescriptorTable(descriptor_table_setters, vertexShader_ins, pixelShader_ins, gpu_handle_iblIrradiance, gpu_handle_iblPrefiteredEnv, gpu_handle_iblBRDFLUT);
			SetSRVRootDescriptorTable(descriptor_table_setters, gpu_handle_srv);
			SetShadowMapsSRVDescriptorTable(descriptor_table_setters, pcamera, vertexShader_ins, pixelShader_ins, gpu_handle_srv_shadowMap);

			meshSetters.push_back(descriptor_table_setters);
		}
		return meshSetters;
	}

#if defined(_EDITOR)
	std::vector<std::vector<DescriptorTableSetter>> Renderable::CreateEditorCameraRenderPassRenderMethods(CameraID edCam, CameraID cam, RenderPassInstanceID rp)
	{
		using namespace Animation;

		Camera* pcamera = (!cam.empty()) ? (*cam).get() : nullptr;
		Camera* pedCamera = (!edCam.empty()) ? (*edCam).get() : nullptr;
		auto& meshesMaterials = materials.at(rp);

		std::vector<std::vector<DescriptorTableSetter>> meshSetters;

		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			auto& material = meshesMaterials.at(i);
			MaterialInstance* material_ins = (*material).get();
			ShaderInstance* vertexShader_ins = (*material_ins->vertexShaderInstanceID).get();
			ShaderInstance* pixelShader_ins = (*material_ins->pixelShaderInstanceID).get();

			std::vector<DeviceUtils::ConstantsBufferID>& cbuffers = constantsBuffers.at(rp).at(i);
			std::vector<DeviceUtils::ConstantsBuffer*> pcbuffers;
			std::transform(cbuffers.begin(), cbuffers.end(), std::back_inserter(pcbuffers), [](auto& c)
				{
					return (*c).get();
				}
			);
			DeviceUtils::ConstantsBuffer* pcam_cbuffer = (pedCamera) ? (*pedCamera->cameraCb).get() : nullptr;
			DeviceUtils::ConstantsBuffer* pcam_light_cbuffer = (pcamera && pcamera->lightsCB) ? (*pcamera->lightsCB).get() : nullptr;
			DeviceUtils::ConstantsBuffer* pcam_shadowmap_cbuffer = (pcamera && pcamera->shadowMapsCB) ? (*pcamera->shadowMapsCB).get() : nullptr;
			DeviceUtils::ConstantsBuffer* animated_cbuffer = (!animable.empty()) ? (*GetAnimatedConstantsBuffer(SUuuid())).get() : nullptr;

			std::vector<std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE>> gpu_handles_cbuffers;
			std::transform(pcbuffers.begin(), pcbuffers.end(), std::back_inserter(gpu_handles_cbuffers), [](auto& c)
				{
					return c->gpu_xhandle;
				}
			);
			std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> gpu_handles_camera;
			if (pcam_cbuffer)
			{
				gpu_handles_camera = pcam_cbuffer->gpu_xhandle;
			}
			std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> gpu_handles_lights;
			if (pcam_light_cbuffer)
			{
				gpu_handles_lights = pcam_light_cbuffer->gpu_xhandle;
			}
			std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> gpu_handles_shadowMaps;
			if (pcam_shadowmap_cbuffer)
			{
				gpu_handles_shadowMaps = pcam_shadowmap_cbuffer->gpu_xhandle;
			}
			std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> gpu_handles_skinning;
			if (animated_cbuffer)
			{
				gpu_handles_skinning = animated_cbuffer->gpu_xhandle;
			}
			std::map<unsigned int, ::CD3DX12_GPU_DESCRIPTOR_HANDLE> mat_uav = material_ins->uav;

			CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_handle_iblIrradiance = (pcamera && pcamera->iblTextures.contains(TextureShaderUsage_IBLIrradiance)) ? GetTextureInstance(pcamera->iblTextures.at(TextureShaderUsage_IBLIrradiance))->gpuHandle : CD3DX12_GPU_DESCRIPTOR_HANDLE();

			CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_handle_iblPrefiteredEnv = (pcamera && pcamera->iblTextures.contains(TextureShaderUsage_IBLPreFilteredEnvironment)) ? GetTextureInstance(pcamera->iblTextures.at(TextureShaderUsage_IBLPreFilteredEnvironment))->gpuHandle : CD3DX12_GPU_DESCRIPTOR_HANDLE();

			CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_handle_iblBRDFLUT = (pcamera && pcamera->iblTextures.contains(TextureShaderUsage_IBLBRDFLUT)) ? GetTextureInstance(pcamera->iblTextures.at(TextureShaderUsage_IBLBRDFLUT))->gpuHandle : CD3DX12_GPU_DESCRIPTOR_HANDLE();

			std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> gpu_handle_srv;
			for (auto& [textureType, texParam] : pixelShader_ins->srvTexParameters)
			{
				if (texParam.numSRV == 0xFFFFFFFF || iblUsageTexture.contains(textureType)) continue;
				auto& texInstance = GetTextureInstance(material_ins->textures.at(textureType));
				gpu_handle_srv.push_back(texInstance->gpuHandle);
			}

			CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_handle_srv_shadowMap = (pcamera && pcamera->SceneHasShadowMaps()) ? GetShadowMapGpuDescriptorHandleStart(unit) : CD3DX12_GPU_DESCRIPTOR_HANDLE();

			std::vector<DescriptorTableSetter> descriptor_table_setters;

			//f2f
			SetConstantsBuffersDescriptorTables(descriptor_table_setters, gpu_handles_cbuffers);
			SetCameraConstantsBufferDescriptorTable(descriptor_table_setters, gpu_handles_camera, vertexShader_ins, pixelShader_ins);
			SetLightsConstantsBufferDescriptorTable(descriptor_table_setters, gpu_handles_lights, vertexShader_ins, pixelShader_ins);
			SetShadowMapsConstantsBufferDescriptorTable(descriptor_table_setters, pcamera, gpu_handles_shadowMaps, vertexShader_ins, pixelShader_ins);
			SetSkinningConstantsBufferDescriptorTable(descriptor_table_setters, gpu_handles_skinning, vertexShader_ins, pixelShader_ins);
			//not f2f
			SetUAVRootDescriptorTable(descriptor_table_setters, mat_uav, pixelShader_ins);
			SetIBLRootDescriptorTable(descriptor_table_setters, vertexShader_ins, pixelShader_ins, gpu_handle_iblIrradiance, gpu_handle_iblPrefiteredEnv, gpu_handle_iblBRDFLUT);
			SetSRVRootDescriptorTable(descriptor_table_setters, gpu_handle_srv);
			SetShadowMapsSRVDescriptorTable(descriptor_table_setters, pcamera, vertexShader_ins, pixelShader_ins, gpu_handle_srv_shadowMap);

			meshSetters.push_back(descriptor_table_setters);
		}
		return meshSetters;
	}
#endif

	void Renderable::SetConstantsBuffersDescriptorTables(std::vector<DescriptorTableSetter>& setters, std::vector<std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE>> gpu_handles_cbuffers)
	{
		setters.push_back([gpu_handles_cbuffers](CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int backBufferIndex, unsigned int& slot)
			{
				for (auto gpu_xhandle : gpu_handles_cbuffers) {
					commandList->SetGraphicsRootDescriptorTable(slot, gpu_xhandle[backBufferIndex]);
					slot++;
				}
			});
	};

	void Renderable::SetCameraConstantsBufferDescriptorTable(std::vector<DescriptorTableSetter>& setters, std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> gpu_handles_camera, ShaderInstance* vertexShader_ins, ShaderInstance* pixelShader_ins)
	{
		if (!gpu_handles_camera.empty() && (vertexShader_ins->CBV.camera != -1 || pixelShader_ins->CBV.camera != -1)) {
			setters.push_back([gpu_handles_camera](
				CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int backBufferIndex, unsigned int& slot)
				{
					commandList->SetGraphicsRootDescriptorTable(slot++, gpu_handles_camera[backBufferIndex]);
				});
		}
	};

	void Renderable::SetLightsConstantsBufferDescriptorTable(std::vector<DescriptorTableSetter>& setters, std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> gpu_handles_lights, ShaderInstance* vertexShader_ins, ShaderInstance* pixelShader_ins)
	{
		if (vertexShader_ins->CBV.light != -1 || pixelShader_ins->CBV.light != -1) {
			setters.push_back([gpu_handles_lights](
				CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int backBufferIndex, unsigned int& slot)
				{
					commandList->SetGraphicsRootDescriptorTable(slot++, gpu_handles_lights[backBufferIndex]);
				});
		}
	};

	void Renderable::SetShadowMapsConstantsBufferDescriptorTable(std::vector<DescriptorTableSetter>& setters, Camera* camera, std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> gpu_handles_shadowMaps, ShaderInstance* vertexShader_ins, ShaderInstance* pixelShader_ins)
	{
		if (vertexShader_ins->CBV.lightsShadowMap != -1 || pixelShader_ins->CBV.lightsShadowMap != -1) {
			setters.push_back([camera, gpu_handles_shadowMaps](
				CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int backBufferIndex, unsigned int& slot)
				{
					if (camera->SceneHasShadowMaps())
						commandList->SetGraphicsRootDescriptorTable(slot, gpu_handles_shadowMaps[backBufferIndex]);
					slot++;
				});
		}
	};

	void Renderable::SetSkinningConstantsBufferDescriptorTable(std::vector<DescriptorTableSetter>& setters, std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> gpu_handles_skinning, ShaderInstance* vertexShader_ins, ShaderInstance* pixelShader_ins)
	{
		if (vertexShader_ins->CBV.animation != -1 || pixelShader_ins->CBV.animation != -1) {
			setters.push_back([gpu_handles_skinning](CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int backBufferIndex, unsigned int& slot)
				{
					if (!gpu_handles_skinning.empty())
						commandList->SetGraphicsRootDescriptorTable(slot, gpu_handles_skinning[backBufferIndex]);
					slot++;
				});
		}
	};

	void Renderable::SetUAVRootDescriptorTable(std::vector<DescriptorTableSetter>& setters, std::map<unsigned int, ::CD3DX12_GPU_DESCRIPTOR_HANDLE> uav, ShaderInstance* pixelShader_ins)
	{
		setters.push_back([uav, pixelShader_ins](CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int backBufferIndex, unsigned int& slot)
			{

				for (auto& [name, uavParam] : pixelShader_ins->uavParameters)
				{
					if (uav.contains(uavParam.registerId))
					{
						commandList->SetGraphicsRootDescriptorTable(slot, uav.at(uavParam.registerId));
						slot++;
					}
				}
			});
	};

	void Renderable::SetIBLRootDescriptorTable(std::vector<DescriptorTableSetter>& setters, ShaderInstance* vertexShader_ins, ShaderInstance* pixelShader_ins, CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_handle_iblIrradiance, CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_handle_iblPrefiteredEnv, CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_handle_iblBRDFLUT)
	{
		if ((
			vertexShader_ins->SRV.iblIrradiance != -1 &&
			vertexShader_ins->SRV.iblPrefiteredEnv != -1 &&
			vertexShader_ins->SRV.iblBRDFLUT != -1
			) ||
			(
				pixelShader_ins->SRV.iblIrradiance != -1 &&
				pixelShader_ins->SRV.iblPrefiteredEnv != -1 &&
				pixelShader_ins->SRV.iblBRDFLUT != -1)
			)
		{
			setters.push_back([gpu_handle_iblIrradiance, gpu_handle_iblPrefiteredEnv, gpu_handle_iblBRDFLUT](CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int backBufferIndex, unsigned int& slot)
				{
					commandList->SetGraphicsRootDescriptorTable(slot++, gpu_handle_iblIrradiance);
					commandList->SetGraphicsRootDescriptorTable(slot++, gpu_handle_iblPrefiteredEnv);
					commandList->SetGraphicsRootDescriptorTable(slot++, gpu_handle_iblBRDFLUT);
				});

		}
	};

	void Renderable::SetSRVRootDescriptorTable(std::vector<DescriptorTableSetter>& setters, std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> gpu_handles_srv)
	{
		setters.push_back([gpu_handles_srv](CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int backBufferIndex, unsigned int& slot)
			{
				for (CD3DX12_GPU_DESCRIPTOR_HANDLE handle : gpu_handles_srv)
				{
					commandList->SetGraphicsRootDescriptorTable(slot++, handle);
				}
			}
		);
	};

	void Renderable::SetShadowMapsSRVDescriptorTable(std::vector<DescriptorTableSetter>& setters, Camera* camera, ShaderInstance* vertexShader_ins, ShaderInstance* pixelShader_ins, CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_handle_srv_shadowMap)
	{
		if (vertexShader_ins->SRV.lightsShadowMap != -1 || pixelShader_ins->SRV.lightsShadowMap != -1) {
			setters.push_back([camera, gpu_handle_srv_shadowMap](
				CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int backBufferIndex, unsigned int& slot)
				{
					if (camera->SceneHasShadowMaps())
						return commandList->SetGraphicsRootDescriptorTable(slot, gpu_handle_srv_shadowMap);
					slot++;
				}
			);
		}
	};

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

	bool Renderable::HasBoundingBoxComputed()
	{
		//return (!animable.empty()) ? boundingBoxCompute->hasSolution : true;
		return true;
	}

	BoundingBox Renderable::GetBoundingBox()
	{
		if (!animable)
		{
			BoundingBox& bb = boundingBox;
			BoundingBox bbw;
			bb.Transform(bbw, world());
			return bbw;
		}
		else
		{
			BoundingBox bb(AABBCenter(), AABBExtent());
			BoundingBox bbw;
			bb.Transform(bbw, world());
			return bbw;
		}
		/*
		BoundingBox& bb = animable.empty() ? boundingBox : boundingBoxCompute->boundingBox;
		BoundingBox bbw;
		bb.Transform(bbw, world());
		return bbw;
		*/
	}

	void Renderable::WriteMaterialVariablesToConstantsBufferSpace(MaterialInstanceID material, ConstantsBufferID cbvData, unsigned int cbvFrameIndex)
	{
		for (auto& [varName, varMapping] : material->variablesMapping)
		{
			UINT8* source = material->variablesBuffer[varMapping.mapping.bufferIndex].data() + varMapping.mapping.offset;
			UINT8* destination = cbvData->mappedConstantBuffer + (cbvFrameIndex * cbvData->alignedConstantBufferSize) + varMapping.mapping.offset;
			memcpy(destination, source, varMapping.mapping.size);
		}
	}

	void Renderable::WriteConstantsBuffer(std::string constantName, void* data, unsigned int backbufferIndex, unsigned int slot, size_t offset)
	{
		if (constantsBuffersLock[backbufferIndex]->load() == true)
			return;
		if (!constantsWriter.contains(constantName))
			return;
		auto& writers = constantsWriter.at(constantName);
		for (auto& writer : writers)
		{
			writer(data, backbufferIndex, offset, slot);
		}
	}

	void Renderable::WriteAnimationConstantsBuffer(unsigned int frame)
	{
		if (animable.empty()) return;

		using namespace Animation;
		WriteBoneTransformationsToConstantsBuffer(SUuuid(), bonesTransformation, frame);
	}

	void Renderable::WriteConstantsBuffer(unsigned int frame)
	{
		XMMATRIX w = world();
		WriteConstantsBuffer("world", &w, frame);
	}

	//Animation
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
	}

	void Renderable::SetCurrentAnimation(SequencePlayer* seqPlayer)
	{
		if (seqPlayer == nullptr)
		{
			if (animationPlayers.contains(MAKESUUUID(unit, uuid())))
			{
				animationPlayers.erase(MAKESUUUID(unit, uuid()));
			}
		}
		else
		{
			animationPlayers.insert_or_assign(MAKESUUUID(unit, uuid()), seqPlayer);
		}
	}

	void Renderable::SetCurrentAnimation(std::string anim, float startTime, float timeFactor, bool play, bool loop)
	{
		if (!animationPlayers.contains(MAKESUUUID(unit, uuid())) || animationPlayers.at(MAKESUUUID(unit, uuid())) != &sequencePlayer)
			SetCurrentAnimation(&sequencePlayer);

		animationSequence(anim);
		sequencePlayer.DestroySequenceTriggers();
		sequencePlayer.sequence = animationsSequences.sequences.at(anim);
		sequencePlayer.loop = loop;
		sequencePlayer.newSequence = true;
		sequencePlayer.ResetFrames();
		sequencePlayer.ApplyFrameValues();
		sequencePlayer.CreateSequenceTriggers();
		StepAnimation(0.0f);
		animationTimeFactor(timeFactor);
		forceAnimation = true;
	}

	void Renderable::CreateAnimationThread()
	{
		animationThreadAlive = std::make_unique<std::atomic_bool>(true);
		animationThread = std::thread([](Renderable* r)
			{
				auto& scene = GetSceneUnit(r->unit);
				std::string animationThreadEvent = "CreateAnimationThread:" + r->name() + ":" + r->uuid();
				while (r->animationThreadAlive->load())
				{
					using namespace Animation;

					r->animationStepLock->wait(false);

					if (!r->animationThreadAlive->load()) break;

					auto& animations = r->animable->animations;

#if defined(_DEVELOPMENT)
					PIXScopedEvent(0, nostd::StringToWString(animationThreadEvent).c_str());
#endif
					TraverseMultiplycationQueue(r->animationTime(), r->animation(), animations, r->bonesTransformation, r->sequenceBoneTransformations, r->globalNodeTransforms);

					r->animationStepLock->store(false);
					r->animationStepLock->notify_one();
				}
			}
		, this);
		animationThread.detach();
	}

	void Renderable::StepAnimation(double elapsedSeconds)
	{
		if (animable.empty()) return;

		if (lastAnimationTime == animationTime() && !forceAnimation)
			return;

		forceAnimation = false;
		lastAnimationTime = animationTime();

		animationStepLock->wait(true);
		animationStepLock->store(true);
		animationStepLock->notify_one();
	}

	std::tuple<XMMATRIX, XMFLOAT3, XMFLOAT3, XMVECTOR, XMFLOAT3> Renderable::GetBoneTransformation(std::string bone)
	{
		if (bone.empty())
		{
			return std::make_tuple<XMMATRIX, XMFLOAT3, XMFLOAT3, XMVECTOR, XMFLOAT3>(
				XMMatrixIdentity(),
				{ 0.0f, 0.0f, 0.0f },
				{ 0.0f, 0.0f, 0.0f },
				XMQuaternionIdentity(),
				{ 1.0f, 1.0f, 1.0f }
			);
		}
		auto& nodesTransforms = globalNodeTransforms;
		return Animation::GetBoneTransformation(world(), nodesTransforms, bone);
	}

	std::vector<std::string> Renderable::GetBones()
	{
		std::vector<std::string> bones = nostd::GetKeysFromMap(animable->animations->bonesParents);
		std::sort(bones.begin(), bones.end(), [](std::string a, std::string b) { return a < b; });
		return bones;
	}

	void Renderable::Destroy()
	{
		using namespace ComputeShader;

		if (animationThreadAlive)
		{
			animationThreadAlive->store(false);
			animationStepLock->store(true);
			animationStepLock->notify_one();
			if (animationThread.joinable())
			{
				animationThread.join();
			}
		}

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
#if defined(_EDITOR)
			Model3DJsonID model3dJson = model();
			model3dJson->RemoveUpdateListener(Model3DJson::Update_animationSequences, SUuuid());
#endif

			DeleteModel3DInstance(model3D());
			//if (!boundingBoxCompute.empty())
			//{
			//	DeleteRenderableBoundingBox(boundingBoxCompute());
			//	boundingBoxCompute.clear();
			//}
		}

		if (at("physicObject").is_string())
		{
			DestroyPhysicObject(at("physicObject"));
		}

#include <Attributes/JDestroy.h>
#include <RenderableAtt.h>
#include <JEnd.h>

		SceneObject::Destroy();
	}

	//RENDER
	void Renderable::Render(SceneUnitId unit, RenderPassInstanceID renderPass, CameraID camera)
	{
		using namespace Animation;
		using namespace Scene;

		if (!RenderReady() || markedForDelete || !visible() || !materials.contains(renderPass) || renderException) return;

		std::tuple<CameraID, RenderPassInstanceID> key = std::make_tuple(camera, renderPass);

		auto& scene = GetSceneUnit(unit);
		unsigned int frame = scene->Frame();
		auto& commandList = scene->GetCommandList();

#if defined(_DEVELOPMENT)
		{
			PIXScopedEvent(commandList.p, 0, name().c_str());
#endif
			auto& meshesRootSignatures = rootSignatures.at(renderPass);
			auto& meshesPipelineStates = pipelineStates.at(renderPass);

			for (unsigned int i = 0; i < meshes.size(); i++)
			{
				if (skipMeshes_contains(i)) continue;

				commandList->IASetPrimitiveTopology(topology());
				commandList->SetGraphicsRootSignature(meshesRootSignatures.at(i));
				commandList->SetPipelineState(meshesPipelineStates.at(i));

				unsigned int slot = 0U;
				for (auto& setter : descriptorsRenders.at(key).at(i))
				{
					setter(commandList, frame, slot);
				}

				auto& mesh = meshes.at(i);
				commandList->IASetVertexBuffers(0, 1, &mesh->vbvData.vertexBufferView);
				commandList->IASetIndexBuffer(&mesh->ibvData.indexBufferView);
				commandList->DrawIndexedInstanced(mesh->ibvData.indexBufferView.SizeInBytes / sizeof(unsigned int), 1, 0, 0, 0);
			}
#if defined(_DEVELOPMENT)
		}
#endif
	}

	bool Renderable::RenderReady()
	{
		return renderReady;
	}

	void Renderable::RenderReady(bool value)
	{
		renderReady = value;
	}

	std::vector<ScriptBinding> Renderable::GetScriptBindings()
	{
		std::vector<ScriptBinding> bindings;
		for (auto& [name, _] : at("controllers").items())
		{
			bindings.push_back(ScriptBinding(uuid(), name, name));
		}
		return bindings;
	}

#if defined(_EDITOR)
	std::map<std::string, ScriptBinding> Renderable::GetScriptBindingOptions()
	{
		std::map<std::string, ScriptBinding> options = SceneObject::GetScriptBindingOptions();

		for (auto& [key, _] : at("controllers").items())
		{
			std::string name = std::string(at("name")) + "/" + std::string(key);
			options.insert_or_assign(name, ScriptBinding(at("uuid"), key));
		}

		for (unsigned int i = 0; i < at("physicObject").size(); i++)
		{
			std::string name = std::string(at("name")) + "/physicObject/" + std::to_string(i);
			options.insert_or_assign(name, ScriptBinding(at("uuid"), i));
		}

		return options;
	}
#endif

	void RenderablesStep(SceneUnitId id, float dt)
	{
		auto& Renderables = GetRenderables(id);
		std::set<RenderableID> r;
		std::transform(Renderables.begin(), Renderables.end(), std::inserter(r, r.begin()), [&](auto o) { return MAKESUUUID(id, o); });
		std::erase_if(r, [](RenderableID r) { return !SceneObjectExists(r()) || !r->RenderReady(); });

		auto checkCamera = [](RenderableID r)
			{
				if (!r->dirty(Renderable::Update_cameras)) return;
				r->clean(Renderable::Update_cameras);

				auto& prev = r->UpdatePrevValues.at("cameras");
				std::vector<std::string> prevCams;
				for (auto& it : prev.items())
				{
					if (it.value() != "")
					{
						prevCams.push_back(it.value());
					}
				}
				auto& curr = r->at("cameras");
				std::vector<std::string> currCams;
				for (auto& it : curr.items())
				{
					if (it.value() != "")
					{
						currCams.push_back(it.value());
					}
				}
				if (prevCams != currCams)
				{
					LoadingProcessor loading = CreateLoadingProcessor();
					r->UnbindCameras();
					r->BindShadowMapCameras();
					r->BindCameras();
				}
			};

		std::for_each(r.begin(), r.end(), checkCamera);

		std::set<RenderableID> cleanRot;
		std::for_each(r.begin(), r.end(), [&](auto& o)
			{
				if (o->dirty(Renderable::Update_rotation))
				{
					o->updateRotationQ();
					cleanRot.insert(o);
				}
			}
		);

		std::set<RenderableID> rGPose;
		std::copy_if(r.begin(), r.end(), std::inserter(rGPose, rGPose.begin()), [](auto& o)
			{
				bool updated = o->dirty(Renderable::Update_position) || o->dirty(Renderable::Update_rotation);
				return updated && !o->at("physicObject").empty();
			}
		);

		for (auto& rp : rGPose)
		{
			rp->clean(Renderable::Update_position);
			rp->clean(Renderable::Update_rotation);
			for (PhysicObjectID phO : GetPhysicsObjectsBySceneObjectUUID(rp->SUuuid()))
			{
				phO->UpdateGlobalPoseFromRenderable();
			}
		}

		std::set<RenderableID> rGeom;
		std::copy_if(r.begin(), r.end(), std::inserter(rGeom, rGeom.begin()), [](auto& o)
			{
				bool updated = o->dirty(Renderable::Update_scale);
				o->clean(Renderable::Update_scale);
				return updated && !o->at("physicObject").empty();
			}
		);

		for (auto& rp : rGeom)
		{
			for (PhysicObjectID phO : GetPhysicsObjectsBySceneObjectUUID(rp->SUuuid()))
			{
				phO->DestroyPhysicsBehavior();
				phO->CreatePhysicsBehavior();
#if defined(_EDITOR)
				phO->CreatePhysicsAvatar();
#endif
			}
		}

		std::set<RenderableID> rMeshMaterial;
		std::copy_if(r.begin(), r.end(), std::inserter(rMeshMaterial, rMeshMaterial.begin()), [](auto& o)
			{
				return o->dirty(Renderable::Update_meshMaterial);
			}
		);

		if (rMeshMaterial.size() > 0)
		{
			for (auto o : rMeshMaterial)
			{
				o->clean(Renderable::Update_meshMaterial);
			}
		}

		std::set<RenderableID> rDepthStencil;
		std::copy_if(r.begin(), r.end(), std::inserter(rDepthStencil, rDepthStencil.begin()), [](auto& o)
			{
				return o->dirty(Renderable::Update_depthStencil);
			}
		);

		if (rDepthStencil.size() > 0)
		{
			for (auto o : rDepthStencil)
			{
				o->clean(Renderable::Update_depthStencil);
			}
		}

		for (auto o : cleanRot)
		{
			o->clean(Renderable::Update_rotation);
		}

		std::set<RenderableID> todelete;
		std::copy_if(r.begin(), r.end(), std::inserter(todelete, todelete.begin()), [](auto r)
			{
				return r->markedForDelete;
			}
		);

		for (auto renderable : todelete)
		{
			if (renderable->deleteFrames > 0U)
			{
				renderable->deleteFrames--;
				continue;
			}

			auto list = GetPhysicsObjectsBySceneObjectUUID(renderable->SUuuid());
			for (PhysicObjectID phO : list)
			{
				phO->DestroyPhysicsBehavior();
#if defined(_EDITOR)
				phO->DestroyPhysicsAvatar();
#endif
				DestroyPhysicObject(phO());
				renderable->clear();
			}
			EraseRenderableFromRenderables(renderable->unit, renderable.uuid());
			EraseRenderableFromAnimables(renderable->unit, renderable.uuid());
			EraseRenderableFromShadowCasts(renderable->unit, renderable.uuid());
			DeleteRenderableSceneObject(renderable);
		}

		for (auto& [renderable, player] : animationPlayers)
		{
			if (renderable->markedForDelete || !renderable->RenderReady())
				continue;

#if defined(_EDITOR)
			if (Editor::IsPlaying(renderable->unit) && !Editor::IsPaused(renderable->unit))
#endif
			{
				player->Step(dt * 1000.0f * renderable->animationTimeFactor());
			}
			player->ApplyFrameValues();
			player->ApplyFrameTriggerValues();
		}
	}

	void DestroyRenderables()
	{
		for (auto& [id, container] : RenderableSUsceneObjects)
		{
			for (auto& [uuid, _] : container)
			{
				RenderableID r = MAKESUUUID(id, uuid);
				DeleteRenderableSceneObject(r);
			}
		}
#include <TrackUUID/JClear.h>
#include <RenderableAtt.h>
#include <JEnd.h>
	}

	void DestroyRenderables(SceneUnitId id)
	{
		std::set<JUUID> uuids;
		std::transform(RenderableSUsceneObjects.at(id).begin(), RenderableSUsceneObjects.at(id).end(), std::inserter(uuids, uuids.begin()), [](auto& pair) { return pair.first; });
		for (auto& uuid : uuids)
		{
			DeleteRenderableSceneObject(MAKESUUUID(id, uuid));
		}
#include <TrackUUID/JClearUnit.h>
#include <RenderableAtt.h>
#include <JEnd.h>
	}

	void DeleteRenderable(SceneUnitId id, JUUID uuid)
	{
		RenderableID r = MAKESUUUID(id, uuid);
		r->markedForDelete = true;
	}

	/*
	void RunBoundingBoxComputeShaders(SceneUnitId id)
	{
		using namespace Scene;

		if (!RenderableSUsceneObjects.contains(id)) return;

		auto& scene = GetSceneUnit(id);
		for (auto& [uuid, _] : RenderableSUsceneObjects.at(id))
		{
			RenderableID renderable = MAKESUUUID(id, uuid);

			if (renderable->boundingBoxCompute.empty() ||
				!renderable->boundingBoxCompute->canCompute ||
				!renderable->RenderReady()
				)
				continue;

			renderable->boundingBoxCompute->Compute(id);
		}
	}
	*/

	/*
	void RunBoundingBoxComputeShadersSolution(SceneUnitId id)
	{
		using namespace Scene;

		if (!RenderableSUsceneObjects.contains(id)) return;

		auto& scene = GetSceneUnit(id);
		for (auto& [uuid, _] : RenderableSUsceneObjects.at(id))
		{
			RenderableID renderable = MAKESUUUID(id, uuid);

			if (renderable->boundingBoxCompute.empty() ||
				!renderable->boundingBoxCompute->canCompute ||
				!renderable->RenderReady()
				)
				continue;

			renderable->boundingBoxCompute->Solution(id);
		}
	}
	*/

#if defined(_EDITOR)
	void WriteRenderablesJson(SceneUnitId id, nlohmann::json& json)
	{
#include <Editor/JSaveFile.h>
#include <RenderableAtt.h>
#include <JEnd.h>
	}
#endif
}
