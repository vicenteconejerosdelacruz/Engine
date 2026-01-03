#include "pch.h"
#include <Scene.h>
#include <SceneObject.h>
#include <Renderer.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <Level.h>
//#include <Light/Light.h>
//#include <Renderable/Renderable.h>
//#include <Camera/Camera.h>
//#include <RenderPass/RenderPass.h>
//#include <Sound/SoundFX.h>
//#include <Binder.h>
//#include <StepTimer.h>
//#if defined(_EDITOR)
//#include <Editor.h>
//#endif

extern std::unique_ptr<Renderer> renderer;
extern void AudioStep(float step);
#if defined(_EDITOR)
namespace Editor
{
	extern bool IsPlaying(SceneUnitId unit);
	extern bool IsPaused(SceneUnitId unit);
	extern void DrawEditor();
	extern void EraseSceneObjectFromSelection(SceneUnitId unit, JUUID uuid);
	extern void MarkScenePanelAssetsAsDirty();
	extern void UpdateBoundingBox(SceneUnitId unit);
	extern void SwitchToSceneUnitEditorCamera(SceneUnitId unit);
	extern void SwitchToSceneUnitEditorPlayCamera(SceneUnitId unit);
	extern void HandleEditorMouseMovements(SceneUnitId id);
	extern void CreateRegisteredBillboards(SceneUnitId id);
	extern void UpdateBillboards();
	extern void BindRenderableToPickingPass(RenderableUUID uuid);
};
#endif

namespace Scene
{
	std::unordered_map<JUUID, SceneObjectType> sceneObjectsTypes;
	std::map<SceneUnitId, std::unique_ptr<SceneUnit>> scenesUnits;
	std::map<SceneUnitId, SceneUnitId> attachedUnits; //<Attached, Destination>
	std::set<SceneUnitId> renderableSceneUnits;

	static std::mutex sceneUnitsMutex;

	void CreateSceneLevelAsync(std::string filename, nlohmann::json data, std::function<void(SceneUnitId)> levelLoaded, std::function<void(std::string, unsigned int, unsigned int)> progress)
	{
		using namespace Scene::Level;

		std::thread levelThread([](std::string filename, nlohmann::json data, std::function<void(SceneUnitId)> levelLoaded, std::function<void(std::string, unsigned int, unsigned int)> progress)
			{
				SceneUnitId unit = nostd::threadIdHash();
				auto& scene = CreateScene(unit, filename);

				LoadLevel(scene, filename, data, progress);

				scene->sceneUnitLoaded->store(true);

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
				SceneUnitId unit = nostd::threadIdHash();
				auto& scene = CreateAttachableScene(parentUnit, unit);

				LoadLevel(scene, filename, data, progress);

				scene->sceneUnitLoaded->store(true);

				levelLoaded(unit);

				attachedUnits.insert_or_assign(unit, parentUnit);

			}, parentUnit, filename, data, levelLoaded, progress
		);
		levelThread.detach();
	}

	std::unique_ptr<SceneUnit>& CreateScene(SceneUnitId unit, std::string unitName, unsigned int numProcessors)
	{
		std::lock_guard<std::mutex> lock(sceneUnitsMutex);
		scenesUnits.insert_or_assign(unit, std::make_unique<SceneUnit>(unit, unitName));
		scenesUnits.at(unit)->loadingProcessor.Init(renderer->d3dDevice, unit, 1U);
		if (numProcessors > 0U)
		{
			scenesUnits.at(unit)->commandsProcessor.Init(renderer->d3dDevice, unit, numProcessors);
			scenesUnits.at(unit)->computeProcessor.Init(renderer->d3dDevice, unit, numProcessors);
		}
		return scenesUnits.at(unit);
	}

	std::unique_ptr<SceneUnit>& CreateAttachableScene(SceneUnitId parentUnit, SceneUnitId unit, std::string unitName)
	{
		auto& scene = CreateScene(unit, unitName, 0U);
		scene->attached = true;
		scene->parentUnit = parentUnit;
		return scene;
	}

