#include "pch.h"
#include <Scene.h>
#include <SceneObject.h>
#include <Renderer.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <Level.h>
#include <unordered_map>

extern std::unique_ptr<Renderer> renderer;
#if defined(_EDITOR)
namespace Editor
{
	extern bool IsPlaying(SceneUnitId id);
	extern bool IsPaused(SceneUnitId id);
	extern void DrawEditor();
	extern void EraseSceneObjectFromSelection(SceneUnitId unit, JUUID uuid);
	extern void MarkSceneUnitAsModified(SceneUnitId id);
	extern void MarkScenePanelAssetsAsDirty();
	extern void UpdateBoundingBox(SceneUnitId id);
	extern void WriteSceneUnitEditorPlayCameraConstantsBuffer(SceneUnitId id);
	extern void SwitchToSceneUnitEditorCamera(SceneUnitId id);
	extern void SwitchToSceneUnitEditorPlayCamera(SceneUnitId id);
	extern void RemoveSceneUnitEditorCameraFromWindowCameras(SceneUnitId id);
	extern void AddSceneUnitEditorCameraToWindowCameras(SceneUnitId id);
	extern void HandleEditorMouseMovements(SceneUnitId id);
	extern void CreateRegisteredBillboards(SceneUnitId id);
	extern void UpdateBillboards();
	extern void BindRenderableToPickingPass(RenderableID uuid);
	extern void DeleteSceneUnitLevel(SceneUnitId id);
	extern void DeleteSceneUnitGizmos(SceneUnitId id);
	extern void DeleteSceneUnitSelection(SceneUnitId id);
	extern void DeleteSceneUnitGameController(SceneUnitId id);
	extern void DeleteSceneUnitPhysicsController(SceneUnitId id);
	extern void DeleteSceneUnitBoundingBox(SceneUnitId id);
	extern void DeleteSceneUnitBillboards(SceneUnitId id);
	extern void DeleteSceneUnitEditorIndependentCamera(SceneUnitId id);
};
#endif

namespace Game
{
	extern void DestroySeneUnitGame(SceneUnitId id);
};

namespace Scene
{
	std::map<SceneUnitId, std::unique_ptr<SceneUnit>> scenesUnits;
	std::map<SceneUnitId, SceneUnitId> attachedUnits; //<Attached, Destination>
	std::set<SceneUnitId> renderableSceneUnits;
	std::map<size_t, CommandsProcessor> loadingProcessor;

	std::tuple<size_t, CommandsProcessor&> CreateLoadingProcessor()
	{
		size_t thread_id = nostd::threadIdHash();
		loadingProcessor.insert_or_assign(thread_id, CommandsProcessor(renderer->d3dDevice, 1));
		return std::tie(thread_id, loadingProcessor.at(thread_id));
	}

	CommandsProcessor& GetLoadingProcessor(size_t id)
	{
		id = id ? id : nostd::threadIdHash();
		return loadingProcessor.at(id);
	}

	void DestroyLoadingProcessor(size_t id)
	{
		id = id ? id : nostd::threadIdHash();
		loadingProcessor.erase(id);
	}

	void CreateSceneLevelAsync(std::string filename, nlohmann::json data, std::function<void(SceneUnitId)> levelLoaded, std::function<void(std::string, unsigned int, unsigned int)> progress)
	{
		using namespace Scene::Level;

		std::thread levelThread([](std::string filename, nlohmann::json data, std::function<void(SceneUnitId)> levelLoaded, std::function<void(std::string, unsigned int, unsigned int)> progress)
			{
				SceneUnitId id = nostd::threadIdHash();

				auto& scene = CreateScene(id, filename, Renderer::numFrames);
				auto [lid, _] = CreateLoadingProcessor();

				LoadLevel(scene, filename, data, progress);
				levelLoaded(id);
			}, filename, data, levelLoaded, progress
		);
		levelThread.detach();
	}

	void CreateIsolatedSceneLevelAsync(std::string filename, nlohmann::json data, std::function<void(SceneUnitId)> levelLoaded, std::function<void(std::string, unsigned int, unsigned int)> progress)
	{
		using namespace Scene::Level;

		std::thread levelThread([](std::string filename, nlohmann::json data, std::function<void(SceneUnitId)> levelLoaded, std::function<void(std::string, unsigned int, unsigned int)> progress)
			{
				SceneUnitId unit = nostd::threadIdHash();
				auto& scene = CreateScene(unit, filename);
				scene->SetIsolated(true);
				auto [lid, _] = CreateLoadingProcessor();

				LoadLevel(scene, filename, data, progress);
				levelLoaded(unit);
			}, filename, data, levelLoaded, progress
		);
		levelThread.detach();
	}

	void AttachLevelIntoScene(SceneUnitId parentUnit, std::string filename, nlohmann::json data, std::function<void(SceneUnitId)> levelLoaded, std::function<void(std::string, unsigned int, unsigned int)> progress)
	{
		using namespace Scene::Level;

		std::thread levelThread([](SceneUnitId parentUnit, std::string filename, nlohmann::json data, std::function<void(SceneUnitId)> levelLoaded, std::function<void(std::string, unsigned int, unsigned int)> progress)
			{
				auto& scene = GetSceneUnit(parentUnit);
				auto [lid, _] = CreateLoadingProcessor();

				LoadLevel(scene, filename, data, progress);
				levelLoaded(parentUnit);
			}, parentUnit, filename, data, levelLoaded, progress
		);
		levelThread.detach();
	}

	std::unique_ptr<SceneUnit>& CreateScene(SceneUnitId unit, std::string unitName, unsigned int numProcessors)
	{
		scenesUnits.insert_or_assign(unit, std::make_unique<SceneUnit>(unit, unitName));
		//scenesUnits.at(unit)->InitLoadingProcessor(renderer->d3dDevice, unit, 1U);
		if (numProcessors > 0U)
		{
			scenesUnits.at(unit)->InitFrame2FrameProcessor(renderer->d3dDevice, numProcessors, unit);
			scenesUnits.at(unit)->InitComputeProcessor(renderer->d3dDevice, numProcessors, unit);
		}
		return scenesUnits.at(unit);
	}

