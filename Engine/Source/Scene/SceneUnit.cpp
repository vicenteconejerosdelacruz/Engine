#include "pch.h"
#include <SceneUnit.h>
#include <Renderer.h>
#include <Renderable/Renderable.h>
#include <SceneObject.h>
#include <Scripting.h>
#if defined(_EDITOR)
#include <Editor.h>
#endif

extern std::unique_ptr<JRenderer> renderer;

#if defined(_EDITOR)
namespace Editor
{
	extern std::unordered_map<SceneUnitId, CameraID> editorCameraUUID;
}
#endif

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
		binder.unit = unit;
#if defined(_EDITOR)
		canBuildAssetsTree = std::make_unique<std::atomic_uint>(0U);
#endif
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
		DestroyBoundaries(id);
		DestroyRenderables(id);
		DestroySoundEffects(id);
		DestroyLights(id);
		DestroyCameras(id);
		DestroyPhysicScenes(id);
		DestroySceneControllers(id);
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

	void SceneUnit::InitFrame2FrameProcessor(CComPtr<ID3D12Device2> d3dDevice, size_t capacity, size_t id)
	{
		commandsProcessor = std::make_unique<CommandsProcessor>(d3dDevice, capacity, id);
	}

	void SceneUnit::InitComputeProcessor(CComPtr<ID3D12Device2> d3dDevice, size_t capacity, size_t id)
	{
		computeProcessor = std::make_unique<CommandsProcessor>(d3dDevice, capacity, id);
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

	//F2F
	CComPtr<ID3D12GraphicsCommandList2>& SceneUnit::GetCommandList()
	{
		return commandsProcessor->GetCommandList();
	}

	void SceneUnit::ResetCommandList()
	{
		commandsProcessor->ResetCommandList();
	}

	void SceneUnit::NextCommandList()
	{
		commandsProcessor->Next();
	}

	unsigned int SceneUnit::Frame() const
	{
		return commandsProcessor->frame;
	}

	void SceneUnit::CloseCommandList()
	{
		commandsProcessor->CloseCommandList();
	}

	void SceneUnit::SubmitCommandList()
	{
		commandsProcessor->ExecuteCommandList();
	}

	void SceneUnit::Render()
	{
		using namespace Scene;
#if defined(_EDITOR)
		using namespace Editor;
#endif

#if defined(_DEVELOPMENT)
		std::string RenderEvent = "SceneUnit::Render(" + unitName + ")";
		PIXScopedEvent(0, nostd::StringToWString(RenderEvent).c_str());
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
				if (GetCountFromSwapChainCameras(id) > 0 && editorCameraUUID.contains(id))
				{
					using namespace Editor;
					CameraID camera = editorCameraUUID.at(id);
					RenderPickingPass(id, camera);
				}
#endif
				RenderSceneShadowMaps(id);
				RenderSceneCameras(id);
				RenderControllers(id);
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

	CComPtr<ID3D12GraphicsCommandList2>& SceneUnit::GetComputeCommandList()
	{
		return computeProcessor->GetCommandList();
	}

	void SceneUnit::ResetComputeCommandList()
	{
		computeProcessor->ResetCommandList();
	}

	void SceneUnit::CloseComputeCommandList()
	{
		computeProcessor->CloseCommandList();
	}

	void SceneUnit::SubmitComputeCommandList()
	{
		computeProcessor->ExecuteCommandList();
	}

	void SceneUnit::NextComputeCommandList()
	{
		computeProcessor->Next();
	}

	void SceneUnit::RunComputeShaders()
	{
		using namespace Scene;
		ResetComputeCommandList();
		//RunBoundingBoxComputeShaders(id);
	}

	void SceneUnit::SolveComputeShaders()
	{
		using namespace Scene;
		//RunBoundingBoxComputeShadersSolution(id);
		CloseSubmitAndNextComputeCommandList();
	}
#if defined(_EDITOR)
	bool SceneUnit::CanBuildAssetsTree()
	{
		return canBuildAssetsTree->load() == 0U;
	}

	void SceneUnit::SetCanBuildAssetsTree(bool value)
	{
		if (!value)
		{
			canBuildAssetsTree->fetch_add(1U);
		}
		else
		{
			unsigned int prev = canBuildAssetsTree->fetch_sub(1U);
			assert(prev != 0);
		}
	}
#endif

};