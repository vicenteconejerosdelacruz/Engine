#include "pch.h"
#include <Scene.h>
#include <set>
#include <SceneObject.h>
#include <Light/Light.h>
#include <Renderable/Renderable.h>
#include <Camera/Camera.h>
#include <RenderPass/RenderPass.h>
#include <Sound/SoundFX.h>
#include <Renderer.h>
#include <Binder.h>
#include <StepTimer.h>
#if defined(_EDITOR)
#include <Editor.h>
#endif

extern std::unique_ptr<Renderer> renderer;
extern void AnimableStep(double elapsedSeconds);
extern void AudioStep(float step);

namespace Scene
{
	std::unordered_map<SceneObjectType, std::set<JUUID>> sceneObjects;
	std::unordered_map<JUUID, SceneObjectType> sceneObjectsTypes;
	std::set<JUUID>& GetSceneObjects(SceneObjectType type)
	{
		if (!sceneObjects.contains(type))
			sceneObjects[type].clear();
		return sceneObjects.at(type);
	}
	std::unordered_map<JUUID, SceneObjectType>& GetSceneObjectsTypes()
	{
		return sceneObjectsTypes;
	}
	SceneObjectType GetSceneObjectType(JUUID uuid)
	{
		return sceneObjectsTypes.at(uuid);
	}
	bool SceneObjectExists(JUUID uuid)
	{
		return sceneObjectsTypes.contains(uuid);
	}
	Binder binder;

	void BindSceneObjects()
	{
		std::unordered_map<SceneObjectType, std::function<void(JUUID)>> typeBinder =
		{
			{ SO_Renderables, [](JUUID uuid)
				{
					auto& so = GetRenderableSceneObject(uuid);
					so->BindToScene();
					so->BindControllers();
				}
			},
			{ SO_Cameras, [](JUUID uuid)
				{
					auto& so = GetCameraSceneObject(uuid);
					so->BindToScene();
					so->BindControllers();
				}
			},
			{ SO_Lights, [](JUUID uuid)
				{
					auto& so = GetLightSceneObject(uuid);
					so->BindToScene();
					so->BindControllers();
				}
			},
			{ SO_SoundEffects, [](JUUID uuid)
				{
					auto& so = GetSoundFXSceneObject(uuid);
					so->BindToScene();
					so->BindControllers();
				}
			}
		};
		for (auto& [uuid, type] : sceneObjectsTypes)
		{
			typeBinder.at(type)(uuid);
		}
	}

