#include "pch.h"
#include <SceneUnit.h>
#include <Renderer.h>
#include <Renderable/Renderable.h>
#include <SceneObject.h>
#if defined(_EDITOR)
#include <Editor.h>
#endif

extern std::unique_ptr<Renderer> renderer;

namespace Scene
{
	extern void RenderSceneCameras(SceneUnitId id);
	extern SceneObject* GetSceneObjectPointer(SceneUnitId id, JUUID uuid);
	extern void MoveSceneObjectUnit(JUUID uuid, SceneUnitId fromId, SceneUnitId toId);

	SceneUnit::SceneUnit(SceneUnitId unit, std::string name)
	{
		id = unit;
		unitName = name;
		markedForDelete = false;
		deleteCallback = nullptr;
		isolated = false;
		loading = std::make_unique<std::atomic_bool>(false);
		canSubmitLoading = std::make_unique<std::atomic_bool>(false);
		loadingComplete = std::make_unique<std::atomic_bool>(false);
		binder.unit = unit;
		CreateShadowMapResources(id);
	}

	SceneUnit::~SceneUnit()
	{
		DestroyShadowMapResources(id);
	}

	SceneUnitId SceneUnit::Id()
	{
		return id;
	}

	void SceneUnit::MarkForDelete(std::function<void()> cb)
	{
		markedForDelete = true;
		deleteCallback = cb;
	}

	bool SceneUnit::MarkedForDelete()
	{
		return markedForDelete;
	}

	unsigned int SceneUnit::DeleteFrames()
	{
		return deleteFrames;
	}

	void SceneUnit::DecreaseDeleteFrames()
	{
		deleteFrames--;
	}

	void SceneUnit::CallDeleteCallback()
	{
		if (deleteCallback)
			deleteCallback();
	}

	void SceneUnit::SetIsolated(bool value)
	{
		isolated = value;
	}

	bool SceneUnit::IsIsolated()
	{
		return isolated;
	}

	void SceneUnit::DestroySceneObjects()
	{
		DestroyTriggers(id);
		DestroyRenderables(id);
		DestroySoundEffects(id);
		DestroyLights(id);
		DestroyCameras(id);
		DestroyPhysicScenes(id);
	}

	std::unordered_map<JUUID, SceneObjectType>& SceneUnit::GetSceneObjectTypes()
	{
		return sceneObjectsTypes;
	}

	std::unordered_map<SceneObjectType, std::set<JUUID>>& SceneUnit::GetSceneObjects()
	{
		return sceneObjects;
	}

	std::set<JUUID>& SceneUnit::GetUnboundedSceneObjects()
	{
		return unboundedSceneObjects;
	}

	void SceneUnit::InsertRenderableIntoLoadingPool(RenderableID uuid)
	{
		renderablesInLoadingPool.insert(uuid);
	}

	void SceneUnit::InsertCameraIntoLoadingPool(CameraID uuid)
	{
		camerasInLoadingPool.insert(uuid);
	}

	void SceneUnit::InsertLightIntoLoadingPool(LightID uuid)
	{
		lightsInLoadingPool.insert(uuid);
	}

	std::set<RenderableID>& SceneUnit::GetRenderablesInLoadingPool()
	{
		return renderablesInLoadingPool;
	}

	std::set<CameraID>& SceneUnit::GetCamerasInLoadingPool()
	{
		return camerasInLoadingPool;
	}

	std::set<LightID>& SceneUnit::GetLightsInLoadingPool()
	{
		return lightsInLoadingPool;
	}

	size_t SceneUnit::GetRenderablesLoadingPoolSize()
	{
		return renderablesInLoadingPool.size();
	}

	size_t SceneUnit::GetCamerasLoadingPoolSize()
	{
		return camerasInLoadingPool.size();
	}

	size_t SceneUnit::GetLightsLoadingPoolSize()
	{
		return lightsInLoadingPool.size();
	}

	void SceneUnit::MarkRenderablesInLoadingPoolAsReady()
	{
		for (RenderableID r : GetRenderablesInLoadingPool())
		{
			if (r->RenderReady() || !IsBound(r->uuid()))
				continue;
			r->RenderReady(true);
		}
	}

