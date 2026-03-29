#pragma once

#include <regex>
#include <Material/SamplerDesc.h>
#include <Material/BlendDesc.h>
#include <ShaderMaterials.h>
#include <ImEditor.h>
#include <Shader/Shader.h>
#include <NoStd.h>
#include <SceneObject.h>
#include <JTemplate.h>
#include <DeviceUtils/RenderPass/RenderToTexturePass.h>
#include <Primitives.h>
#include <ScriptBinding.h>

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
	extern std::vector<JUUIDName> GetHtmlUIsUUIDsNames();
	extern std::vector<JUUIDName> GetPhysicGeometrysUUIDsNames();
	extern JNAME GetMeshName(JUUID uuid);
	extern JNAME GetModel3DName(JUUID uuid);
	extern JNAME GetMaterialName(JUUID uuid);
	extern JNAME GetShaderName(JUUID uuid);
	extern JNAME GetSoundName(JUUID uuid);
	extern JNAME GetTextureName(JUUID uuid);
};

namespace Scene
{
	extern std::function<std::vector<JUUIDName>()> GetSceneObjectsByType(SceneUnitId id, SceneObjectType typeToGet);
	extern std::vector<JUUIDName> GetSUSceneObjectsByType(SceneUnitId id, SceneObjectType typeToGet);
	extern SceneObject* GetSceneObjectPointer(SceneUnitId id, JUUID uuid);
};

namespace Editor
{
	extern void MarkTemplatesPanelAssetsAsDirty();
	extern void MarkScenePanelAssetsAsDirty();
	extern void MarkSceneUnitAsModified(SceneUnitId id);
	extern void OpenAnimationSequencer(JUUID uuid);
	extern void WriteSceneUnitDirectionalShadowMapAttributes(SceneUnitId id);
	extern void StartScriptEdition(JObject* object, std::string attribute);
	extern void OpenScriptBindingSelector(JObject* object, std::string attribute, int index, ScriptBinding sb);
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