	JUUID CloneSceneObject(JUUID sceneObject, nlohmann::json parameters)
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
			CreateCamera(data);
		}
		break;
		case SO_Lights:
		{
			CreateLight(data);
		}
		break;
		case SO_Renderables:
		{
			CreateRenderable(data);
		}
		break;
		case SO_SoundEffects:
		{
			CreateSoundFX(data);
		}
		break;
		}
		return uuid;
	}

	void BindToScene(JUUID uuidA, JUUID uuidB)
	{
		binder.insert(uuidA, uuidB);
	}

	void UnbindFromScene(JUUID uuidA)
	{
		binder.erase(uuidA);
	}

	void UnbindFromScene(JUUID uuidA, JUUID uuidB)
	{
		binder.erase(uuidA, uuidB);
	}

	void SceneObjectsStep(DX::StepTimer& timer)
	{
		float dt = static_cast<FLOAT>(timer.GetElapsedSeconds());
#if defined(_EDITOR)
		if (!Editor::IsPlaying() || Editor::IsPaused())
			dt = 0.0f;
#endif
		RenderablesStep();
		AnimableStep(dt);
		LightsStep();
		AudioStep(dt);
		CamerasStep();
	}

	void WriteConstantsBuffers()
	{
		unsigned int backBufferIndex = renderer->backBufferIndex;

		for (RenderableUUID r : GetRenderables())
		{
			r->WriteAnimationConstantsBuffer(backBufferIndex);
			r->WriteConstantsBuffer(backBufferIndex);
		}

		//write the constants buffers of the cameras which renders shadow maps
		for (CameraUUID c : GetCameras())
		{
			if (!c->shadowMapLight().empty()) continue;
			c->WriteLightsConstantsBuffer();
			c->WriteShadowMapsConstantsBuffer();
		}
	}

	void RenderSceneShadowMaps()
	{
#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandQueue.p, 0, L"ShadowMaps");
#endif
		for (LightUUID l : GetLights())
		{
			if (!l->hasShadowMaps()) continue;

			auto renderSceneShadowMap = [&l](unsigned int cameraIndex)
				{
					auto& camera = l->shadowMapCameras.at(cameraIndex);
					auto& rp = camera->renderPassesUUID.at(0);
					for (RenderableUUID r : camera->renderables)
					{
						if (r->castShadows())
						{
							r->Render(rp, camera);
						}
					}
				};

#if defined(_DEVELOPMENT)
			std::string shadowMapEvent = "ShadowMap:" + l->name();
			PIXBeginEvent(renderer->commandList.p, 0, nostd::StringToWString(shadowMapEvent).c_str());
#endif

			l->RenderShadowMap(renderSceneShadowMap);

#if defined(_DEVELOPMENT)
			PIXEndEvent(renderer->commandList.p);
#endif
		}
#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandQueue.p);
#endif
	}

	void RenderSceneCameras()
	{
		auto cameras(GetCameras());
		//get all the available cameras and start filtering thing we don't want to render
		for (auto it = cameras.begin(); it != cameras.end();)
		{
			JUUID uuid = *it;
			//filter out cameras which doesn't exists
			if (!SceneObjectExists(uuid))
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
		std::copy_if(cameras.begin(), cameras.end(), std::back_inserter(nonSwapChainCams), [](JUUID uuid)
			{
				auto& cam = GetCameraSceneObject(uuid);
				//we skip swap chain cams
				if (cam->useSwapChain()) return false;
				//we skip cameras which resolves to the swapchain
				if (cam->renderPassesUUID.size() > 0ULL && cam->renderPassesUUID.back()->type == RenderPassType_SwapChainPass) return false;
				return true;
			}
		);

		//render non swapchain buffer cameras(rtt stuff)
		for (auto& uuid : nonSwapChainCams)
		{
			auto& cam = GetCameraSceneObject(uuid);
#if defined(_DEVELOPMENT)
			PIXBeginEvent(renderer->commandList.p, 0, std::string("nonSwapChain:" + cam->name()).c_str());
#endif
			cam->Render();
#if defined(_DEVELOPMENT)
			PIXEndEvent(renderer->commandList.p);
#endif
		}

		//check if there is any camera with any render pass resolving to the swapchain
		bool resolvedToSwapchain = std::any_of(nonSwapChainCams.begin(), nonSwapChainCams.end(), [](JUUID camUUID)
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
		if (GetCountFromSwapChainCameras() > 0ULL && !resolvedToSwapchain)
		{
			JUUID camUUID = *GetSwapChainCameras().begin();
			auto& cam = GetFromSwapChainCameras(camUUID);
#if defined(_DEVELOPMENT)
			PIXBeginEvent(renderer->commandList.p, 0, std::string("SwapChain:" + cam->name()).c_str());
#endif
			cam->Render();
#if defined(_DEVELOPMENT)
			PIXEndEvent(renderer->commandList.p);
#endif
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

				std::vector<JUUIDName> sceneObjectsTypeList;
				for (auto& [type, uuids] : sceneObjects)
				{
					if (typeToGet != type) continue;

					for (auto& uuid : uuids)
					{
						JUUIDName uuidName = getJUUIDName.at(type)(uuid);
						if (!std::get<0>(uuidName).empty())
							sceneObjectsTypeList.push_back(uuidName);
					}
				}
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
		for (auto& [type, uuids] : sceneObjects)
		{
			for (auto& uuid : uuids)
			{
				JUUIDName uuidName = getJUUIDName.at(type)(uuid);
				if (!std::get<0>(uuidName).empty())
					sceneObjectsTypeList.push_back(uuidName);
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

	void CreateSceneObject(SceneObjectType so, nlohmann::json json)
	{
		const std::map<SceneObjectType, std::function<void(nlohmann::json json)>> CreateSO =
		{
			{ SO_Renderables, [](nlohmann::json json) {
				RenderableUUID o = getUUID();
				nlohmann::json patch = { {"uuid", o()}};
				json.merge_patch(patch);
				CreateRenderable(json);
				o->BindToScene();
#if defined(_EDITOR)
				if (!Editor::IsPlaying())
				{
					Editor::MarkScenePanelAssetsAsDirty();
					Editor::BindRenderableToPickingPass(o());
				}
#endif
				o->BindShadowMapCameras();
			}},
			{ SO_Lights, [](nlohmann::json json) {
				LightUUID o = getUUID();
				nlohmann::json patch = { {"uuid", o()}};
				json.merge_patch(patch);
				CreateLight(json);
				o->BindToScene();
#if defined(_EDITOR)
				if (!Editor::IsPlaying())
				{
					Editor::MarkScenePanelAssetsAsDirty();
					Editor::RegisterBillboard(o());
				}
#endif
			}},
			{ SO_Cameras, [](nlohmann::json json) {
				CameraUUID o = getUUID();
				nlohmann::json patch = { {"uuid", o()}};
				json.merge_patch(patch);
				CreateCamera(json);
				o->BindToScene();
#if defined(_EDITOR)
				if (!Editor::IsPlaying())
				{
					Editor::MarkScenePanelAssetsAsDirty();
					Editor::RegisterBillboard(o());
				}
#endif
			} },
			{ SO_SoundEffects, [](nlohmann::json json) {
				SoundFXUUID o = getUUID();
				nlohmann::json patch = { {"uuid", o()}};
				json.merge_patch(patch);
				CreateSoundFX(json);
				o->BindToScene();
#if defined(_EDITOR)
				if (!Editor::IsPlaying())
				{
					Editor::MarkScenePanelAssetsAsDirty();
					Editor::RegisterBillboard(o());
				}
#endif
			}}
		};
		CreateSO.at(so)(json);
	}

	void DeleteSceneObjectFromEditor(JUUID uuid)
	{
		SceneObjectType type = GetSceneObjectType(uuid);
		const std::map<SceneObjectType, std::function<void(JUUID)>> DeleteSO =
		{
			{ SO_Renderables, [](JUUID uuid)
			{
				Editor::EraseSceneObjectFromSelection(uuid);
				DeleteRenderable(uuid);
				Editor::MarkScenePanelAssetsAsDirty();
			}
			},
			{ SO_Lights, [](JUUID uuid)
			{
				Editor::EraseSceneObjectFromSelection(uuid);
				DeleteLight(uuid);
				Editor::MarkScenePanelAssetsAsDirty();
			}
			},
			{ SO_Cameras, [](JUUID uuid)
			{
				Editor::EraseSceneObjectFromSelection(uuid);
				DeleteCamera(uuid);
				Editor::MarkScenePanelAssetsAsDirty();
			}
			},
			{ SO_SoundEffects, [](JUUID uuid)
			{
				Editor::EraseSceneObjectFromSelection(uuid);
				DeleteSoundFX(uuid);
				Editor::MarkScenePanelAssetsAsDirty();
			}
			}
		};
		DeleteSO.at(type)(uuid);
	}
#endif
}