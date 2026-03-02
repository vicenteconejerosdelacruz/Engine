#include "pch.h"
#include "Renderable.h"
#include <Scene.h>
#include <Renderer.h>
#include <DeviceUtils/RootSignature/RootSignature.h>
#include <DeviceUtils/PipelineState/PipelineState.h>
#include <Renderable/RenderableBoundingBox.h>
#include <NoMath.h>

extern std::unique_ptr<Renderer> renderer;

#if defined(_EDITOR)
namespace Editor
{
	extern void SelectRenderable(RenderableID renderable);
	extern bool IsPlaying(SceneUnitId unit);
	extern bool IsPaused(SceneUnitId unit);
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
			boundingBoxCompute = CreateRenderableBoundingBox(MAKESUUUID(unit, uuid()));
			sequencePlayer.renderable = SUuuid();
		}

		SetInitialConditions();

#if defined(_EDITOR)
		OnPick = [&] { Editor::SelectRenderable(SUuuid()); };
#endif
	}

	void Renderable::SetInitialConditions()
	{
		animationTransformation = XMMatrixIdentity();

		if (!animable.empty())
		{
			animationTime(0.0f);
			SetCurrentAnimation(animationSequence(), animationTime(), animationTimeFactor(), animationPlay(), animationLoop());
			StepAnimation(0.0f); //take an empty T-Pose step so the skinning can be performed
		}

		updateRotationQ();
	}

	void Renderable::BindToScene()
	{
#include <TrackUUID/JInsert.h>
#include <RenderableAtt.h>
#include <JEnd.h>

		BindCameras();
		BindShadowMapCameras();
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
		if (!boundingBoxCompute.empty())
		{
			DeleteRenderableBoundingBox(boundingBoxCompute());
			boundingBoxCompute.clear();
		}
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
		return (!animationUseTransformation()) ? worldM : XMMatrixMultiply(animationTransformation, worldM);
	}

	void Renderable::CreateMeshInstances()
	{
		using namespace Templates;
		if (!meshMaterial().mesh.empty() && meshMaterial().mesh.contains("primitive") && !meshMaterial().mesh.at("primitive").empty() && !meshMaterial().materialUUID.empty())
		{
			nlohmann::json mesh = meshMaterial().mesh;
			meshes.push_back(GetMeshInstance(unit, mesh)->uuid);
			CreateBoundingBox();
		}
		else if (Model3DTemplateExist(model()))
		{
			CreateModel3DInstance(model(), [this]
				{
					return std::make_unique<Model3DInstance>(unit, model(), uuid()/*, [this](JUUID model3DTemplateUUID)
						{
							auto& model = GetModel3DTemplate(model3DTemplateUUID);
							if (model->dirty(Model3DJson::Update_animationSequences))
							{
								RebuildAnimationSequences();
							}
						}*/
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

		if (!meshMaterial().mesh.empty() && meshMaterial().mesh.contains("primitive") && !meshMaterial().mesh.at("primitive").empty() && !meshMaterial().materialUUID.empty())
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

			/*
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
				std::copy_if(pmo.begin(), pmo.end(), std::back_inserter(mpmo), [i](PassMaterialOverride& o) { return o.meshIndex == i; });
				auto& mesh = meshes.at(i);
				JUUID matUUID = mm.at(i).materialUUID;
				if (!MaterialTemplateExist(matUUID))
				{
					matUUID = GetMaterialUUIDByName(fallbackMaterialName); //fallback
				}
				materials[pass].push_back(pass->GetRenderPassMaterialInstance(unit, matUUID, mesh, shadowed(), mpmo, uuid()));
			}
			*/
		}
		else if (!model3D.empty())
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
				ConstantsBufferID cbuffer = CreateConstantsBuffer(size, Renderer::numFrames, name() + "." + std::to_string(j) + "." + mesh->uuid);
				for (unsigned int n = 0; n < Renderer::numFrames; n++)
				{
					WriteMaterialVariablesToConstantsBufferSpace(mi, cbuffer, n);
				}
				constantsBuffers[pass][i].push_back(cbuffer);
			}
		}
	}

	void Renderable::DestroyConstantsBuffersInstances(CameraID cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			DestroyRenderPassConstantsBuffersInstances(rp);
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

	void Renderable::WriteMaterialVariablesToConstantsBufferSpace(MaterialInstanceID material, ConstantsBufferID cbvData, unsigned int cbvFrameIndex)
	{
		for (auto& [varName, varMapping] : material->variablesMapping)
		{
			UINT8* source = material->variablesBuffer[varMapping.mapping.bufferIndex].data() + varMapping.mapping.offset;
			UINT8* destination = cbvData->mappedConstantBuffer + (cbvFrameIndex * cbvData->alignedConstantBufferSize) + varMapping.mapping.offset;
			memcpy(destination, source, varMapping.mapping.size);
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
		WriteConstantsBuffer("world", w, frame);
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
		sequencePlayer.sequence = animationsSequences.sequences.at(anim);
		sequencePlayer.loop = loop;
		sequencePlayer.newSequence = true;
		sequencePlayer.ResetFrames();
	}

	void Renderable::StepAnimation(double elapsedSeconds)
	{
		if (animable.empty()) return;

		using namespace Animation;
		auto& animations = animable->animations;

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

		if (at("physicObject").is_string())
		{
			DestroyPhysicObject(at("physicObject"));
		}

#include <Attributes/JDestroy.h>
#include <RenderableAtt.h>
#include <JEnd.h>
	}

	//RENDER
	void Renderable::Render(SceneUnitId unit, RenderPassInstanceID renderPass, CameraID camera)
	{
		using namespace Animation;
		using namespace Scene;

		if (!RenderReady() || markedForDelete || !visible() || !materials.contains(renderPass) || renderException) return;

		auto& scene = GetSceneUnit(unit);
		unsigned int frame = scene->Frame();
		auto& commandList = scene->GetCommandList();

#if defined(_DEVELOPMENT)
		PIXBeginEvent(commandList.p, 0, name().c_str());
#endif
		auto& meshesMaterials = materials.at(renderPass);
		auto& meshesRootSignatures = rootSignatures.at(renderPass);
		auto& meshesPipelineStates = pipelineStates.at(renderPass);

		auto setConstantsBuffersDescriptorTables = [&](auto& cbuffers, unsigned int& slot)
			{
				for (auto& cbuffer : cbuffers) {
					cbuffer->SetRootDescriptorTable(commandList, slot, frame);
				}
			};
		auto setCameraConstantsBufferDescriptorTable = [&](auto& material, unsigned int& slot)
			{
				if (!camera.empty() && material->ShaderInstanceHasRegister([](ShaderInstanceID binary) { return binary->CBV.camera; })) {
					camera->cameraCb->SetRootDescriptorTable(commandList, slot, frame);
				}
			};
		auto setLightsConstantsBufferDescriptorTable = [&](MaterialInstanceID material, unsigned int& slot)
			{
				if (material->ShaderInstanceHasRegister([](ShaderInstanceID binary) { return binary->CBV.light; })) {
					camera->GetLightsConstantsBuffer()->SetRootDescriptorTable(commandList, slot, frame);
				}
			};
		auto setShadowMapsConstantsBufferDescriptorTable = [&](auto& material, unsigned int& slot)
			{
				if (material->ShaderInstanceHasRegister([](ShaderInstanceID binary) { return binary->CBV.lightsShadowMap; })) {
					if (camera->SceneHasShadowMaps())
						return camera->GetShadowMapsConstantsBuffer()->SetRootDescriptorTable(commandList, slot, frame);
					slot++;
				}
			};
		auto setSkinningConstantsBufferDescriptorTable = [&](auto& material, unsigned int& slot)
			{
				if (material->ShaderInstanceHasRegister([this](ShaderInstanceID binary) { return binary->CBV.animation; })) {
					if (!animable.empty())
						return GetAnimatedConstantsBuffer(SUuuid())->SetRootDescriptorTable(commandList, slot, frame);
					slot++;
				}
			};
		auto setUAVRootDescriptorTable = [&](auto& material, unsigned int& slot)
			{
				material->SetUAVRootDescriptorTable(commandList, slot);
			};
		auto setIBLRootDescriptorTable = [&](auto& material, unsigned int& slot)
			{
				if (material->ShaderInstanceHasRegister([](ShaderInstanceID binary) { return
					(binary->SRV.iblIrradiance == -1 || binary->SRV.iblPrefiteredEnv == -1 || binary->SRV.iblBRDFLUT == -1) ? -1 : 1; })
					)
				{
					camera->SetIBLRootDescriptorTables(commandList, slot);
				}
			};
		auto setSRVRootDescriptorTable = [&](auto& material, unsigned int& slot)
			{
				material->SetSRVRootDescriptorTable(commandList, slot);
			};
		auto setShadowMapsSRVDescriptorTable = [&](auto& material, unsigned int& slot)
			{
				if (material->ShaderInstanceHasRegister([](ShaderInstanceID binary) { return binary->SRV.lightsShadowMap; })) {
					if (camera->SceneHasShadowMaps())
						return commandList->SetGraphicsRootDescriptorTable(slot, GetShadowMapGpuDescriptorHandleStart(unit));
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

	bool Renderable::RenderReady()
	{
		return renderReady;
	}

	void Renderable::RenderReady(bool value)
	{
		renderReady = value;
	}

	void RenderablesStep(SceneUnitId unit, float dt)
	{
		auto& Renderables = GetRenderables(unit);
		std::set<RenderableID> r;
		std::transform(Renderables.begin(), Renderables.end(), std::inserter(r, r.begin()), [&](auto o) { return MAKESUUUID(unit, o); });

		//is this(hack) or fix the loading system
		auto& scene = GetSceneUnit(unit);
		for (auto& ren : r)
		{
			if (!ren->RenderReady() && scene->IsBound(ren.uuid()))
			{
				ren->RenderReady(true);
				scene->EraseRenderableFromLoadingPool(ren);
			}
		}

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
			for (JUUID p0 : GetPhysicsObjectsBySceneObjectUUID(rp->SUuuid()))
			{
				auto& phO = GetPhysicObject(p0);
				phO->DestroyPhisicsBehavior();
				phO->CreatePhysicsBehavior();
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
				o->visible(false);
			}
			auto& scene = GetSceneUnit(unit);
			scene->SubmitForLoading([=]
				{
					auto destroyMeshInstance = [](auto& vec) { for (auto& mesh : vec) { DestroyMeshInstance(mesh()); } };
					for (auto o : rMeshMaterial)
					{
						destroyMeshInstance(o->meshes);
						o->meshes.clear();
						o->CreateMeshInstances();
					}
				}
			);
			scene->PushLoadingExecutionCallback([=]
				{
					for (auto o : rMeshMaterial)
					{
						o->visible(true);
					}
				}
			);
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
				o->visible(false);
			}
			auto& scene = GetSceneUnit(unit);
			scene->SubmitForLoading([=]
				{
					for (auto o : rDepthStencil)
					{
						for (auto cam : o->bindedCameras)
						{
							o->CreatePipelineStates(cam);
						}
					}
				}
			);
			scene->PushLoadingExecutionCallback([=]
				{
					for (auto o : rDepthStencil)
					{
						o->visible(true);
					}
				}
			);
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
				player->Step(dt * 1000.0f);
			}
			player->ApplyFrameValues(renderable);
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

	void RunBoundingBoxComputeShaders(SceneUnitId id)
	{
		for (auto& [uuid, _] : RenderableSUsceneObjects.at(id))
		{
			RenderableID renderable = MAKESUUUID(id, uuid);

			if (renderable->boundingBoxCompute.empty())
				continue;

			renderable->boundingBoxCompute->Compute(id);
		}
	}

	void RunBoundingBoxComputeShadersSolution(SceneUnitId id)
	{
		for (auto& [uuid, _] : RenderableSUsceneObjects.at(id))
		{
			RenderableID renderable = MAKESUUUID(id, uuid);

			if (renderable->boundingBoxCompute.empty())
				continue;

			renderable->boundingBoxCompute->Solution(id);
		}
	}

#if defined(_EDITOR)
	void WriteRenderablesJson(SceneUnitId id, nlohmann::json& json)
	{
#include <Editor/JSaveFile.h>
#include <RenderableAtt.h>
#include <JEnd.h>
	}
#endif
}
