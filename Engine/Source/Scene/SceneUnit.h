#pragma once

#include <unordered_map>
#include <set>
#include <DeviceUtils/CommandsProcessor/CommandsProcessor.h>
#include <Binder.h>

using namespace DeviceUtils;

enum SceneObjectType;

namespace Scene
{
	class SceneUnit
	{
	public:
		static const unsigned int framesUntilDeletion = 2U;

		SceneUnit(SceneUnitId unit, std::string name);
		~SceneUnit();
		SceneUnitId Id();
		//void Merge(std::unique_ptr<SceneUnit>& other);
		void MarkForDelete();
		bool MarkedForDelete();
		//void SetAttached(bool value);
		//bool IsAttached();
		void SetIsolated(bool value);
		bool IsIsolated();
		std::unordered_map<JUUID, SceneObjectType>& GetSceneObjectTypes();
		std::unordered_map<SceneObjectType, std::set<JUUID>>& GetSceneObjects();
		std::set<JUUID>& GetUnboundedSceneObjects();

		//Loading
		void InsertRenderableIntoLoadingPool(JUUID uuid);
		void InsertCameraIntoLoadingPool(JUUID uuid);
		void InsertLightIntoLoadingPool(JUUID uuid);
		std::set<JUUID>& GetRenderablesInLoadingPool();
		std::set<JUUID>& GetCamerasInLoadingPool();
		std::set<JUUID>& GetLightsInLoadingPool();
		size_t GetRenderablesLoadingPoolSize();
		size_t GetCamerasLoadingPoolSize();
		size_t GetLightsLoadingPoolSize();
		void MarkRenderablesInLoadingPoolAsReady();
		void MarkCamerasInLoadingPoolAsReady();
		void MarkLightsInLoadingPoolAsReady();
		void ClearRenderablesLoadingPool();
		void ClearCamerasLoadingPool();
		void ClearLightsLoadingPool();
		void InitLoadingProcessor(CComPtr<ID3D12Device2> d3dDevice, size_t id, size_t capacity);
		void InitFrame2FrameProcessor(CComPtr<ID3D12Device2> d3dDevice, size_t id, size_t capacity);
		void InitComputeProcessor(CComPtr<ID3D12Device2> d3dDevice, size_t id, size_t capacity);
		void SetLoading(bool value);
		bool IsLoading();
		void SetCanSubmitLoading(bool value);
		bool IsReadyToSubmitLoading();
		void Bind(JUUID uuidA, JUUID uuidB);
		void Unbind(JUUID uuid);
		void Unbind(JUUID uuidA, JUUID uuidB);
		bool IsBound(JUUID uuid);
		void AddSceneObjectToUnboundPool(JUUID uuid);
		bool LoadingCommandListIsOpen();
		CComPtr<ID3D12GraphicsCommandList2>& GetLoadingCommandList(bool OpenIfClosed = true);
		void ResetLoadingCommandList();
		void CloseLoadingCommandList();
		void SubmitLoadingCommandList();
		void CloseSubmitLoadingCommandList()
		{
			CloseLoadingCommandList();
			SubmitLoadingCommandList();
		}
		void Loading();

		//Frame2Frame
		CComPtr<ID3D12GraphicsCommandList2>& GetCommandList(bool OpenIfClosed = true);
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
		CComPtr<ID3D12GraphicsCommandList2>& GetComputeCommandList(bool OpenIfClosed = true);
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

	private:
		SceneUnitId id;
		std::string unitName;
		bool markedForDelete;
		//bool attached;
		bool isolated;
		//bool mergeable;
		//unsigned int deletionFrames;

		//Scene
		Binder binder;
		std::set<JUUID> unboundedSceneObjects;
		std::unordered_map<SceneObjectType, std::set<JUUID>> sceneObjects;
		std::unordered_map<JUUID, SceneObjectType> sceneObjectsTypes;

		//loading
		//std::unique_ptr<std::atomic_bool> abortLoading;
		CommandsProcessor loadingProcessor;
		std::unique_ptr<std::atomic_bool> loading;
		std::unique_ptr<std::atomic_bool> canSubmitLoading;
		std::set<JUUID> renderablesInLoadingPool;
		std::set<JUUID> camerasInLoadingPool;
		std::set<JUUID> lightsInLoadingPool;

		//f2f
		CommandsProcessor commandsProcessor;

		//SceneUnitId parentUnit;
		////scene objects
		//std::unique_ptr<std::atomic_bool> sceneUnitLoaded;
		////binding
		////command list
		//std::unique_ptr<std::atomic_bool> loadingSubmit;
		//bool runningCompute;
		CommandsProcessor computeProcessor;
	};
};