#pragma once

#include <regex>
#include <Material/SamplerDesc.h>
#include <Material/BlendDesc.h>
#include <ShaderMaterials.h>
#include <ImEditor.h>
#include <Shader/ShaderInstance.h>
#include <NoStd.h>
#include <Renderable/Renderable.h>
#include <Camera/Camera.h>
#include <Light/Light.h>
#include <Light/ShadowMap.h>
#include <Sound/SoundFX.h>
#include <RenderPass/RenderPass.h>

namespace Templates
{
	struct ShaderInstance;
	extern std::vector<JUUIDName> GetMaterialsUUIDsNames();
	extern std::vector<JUUIDName> GetMeshesUUIDsNames();
	extern std::vector<JUUIDName> GetModel3DsUUIDsNames();
	extern std::vector<JUUIDName> GetRenderPasssUUIDsNames();
	extern std::vector<JUUIDName> GetShadersUUIDsNames();
	extern std::vector<JUUIDName> GetSoundsUUIDsNames();
	extern std::vector<JUUIDName> GetTexturesUUIDsNames();
	extern std::string GetMeshName(std::string uuid);
	extern std::string GetModel3DName(std::string uuid);
	extern std::string GetMaterialName(std::string uuid);
	extern std::string GetShaderName(std::string uuid);
	extern std::string GetSoundName(std::string uuid);
	extern std::string GetTextureName(std::string uuid);
};

namespace Scene
{
	extern std::function<std::vector<JUUIDName>()> GetSceneObjectsByType(SceneObjectType typeToGet);
};

namespace Editor
{
	extern void MarkTemplatesPanelAssetsAsDirty();
	extern void MarkScenePanelAssetsAsDirty();
	extern void OpenAnimationSequencer(std::string uuid);
};

namespace Game
{
	extern std::vector<std::string> GetControllers();
};

const int defaultTableFlags = ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX;

using namespace Scene;
using namespace Templates;

#include <JTypes.h>
#include <Functions/JEdvEditorDrawer.h>
#include <Functions/JEdvCreatorDrawer.h>
#include <Functions/JEdvCreatorValidator.h>