	void DestroyScene(SceneUnitId unit)
	{
		const std::map<SceneObjectType, std::function<void(JUUID)>> DeleteSO =
		{
			{ SO_Renderables, DeleteRenderable },
			{ SO_Lights, DeleteLight },
			{ SO_Cameras, DeleteCamera },
			{ SO_SoundEffects, DeleteSoundFX }
		};

		auto& scene = GetSceneUnit(unit);
		for (auto& [uuid, type] : scene->sceneObjectsTypes)
		{
#if defined(_EDITOR)
			Editor::EraseSceneObjectFromSelection(unit, uuid);
#endif
			DeleteSO.at(type)(uuid);
#if defined(_EDITOR)
			Editor::MarkScenePanelAssetsAsDirty();
#endif
		}
		scene->MarkForDelete();
	}

	void DestroyScenes(bool inmediate)
	{
		using namespace Scene::Level;
		for (auto& [unit, _] : scenesUnits)
		{
			DestroyScene(unit);
			DestroySceneObjects(unit);
		}
		if (inmediate)
		{
			for (auto& [_, scene] : scenesUnits)
			{
				scene->deletionFrames = 0;
			}
			DeletedScenes();
		}
	}

	bool SceneUnitExits(SceneUnitId unit)
	{
		return scenesUnits.contains(unit);
	}

	std::unique_ptr<SceneUnit>& GetSceneUnit(SceneUnitId unit)
	{
		return scenesUnits.at(unit);
	}

	size_t GetSceneUnitsCount()
	{
		return scenesUnits.size();
	}

	void MergeAttachedSceneUnits()
	{
		for (auto& [unit, parentUnit] : attachedUnits)
		{
			if (!SceneUnitExits(unit) || !SceneUnitExits(parentUnit)) continue;

			auto& scene = GetSceneUnit(unit);
			if (!scene->mergeable || scene->markedForDelete) continue;

			auto& parent = GetSceneUnit(parentUnit);
			parent->Merge(scene);
			scene->MarkForDelete();
		}
	}

	std::set<JUUID>& GetSceneObjects(SceneUnitId unit, SceneObjectType type)
	{
		if (!scenesUnits.at(unit)->sceneObjects.contains(type))
			scenesUnits.at(unit)->sceneObjects[type].clear();
		return scenesUnits.at(unit)->sceneObjects.at(type);
	}

	std::set<JUUID> GetSceneObjects(SceneObjectType type)
	{
		std::set<JUUID> objects;
		for (auto& [_, scene] : scenesUnits)
		{
			auto& typeobjs = scene->sceneObjects.at(type);
			objects.insert(typeobjs.begin(), typeobjs.end());
		}
		return objects;
	}

	std::unordered_map<JUUID, SceneObjectType>& GetSceneObjectsTypes(SceneUnitId unit)
	{
		return scenesUnits.at(unit)->sceneObjectsTypes;
	}

	std::unordered_map<JUUID, SceneObjectType>& GetGlobalSceneObjectsTypes()
	{
		return sceneObjectsTypes;
	}

	std::set<JUUID>& GetUnboundedSceneObjects(SceneUnitId unit)
	{
		return scenesUnits.at(unit)->unboundedSceneObjects;
	}

	SceneObjectType GetSceneObjectType(JUUID uuid)
	{
		return sceneObjectsTypes.at(uuid);
	}

	bool SceneObjectExists(SceneUnitId unit, JUUID uuid)
	{
		return scenesUnits.at(unit)->sceneObjectsTypes.contains(uuid);
	}

