#pragma once

#include <map>
#include <set>
#include <Controller.h>
#include <nlohmann/json.hpp>
#include <JObject.h>
#include <JTypes.h>
#include <Scripting.h>
#include <SceneObjectDecl.h>
#include <SceneObjectDef.h>
#if defined(_EDITOR)
#include <IconsFontAwesome5.h>
#endif

inline const std::unordered_map<SceneObjectType, std::string> SceneObjectTypeToString = {
	{ SO_Renderables, "Renderables" },
	{ SO_Lights,	"Lights" },
	{ SO_Cameras, "Cameras" },
	{ SO_SoundEffects, "SoundEffects" },
	{ SO_PhysicScenes, "PhysicScenes" },
	{ SO_Triggers, "Triggers" },
	{ SO_Boundaries, "Boundaries" },
};

inline const std::unordered_map<std::string, SceneObjectType> StringToSceneObjectType = {
	{ "Renderables", SO_Renderables },
	{ "Lights", SO_Lights },
	{ "Cameras", SO_Cameras },
	{ "SoundEffects", SO_SoundEffects },
	{ "PhysicScenes", SO_PhysicScenes },
	{ "Triggers", SO_Triggers },
	{ "Boundaries", SO_Boundaries },
};

#if defined(_EDITOR)
inline const std::unordered_map<SceneObjectType, const char*> SceneObjectsTypePanelMenuItems = {
	{ SO_Renderables, ICON_FA_SNOWMAN "Renderables" },
	{ SO_Lights, ICON_FA_LIGHTBULB "Lights" },
	{ SO_Cameras, ICON_FA_CAMERA "Cameras" },
	{ SO_SoundEffects, ICON_FA_MUSIC "SoundEffects" },
	{ SO_PhysicScenes, ICON_FA_IGLOO "PhysicScenes" },
	{ SO_Triggers, ICON_FA_IGLOO "Triggers" },
	{ SO_Boundaries, ICON_FA_IGLOO "Boundaries" },
};
#endif

inline const std::unordered_map<SceneObjectType, std::string> SceneObjectTypeJsonContainer =
{
	{ SO_Renderables, "renderables" },
	{ SO_Lights, "lights" },
	{ SO_Cameras, "cameras" },
	{ SO_SoundEffects, "sounds" },
	{ SO_PhysicScenes, "physicScenes" },
	{ SO_Triggers, "triggers" },
	{ SO_Boundaries, "boundaries" },
};

inline const std::unordered_map<std::string, std::string> JsonContainerToString =
{
	{ "renderables", "Renderables" },
	{ "lights", "Lights" },
	{ "cameras", "Cameras" },
	{ "sounds", "SoundEffects" },
	{ "physicScenes", "PhysicScenes" },
	{ "triggers", "Triggers" },
	{ "boundaries", "Boundaries" },
};

inline const std::unordered_map<std::string, std::string> StringToJsonContainer =
{
	{ "Renderables", "renderables" },
	{ "Lights", "lights" },
	{ "Cameras", "cameras" },
	{ "SoundEffects", "sounds" },
	{ "PhysicScenes", "physicScenes" },
	{ "Triggers", "triggers" },
	{ "Boundaries", "boundaries" },
};

template <typename T>
using SceneObjectsContainer = std::map<JUUID, T>;

namespace Scene
{
	struct Renderable;

	struct SceneObject : JObject
	{
		//lifecycle
		SceneObject(SceneUnitId id, nlohmann::json& json) :JObject(json) { unit = id; soName = json.at("name"); }
		virtual void Initialize() {};
		virtual void SetInitialConditions() {};
		virtual void BindToScene() {};
		virtual void Bind(JUUID uuid) {}
		virtual void UnbindFromScene() {};
		virtual void Unbind(JUUID uuid) {}
		virtual void Destroy();

		//transformations
		virtual XMVECTOR rotationQ() { return XMQuaternionIdentity(); }
		virtual XMMATRIX world() { return XMMatrixIdentity(); }

		//boundingbox
		virtual BoundingBox GetBoundingBox() { return BoundingBox(); };

		//datatypes
		virtual SceneObjectType JType() { return SO_None; }
		JUUID Juuid() { return JUUID(at("uuid")); }
		SUUUID SUuuid() { return std::make_tuple(unit, Juuid()); }
		std::string SUuuid_str() { return std::to_string(unit) + "/" + Juuid(); }

		//json patching
		virtual void JUpdate(nlohmann::json p);
		virtual void JPatch(nlohmann::json p);

#if defined(_EDITOR)
		//Billboard
		virtual RenderableID CreateBillboard(CameraID camera) { return RenderableID(); }
		virtual void UpdateBillboard(RenderableID renderable) {}

		//Gizmos
		virtual bool CanInteractWithGizmo(ImGuizmo::OPERATION operation) { return false; }
		virtual std::map<std::string, ScriptBinding> GetScriptBindingOptions();
#endif
		//scene unit for which this scene object belongs
		SceneUnitId unit;
		std::string soName;
		std::vector<std::function<void()>> destroyCallbacks;
	};
};

#include <Renderable/Renderable.h>
#include <Light/Light.h>
#include <Camera/Camera.h>
#include <Sound/SoundFX.h>
#include <PhysicScene.h>
#include <Trigger.h>
#include <Boundary.h>