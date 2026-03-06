#pragma once

#include <unordered_map>
#include <set>
#include <DeviceUtils/CommandsProcessor/CommandsProcessor.h>
#include <Binder.h>

using namespace DeviceUtils;

enum SceneObjectType;

namespace Scene
{
	DEF_SCENEOBJECT_ID_DEP(Light);

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

		//Loading
		void InsertRenderableIntoLoadingPool(RenderableID uuid);
		void InsertCameraIntoLoadingPool(CameraID uuid);
		void InsertLightIntoLoadingPool(LightID uuid);
		std::set<RenderableID>& GetRenderablesInLoadingPool();
		std::set<CameraID>& GetCamerasInLoadingPool();
		std::set<LightID>& GetLightsInLoadingPool();
		size_t GetRenderablesLoadingPoolSize();
		size_t GetCamerasLoadingPoolSize();
		size_t GetLightsLoadingPoolSize();
		void MarkRenderablesInLoadingPoolAsReady();
		void MarkCamerasInLoadingPoolAsReady();
		void MarkLightsInLoadingPoolAsReady();
		void ClearRenderablesLoadingPool();
		void ClearCamerasLoadingPool();
		void ClearLightsLoadingPool();
		void EraseRenderableFromLoadingPool(RenderableID r);
		void EraseCameraFromLoadingPool(CameraID c);
		void EraseLightFromLoadingPool(LightID l);
		void InitLoadingProcessor(CComPtr<ID3D12Device2> d3dDevice, size_t id, size_t capacity);
		void InitFrame2FrameProcessor(CComPtr<ID3D12Device2> d3dDevice, size_t id, size_t capacity);
		void InitComputeProcessor(CComPtr<ID3D12Device2> d3dDevice, size_t id, size_t capacity);
		void SetLoading(bool value);
		bool IsLoading();
		void SetCanSubmitLoading(bool value);
		bool IsReadyToSubmitLoading();
		void SetLoadingComplete(bool value);
		bool IsLoadingComplete();
		void Bind(JUUID uuidA, JUUID uuidB);
		void Unbind(JUUID uuid);
		void Unbind(JUUID uuidA, JUUID uuidB);
		bool IsBound(JUUID uuid);
		void AddSceneObjectToUnboundPool(JUUID uuid);
		void RemoveSceneObjectFromUnboundPool(JUUID uuid);
		bool LoadingCommandListIsOpen();
		CComPtr<ID3D12GraphicsCommandList2>& GetLoadingCommandList(bool OpenIfClosed = true);
		void ResetLoadingCommandList();
		void CloseLoadingCommandList();
		void SubmitLoadingCommandList();
		void PushLoadingExecutionCallback(std::function<void()> cb);
		void CloseSubmitLoadingCommandList()
		{
			CloseLoadingCommandList();
			SubmitLoadingCommandList();
		}
		void Loading();
		void SubmitForLoading(std::function<void()> loader);

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
		unsigned int deleteFrames = Renderer::numFrames;
		std::function<void()> deleteCallback;
		bool isolated;

		//Scene
		Binder binder;
		std::set<JUUID> unboundedSceneObjects;
		std::unordered_map<SceneObjectType, std::set<JUUID>> sceneObjects;
		std::unordered_map<JUUID, SceneObjectType> sceneObjectsTypes;

		//loading
		CommandsProcessor loadingProcessor;
		std::unique_ptr<std::atomic_bool> loading;
		std::unique_ptr<std::atomic_bool> canSubmitLoading;
		std::unique_ptr<std::atomic_bool> loadingComplete;
		std::set<RenderableID> renderablesInLoadingPool;
		std::set<CameraID> camerasInLoadingPool;
		std::set<LightID> lightsInLoadingPool;
		std::vector<std::function<void()>> postLoadingExecutionCallbacks;

		//f2f
		CommandsProcessor commandsProcessor;
		CommandsProcessor computeProcessor;
	};
};