#pragma once
//#include <Scene.h>
#include <wrl.h>
#include <JObject.h>
#include <Templates.h>
#include <SimpleMath.h>
#include <SceneUnitId.h>

#define _EDITOR_BOUNDINGBOX
#define _EDITOR_PICKINGPASS
#define _EDITOR_BILLBOARD

#if defined(_EDITOR_PICKINGPASS)
//#define _EDITOR_PICKINGPASS_EVERY_FRAME
#endif

enum SceneObjectType;
enum TemplateType;

namespace Scene
{
	struct SceneObject;
	struct SoundFX;
	struct Camera;
	struct Renderable;
	struct Light;
	struct Trigger;
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
	void CreateSceneUnitPhysicsController(SceneUnitId id);
	void CreateSceneUnitBoundingBox(SceneUnitId id);
	void CreateSceneUnitBillboards(SceneUnitId id);
	void CreateSceneUnitEditorIndependentCamera(SceneUnitId id);
	CameraID GetLevelCamera(SceneUnitId id);
	void DeleteSceneUnitLevel(SceneUnitId id);
	void DeleteSceneUnitGizmos(SceneUnitId id);
	void DeleteSceneUnitSelection(SceneUnitId id);
	void DeleteSceneUnitGameController(SceneUnitId id);
	void DeleteSceneUnitPhysicsController(SceneUnitId id);
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
	void ResizeEditorResources(unsigned int width, unsigned int height);

	//Editor Drawing
	void AddSceneUnitToEditor(SceneUnitId id);
	void SetCurrentSceneUnit(SceneUnitId id);
	void DrawEditor();
	void WriteSceneUnitEditorPlayCameraConstantsBuffer(SceneUnitId id);
	void WriteSceneUnitDirectionalShadowMapAttributes(SceneUnitId id);
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
	RECT GetPhysicsControllerRect();
	void DrawPhysicsController();
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
	void OpenPopupForSceneObject(SceneUnitId id, JUUID uuid, std::set<std::string> selected_uuids);
	void OpenPopupForTemplate(JUUID uuid);

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
	void DrawPickedObjectsGizmo(SceneUnitId unit, CameraID camera);
	void BeginGizmoInteraction(CameraID camera, std::function<void(DirectX::XMFLOAT4X4, DirectX::XMFLOAT4X4)> interaction = [](DirectX::XMFLOAT4X4, DirectX::XMFLOAT4X4) {});

	//SceneObject Selection
	void SelectSceneObject(SceneUnitId unit, JUUID uuid);
	void SelectRenderable(RenderableID renderable);
	void SelectLight(LightID light);
	void SelectCamera(CameraID camera);
	void SelectSoundEffect(SoundFXID soundfx);
	void SelectTrigger(TriggerID trigger);
	void SelectBoundary(BoundaryID boundary);
	void ToggleSceneObjectFromSelection(SceneUnitId unit, JUUID uuid);
	void SetSceneObjectSelection(SceneUnitId unit, JUUID uuid, bool selected);
	void InsertSceneObjectToSelection(SceneUnitId unit, JUUID uuid);
	void EraseSceneObjectFromSelection(SceneUnitId unit, JUUID uuid);
	void ClearSceneObjectsSelection(SceneUnitId unit);

	//BoundingBox
	void UpdateBoundingBox(SceneUnitId unit);

	//Mouse Processing
	bool MouseIsInGameArea(std::unique_ptr<DirectX::Mouse>& mouse);
	void GameAreaMouseProcessing(std::unique_ptr<DirectX::Mouse>& mouse, CameraID camera);

	//SceneObject Picking
	bool PickingPassExists(SceneUnitId id);
	void CreatePickingPass(SceneUnitId id);
	void BindPickingRenderables(SceneUnitId id);
	void BindRenderableToPickingPass(RenderableID r);
	void UnbindRenderableFromPickingPass(RenderableID r);
	void RenderPickingPass(SceneUnitId id, CameraID camera);
	void PickFromScene(SceneUnitId id);
	void PickSceneObject(SceneUnitId id, unsigned int pickedObjectId);

	//JObjects Creation
	void StartSceneObjectCreation(SceneObjectType type);
	void StartTemplateCreation(TemplateType type);

	//Billboards
	RenderableID CreateBillboardFromMaterials(SceneUnitId id, CameraID camera, std::string name, std::string material, std::string pickingMaterial);
	void RegisterBillboard(SceneUnitId id, JUUID sceneObject);
	std::set<RenderableID> GetBillboards(SceneUnitId id);
	RenderableID GetBillboard(SceneUnitId id, JUUID sceneObject);
	void DestroyBillboard(SceneUnitId id, JUUID sceneObject);
	void CreateRegisteredBillboards(SceneUnitId id);
	bool PendingBillboards(SceneUnitId id);
	void ShowBillboards(SceneUnitId id);
	void ShowBillboard(RenderableID billboard);
	void HideBillboards(SceneUnitId id);
	void HideBillboard(RenderableID billboard);
	void UpdateBillboards();
	void DestroyPendingBillboards();

	//Game Mode Activation
	bool IsPlaying(SceneUnitId id);
	bool IsPaused(SceneUnitId id);
	void SwitchToPlayMode(SceneUnitId id);
	void SwitchToPauseMode(SceneUnitId id);
	void SwitchToUnPausedMode(SceneUnitId id);
	void SwitchToNonPlayMode(SceneUnitId id);

	//Physics Objects Drawing
	//Register
	bool StaticBodiesSceneUnitRegistered(SceneUnitId id);
	bool DynamicBodiesSceneUnitRegistered(SceneUnitId id);
	bool CharactersSceneUnitRegistered(SceneUnitId id);
	bool TriggersSceneUnitRegistered(SceneUnitId id);

	//Should Draw
	bool StaticBodiesShouldDraw(SceneUnitId id);
	bool DynamicBodiesShouldDraw(SceneUnitId id);
	bool CharactersShouldDraw(SceneUnitId id);
	bool TriggersShouldDraw(SceneUnitId id);

	//Switch drawing state
	void SwitchStaticBodiesDrawing(SceneUnitId id);
	void SwitchDynamicBodiesDrawing(SceneUnitId id);
	void SwitchCharactersDrawing(SceneUnitId id);
	void SwitchTriggersDrawing(SceneUnitId id);

	//Physics Objects registration
	void RegisterStaticBody(PhysicObjectID phO);
	void RegisterDynamicBody(PhysicObjectID phO);
	void RegisterCharacter(PhysicObjectID phO);
	void RegisterTrigger(PhysicObjectID phO);

	//Physics Objects unregistration
	void UnRegisterStaticBody(PhysicObjectID phO);
	void UnRegisterDynamicBody(PhysicObjectID phO);
	void UnRegisterCharacter(PhysicObjectID phO);
	void UnRegisterTrigger(PhysicObjectID phO);

	//Physics Objects list
	std::set<PhysicObjectID> GetStaticBodies(SceneUnitId id);
	std::set<PhysicObjectID> GetDynamicBodies(SceneUnitId id);
	std::set<PhysicObjectID> GetCharacters(SceneUnitId id);
	std::set<PhysicObjectID> GetTriggers(SceneUnitId id);

	//Script Editor
	void StartScriptEdition(JObject* object, std::string attribute);
	void StartScriptEdition(std::string att, std::string script, std::function<void(std::string)> writer);
	void OpenScriptBindingSelector(JObject* object, std::string attribute, int index, ScriptBinding sb);
}
