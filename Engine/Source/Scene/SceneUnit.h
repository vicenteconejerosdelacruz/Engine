#pragma once

#include <unordered_map>
#include <set>
#include <DeviceUtils/CommandsProcessor/CommandsProcessor.h>
#include <Binder.h>
#include <v8.h>
#include <nov8.h>

using namespace DeviceUtils;
using namespace nov8;

enum SceneObjectType;

namespace Scene
{
	struct SceneObject;

	class SceneUnit
	{
	public:
		SceneUnit(SceneUnitId unit, std::string name);
		~SceneUnit();
		SceneUnitId Id();
		void MarkForDelete(std::function<void()> cb = nullptr);
		bool MarkedForDelete();
		unsigned int DeleteFrames();
		void DecreaseDeleteFrames();
		void CallDeleteCallback();
		void SetIsolated(bool value);
		bool IsIsolated();
		void DestroySceneObjects();
		std::unordered_map<JUUID, SceneObjectType>& GetSceneObjectTypes();
		std::unordered_map<SceneObjectType, std::set<JUUID>>& GetSceneObjects();
		std::set<JUUID>& GetUnboundedSceneObjects();

		//Scene
		void Bind(JUUID uuidA, JUUID uuidB);
		void Unbind(JUUID uuid);
		void Unbind(JUUID uuidA, JUUID uuidB);
		bool IsBound(JUUID uuid);
		void AddSceneObjectToUnboundPool(JUUID uuid);
		void RemoveSceneObjectFromUnboundPool(JUUID uuid);

		//Frame2Frame
		void InitFrame2FrameProcessor(CComPtr<ID3D12Device2> d3dDevice, size_t capacity, size_t id);
		CComPtr<ID3D12GraphicsCommandList2>& GetCommandList();
		void ResetCommandList();
		void CloseCommandList();
		void SubmitCommandList();
		void NextCommandList();
		void CloseSubmitAndNextCommandList()
		{
			CloseCommandList();
			SubmitCommandList();
			NextCommandList();
		}
		unsigned int Frame() const;

		//Render
		void Render();
		void PostRender();

		//Compute
		void InitComputeProcessor(CComPtr<ID3D12Device2> d3dDevice, size_t capacity, size_t id);
		bool HasComputeProcessor() { return computeProcessor != nullptr; }
		CComPtr<ID3D12GraphicsCommandList2>& GetComputeCommandList();
		void CloseSubmitAndNextComputeCommandList()
		{
			CloseComputeCommandList();
			SubmitComputeCommandList();
			NextComputeCommandList();
		}
		void ResetComputeCommandList();
		void CloseComputeCommandList();
		void SubmitComputeCommandList();
		void NextComputeCommandList();
		void RunComputeShaders();
		void SolveComputeShaders();
#if defined(_EDITOR)
		bool CanBuildAssetsTree();
		void SetCanBuildAssetsTree(bool value);
#endif

		//Scripting
		void CreateScriptingSceneTemplate();
		void ClearScriptingSceneTemplate();
		Global<ObjectTemplate>& GetScriptingSceneTemplate();
		void AddToScriptingSceneTemplate(SceneObject* so);
		void EraseFromScriptingSceneTemplate(SceneObject* so);

	private:
		SceneUnitId id;
		std::string unitName;
		bool markedForDelete;
		unsigned int deleteFrames = JRenderer::numFrames;
		std::function<void()> deleteCallback;
		bool isolated;
#if defined(_EDITOR)
		std::unique_ptr<std::atomic_uint> canBuildAssetsTree;
#endif

		//Scene
		Binder binder;
		std::set<JUUID> unboundedSceneObjects;
		std::unordered_map<SceneObjectType, std::set<JUUID>> sceneObjects;
		std::unordered_map<JUUID, SceneObjectType> sceneObjectsTypes;

		//f2f
		std::unique_ptr<CommandsProcessor> commandsProcessor;
		std::unique_ptr<CommandsProcessor> computeProcessor;

		//Scripting
		Global<ObjectTemplate> sceneTemplate;
		std::map<SceneObjectType, Global<ObjectTemplate>> containersTemplates;
	};
};