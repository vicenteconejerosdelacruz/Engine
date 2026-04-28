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
		CreateSceneObjectScriptTemplate();
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
		using namespace v8;
		Isolate* isolate = Scripting::GetIsolate();
		Locker locker(isolate);
		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);

		for (auto& [_, tpl] : att_context.att_templates)
		{
			tpl.Reset();
		}
		att_context.att_accessors.clear();
		att_context.att_functions.clear();
		att_context.att_idx_handlers.clear();

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

	void SceneObject::CreateSceneObjectScriptTemplate()
	{
		using namespace Scripting;

		Isolate* isolate = GetIsolate();
		Locker locker(isolate);
		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<ObjectTemplate> localTemplate = ObjectTemplate::New(isolate);
		objectTemplate = Global<ObjectTemplate>(isolate, localTemplate);
		Scripting::CreateSceneObjectScriptTemplate(isolate, objectTemplate, this, att_context);
	}
}