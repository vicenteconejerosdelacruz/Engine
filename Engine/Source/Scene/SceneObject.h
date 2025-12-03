#pragma once

#include <map>
#include <JObject.h>
#include <nlohmann/json.hpp>
#if defined(_EDITOR)
#include <IconsFontAwesome5.h>
#endif
#include <set>
#include <Scripting.h>

enum SceneObjectType {
	SO_None,
	SO_Renderables,
	SO_Lights,
	SO_Cameras,
	SO_SoundEffects
};

inline const std::unordered_map<SceneObjectType, std::string> SceneObjectTypeToString = {
	{ SO_Renderables, "Renderables" },
	{ SO_Lights,	"Lights" },
	{ SO_Cameras, "Cameras" },
	{ SO_SoundEffects, "SoundEffects" }
};

inline const std::unordered_map<std::string, SceneObjectType> StringToSceneObjectType = {
	{ "Renderables", SO_Renderables },
	{ "Lights", SO_Lights },
	{ "Cameras", SO_Cameras },
	{ "SoundEffects", SO_SoundEffects }
};

#if defined(_EDITOR)
inline const std::unordered_map<SceneObjectType, const char*> SceneObjectsTypePanelMenuItems = {
	{ SO_Renderables, ICON_FA_SNOWMAN "Renderables" },
	{ SO_Lights, ICON_FA_LIGHTBULB "Lights" },
	{ SO_Cameras, ICON_FA_CAMERA "Cameras" },
	{ SO_SoundEffects, ICON_FA_MUSIC "SoundEffects" }
};
#endif

inline const std::unordered_map<SceneObjectType, std::string> SceneObjectTypeJsonContainer =
{
	{ SO_Renderables, "renderables" },
	{ SO_Lights, "lights" },
	{ SO_Cameras, "cameras" },
	{ SO_SoundEffects, "sounds" }
};

inline const std::unordered_map<std::string, std::string> JsonContainerToString =
{
	{ "renderables", "Renderables" },
	{ "lights", "Lights" },
	{ "cameras", "Cameras" },
	{ "sounds", "SoundEffects" }
};

inline const std::unordered_map<std::string, std::string> StringToJsonContainer =
{
	{ "Renderables", "renderables" },
	{ "Lights", "lights" },
	{ "Cameras", "cameras" },
	{ "SoundEffects", "sounds" }
};

template <typename T>
using SceneObjectsContainer = std::unordered_map<JUUID, T>;

namespace Scene
{
	struct Renderable;

	struct SceneObject : JObject
	{
		SceneObject(nlohmann::json& json) :JObject(json) {}
		virtual void Initialize();
		virtual void BindToScene();
		virtual void UnbindFromScene() {};
		virtual XMVECTOR rotationQ() { return XMQuaternionIdentity(); }
		virtual XMMATRIX world() { return XMMatrixIdentity(); }
		virtual BoundingBox GetBoundingBox() { return BoundingBox(); };
		virtual void Bind(JUUID uuid) {}
		virtual void Unbind(JUUID uuid) {}

		virtual SceneObjectType JType() { return SO_None; }
		JUUID Juuid() { return JUUID(at("uuid")); }
		virtual void JUpdate(nlohmann::json p);
		virtual void JPatch(nlohmann::json p);

		std::set<JUUID> controllers;
		virtual void BindControllers();
		virtual void UnbindControllers();

		virtual void BindToV8Context(v8pp::context& context);

#if defined(_EDITOR)
		virtual JUUID CreateBillboard(CameraUUID camera) { return ""; }
		virtual void UpdateBillboard(JUUID billboard) {}
		virtual bool CanInteractWithGizmo(ImGuizmo::OPERATION operation) { return false; }
#endif
	};
};