	void CreateSceneUnitSceneObjects(SceneUnitId unit)
	{
		CreateRenderableSceneObjects(unit);
		CreateCameraSceneObjects(unit);
		CreateLightSceneObjects(unit);
		CreateSoundFXSceneObjects(unit);
		CreatePhysicSceneSceneObjects(unit);
		CreateTriggerSceneObjects(unit);
		CreateBoundarySceneObjects(unit);
		CreateSceneControllerSceneObjects(unit);
	}

	void DestroyScene(SceneUnitId id)
	{
#if defined(_EDITOR)
		using namespace Editor;
#endif
		DestroySeneUnitGame(id);

		auto& scene = GetSceneUnit(id);
		scene->DestroySceneObjects();

#if defined(_EDITOR)
		DeleteSceneUnitLevel(id);
		DeleteSceneUnitGizmos(id);
		DeleteSceneUnitSelection(id);
		DeleteSceneUnitGameController(id);
		DeleteSceneUnitPhysicsController(id);
		DeleteSceneUnitBoundingBox(id);
		DeleteSceneUnitBillboards(id);
		DeleteSceneUnitEditorIndependentCamera(id);
#endif
		scene->CallDeleteCallback();
		scenesUnits.erase(id);
	}

	void DestroyScenes(bool inmediate)
	{
		std::set<SceneUnitId> units;
		std::transform(scenesUnits.begin(), scenesUnits.end(), std::inserter(units, units.begin()), [](auto& pair) { return pair.first; });
		for (auto& id : units)
		{
			DestroyScene(id);
		}
	}

	bool SceneUnitExits(SceneUnitId unit)
	{
		return scenesUnits.contains(unit);
	}

	void SceneUnitsStep()
	{
		std::set<SceneUnitId> scenesToDelete;
		for (auto& [unit, scene] : scenesUnits)
		{
			if (scene->MarkedForDelete())
			{
				if (scene->DeleteFrames() > 0)
				{
					scene->DecreaseDeleteFrames();
				}
				else if (scene->DeleteFrames() == 0)
				{
					scenesToDelete.insert(unit);
				}
			}
		}

		for (auto& id : scenesToDelete)
		{
			DestroyScene(id);
		}
	}

	std::unique_ptr<SceneUnit>& GetSceneUnit(SceneUnitId unit)
	{
		return scenesUnits.at(unit);
	}

	size_t GetSceneUnitsCount()
	{
		return scenesUnits.size();
	}

	std::set<SceneUnitId> GetSceneUnitIds()
	{
		std::set<SceneUnitId> unitIds;
		std::transform(scenesUnits.begin(), scenesUnits.end(), std::inserter(unitIds, unitIds.begin()), [](auto& pair) { return pair.first; });
		return unitIds;
	}

	SceneUnitId GetNextSceneUnitId(SceneUnitId id)
	{
		std::vector<SceneUnitId> unitIds;
		std::transform(scenesUnits.begin(), scenesUnits.end(), std::back_inserter(unitIds), [](auto& pair) { return pair.first; });
		auto it = std::find(unitIds.begin(), unitIds.end(), id);
		int index = static_cast<int>(it - unitIds.begin());
		return unitIds.at((index + 1) % unitIds.size());
	}

	bool SceneIsIsolated(SceneUnitId id)
	{
		return scenesUnits.at(id)->IsIsolated();
	}

	void ResizeReleaseScenePasses()
	{
#if defined(_EDITOR)
		using namespace Editor;
#endif
		for (auto& [id, _] : scenesUnits)
		{
#if defined(_EDITOR)
			RemoveSceneUnitEditorCameraFromWindowCameras(id);
#endif
			auto& cameras = GetWindowCameras(id);
			for (auto& uuid : cameras)
			{
				CameraID cam = MAKESUUUID(id, uuid);
				cam->ResizeReleasePasses();
			}
#if defined(_EDITOR)
			AddSceneUnitEditorCameraToWindowCameras(id);
#endif
		}
	}

	void ResizeScenePasses(unsigned int width, unsigned int height)
	{
#if defined(_EDITOR)
		using namespace Editor;
#endif
		for (auto& [id, _] : scenesUnits)
		{
#if defined(_EDITOR)
			RemoveSceneUnitEditorCameraFromWindowCameras(id);
#endif
			auto& cameras = GetWindowCameras(id);
			for (auto& uuid : cameras)
			{
				CameraID cam = MAKESUUUID(id, uuid);
				cam->ResizePasses(width, height);
			}
#if defined(_EDITOR)
			AddSceneUnitEditorCameraToWindowCameras(id);
#endif
		}
	}

	std::set<JUUID>& GetSceneObjects(SceneUnitId unit, SceneObjectType type)
	{
		if (!scenesUnits.at(unit)->GetSceneObjects().contains(type))
			scenesUnits.at(unit)->GetSceneObjects()[type].clear();
		return scenesUnits.at(unit)->GetSceneObjects().at(type);
	}

	std::unordered_map<JUUID, SceneObjectType>& GetSceneObjectsTypes(SceneUnitId unit)
	{
		return scenesUnits.at(unit)->GetSceneObjectTypes();
	}

	std::set<JUUID>& GetUnboundedSceneObjects(SceneUnitId id)
	{
		return scenesUnits.at(id)->GetUnboundedSceneObjects();
	}

	SceneObjectType GetSceneObjectType(SceneUnitId id, JUUID uuid)
	{
		return scenesUnits.at(id)->GetSceneObjectTypes().at(uuid);
	}

	bool SceneObjectExists(SceneUnitId id, JUUID uuid)
	{
		return scenesUnits.at(id)->GetSceneObjectTypes().contains(uuid);
	}

	bool SceneObjectExists(SUUUID suuuid)
	{
		return scenesUnits.at(std::get<0>(suuuid))->GetSceneObjectTypes().contains(std::get<1>(suuuid));
	}

