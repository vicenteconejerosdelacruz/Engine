#pragma once

#include <JObject.h>
#include <nlohmann/json.hpp>
#include <TemplateDecl.h>
#include <TemplateDef.h>
#include <JTypes.h>
#if defined(_EDITOR)
#include <IconsFontAwesome5.h>
#endif

#if defined(_EDITOR)
namespace Editor
{
	extern bool templatesModified;
};
#endif

enum TemplateType {
	T_None,
	T_Shaders,
	T_Materials,
	T_Models3D,
	T_Sounds,
	T_Textures,
	T_RenderPasses,
	T_PhysicGeometries
};

inline const std::unordered_map<TemplateType, std::string> TemplateTypeToString = {
	{ T_Shaders, "Shaders" },
	{ T_Materials, "Materials" },
	{ T_Models3D, "Models3D" },
	{ T_Sounds, "Sounds" },
	{ T_Textures, "Textures" },
	{ T_RenderPasses, "RenderPasses"},
	{ T_PhysicGeometries, "PhysicGeometries" }
};

inline const std::unordered_map<std::string, TemplateType> StringToTemplateType = {
	{ "Shaders", T_Shaders },
	{ "Materials", T_Materials },
	{ "Models3D", T_Models3D },
	{ "Sounds", T_Sounds },
	{ "Textures", T_Textures },
	{ "RenderPasses", T_RenderPasses },
	{ "PhysicGeometries", T_PhysicGeometries }
};

#if defined(_EDITOR)
inline const std::unordered_map<TemplateType, const char* > TemplateTypePanelMenuItems = {
	{ T_Shaders, ICON_FA_FILE "Shaders" },
	{ T_Materials, ICON_FA_TSHIRT "Materials" },
	{ T_Models3D, ICON_FA_CUBE "Models3D" },
	{ T_Sounds, ICON_FA_MUSIC "Sounds" },
	{ T_Textures, ICON_FA_IMAGE "Textures" },
	{ T_RenderPasses, ICON_FA_TV "RenderPasses"},
	{ T_PhysicGeometries, ICON_FA_HOUSE_USER "PhysicGeometries" }
};
#endif

template <typename T>
using TemplatesContainer = std::unordered_map<JUUID, T>;

namespace Templates
{
	struct JTemplate : JObject
	{
		JTemplate(nlohmann::json& json) :JObject(json) {}
		virtual TemplateType JType() { return T_None; }
		virtual void JUpdate(nlohmann::json p)
		{
#if defined(_EDITOR)
			Editor::templatesModified = true;
#endif
			JObject::JUpdate(p);
		}
		virtual void JPatch(nlohmann::json p)
		{
#if defined(_EDITOR)
			Editor::templatesModified = true;
#endif
			JObject::JPatch(p);
		}
	};
};

#include <Shader/Shader.h>
#include <Material/Material.h>
#include <Model3D/Model3D.h>
#include <RenderPass/RenderPass.h>
#include <Sound/Sound.h>
#include <Textures/Texture.h>
#include <Mesh/Mesh.h>
#include <PhysicGeometry.h>
