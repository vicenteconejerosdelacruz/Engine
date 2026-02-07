#pragma once
//#include <Scene.h>
#include <wrl.h>
#include <JObject.h>
#include <Templates.h>
#include <DirectXMath.h>

#define _EDITOR_BOUNDINGBOX
#define _EDITOR_PICKINGPASS
#define _EDITOR_BILLBOARD

enum SceneObjectType;
enum TemplateType;

namespace Scene
{
	struct SceneObject;
	struct SoundFX;
	struct Camera;
	struct Renderable;
	struct Light;
};

namespace DirectX
{
	class Mouse;
	struct XMFLOAT4X4;
};

using namespace Scene;

enum MouseGameAreaMode
{
	MOUSE_GAMEAREA_MODE_NONE,
	MOUSE_GAMEAREA_MODE_PICKING,
	MOUSE_GAMEAREA_MODE_GIZMO,
	MOUSE_GAMEAREA_MODE_CAMERA
};

namespace Editor {

	static const LONG ApplicationBarBottom = 19L;
	static const LONG RightPanelWidth = 400L;
	static const float cameraEditorDistance = -10.0f;

	//Editor LifeCycle
	void CreateSceneUnitGizmos(SceneUnitId id);
	void CreateSceneUnitSelection(SceneUnitId id);
	void CreateSceneUnitGameController(SceneUnitId id);
	void CreateSceneUnitBoundingBox(SceneUnitId id);
	void CreateSceneUnitBillboards(SceneUnitId id);
	void CreateSceneUnitEditorIndependentCamera(SceneUnitId id);
	void DeleteSceneUnitLevel(SceneUnitId id);
	void DeleteSceneUnitGizmos(SceneUnitId id);
	void DeleteSceneUnitSelection(SceneUnitId id);
	void DeleteSceneUnitGameController(SceneUnitId id);
	void DeleteSceneUnitBoundingBox(SceneUnitId id);
	void DeleteSceneUnitBillboards(SceneUnitId id);
	void DeleteSceneUnitEditorIndependentCamera(SceneUnitId id);
	void CopySceneUnitEditorCameraRenderPasses(SceneUnitId id);
	void MarkSceneUnitAsModified(SceneUnitId id);
	void InitEditor();
	void LoadWorkbench();
	void SaveWorkbench(std::string topItem = "");
	void LevelLoadingProgress(std::string asset, unsigned int count, unsigned int total);
	void OnLevelLoaded(SceneUnitId id);
	void CloseScene(SceneUnitId id, std::function<void()> onCloseScene = [] {});
	void QuitEditor();
	void ImGuiImplRenderInit();
	void SetupImGuiStyle();
	void DestroyEditor();
	bool WndProcHandlerEditor(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void EditorStep();
	void HandleEditorMouseMovements(SceneUnitId id);
	std::set<std::string> GetOpenedScenes(bool skipDefault = true);
	std::set<SceneUnitId> GetOpenedSceneUnitIds(bool skipDefault = true);

	//Editor Drawing
	void AddSceneUnitToEditor(SceneUnitId id);
	void SetCurrentSceneUnit(SceneUnitId id);
	void DrawEditor();
	void WriteSceneUnitEditorPlayCameraConstantsBuffer(SceneUnitId unit);
	JUUID GetSceneUnitEditorCamera(SceneUnitId id);
	void SwitchToSceneUnitEditorCamera(SceneUnitId id);
	void SwitchToSceneUnitEditorPlayCamera(SceneUnitId id);
	void RemoveSceneUnitEditorCameraFromWindowCameras(SceneUnitId id);
	void AddSceneUnitEditorCameraToWindowCameras(SceneUnitId id);
	void DrawApplicationBar();
	void DrawLevelSelectorModal();
	void HandleApplicationDragTitleBar(RECT& dragRect);
	RECT GetGameControllerRect();
	void DrawGameController();
	void DrawLevelsTabs();
	void SaveLevelAs();
	bool SaveFileDialog(std::wstring& path, std::wstring defaultDirectory = L"", std::wstring defaultFileName = L"", std::pair<COMDLG_FILTERSPEC*, int>* pFilterInfo = nullptr);
	std::string GetLevelString(SceneUnitId id);
	std::string GetLevelName(SceneUnitId id);
	void ChangeLevelName(SceneUnitId id, std::string levelFileName);
	void SaveLevelToFile(SceneUnitId id, std::string levelFileName);
	void SaveTemplates();
	void DrawRightPanel();
	void PromptTemplateDeletion(std::vector<nlohmann::json> references, std::function<void(std::vector<nlohmann::json>)> OnDelete, std::function<void()> OnCancel);
	void CloseDeletionPrompt();
	void BuildAssetsTree();

	//SceneObjects Panel
	void OnChangeSceneObjectTab(std::string newTab);
	void OpenSceneObject(JUUID uuid);
	void OpenSceneObjectOnNextFrame(JUUID uuid);
	void MarkScenePanelAssetsAsDirty();

	//Templates Panel
	void OnChangeTemplateTab(std::string newTab);
	void OpenTemplate(JUUID uuid);
	void OpenTemplateOnNextFrame(JUUID uuid);
	void MarkTemplatesPanelAssetsAsDirty();
	void RemoveFromTemplateSelection(std::set<JUUID> uuids);

	//JObject's Preview Panel
	void SendEditorPreview(JUUID uuid, auto GetJObject, auto drawers);
	void SendEditorDestroyPreview(JUUID uuid, auto GetJObject);

	//Model3D Animation Sequencer
	void OpenAnimationSequencer(JUUID uuid);

	//Gizmos
	void ResetGizmoVariableWorkers(SceneUnitId unit);
	bool InteractWithGizmos(SceneUnitId unit, std::set<SceneObject*>& objects2Gizmo);
	void DrawPickedObjectsGizmo(SceneUnitId unit, CameraSUUUID camera);
	void BeginGizmoInteraction(CameraSUUUID camera, std::function<void(DirectX::XMFLOAT4X4, DirectX::XMFLOAT4X4)> interaction = [](DirectX::XMFLOAT4X4, DirectX::XMFLOAT4X4) {});

	//SceneObject Selection
	void SelectSceneObject(SceneUnitId unit, JUUID uuid);
	void SelectRenderable(SceneUnitId unit, JUUID ruuid);
	void SelectLight(SceneUnitId unit, JUUID luuid);
	void SelectCamera(SceneUnitId unit, JUUID cuuid);
	void SelectSoundEffect(SceneUnitId unit, JUUID suuid);
	void ToggleSceneObjectFromSelection(SceneUnitId unit, JUUID uuid);
	void SetSceneObjectSelection(SceneUnitId unit, JUUID uuid, bool selected);
	void InsertSceneObjectToSelection(SceneUnitId unit, JUUID uuid);
	void EraseSceneObjectFromSelection(SceneUnitId unit, JUUID uuid);
	void ClearSceneObjectsSelection(SceneUnitId unit);

	//BoundingBox
	void UpdateBoundingBox(SceneUnitId unit);

	//Mouse Processing
	bool MouseIsInGameArea(std::unique_ptr<DirectX::Mouse>& mouse);
	void GameAreaMouseProcessing(std::unique_ptr<DirectX::Mouse>& mouse, CameraSUUUID camera);

	//SceneObject Picking
	bool PickingPassExists(SceneUnitId id);
	void CreatePickingPass(SceneUnitId id);
	void BindPickingRenderables(SceneUnitId id);
	void BindRenderableToPickingPass(RenderableSUUUID r);
	void UnbindRenderableFromPickingPass(RenderableSUUUID r);
	void RenderPickingPass(SceneUnitId id, CameraSUUUID camera);
	void PickFromScene(SceneUnitId id);
	void PickSceneObject(SceneUnitId id, unsigned int pickedObjectId);

	//JObjects Creation
	void StartSceneObjectCreation(SceneObjectType type);
	void StartTemplateCreation(TemplateType type);

	//Billboards
	JUUID CreateBillboardFromMaterials(SceneUnitId id, CameraSUUUID camera, std::string name, std::string material, std::string pickingMaterial);
	void RegisterBillboard(SceneUnitId id, JUUID sceneObject);
	JUUID GetBillboard(SceneUnitId id, JUUID sceneObject);
	void DestroyBillboard(SceneUnitId id, JUUID sceneObject);
	void CreateRegisteredBillboards(SceneUnitId id);
	bool PendingBillboards(SceneUnitId id);
	void ShowBillboards(SceneUnitId id);
	void ShowBillboard(RenderableSUUUID billboard);
	void HideBillboards(SceneUnitId id);
	void HideBillboard(RenderableSUUUID billboard);
	void UpdateBillboards();
	void DestroyPendingBillboards();

	//Game Mode Activation
	bool IsPlaying(SceneUnitId id);
	bool IsPaused(SceneUnitId id);
	void SwitchToPlayMode(SceneUnitId id);
	void SwitchToPauseMode(SceneUnitId id);
	void SwitchToUnPausedMode(SceneUnitId id);
	void SwitchToNonPlayMode(SceneUnitId id);
}