	void MoveSceneObjectUnit(JUUID uuid, SceneUnitId fromId, SceneUnitId toId)
	{
		auto mover = [](auto mvfn, auto uuid, auto fromId, auto toId)
			{
				auto* so = GetSceneObjectPointer(fromId, uuid);
				so->unit = toId;
				auto& obj = mvfn(fromId).at(uuid);
				mvfn(toId).insert_or_assign(uuid, std::move(obj));
				mvfn(fromId).erase(uuid);
				so->BindToScene();
			};
		std::map<SceneObjectType, std::function<void(JUUID, SceneUnitId, SceneUnitId)>> moveObject =
		{
			{ SO_Renderables, [&](JUUID uuid, SceneUnitId fromId, SceneUnitId toId)
			{
				mover(GetRenderablesSceneObjects,uuid,fromId,toId);
			}
			},
			{ SO_Cameras, [&](JUUID uuid, SceneUnitId fromId, SceneUnitId toId)
			{
				mover(GetCamerasSceneObjects,uuid,fromId,toId);
			}
			},
			{ SO_Lights, [&](JUUID uuid, SceneUnitId fromId, SceneUnitId toId)
			{
				mover(GetLightsSceneObjects,uuid,fromId,toId);
			}
			},
			{ SO_SoundEffects, [&](JUUID uuid, SceneUnitId fromId, SceneUnitId toId)
			{
				mover(GetSoundFXsSceneObjects,uuid,fromId,toId);
			}
			},
			{ SO_PhysicScenes, [&](JUUID uuid, SceneUnitId fromId, SceneUnitId toId)
			{
				mover(GetPhysicScenesSceneObjects,uuid,fromId,toId);
			}
			},
			{ SO_Triggers, [&](JUUID uuid, SceneUnitId fromId, SceneUnitId toId)
			{
				mover(GetTriggersSceneObjects,uuid,fromId,toId);
			}
			},
			{ SO_Boundaries, [&](JUUID uuid, SceneUnitId fromId, SceneUnitId toId)
			{
				mover(GetBoundarysSceneObjects,uuid,fromId,toId);
			}
			},
			{ SO_SceneControllers, [&](JUUID uuid, SceneUnitId fromId, SceneUnitId toId)
			{
				mover(GetSceneControllersSceneObjects,uuid,fromId,toId);
			}
			},
		};

		SceneObjectType type = GetSceneObjectType(fromId, uuid);
		moveObject.at(type)(uuid, fromId, toId);
	}

	void ResetRenderableScenes()
	{
		renderableSceneUnits.clear();
	}

	void EnableSceneUnitRendering(SceneUnitId id)
	{
		renderableSceneUnits.insert(id);
	}

	void RemoveSceneUnitRendering(SceneUnitId id)
	{
		renderableSceneUnits.erase(id);
	}

	bool SceneUnitRenderingExists(SceneUnitId id)
	{
		return renderableSceneUnits.contains(id);
	}

#if defined(_EDITOR)
	bool SceneCanBuildAssetsTree(SceneUnitId id)
	{
		if (SceneUnitRenderingExists(id))
		{
			return GetSceneUnit(id)->CanBuildAssetsTree();
		}
		return false;
	}
#endif

	void BindSceneObjects(SceneUnitId id)
	{
		std::unordered_map<SceneObjectType, std::function<void(SceneUnitId, JUUID)>> typeBinder =
		{
			{
				SO_Renderables, [](SceneUnitId id, JUUID uuid)
				{
					auto& so = GetRenderableSceneObject(id, uuid);
					so->BindToScene();
				}
			},
			{
				SO_Cameras, [](SceneUnitId id, JUUID uuid)
				{
					auto& so = GetCameraSceneObject(id, uuid);
					so->BindToScene();
				}
			},
			{
				SO_Lights, [](SceneUnitId id, JUUID uuid)
				{
					auto& so = GetLightSceneObject(id, uuid);
					so->BindToScene();
				}
			},
			{
				SO_SoundEffects, [](SceneUnitId id, JUUID uuid)
				{
					auto& so = GetSoundFXSceneObject(id, uuid);
					so->BindToScene();
				}
			},
			{
				SO_PhysicScenes, [](SceneUnitId id, JUUID uuid)
				{
					auto& so = GetPhysicSceneSceneObject(id, uuid);
					so->BindToScene();
				}
			},
			{
				SO_Triggers, [](SceneUnitId id, JUUID uuid)
				{
					auto& so = GetTriggerSceneObject(id, uuid);
					so->BindToScene();
				}
			},
			{
				SO_Boundaries, [](SceneUnitId id, JUUID uuid)
				{
					auto& so = GetBoundarySceneObject(id, uuid);
					so->BindToScene();
				}
			},
			{
				SO_SceneControllers, [](SceneUnitId id, JUUID uuid)
				{
					auto& so = GetSceneControllerSceneObject(id, uuid);
					so->BindToScene();
				}
			}
		};

		std::set<JUUID> uuids = GetUnboundedSceneObjects(id);
		for (auto& uuid : uuids)
		{
			typeBinder.at(GetSceneObjectType(id, uuid))(id, uuid);
		}
		GetUnboundedSceneObjects(id).clear();
	}

	JUUID CloneSceneObject(SceneUnitId id, JUUID sceneObject, nlohmann::json parameters)
	{
		SceneObject* sceneObjectO = GetSceneObjectPointer(id, sceneObject);
		SceneObjectType type = sceneObjectO->JType();
		//std::string dump = sceneObjectO->dump();
		//nlohmann::json data = nlohmann::json::parse(dump);
		nlohmann::json data;
		sceneObjectO->WriteJson(data);

		JUUID uuid = getUUID();
		std::string name = data.at("name");
		name += "_clone";
		data.at("name") = name;
		data.at("uuid") = uuid;

		data.merge_patch(parameters);

		switch (type)
		{
		case SO_Cameras:
		{
			CreateCamera(id, data);
		}
		break;
		case SO_Lights:
		{
			CreateLight(id, data);
		}
		break;
		case SO_Renderables:
		{
			CreateRenderable(id, data);
		}
		break;
		case SO_SoundEffects:
		{
			CreateSoundFX(id, data);
		}
		break;
		case SO_PhysicScenes:
		{
			CreatePhysicScene(id, data);
		}
		break;
		case SO_Triggers:
		{
			CreateTrigger(id, data);
		}
		break;
		case SO_Boundaries:
		{
			CreateBoundary(id, data);
		}
		break;
		case SO_SceneControllers:
		{
			CreateSceneController(id, data);
		}
		break;
		}
		return uuid;
	}