	void SceneUnit::MarkCamerasInLoadingPoolAsReady()
	{
		for (CameraID c : GetCamerasInLoadingPool())
		{
			if (c->RenderReady() || !IsBound(c->uuid()))
				continue;
			c->RenderReady(true);
		}
	}

	void SceneUnit::MarkLightsInLoadingPoolAsReady()
	{
		for (LightID l : GetLightsInLoadingPool())
		{
			if (l->RenderReady() || !IsBound(l->uuid()))
				continue;
			l->RenderReady(true);
		}
	}

	void SceneUnit::ClearRenderablesLoadingPool()
	{
		renderablesInLoadingPool.clear();
	}

	void SceneUnit::ClearCamerasLoadingPool()
	{
		camerasInLoadingPool.clear();
	}

	void SceneUnit::ClearLightsLoadingPool()
	{
		lightsInLoadingPool.clear();
	}

	void SceneUnit::EraseRenderableFromLoadingPool(RenderableID r)
	{
		if (renderablesInLoadingPool.contains(r))
			renderablesInLoadingPool.erase(r);
	}

	void SceneUnit::EraseCameraFromLoadingPool(CameraID c)
	{
		if (camerasInLoadingPool.erase(c))
			camerasInLoadingPool.erase(c);
	}

	void SceneUnit::EraseLightFromLoadingPool(LightID l)
	{
		if (lightsInLoadingPool.contains(l))
			lightsInLoadingPool.erase(l);
	}

	void SceneUnit::InitLoadingProcessor(CComPtr<ID3D12Device2> d3dDevice, size_t id, size_t capacity)
	{
		loadingProcessor.Init(d3dDevice, id, capacity);
		SetLoading(true);
	}

	void SceneUnit::InitFrame2FrameProcessor(CComPtr<ID3D12Device2> d3dDevice, size_t id, size_t capacity)
	{
		commandsProcessor.Init(d3dDevice, id, capacity);
	}

	void SceneUnit::InitComputeProcessor(CComPtr<ID3D12Device2> d3dDevice, size_t id, size_t capacity)
	{
		computeProcessor.Init(d3dDevice, id, capacity);
	}

	void SceneUnit::SetLoading(bool value)
	{
		loading->store(value);
	}

	bool SceneUnit::IsLoading()
	{
		return loading->load();
	}

	void SceneUnit::SetCanSubmitLoading(bool value)
	{
		canSubmitLoading->store(value);
	}

	bool SceneUnit::IsReadyToSubmitLoading()
	{
		return canSubmitLoading->load();
	}

	void SceneUnit::SetLoadingComplete(bool value)
	{
		loadingComplete->store(value);
	}

	bool SceneUnit::IsLoadingComplete()
	{
		return loadingComplete->load();
	}

	void SceneUnit::Bind(JUUID uuidA, JUUID uuidB)
	{
		binder.insert(uuidA, uuidB);
	}

	void SceneUnit::Unbind(JUUID uuid)
	{
		binder.erase(uuid);
	}

	void SceneUnit::Unbind(JUUID uuidA, JUUID uuidB)
	{
		binder.erase(uuidA, uuidB);
	}

	bool SceneUnit::IsBound(JUUID uuid)
	{
		return sceneObjectsTypes.contains(uuid) && !unboundedSceneObjects.contains(uuid);
	}

	void SceneUnit::AddSceneObjectToUnboundPool(JUUID uuid)
	{
		unboundedSceneObjects.insert(uuid);
	}

	void SceneUnit::RemoveSceneObjectFromUnboundPool(JUUID uuid)
	{
		if (unboundedSceneObjects.contains(uuid))
			unboundedSceneObjects.insert(uuid);
	}

	bool SceneUnit::LoadingCommandListIsOpen()
	{
		return loadingProcessor.IsOpen();
	}

	CComPtr<ID3D12GraphicsCommandList2>& SceneUnit::GetLoadingCommandList(bool OpenIfClosed)
	{
		return loadingProcessor.GetCommandList(OpenIfClosed);
	}

	void SceneUnit::ResetLoadingCommandList()
	{
		loadingProcessor.ResetCommandList();
	}

	void SceneUnit::CloseLoadingCommandList()
	{
		loadingProcessor.CloseCommandList();
	}

