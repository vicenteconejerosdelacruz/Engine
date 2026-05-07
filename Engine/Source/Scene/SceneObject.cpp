#include "pch.h"
#include "SceneObject.h"

#if defined(_EDITOR)
namespace Editor
{
	void MarkSceneUnitAsModified(SceneUnitId id);
};
#endif

namespace Scene
{
	void SceneObject::Initialize()
	{
	}

	void SceneObject::JUpdate(nlohmann::json p)
	{
#if defined(_EDITOR)
		Editor::MarkSceneUnitAsModified(unit);
#endif
		JObject::JUpdate(p);
	}

	void SceneObject::JPatch(nlohmann::json p)
	{
#if defined(_EDITOR)
		Editor::MarkSceneUnitAsModified(unit);
#endif
		JObject::JPatch(p);
	}

	void SceneObject::Destroy()
	{
		for (auto& cb : destroyCallbacks)
		{
			cb();
		}
	}

#if defined(_EDITOR)
	std::map<std::string, ScriptBinding> SceneObject::GetScriptBindingOptions()
	{
		return {
			{ at("name"), ScriptBinding(std::string(at("uuid"))) }
		};
	}
#endif
}