	SceneUnitId GetSceneObjectSceneUnitId(JUUID uuid)
	{
		std::unordered_map<SceneObjectType, std::function<SceneUnitId(JUUID)>> getUnitId =
		{
			{ SO_Renderables, [](JUUID uuid) { return GetRenderableSceneObject(uuid)->unit; } },
			{ SO_Cameras, [](JUUID uuid) { return GetCameraSceneObject(uuid)->unit; } },
			{ SO_Lights, [](JUUID uuid) { return GetLightSceneObject(uuid)->unit; } },
			{ SO_SoundEffects, [](JUUID uuid) { return GetSoundFXSceneObject(uuid)->unit; } }
		};

		SceneUnitId unit = getUnitId.at(GetSceneObjectType(uuid))(uuid);
		return unit;
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

	void BindSceneObjects()
	{
		std::unordered_map<SceneObjectType, std::function<void(JUUID)>> typeBinder =
		{
			{
				SO_Renderables, [](JUUID uuid)
				{
					auto& so = GetRenderableSceneObject(uuid);
					so->BindToScene();
				}
			},
			{
				SO_Cameras, [](JUUID uuid)
				{
					auto& so = GetCameraSceneObject(uuid);
					so->BindToScene();
				}
			},
			{ SO_Lights, [](JUUID uuid)
				{
				auto& so = GetLightSceneObject(uuid);
				so->BindToScene();
			}
			},
			{
				SO_SoundEffects, [](JUUID uuid)
				{
					auto& so = GetSoundFXSceneObject(uuid);
					so->BindToScene();
				}
			}
		};
		for (auto& uuid : GetUnboundedSceneObjects())
		{
			typeBinder.at(sceneObjectsTypes.at(uuid))(uuid);
		}
		GetUnboundedSceneObjects().clear();
	}

	JUUID CloneSceneObject(SceneUnitId unit, JUUID sceneObject, nlohmann::json parameters)
	{
		SceneObject* sceneObjectO = GetSceneObjectPointer(sceneObject);
		SceneObjectType type = sceneObjectO->JType();
		std::string dump = sceneObjectO->dump();
		nlohmann::json data = nlohmann::json::parse(dump);

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
			CreateCamera(data, unit);
		}
		break;
		case SO_Lights:
		{
			CreateLight(data, unit);
		}
		break;
		case SO_Renderables:
		{
			CreateRenderable(data, unit);
		}
		break;
		case SO_SoundEffects:
		{
			CreateSoundFX(data, unit);
		}
		break;
		}
		return uuid;
	}

	void BindToScene(SceneUnitId unit, JUUID uuidA, JUUID uuidB)
	{
		scenesUnits.at(unit)->binder.insert(uuidA, uuidB);
	}

	void UnbindFromScene(SceneUnitId unit, JUUID uuidA)
	{
		scenesUnits.at(unit)->binder.erase(uuidA);
	}

	void UnbindFromScene(SceneUnitId unit, JUUID uuidA, JUUID uuidB)
	{
		scenesUnits.at(unit)->binder.erase(uuidA, uuidB);
	}

	void SceneObjectsStep(DX::StepTimer& timer)
	{
		AudioStep(static_cast<FLOAT>(timer.GetElapsedSeconds()));
		for (auto& [unit, _] : scenesUnits)
		{
			float dt = static_cast<FLOAT>(timer.GetElapsedSeconds());
#if defined(_EDITOR)
			Editor::UpdateBoundingBox(unit);

			if (!Editor::IsPlaying(unit) || Editor::IsPaused(unit))
				dt = 0.0f;
#endif
			RenderablesStep(unit, dt);
			AnimableStep(unit, dt);
			LightsStep(unit);
			CamerasStep(unit);
		}
	}

	void WriteConstantsBuffers(SceneUnitId unit)
	{
		auto& scene = GetSceneUnit(unit);

		for (RenderableUUID r : GetRenderables(unit))
		{
			if (!r->renderReady) continue;

			r->WriteAnimationConstantsBuffer(scene->Frame());
			r->WriteConstantsBuffer(scene->Frame());
		}

		//write the constants buffers of the cameras which renders shadow maps
		for (CameraUUID c : GetCameras(unit))
		{
			if (!c->shadowMapLight().empty()) continue;
			c->WriteLightsConstantsBuffer(scene->Frame());
			c->WriteShadowMapsConstantsBuffer(scene->Frame());
		}
	}

	void RenderSceneShadowMaps(SceneUnitId unit)
	{
		auto& commandList = GetSceneUnit(unit)->GetCommandList();

		for (LightUUID l : GetLights(unit))
		{
			if (!l->hasShadowMaps()) continue;

			auto renderSceneShadowMap = [&](unsigned int cameraIndex)
				{
					auto& camera = l->shadowMapCameras.at(cameraIndex);
					auto& rp = camera->renderPassesUUID.at(0);
					for (RenderableUUID r : camera->renderables)
					{
						if (r->castShadows())
						{
							r->Render(unit, rp, camera);
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
		}
	}

	void RenderSceneCameras(SceneUnitId unit)
	{
		auto cameras(GetCameras(unit));
		//get all the available cameras and start filtering thing we don't want to render
		for (auto it = cameras.begin(); it != cameras.end();)
		{
			JUUID uuid = *it;
			//filter out cameras which doesn't exists
			if (!SceneObjectExists(unit, uuid))
			{
				it = cameras.erase(it);
				continue;
			}

			//filter out cameras used to render shadow maps
			auto& cam = GetCameraSceneObject(uuid);
			if (!cam->shadowMapLight().empty())
			{
				it = cameras.erase(it);
				continue;
			}
			it++;
		}

		//create a vector of cameras rendering to rtt's
		std::vector<JUUID> nonSwapChainCams;
		std::copy_if(cameras.begin(), cameras.end(), std::back_inserter(nonSwapChainCams), [unit](JUUID uuid)
			{
				auto& cam = GetCameraSceneObject(uuid);
				//we skip swap chain cams
				if (cam->useSwapChain()) return false;
				//we skip cameras which resolves to the swapchain
				if (cam->renderPassesUUID.size() > 0ULL && cam->renderPassesUUID.back()->type == RenderPassType_SwapChainPass) return false;
				return true;
			}
		);

		auto& commandList = GetSceneUnit(unit)->GetCommandList();

		//render non swapchain buffer cameras(rtt stuff)
		for (auto& uuid : nonSwapChainCams)
		{
			auto& cam = GetCameraSceneObject(uuid);
#if defined(_DEVELOPMENT)
			PIXBeginEvent(commandList.p, 0, std::string("nonSwapChain:" + cam->name()).c_str());
#endif
			cam->Render();
#if defined(_DEVELOPMENT)
			PIXEndEvent(commandList.p);
#endif
		}

		//check if there is any camera with any render pass resolving to the swapchain
		bool resolvedToSwapchain = std::any_of(nonSwapChainCams.begin(), nonSwapChainCams.end(), [unit](JUUID camUUID)
			{
				auto& cam = GetCameraSceneObject(camUUID);
				return std::any_of(cam->renderPassesUUID.begin(), cam->renderPassesUUID.end(), [](RenderPassInstanceUUID& pass)
					{
						return pass->renderCallbackOverride == RenderPassRenderCallbackOverride_Resolve;
					}
				);
			}
		);

		//if there are cameras resolving to swapchain but there was nothing already resolving to one. we render with the swapchain cameras
		if (GetCountFromSwapChainCameras(unit) > 0ULL && !resolvedToSwapchain)
		{
			JUUID camUUID = *GetSwapChainCameras(unit).begin();
			auto& cam = GetFromSwapChainCameras(unit, camUUID);
#if defined(_DEVELOPMENT)
			PIXBeginEvent(commandList.p, 0, std::string("SwapChain:" + cam->name()).c_str());
#endif
			cam->Render();
#if defined(_DEVELOPMENT)
			PIXEndEvent(commandList.p);
#endif
		}
	}

	void AnimableStep(SceneUnitId unit, double elapsedSeconds)
	{
		for (RenderableUUID r : GetAnimables(unit))
		{
			r->StepAnimation(elapsedSeconds);
		}
	}

	void SceneRender()
	{
#if defined(_EDITOR)
		using namespace Editor;
		UpdateBillboards();
#endif
		for (auto& [unit, scene] : scenesUnits)
		{
			if (scene->markedForDelete) continue;

			scene->Loading();

			if (!renderableSceneUnits.contains(unit) && !scene->attached) continue;

#if defined(_EDITOR)
			if (!scene->attached)
			{
				SwitchToSceneUnitEditorCamera(unit);
			}
#endif
			scene->Render();
#if defined(_EDITOR)
			if (!scene->attached)
			{
				SwitchToSceneUnitEditorPlayCamera(unit);
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
			if (scene->markedForDelete || (!renderableSceneUnits.contains(unit) && !scene->attached)) continue;

			scene->PostRender();
		}

		MergeAttachedSceneUnits();
	}

	void RunComputeShaders()
	{
		for (auto& [unit, scene] : scenesUnits)
		{
			if (scene->markedForDelete || scene->attached) continue;
			scene->RunComputeShaders();
		}
	}

	void SolveComputeShaders()
	{
		for (auto& [unit, scene] : scenesUnits)
		{
			if (scene->markedForDelete || scene->attached) continue;
			scene->SolveComputeShaders();
		}
	}

	void DeletedScenes()
	{
		for (auto it = scenesUnits.begin(); it != scenesUnits.end();)
		{
			if (it->second->markedForDelete)
			{
				if (it->second->deletionFrames > 0)
				{
					it->second->deletionFrames--;
					it++;
				}
				else
				{
					it = scenesUnits.erase(it);
				}
			}
			else
			{
				it++;
			}
		}
	}

	SceneObject* GetSceneObjectPointer(JUUID uuid)
	{
		std::map<SceneObjectType, std::function<SceneObject* (JUUID uuid)>> getP =
		{
			{ SO_Renderables, [](JUUID uuid)
				{
					auto& o = GetRenderableSceneObject(uuid);
					return static_cast<SceneObject*>(o.get());
				}
			},
			{ SO_Cameras, [](JUUID uuid)
				{
					auto& o = GetCameraSceneObject(uuid);
					return static_cast<SceneObject*>(o.get());
				}
			},
			{ SO_Lights, [](JUUID uuid)
				{
					auto& o = GetLightSceneObject(uuid);
					return static_cast<SceneObject*>(o.get());
				}
			},
			{ SO_SoundEffects, [](JUUID uuid)
				{
					auto& o = GetSoundFXSceneObject(uuid);
					return static_cast<SceneObject*>(o.get());
				}
			}
		};

		SceneObjectType type = GetSceneObjectType(uuid);
		return getP.at(type)(uuid);
	}

#if defined(_EDITOR)
	std::function<std::vector<JUUIDName>()> GetSceneObjectsByType(SceneObjectType typeToGet)
	{
		return [typeToGet]
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

				std::unordered_map<SceneObjectType, std::function<JUUIDName(JUUID)>> getJUUIDName =
				{
					{ SO_Renderables, [str2JUUIDName](JUUID uuid)
						{
							RenderableUUID o = uuid;
							if (o->hidden()) return JUUIDName();
							return str2JUUIDName(SceneObjectTypeToString.at(SO_Renderables), o->uuid(),o->name());
						}
					},
					{ SO_Cameras, [str2JUUIDName](JUUID uuid)
						{
							CameraUUID o = uuid;
							if (o->hidden()) return JUUIDName();
							return str2JUUIDName(SceneObjectTypeToString.at(SO_Cameras), o->uuid(),o->name());
						}
					},
					{ SO_Lights, [str2JUUIDName](JUUID uuid)
						{
							LightUUID o = uuid;
							if (o->hidden()) return JUUIDName();
							return str2JUUIDName(SceneObjectTypeToString.at(SO_Lights), o->uuid(),o->name());
						}
					},
					{ SO_SoundEffects, [str2JUUIDName](JUUID uuid)
						{
							SoundFXUUID o = uuid;
							if (o->hidden()) return JUUIDName();
							return str2JUUIDName(SceneObjectTypeToString.at(SO_SoundEffects), o->uuid(),o->name());
						}
					}
				};

				auto uuids = GetSceneObjects(typeToGet);
				std::vector<JUUIDName> sceneObjectsTypeList;
				for (auto& uuid : uuids)
				{
					JUUIDName uuidName = getJUUIDName.at(typeToGet)(uuid);
					if (!std::get<0>(uuidName).empty())
						sceneObjectsTypeList.push_back(uuidName);
				}
				/*
				for (auto& [type, uuids] : scenesUnits.at(unit).sceneObjects)
				{
					if (typeToGet != type) continue;

					for (auto& uuid : uuids)
					{
						JUUIDName uuidName = getJUUIDName.at(type)(uuid);
						if (!std::get<0>(uuidName).empty())
							sceneObjectsTypeList.push_back(uuidName);
					}
				}*/
				return sceneObjectsTypeList;
			};
	}

	std::vector<JUUIDName> GetSceneObjectsTypesList()
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

		std::unordered_map<SceneObjectType, std::function<JUUIDName(JUUID)>> getJUUIDName =
		{
			{ SO_Renderables, [str2JUUIDName](JUUID uuid)
				{
					RenderableUUID o = uuid;
					if (o->hidden() || o->markedForDelete) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_Renderables), o->uuid(),o->name());
				}
			},
			{ SO_Cameras, [str2JUUIDName](JUUID uuid)
				{
					CameraUUID o = uuid;
					if (o->hidden() || o->markedForDelete) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_Cameras), o->uuid(),o->name());
				}
			},
			{ SO_Lights, [str2JUUIDName](JUUID uuid)
				{
					LightUUID o = uuid;
					if (o->hidden() || o->markedForDelete) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_Lights), o->uuid(),o->name());
				}
			},
			{ SO_SoundEffects, [str2JUUIDName](JUUID uuid)
				{
					SoundFXUUID o = uuid;
					if (o->hidden() || o->markedForDelete) return JUUIDName();
					return str2JUUIDName(SceneObjectTypeToString.at(SO_SoundEffects), o->uuid(),o->name());
				}
			}
		};

		std::vector<JUUIDName> sceneObjectsTypeList;
		for (auto& [unit, scene] : scenesUnits)
		{
			if (scenesUnits.at(unit)->loading->load()) continue;

			for (auto& [type, uuids] : scenesUnits.at(unit)->sceneObjects)
			{
				for (auto& uuid : uuids)
				{
					JUUIDName uuidName = getJUUIDName.at(type)(uuid);
					if (!std::get<0>(uuidName).empty())
						sceneObjectsTypeList.push_back(uuidName);
				}
			}
		}
		return sceneObjectsTypeList;
	}

	std::vector<std::pair<std::string, JsonToEditorValueType>> GetSceneObjectAttributes(SceneObjectType so)
	{
		const std::map<SceneObjectType, std::function<std::vector<std::pair<std::string, JsonToEditorValueType>>()>> GetSOAtts =
		{
			{ SO_Renderables, GetRenderableAttributes },
			{ SO_Lights, GetLightAttributes },
			{ SO_Cameras, GetCameraAttributes },
			{ SO_SoundEffects, GetSoundFXAttributes }
		};
		return GetSOAtts.at(so)();
	}

	std::map<std::string, JEdvEditorDrawerFunction> GetSceneObjectDrawers(SceneObjectType so)
	{
		const std::map<SceneObjectType, std::function<std::map<std::string, JEdvEditorDrawerFunction>()>> GetSODrawers =
		{
			{ SO_Renderables, GetRenderableDrawers },
			{ SO_Lights, GetLightDrawers },
			{ SO_Cameras, GetCameraDrawers },
			{ SO_SoundEffects, GetSoundFXDrawers }
		};
		return GetSODrawers.at(so)();
	}

	std::map<std::string, JEdvEditorDrawerFunction> GetSceneObjectPreviewers(SceneObjectType so)
	{
		const std::map<SceneObjectType, std::function<std::map<std::string, JEdvEditorDrawerFunction>()>> GetSOPreviewers =
		{
			{ SO_Renderables, GetRenderablePreviewers },
			{ SO_Lights, GetLightPreviewers },
			{ SO_Cameras, GetCameraPreviewers },
			{ SO_SoundEffects, GetSoundFXPreviewers }
		};
		return GetSOPreviewers.at(so)();
	}

	nlohmann::json GetSceneObjectJson(SceneObjectType so)
	{
		const std::map<SceneObjectType, std::function<nlohmann::json()>> GetSOJson =
		{
			{ SO_Renderables, CreateRenderableJson },
			{ SO_Lights, CreateLightJson },
			{ SO_Cameras, CreateCameraJson },
			{ SO_SoundEffects, CreateSoundFXJson }
		};
		return GetSOJson.at(so)();
	}

	std::vector<std::string> GetSceneObjectRequiredAttributes(SceneObjectType so)
	{
		const std::map<SceneObjectType, std::function<std::vector<std::string>()>> GetSORequiredAtts =
		{
			{ SO_Renderables, GetRenderableRequiredAttributes },
			{ SO_Lights, GetLightRequiredAttributes },
			{ SO_Cameras, GetCameraRequiredAttributes },
			{ SO_SoundEffects, GetSoundFXRequiredAttributes }
		};
		return GetSORequiredAtts.at(so)();
	}

	std::map<std::string, JEdvCreatorDrawerFunction> GetSceneObjectCreatorDrawers(SceneObjectType so)
	{
		const std::map<SceneObjectType, std::function<std::map<std::string, JEdvCreatorDrawerFunction>()>> GetSODrawers =
		{
			{ SO_Renderables, GetRenderableCreatorDrawers },
			{ SO_Lights, GetLightCreatorDrawers },
			{ SO_Cameras, GetCameraCreatorDrawers },
			{ SO_SoundEffects, GetSoundFXCreatorDrawers }
		};
		return GetSODrawers.at(so)();
	}

	std::map<std::string, JEdvCreatorValidatorFunction> GetSceneObjectValidators(SceneObjectType so)
	{
		const std::map<SceneObjectType, std::function<std::map<std::string, JEdvCreatorValidatorFunction>()>> GetSOValidators =
		{
			{ SO_Renderables, GetRenderableCreatorValidator },
			{ SO_Lights, GetLightCreatorValidator },
			{ SO_Cameras, GetCameraCreatorValidator },
			{ SO_SoundEffects, GetSoundFXCreatorValidator }
		};
		return GetSOValidators.at(so)();
	}

	void DeleteSceneObjectFromEditor(JUUID uuid)
	{
		using namespace Editor;

		SceneObjectType type = GetSceneObjectType(uuid);
		const std::map<SceneObjectType, std::function<void(JUUID)>> DeleteSO =
		{
			{ SO_Renderables, [](JUUID uuid)
			{
				RenderableUUID r = uuid;
				EraseSceneObjectFromSelection(r->unit, uuid);
				DeleteRenderable(uuid);
			}
			},
			{ SO_Lights, [](JUUID uuid)
			{
				LightUUID l = uuid;
				EraseSceneObjectFromSelection(l->unit, uuid);
				DeleteLight(uuid);
			}
			},
			{ SO_Cameras, [](JUUID uuid)
			{
				CameraUUID c = uuid;
				EraseSceneObjectFromSelection(c->unit, uuid);
				DeleteCamera(uuid);
			}
			},
			{ SO_SoundEffects, [](JUUID uuid)
			{
				SoundFXUUID s = uuid;
				EraseSceneObjectFromSelection(s->unit, uuid);
				DeleteSoundFX(uuid);
			}
			}
		};
		DeleteSO.at(type)(uuid);
		MarkScenePanelAssetsAsDirty();
	}
#endif

	void CreateSceneObject(SceneUnitId unit, SceneObjectType so, nlohmann::json json)
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

		AttachLevelIntoScene(unit, "new-scene-object", data, [so, uuid](SceneUnitId)
			{
#if defined(_EDITOR)
				if (so == SO_Renderables)
				{
					BindRenderableToPickingPass(uuid);
				}
#endif
			}, [](std::string, unsigned int, unsigned int) {});
	}
}