	void SceneUnit::SubmitLoadingCommandList()
	{
		renderer->ExecuteCommands(GetLoadingCommandList(false), [&]
			{
				SetLoading(false);
				SetCanSubmitLoading(false);
				MarkRenderablesInLoadingPoolAsReady();
				MarkCamerasInLoadingPoolAsReady();
				MarkLightsInLoadingPoolAsReady();
				ClearRenderablesLoadingPool();
				ClearCamerasLoadingPool();
				ClearLightsLoadingPool();
#if defined(_EDITOR)
				using namespace Editor;
				MarkScenePanelAssetsAsDirty();
#endif
				for (auto& cb : postLoadingExecutionCallbacks)
				{
					cb();
				}
				postLoadingExecutionCallbacks.clear();
			}
		);
	}

	void SceneUnit::PushLoadingExecutionCallback(std::function<void()> cb)
	{
		postLoadingExecutionCallbacks.push_back(cb);
	}

	void SceneUnit::Loading()
	{
		using namespace Scene;

		if (!IsLoading() || !IsReadyToSubmitLoading()) return;

		CloseSubmitLoadingCommandList();
	}

	void SceneUnit::SubmitForLoading(std::function<void()> loader)
	{
		bool doSubmit = !LoadingCommandListIsOpen();
		if (doSubmit)
		{
			ResetLoadingCommandList();
			SetLoading(true);
			SetCanSubmitLoading(false);
		}

		loader();

		if (doSubmit)
		{
			SetCanSubmitLoading(true);
		}
	}

	//F2F
	CComPtr<ID3D12GraphicsCommandList2>& SceneUnit::GetCommandList(bool OpenIfClosed)
	{
		return commandsProcessor.GetCommandList(OpenIfClosed);
	}

	void SceneUnit::ResetCommandList()
	{
		commandsProcessor.ResetCommandList();
	}

	void SceneUnit::NextCommandList()
	{
		commandsProcessor.Next();
	}

	unsigned int SceneUnit::Frame() const
	{
		return commandsProcessor.frame;
	}

	void SceneUnit::CloseCommandList()
	{
		commandsProcessor.CloseCommandList();
	}

	void SceneUnit::SubmitCommandList()
	{
		renderer->ExecuteCommands(GetCommandList(false), [&]
			{
				RunTextureUploadFreeResources();
			}
		);
	}

	void SceneUnit::Render()
	{
		using namespace Scene;
#if defined(_EDITOR)
		using namespace Editor;
#endif

		if (renderer->GetBackBufferIndex() == Frame())
		{

#if defined(_EDITOR)
			if (!IsPlaying(id))
				HandleEditorMouseMovements(id);
#endif
			WriteConstantsBuffers(id);
			ResetCommandList();
			{
#if defined(_EDITOR)
				if (GetCountFromSwapChainCameras(id) > 0)
				{
					CameraID camera = MAKESUUUID(id, (*GetSwapChainCameras(id).begin()));
					RenderPickingPass(id, camera);
				}
#endif
				RenderSceneShadowMaps(id);
				RenderSceneCameras(id);
			}
			CloseSubmitAndNextCommandList();
		}
	}

	void SceneUnit::PostRender()
	{
#if defined(_EDITOR)
		using namespace Editor;
#endif
		PickFromScene(id);
	}

	CComPtr<ID3D12GraphicsCommandList2>& SceneUnit::GetComputeCommandList(bool OpenIfClosed)
	{
		return computeProcessor.GetCommandList(OpenIfClosed);
	}

	void SceneUnit::ResetComputeCommandList()
	{
		computeProcessor.ResetCommandList();
	}

	void SceneUnit::CloseComputeCommandList()
	{
		computeProcessor.CloseCommandList();
	}

	void SceneUnit::SubmitComputeCommandList()
	{
		renderer->ExecuteCommands(GetComputeCommandList(false));
	}

	void SceneUnit::NextComputeCommandList()
	{
		computeProcessor.Next();
	}

	void SceneUnit::RunComputeShaders()
	{
		using namespace Scene;
		ResetComputeCommandList();
		RunBoundingBoxComputeShaders(id);
	}

	void SceneUnit::SolveComputeShaders()
	{
		using namespace Scene;
		RunBoundingBoxComputeShadersSolution(id);
		CloseSubmitAndNextComputeCommandList();
	}
};