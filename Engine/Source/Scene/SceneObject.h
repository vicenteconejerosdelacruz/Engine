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
		//lifecycle
		SceneObject(SceneUnitId id, nlohmann::json& json) :JObject(json) { unit = id; }
		virtual void Initialize() {};
		virtual void SetInitialConditions() {};
		virtual void BindToScene() {};
		virtual void Bind(JUUID uuid) {}
		virtual void UnbindFromScene() {};
		virtual void Unbind(JUUID uuid) {}

		//transformations
		virtual XMVECTOR rotationQ() { return XMQuaternionIdentity(); }
		virtual XMMATRIX world() { return XMMatrixIdentity(); }

		//boundingbox
		virtual BoundingBox GetBoundingBox() { return BoundingBox(); };

		//datatypes
		virtual SceneObjectType JType() { return SO_None; }
		JUUID Juuid() { return JUUID(at("uuid")); }
		SUUUID SUuuid() { return std::make_tuple(unit, Juuid()); }

		//json patching
		virtual void JUpdate(nlohmann::json p);
		virtual void JPatch(nlohmann::json p);

		//Scripting
		virtual void BindToV8Context(v8pp::context& context);

#if defined(_EDITOR)
		//Billboard
		virtual RenderableID CreateBillboard(CameraID camera) { return RenderableID(); }
		virtual void UpdateBillboard(RenderableID renderable) {}

		//Gizmos
		virtual bool CanInteractWithGizmo(ImGuizmo::OPERATION operation) { return false; }
#endif
		//scene unit for which this scene object belongs
		SceneUnitId unit;
	};
};

#include <Renderable/Renderable.h>
#include <Light/Light.h>
#include <Camera/Camera.h>
#include <Sound/SoundFX.h>