	void BindToScene(SceneUnitId id, JUUID uuidA, JUUID uuidB)
	{
		scenesUnits.at(id)->Bind(uuidA, uuidB);
	}

	void UnbindFromScene(SceneUnitId unit, JUUID uuidA)
	{
		scenesUnits.at(unit)->Unbind(uuidA);
	}

	void UnbindFromScene(SceneUnitId unit, JUUID uuidA, JUUID uuidB)
	{
		scenesUnits.at(unit)->Unbind(uuidA, uuidB);
	}

	void SceneObjectsStep(DX::StepTimer& timer)
	{
		for (auto& [unit, scene] : scenesUnits)
		{
			if (scene->MarkedForDelete()) continue;
			float dt = static_cast<FLOAT>(timer.GetElapsedSeconds());
#if defined(_EDITOR)
			Editor::UpdateBoundingBox(unit);

			if (!Editor::IsPlaying(unit) || Editor::IsPaused(unit))
				dt = 0.0f;
#endif
			TriggersStep(unit);
			BoundariesStep(unit);
			PhysicSceneStep(unit, dt);
			SoundFXsStep(unit, dt);
			RenderablesStep(unit, dt);
			AnimableStep(unit, dt);
			LightsStep(unit);
			CamerasStep(unit);
		}
	}

	void WriteConstantsBuffers(SceneUnitId id)
	{
		auto& scene = GetSceneUnit(id);

		for (JUUID uuid : GetRenderables(id))
		{
			RenderableID r = MAKESUUUID(id, uuid);
			if (!r->RenderReady()) continue;

			r->WriteAnimationConstantsBuffer(scene->Frame());
			r->WriteConstantsBuffer(scene->Frame());
		}

		//write the constants buffers of the cameras which renders shadow maps
		for (JUUID uuid : GetCameras(id))
		{
			CameraID c = MAKESUUUID(id, uuid);
			if (!c->RenderReady() || !c->shadowMapLight().empty()) continue;

			c->WriteLightsConstantsBuffer(scene->Frame());
			c->WriteShadowMapsConstantsBuffer(scene->Frame());
		}
	}

	void RenderSceneShadowMaps(SceneUnitId id)
	{
		auto& commandList = GetSceneUnit(id)->GetCommandList();

		for (JUUID uuid : GetLights(id))
		{
			LightID l = MAKESUUUID(id, uuid);

			if (!l->RenderReady() || !l->hasShadowMaps()) continue;

			auto renderSceneShadowMap = [&](unsigned int cameraIndex)
				{
					auto& camera = l->shadowMapCameras.at(cameraIndex);
					auto& rp = camera->renderPassesUUID.at(0);
					for (RenderableID r : camera->renderables)
					{
						if (r->castShadows())
						{
							r->Render(id, rp, camera);
						}
					}
				};

#if defined(_DEVELOPMENT)
			std::string shadowMapEvent = "ShadowMap:" + l->name();
			PIXBeginEvent(commandList.p, 0, nostd::StringToWString(shadowMapEvent).c_str());
#endif

			l->RenderShadowMap(renderSceneShadowMap);

#if defined(_DEVELOPMENT)
			PIXEndEvent(commandList.p);
#endif

#if defined(_EDITOR)
			if (l->shadowMapMinMaxChainRenderPass.empty() || l->destroySMChain) continue;

#if defined(_DEVELOPMENT)
			std::string shadowMapMinMaxEvent = "ShadowMapMinMaxChain:" + l->name();
			PIXBeginEvent(commandList.p, 0, nostd::StringToWString(shadowMapMinMaxEvent).c_str());
#endif

			l->RenderShadowMapMinMaxChain();

#if defined(_DEVELOPMENT)
			PIXEndEvent(commandList.p);
#endif

#endif
		}
	}

	void RenderSceneCameras(SceneUnitId id)
	{
		auto cameras(GetCameras(id));
		//get all the available cameras and start filtering thing we don't want to render
		for (auto it = cameras.begin(); it != cameras.end();)
		{
			JUUID uuid = *it;
			//filter out cameras which doesn't exists
			if (!SceneObjectExists(id, uuid))
			{
				it = cameras.erase(it);
				continue;
			}

			//filter out cameras used to render shadow maps
			CameraID cam = MAKESUUUID(id, uuid);
			if (!cam->RenderReady() || !cam->shadowMapLight().empty())
			{
				it = cameras.erase(it);
				continue;
			}
			it++;
		}

		//create a vector of cameras rendering to rtt's
		std::vector<JUUID> nonSwapChainCams;
		std::copy_if(cameras.begin(), cameras.end(), std::back_inserter(nonSwapChainCams), [&](JUUID uuid)
			{
				CameraID cam = MAKESUUUID(id, uuid);
				//we skip swap chain cams
				if (cam->useSwapChain()) return false;
				//we skip cameras which resolves to the swapchain
				if (cam->renderPassesUUID.size() > 0ULL && cam->renderPassesUUID.back()->type == RenderPassType_SwapChainPass) return false;
				return true;
			}
		);

		auto& commandList = GetSceneUnit(id)->GetCommandList();

		//render non swapchain buffer cameras(rtt stuff)
		for (auto& uuid : nonSwapChainCams)
		{
			CameraID cam = MAKESUUUID(id, uuid);
#if defined(_DEVELOPMENT)
			PIXBeginEvent(commandList.p, 0, std::string("nonSwapChain:" + cam->name()).c_str());
#endif
			cam->Render();
#if defined(_DEVELOPMENT)
			PIXEndEvent(commandList.p);
#endif
		}

		//check if there is any camera with any render pass resolving to the swapchain
		bool resolvedToSwapchain = std::any_of(nonSwapChainCams.begin(), nonSwapChainCams.end(), [&](JUUID uuid)
			{
				CameraID cam = MAKESUUUID(id, uuid);
				return std::any_of(cam->renderPassesUUID.begin(), cam->renderPassesUUID.end(), [](RenderPassInstanceID& pass)
					{
						return pass->renderCallbackOverride == RenderPassRenderCallbackOverride_Resolve;
					}
				);
			}
		);

		//if there are cameras resolving to swapchain but there was nothing already resolving to one. we render with the swapchain cameras
		if (GetCountFromSwapChainCameras(id) > 0ULL && !resolvedToSwapchain)
		{
			JUUID camUUID = *GetSwapChainCameras(id).begin();
			auto& cam = GetFromSwapChainCameras(id, camUUID);
#if defined(_DEVELOPMENT)
			PIXBeginEvent(commandList.p, 0, std::string("SwapChain:" + cam->name()).c_str());
#endif
			cam->Render();
#if defined(_DEVELOPMENT)
			PIXEndEvent(commandList.p);
#endif
		}
	}

