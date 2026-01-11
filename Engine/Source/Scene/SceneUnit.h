#pragma once

#include <unordered_map>
#include <set>
#include <DeviceUtils/CommandsProcessor/CommandsProcessor.h>
#include <Binder.h>

using namespace DeviceUtils;

enum SceneObjectType;

namespace Scene
{
	struct SceneUnit
	{
		static const unsigned int framesUntilDeletion = 2U;

		SceneUnit(SceneUnitId unit, std::string name);
		~SceneUnit();
		void Merge(std::unique_ptr<SceneUnit>& other);
		void MarkForDelete();

		//Loading
		CComPtr<ID3D12GraphicsCommandList2>& GetLoadingCommandList();
		void ResetLoadingCommandList();
		void CloseLoadingCommandList();
		void SubmitLoadingCommandList();
		void CloseSubmitLoadingCommandList()
		{
			CloseLoadingCommandList();
			SubmitLoadingCommandList();
		}

		//Frame2Frame
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
		bool IsBound(JUUID uuid);

		//Render
		void Loading();
		void Render();
		void PostRender();
		//Compute
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

		std::unique_ptr<std::atomic_bool> abortLoading;
		SceneUnitId id;
		std::string unitName;
		bool markedForDelete;
		unsigned int deletionFrames;
		bool attached;
		bool mergeable;
		bool isolated;
		SceneUnitId parentUnit;
		//scene objects
		std::unique_ptr<std::atomic_bool> sceneUnitLoaded;
		std::unordered_map<SceneObjectType, std::set<JUUID>> sceneObjects;
		std::unordered_map<JUUID, SceneObjectType> sceneObjectsTypes;
		//binding
		std::set<JUUID> unboundedSceneObjects;
		Binder binder;
		//command list
		std::set<JUUID> renderablesInLoadingPool;
		std::unique_ptr<std::atomic_bool> loading;
		std::unique_ptr<std::atomic_bool> loadingSubmit;
		CommandsProcessor loadingProcessor;
		CommandsProcessor commandsProcessor;
		bool runningCompute;
		CommandsProcessor computeProcessor;
	};
};