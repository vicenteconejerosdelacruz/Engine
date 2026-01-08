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

	SceneUnit::SceneUnit(SceneUnitId unit, std::string name)
	{
		id = unit;
		unitName = name;
		markedForDelete = false;
		deletionFrames = framesUntilDeletion;
		attached = false;
		mergeable = false;
		runningCompute = false;
		abortLoading = std::make_unique<std::atomic_bool>(false);
		sceneUnitLoaded = std::make_unique<std::atomic_bool>(false);
		loading = std::make_unique<std::atomic_bool>(false);
		loadingSubmit = std::make_unique<std::atomic_bool>(false);
		binder.unit = unit;
		CreateShadowMapResources(id);
	}

	SceneUnit::~SceneUnit()
	{
		DestroyShadowMapResources(id);
	}

	void SceneUnit::Merge(std::unique_ptr<SceneUnit>& other)
	{
		for (auto& [type, juuids] : other->sceneObjects)
		{
			for (auto& uuid : juuids)
			{
				auto* so = GetSceneObjectPointer(id, uuid);
				so->unit = id;
				so->BindToScene();
				sceneObjects[type].insert(uuid);
			}
		}
		for (auto& [uuid, type] : other->sceneObjectsTypes)
		{
			sceneObjectsTypes.insert_or_assign(uuid, type);
		}
	}

	void SceneUnit::MarkForDelete()
	{
		markedForDelete = true;
		//sceneObjects.clear();
		//sceneObjectsTypes.clear();
	}

	CComPtr<ID3D12GraphicsCommandList2>& SceneUnit::GetLoadingCommandList()
	{
		return loadingProcessor.GetCommandList();
	}

	void SceneUnit::ResetLoadingCommandList()
	{
		loadingProcessor.ResetCommandList();
		loading->store(true);
		loadingSubmit->store(false);
	}

	void SceneUnit::CloseLoadingCommandList()
	{
		loadingProcessor.CloseCommandList();
	}

	void SceneUnit::SubmitLoadingCommandList()
	{
		renderer->ExecuteCommands(GetLoadingCommandList());
	}

	CComPtr<ID3D12GraphicsCommandList2>& SceneUnit::GetCommandList()
	{
		return commandsProcessor.GetCommandList();
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

	bool SceneUnit::IsBound(JUUID uuid)
	{
		return sceneObjectsTypes.contains(uuid) && !unboundedSceneObjects.contains(uuid);
	}

	void SceneUnit::CloseCommandList()
	{
		commandsProcessor.CloseCommandList();
	}

	void SceneUnit::SubmitCommandList()
	{
		renderer->ExecuteCommands(GetCommandList());
	}

	void SceneUnit::Loading()
	{
		using namespace Scene;

		if (!sceneUnitLoaded->load())
			return;

		if (loading->load())
		{
			loadingSubmit->store(true);
			loading->store(false);
			CloseSubmitLoadingCommandList();
		}
	}

	void SceneUnit::Render()
	{
		using namespace Scene;
#if defined(_EDITOR)
		using namespace Editor;
#endif
		if (!sceneUnitLoaded->load())
			return;

		if (renderer->GetBackBufferIndex() == Frame() && !attached)
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
					CameraSUUUID camera = MAKESUUUID(id, (*GetSwapChainCameras(id).begin()));
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

		if (!sceneUnitLoaded->load())
			return;

		if (loadingSubmit->load())
		{
			loadingSubmit->store(false);
			for (auto& uuid : renderablesInLoadingPool)
			{
				RenderableSUUUID r = MAKESUUUID(id, uuid);
				if (r->renderReady || !IsBound(uuid))
					continue;
				r->renderReady = true;
			}
			renderablesInLoadingPool.clear();
			if (attached)
				mergeable = true;
#if defined(_EDITOR)
			MarkScenePanelAssetsAsDirty();
#endif
		}
		PickFromScene(id);
	}

	CComPtr<ID3D12GraphicsCommandList2>& SceneUnit::GetComputeCommandList()
	{
		return computeProcessor.GetCommandList();
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
		renderer->ExecuteCommands(GetComputeCommandList());
	}

	void SceneUnit::NextComputeCommandList()
	{
		computeProcessor.Next();
	}

	void SceneUnit::RunComputeShaders()
	{
		using namespace Scene;

		if (!sceneUnitLoaded->load())
			return;

		runningCompute = true;
		ResetComputeCommandList();
		RunBoundingBoxComputeShaders(id);
	}

	void SceneUnit::SolveComputeShaders()
	{
		using namespace Scene;

		if (!sceneUnitLoaded->load() || !runningCompute)
			return;

		RunBoundingBoxComputeShadersSolution(id);
		CloseSubmitAndNextComputeCommandList();
		runningCompute = false;
	}
};