	void AnimableStep(SceneUnitId id, double elapsedSeconds)
	{
		for (JUUID uuid : GetAnimables(id))
		{
			RenderableID r = MAKESUUUID(id, uuid);
			r->StepAnimation(elapsedSeconds);
		}
	}

	void SceneRender()
	{
#if defined(_EDITOR)
		using namespace Editor;
		UpdateBillboards();
#endif
		for (auto& [id, scene] : scenesUnits)
		{
			if (!SceneUnitExits(id) || !SceneUnitRenderingExists(id) || scene->MarkedForDelete()) continue;

			//scene->Loading();
			//
			//if (!renderableSceneUnits.contains(unit) || !scene->IsLoadingComplete()) continue;
#if defined(_EDITOR)
			if (!Editor::IsPlaying(id) && !scene->IsIsolated())
			{
				WriteSceneUnitEditorPlayCameraConstantsBuffer(id);
				SwitchToSceneUnitEditorCamera(id);
			}
#endif
			scene->Render();
#if defined(_EDITOR)
			if (!Editor::IsPlaying(id) && !scene->IsIsolated())
			{
				SwitchToSceneUnitEditorPlayCamera(id);
			}
#endif
		}
#if defined(_EDITOR)
		DrawEditor();
#endif
	}

	void ScenePostRender()
	{
#if defined(_EDITOR)
		using namespace Editor;
#endif
		for (auto& [unit, scene] : scenesUnits)
		{
			//if (scene->MarkedForDelete() || !scene->IsLoadingComplete() || (!renderableSceneUnits.contains(unit))) continue;

			scene->PostRender();
		}
	}

	static std::set<size_t> computeRunners;
	void RunComputeShaders()
	{
		for (auto& [unit, scene] : scenesUnits)
		{
			//if (scene->MarkedForDelete() || !scene->IsLoadingComplete()) continue;
			if (scene->MarkedForDelete() || !scene->HasComputeProcessor()) continue;
			computeRunners.insert(unit);
			scene->RunComputeShaders();
		}
	}

	void SolveComputeShaders()
	{
		for (auto& [unit, scene] : scenesUnits)
		{
			//if (scene->MarkedForDelete() || !scene->IsLoadingComplete()) continue;
			if (!computeRunners.contains(unit)) continue;
			scene->SolveComputeShaders();
		}
		computeRunners.clear();
	}

	SceneObject* GetSceneObjectPointer(SUUUID suuid)
	{
		return GetSceneObjectPointer(FROMSUUUID(suuid));
	}

	v8_templates_creators GetSceneObjectV8TemplatesCreators(SUUUID suuuid)
	{
		return GetSceneObjectPointer(suuuid)->GetV8TemplatesCreators();
	}

	v8_context_creators GetSceneObjectV8ContextCreators(SUUUID suuuid)
	{
		return GetSceneObjectPointer(suuuid)->GetV8ContextCreators();
	}

