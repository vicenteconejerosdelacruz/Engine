#pragma once
#include <UUID.h>
#include <v8.h>
#include <v8-object.h>
#include <libplatform/libplatform.h>
#include "SceneUnitScripting.h"

using namespace v8;

namespace Scene
{
	DEF_SCENEOBJECT_ID_DEP(Renderable);
};
using namespace Scene;
namespace Scripting
{
	void InitScripting(const char* path);
	void ShutdownScripting();
	Isolate* GetIsolate();
	SceneUnitScripting* GetSceneUnitScripting(SceneUnitId id);
	void CreateScriptingSceneTemplate(SceneUnitId id);
	std::string SafePath(std::string path);
	Local<Object> WrapProxy(Isolate* isolate, SceneUnitScripting& script, JObject* owner, std::string path, size_t flag);
	template <typename T>
	Local<Object> WrapJObject(Isolate* isolate, SceneUnitScripting& script, T* instance) {
		if (!instance) return Local<Object>();

		std::string className = instance->GetJClassName();

		//get or create the template passing a lambda that configures the object if needed
		Local<ObjectTemplate> tpl = SceneUnitScripting::GetOrCreateTemplate(isolate, script.id, className,
			[&script](Isolate* isolate, Local<ObjectTemplate> templ, SceneUnitScripting* scriptPtr) {

				//set the internal field count for attaching the jobject
				templ->SetInternalFieldCount(1);
				//configure the attributes through a handler
				templ->SetHandler(v8::NamedPropertyHandlerConfiguration(
					v8_jobject_getter,
					v8_jobject_setter,
					nullptr,
					nullptr,
					v8_jobject_enumerator,
					v8::External::New(isolate, scriptPtr) // scriptPtr as "this"
				));

				//call RegisterScript for registering the class methods
				T::RegisterScript(isolate, templ, scriptPtr);
			}
		);

		Local<Context> context = isolate->GetCurrentContext();
		MaybeLocal<Object> maybeObj = tpl->NewInstance(context);
		Local<Object> obj;

		if (maybeObj.ToLocal(&obj)) {
			//store the pointer of the instance(jobject derived) as field 0 pointer
			obj->SetAlignedPointerInInternalField(0, static_cast<void*>(instance));
			return obj;
		}

		return Local<Object>();
	}
	void RunScript(
		std::string script,
		SUUUID suuuid,
		std::function<void(Local<Context> context, Isolate* isolate, std::unique_ptr<SceneUnitScripting>& scriptData)> contextBinder = [](
			Local<Context> context, Isolate* isolate, std::unique_ptr<SceneUnitScripting>& scriptData
			)
		{}
	);
	void BindSceneObjectControllers(Local<Context> context, Isolate* isolate, std::unique_ptr<SceneUnitScripting>& scriptData, std::set<JUUIDName> controllers);
	void ExecuteSource(Local<Context>& context, std::string script);
}