	SceneObject* GetSceneObjectPointer(SceneUnitId id, JUUID uuid)
	{
		std::map<SceneObjectType, std::function<SceneObject* (SceneUnitId id, JUUID uuid)>> getP =
		{
			{ SO_Renderables, [](SceneUnitId id, JUUID uuid)
				{
					auto& o = GetRenderableSceneObject(id, uuid);
					return static_cast<SceneObject*>(o.get());
				}
			},
			{ SO_Cameras, [](SceneUnitId id, JUUID uuid)
				{
					auto& o = GetCameraSceneObject(id, uuid);
					return static_cast<SceneObject*>(o.get());
				}
			},
			{ SO_Lights, [](SceneUnitId id, JUUID uuid)
				{
					auto& o = GetLightSceneObject(id, uuid);
					return static_cast<SceneObject*>(o.get());
				}
			},
			{ SO_SoundEffects, [](SceneUnitId id, JUUID uuid)
				{
					auto& o = GetSoundFXSceneObject(id, uuid);
					return static_cast<SceneObject*>(o.get());
				}
			},
			{ SO_PhysicScenes, [](SceneUnitId id, JUUID uuid)
				{
					auto& o = GetPhysicSceneSceneObject(id, uuid);
					return static_cast<SceneObject*>(o.get());
				}
			},
			{ SO_Triggers, [](SceneUnitId id, JUUID uuid)
				{
					auto& o = GetTriggerSceneObject(id, uuid);
					return static_cast<SceneObject*>(o.get());
				}
			},
			{ SO_Boundaries, [](SceneUnitId id, JUUID uuid)
				{
					auto& o = GetBoundarySceneObject(id, uuid);
					return static_cast<SceneObject*>(o.get());
				}
			},
			{ SO_SceneControllers, [](SceneUnitId id, JUUID uuid)
				{
					auto& o = GetSceneControllerSceneObject(id, uuid);
					return static_cast<SceneObject*>(o.get());
				}
			}
		};

		SceneObjectType type = GetSceneObjectType(id, uuid);
		return getP.at(type)(id, uuid);
	}

#if defined(_EDITOR)
	std::function<std::vector<JUUIDName>()> GetSceneObjectsByType(SceneUnitId id, SceneObjectType typeToGet)
	{
		return [&]
			{
				auto str2JUUIDName = [](std::string type, std::string uuid, std::string name)
					{
						JUUIDName uuidname;
						std::string& u = std::get<0>(uuidname);
						std::string& n = std::get<1>(uuidname);
						u = uuid;
						n = type + "/" + name;
						return uuidname;
					};

				std::unordered_map<SceneObjectType, std::function<JUUIDName(SceneUnitId, JUUID)>> getJUUIDName =
				{
					{ SO_Renderables, [str2JUUIDName](SceneUnitId id, JUUID uuid)
						{
							RenderableID o = MAKESUUUID(id,uuid);
							if (o->hidden()) return JUUIDName();
							return str2JUUIDName(SceneObjectTypeToString.at(SO_Renderables), o->uuid(),o->name());
						}
					},
					{ SO_Cameras, [str2JUUIDName](SceneUnitId id, JUUID uuid)
						{
							CameraID o = MAKESUUUID(id,uuid);
							if (o->hidden()) return JUUIDName();
							return str2JUUIDName(SceneObjectTypeToString.at(SO_Cameras), o->uuid(),o->name());
						}
					},
					{ SO_Lights, [str2JUUIDName](SceneUnitId id, JUUID uuid)
						{
							LightID o = MAKESUUUID(id,uuid);
							if (o->hidden()) return JUUIDName();
							return str2JUUIDName(SceneObjectTypeToString.at(SO_Lights), o->uuid(),o->name());
						}
					},
					{ SO_SoundEffects, [str2JUUIDName](SceneUnitId id, JUUID uuid)
						{
							SoundFXID o = MAKESUUUID(id,uuid);
							if (o->hidden()) return JUUIDName();
							return str2JUUIDName(SceneObjectTypeToString.at(SO_SoundEffects), o->uuid(),o->name());
						}
					},
					{ SO_PhysicScenes, [str2JUUIDName](SceneUnitId id, JUUID uuid)
						{
							PhysicSceneID o = MAKESUUUID(id,uuid);
							if (o->hidden()) return JUUIDName();
							return str2JUUIDName(SceneObjectTypeToString.at(SO_PhysicScenes), o->uuid(),o->name());
						}
					},
					{ SO_Triggers, [str2JUUIDName](SceneUnitId id, JUUID uuid)
						{
							TriggerID o = MAKESUUUID(id,uuid);
							if (o->hidden()) return JUUIDName();
							return str2JUUIDName(SceneObjectTypeToString.at(SO_Triggers), o->uuid(),o->name());
						}
					},
					{ SO_Boundaries, [str2JUUIDName](SceneUnitId id, JUUID uuid)
						{
							BoundaryID o = MAKESUUUID(id,uuid);
							if (o->hidden()) return JUUIDName();
							return str2JUUIDName(SceneObjectTypeToString.at(SO_Boundaries), o->uuid(),o->name());
						}
					},
					{ SO_SceneControllers, [str2JUUIDName](SceneUnitId id, JUUID uuid)
						{
							SceneControllerID o = MAKESUUUID(id,uuid);
							if (o->hidden()) return JUUIDName();
							return str2JUUIDName(SceneObjectTypeToString.at(SO_SceneControllers), o->uuid(),o->name());
						}
					}
				};

				auto uuids = GetSceneObjects(id, typeToGet);
				std::vector<JUUIDName> sceneObjectsTypeList;
				for (auto& uuid : uuids)
				{
					JUUIDName uuidName = getJUUIDName.at(typeToGet)(id, uuid);
					if (!std::get<0>(uuidName).empty())
						sceneObjectsTypeList.push_back(uuidName);
				}
				return sceneObjectsTypeList;
			};
	}

	std::vector<JUUIDName> GetSUSceneObjectsByType(SceneUnitId id, SceneObjectType typeToGet)
	{
		auto str2JUUIDName = [](std::string type, std::string uuid, std::string name)
			{
				JUUIDName uuidname;
				std::string& u = std::get<0>(uuidname);
				std::string& n = std::get<1>(uuidname);
				u = uuid;
				n = type + "/" + name;
				return uuidname;
			};

		std::unordered_map<SceneObjectType, std::function<JUUIDName(SceneUnitId, JUUID)>> getJUUIDName =
		{
			{ SO_Renderables, [str2JUUIDName](SceneUnitId id, JUUID uuid)
				{
					RenderableID o = MAKESUUUID(id,uuid);
					if (o->hidden()) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_Renderables), o->uuid(),o->name());
				}
			},
			{ SO_Cameras, [str2JUUIDName](SceneUnitId id, JUUID uuid)
				{
					CameraID o = MAKESUUUID(id,uuid);
					if (o->hidden()) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_Cameras), o->uuid(),o->name());
				}
			},
			{ SO_Lights, [str2JUUIDName](SceneUnitId id, JUUID uuid)
				{
					LightID o = MAKESUUUID(id,uuid);
					if (o->hidden()) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_Lights), o->uuid(),o->name());
				}
			},
			{ SO_SoundEffects, [str2JUUIDName](SceneUnitId id, JUUID uuid)
				{
					SoundFXID o = MAKESUUUID(id,uuid);
					if (o->hidden()) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_SoundEffects), o->uuid(),o->name());
				}
			},
			{ SO_PhysicScenes, [str2JUUIDName](SceneUnitId id, JUUID uuid)
				{
					PhysicSceneID o = MAKESUUUID(id,uuid);
					if (o->hidden()) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_PhysicScenes), o->uuid(),o->name());
				}
			},
			{ SO_Triggers, [str2JUUIDName](SceneUnitId id, JUUID uuid)
				{
					TriggerID o = MAKESUUUID(id,uuid);
					if (o->hidden()) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_Triggers), o->uuid(),o->name());
				}
			},
			{ SO_Boundaries, [str2JUUIDName](SceneUnitId id, JUUID uuid)
				{
					BoundaryID o = MAKESUUUID(id,uuid);
					if (o->hidden()) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_Boundaries), o->uuid(),o->name());
				}
			},
			{ SO_SceneControllers, [str2JUUIDName](SceneUnitId id, JUUID uuid)
				{
					SceneControllerID o = MAKESUUUID(id,uuid);
					if (o->hidden()) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_SceneControllers), o->uuid(),o->name());
				}
			}
		};

		auto uuids = GetSceneObjects(id, typeToGet);
		std::vector<JUUIDName> sceneObjectsTypeList;
		for (auto& uuid : uuids)
		{
			JUUIDName uuidName = getJUUIDName.at(typeToGet)(id, uuid);
			if (!std::get<0>(uuidName).empty())
				sceneObjectsTypeList.push_back(uuidName);
		}
		return sceneObjectsTypeList;
	}

	std::vector<JUUIDName> GetSceneObjectsTypesList(SceneUnitId id)
	{
		auto str2JUUIDName = [](std::string type, std::string uuid, std::string name)
			{
				JUUIDName uuidname;
				std::string& u = std::get<0>(uuidname);
				std::string& n = std::get<1>(uuidname);
				u = uuid;
				n = type + "/" + name;
				return uuidname;
			};

		std::unordered_map<SceneObjectType, std::function<JUUIDName(SceneUnitId, JUUID)>> getJUUIDName =
		{
			{ SO_Renderables, [str2JUUIDName](SceneUnitId id, JUUID uuid)
				{
					RenderableID o = MAKESUUUID(id,uuid);
					if (o->hidden() || o->markedForDelete) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_Renderables), o->uuid(),o->name());
				}
			},
			{ SO_Cameras, [str2JUUIDName](SceneUnitId id, JUUID uuid)
				{
					CameraID o = MAKESUUUID(id,uuid);
					if (o->hidden() || o->markedForDelete) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_Cameras), o->uuid(),o->name());
				}
			},
			{ SO_Lights, [str2JUUIDName](SceneUnitId id, JUUID uuid)
				{
					LightID o = MAKESUUUID(id,uuid);
					if (o->hidden() || o->markedForDelete) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_Lights), o->uuid(),o->name());
				}
			},
			{ SO_SoundEffects, [str2JUUIDName](SceneUnitId id, JUUID uuid)
				{
					SoundFXID o = MAKESUUUID(id,uuid);
					if (o->hidden() || o->markedForDelete) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_SoundEffects), o->uuid(),o->name());
				}
			},
			{ SO_PhysicScenes, [str2JUUIDName](SceneUnitId id, JUUID uuid)
				{
					PhysicSceneID o = MAKESUUUID(id,uuid);
					if (o->hidden() || o->markedForDelete) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_PhysicScenes), o->uuid(),o->name());
				}
			},
			{ SO_Triggers, [str2JUUIDName](SceneUnitId id, JUUID uuid)
				{
					TriggerID o = MAKESUUUID(id,uuid);
					if (o->hidden() || o->markedForDelete) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_Triggers), o->uuid(),o->name());
				}
			},
			{ SO_Boundaries, [str2JUUIDName](SceneUnitId id, JUUID uuid)
				{
					BoundaryID o = MAKESUUUID(id,uuid);
					if (o->hidden() || o->markedForDelete) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_Boundaries), o->uuid(),o->name());
				}
			},
			{ SO_SceneControllers, [str2JUUIDName](SceneUnitId id, JUUID uuid)
				{
					SceneControllerID o = MAKESUUUID(id,uuid);
					if (o->hidden() || o->markedForDelete) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_SceneControllers), o->uuid(),o->name());
				}
			}
		};

		std::vector<JUUIDName> sceneObjectsTypeList;
		for (auto& [type, uuids] : scenesUnits.at(id)->GetSceneObjects())
		{
			for (auto& uuid : uuids)
			{
				if (!SceneObjectExists(id, uuid))
					continue;
				JUUIDName uuidName = getJUUIDName.at(type)(id, uuid);
				if (!std::get<0>(uuidName).empty())
					sceneObjectsTypeList.push_back(uuidName);
			}
		}
		return sceneObjectsTypeList;
	}

	std::vector<std::pair<std::string, JsonToEditorValueType>> GetSceneObjectAttributes(SceneObjectType so)
	{
		const std::unordered_map<SceneObjectType, std::function<std::vector<std::pair<std::string, JsonToEditorValueType>>()>> GetSOAtts =
		{
			{ SO_Renderables, GetRenderableAttributes },
			{ SO_Lights, GetLightAttributes },
			{ SO_Cameras, GetCameraAttributes },
			{ SO_SoundEffects, GetSoundFXAttributes },
			{ SO_PhysicScenes, GetPhysicSceneAttributes },
			{ SO_Triggers, GetTriggerAttributes },
			{ SO_Boundaries, GetBoundaryAttributes },
			{ SO_SceneControllers, GetSceneControllerAttributes },
		};
		return GetSOAtts.at(so)();
	}

	std::map<std::string, JEdvEditorDrawerFunction> GetSceneObjectDrawers(SceneObjectType so)
	{
		const std::unordered_map<SceneObjectType, std::function<std::map<std::string, JEdvEditorDrawerFunction>()>> GetSODrawers =
		{
			{ SO_Renderables, GetRenderableDrawers },
			{ SO_Lights, GetLightDrawers },
			{ SO_Cameras, GetCameraDrawers },
			{ SO_SoundEffects, GetSoundFXDrawers },
			{ SO_PhysicScenes, GetPhysicSceneDrawers },
			{ SO_Triggers, GetTriggerDrawers },
			{ SO_Boundaries, GetBoundaryDrawers },
			{ SO_SceneControllers, GetSceneControllerDrawers },
		};
		return GetSODrawers.at(so)();
	}

	std::map<std::string, JEdvEditorDrawerFunction> GetSceneObjectPreviewers(SceneObjectType so)
	{
		const std::unordered_map<SceneObjectType, std::function<std::map<std::string, JEdvEditorDrawerFunction>()>> GetSOPreviewers =
		{
			{ SO_Renderables, GetRenderablePreviewers },
			{ SO_Lights, GetLightPreviewers },
			{ SO_Cameras, GetCameraPreviewers },
			{ SO_SoundEffects, GetSoundFXPreviewers },
			{ SO_PhysicScenes, GetPhysicScenePreviewers },
			{ SO_Triggers, GetTriggerPreviewers },
			{ SO_Boundaries, GetBoundaryPreviewers },
			{ SO_SceneControllers, GetSceneControllerPreviewers },
		};
		return GetSOPreviewers.at(so)();
	}

	nlohmann::json GetSceneObjectJson(SceneObjectType so)
	{
		const std::unordered_map<SceneObjectType, std::function<nlohmann::json()>> GetSOJson =
		{
			{ SO_Renderables, CreateRenderableJson },
			{ SO_Lights, CreateLightJson },
			{ SO_Cameras, CreateCameraJson },
			{ SO_SoundEffects, CreateSoundFXJson },
			{ SO_PhysicScenes, CreatePhysicSceneJson },
			{ SO_Triggers, [] { auto json = CreateTriggerJson(); json["trigger"] = true; return json; } }, //inject this att to handle no trigger with meshes
			{ SO_Boundaries, [] { auto json = CreateBoundaryJson(); json["boundary"] = true; return json; } }, //inject this att to handle no boundary with meshes
			{ SO_SceneControllers, CreateSceneControllerJson },
		};
		return GetSOJson.at(so)();
	}

	std::vector<std::string> GetSceneObjectRequiredAttributes(SceneObjectType so)
	{
		const std::unordered_map<SceneObjectType, std::function<std::vector<std::string>()>> GetSORequiredAtts =
		{
			{ SO_Renderables, GetRenderableRequiredAttributes },
			{ SO_Lights, GetLightRequiredAttributes },
			{ SO_Cameras, GetCameraRequiredAttributes },
			{ SO_SoundEffects, GetSoundFXRequiredAttributes },
			{ SO_PhysicScenes, GetPhysicSceneRequiredAttributes },
			{ SO_Triggers, GetTriggerRequiredAttributes },
			{ SO_Boundaries, GetBoundaryRequiredAttributes },
			{ SO_SceneControllers, GetSceneControllerRequiredAttributes },

		};
		return GetSORequiredAtts.at(so)();
	}

	std::map<std::string, JEdvCreatorDrawerFunction> GetSceneObjectCreatorDrawers(SceneObjectType so)
	{
		const std::unordered_map<SceneObjectType, std::function<std::map<std::string, JEdvCreatorDrawerFunction>()>> GetSODrawers =
		{
			{ SO_Renderables, GetRenderableCreatorDrawers },
			{ SO_Lights, GetLightCreatorDrawers },
			{ SO_Cameras, GetCameraCreatorDrawers },
			{ SO_SoundEffects, GetSoundFXCreatorDrawers },
			{ SO_PhysicScenes, GetPhysicSceneCreatorDrawers },
			{ SO_Triggers, GetTriggerCreatorDrawers },
			{ SO_Boundaries, GetBoundaryCreatorDrawers },
			{ SO_SceneControllers, GetSceneControllerCreatorDrawers },
		};
		return GetSODrawers.at(so)();
	}

	std::map<std::string, JEdvCreatorValidatorFunction> GetSceneObjectValidators(SceneObjectType so)
	{
		const std::unordered_map<SceneObjectType, std::function<std::map<std::string, JEdvCreatorValidatorFunction>()>> GetSOValidators =
		{
			{ SO_Renderables, GetRenderableCreatorValidator },
			{ SO_Lights, GetLightCreatorValidator },
			{ SO_Cameras, GetCameraCreatorValidator },
			{ SO_SoundEffects, GetSoundFXCreatorValidator },
			{ SO_PhysicScenes, GetPhysicSceneCreatorValidator },
			{ SO_Triggers, GetTriggerCreatorValidator },
			{ SO_Boundaries, GetBoundaryCreatorValidator },
			{ SO_SceneControllers, GetSceneControllerCreatorValidator },
		};
		return GetSOValidators.at(so)();
	}

	void DeleteSceneObjectFromEditor(SceneUnitId id, JUUID uuid)
	{
		using namespace Editor;

		SceneObjectType type = GetSceneObjectType(id, uuid);
		const std::map<SceneObjectType, std::function<void(SceneUnitId, JUUID)>> DeleteSO =
		{
			{ SO_Renderables, DeleteRenderable },
			{ SO_Lights, DeleteLight },
			{ SO_Cameras, DeleteCamera },
			{ SO_SoundEffects, DeleteSoundFX },
			{ SO_PhysicScenes, DeletePhysicScene },
			{ SO_Triggers, DeleteTrigger },
			{ SO_Boundaries, DeleteBoundary },
			{ SO_SceneControllers, DeleteSceneController },
		};
		EraseSceneObjectFromSelection(id, uuid);
		DeleteSO.at(type)(id, uuid);
		MarkScenePanelAssetsAsDirty();
		MarkSceneUnitAsModified(id);
	}
#endif

	void CreateSceneObject(SceneUnitId id, SceneObjectType so, nlohmann::json json)
	{
#if defined(_EDITOR)
		using namespace Editor;
#endif

		JUUID uuid = getUUID();
		nlohmann::json patch = { {"uuid", uuid } };
		json.merge_patch(patch);
		nlohmann::json data = {
			{ SceneObjectTypeJsonContainer.at(so), { json } }
		};

		AttachLevelIntoScene(id, "new-scene-object", data, [=](SceneUnitId)
			{
#if defined(_EDITOR)
				if (so == SO_Renderables)
				{
					BindRenderableToPickingPass(MAKESUUUID(id, uuid));
				}
				MarkSceneUnitAsModified(id);
				MarkScenePanelAssetsAsDirty();
#endif
			},
			[](std::string, unsigned int, unsigned int) {}
		);
	}
}