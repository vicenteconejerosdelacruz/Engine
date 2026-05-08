#include "pch.h"
#include "Scripting.h"
#include <SceneObject.h>
#include <NoV8.h>
#include "SceneUnitScripting.h"

using namespace v8;
using namespace nov8;

namespace Scene
{
	extern SceneObject* GetSceneObjectPointer(SUUUID suuid);
};
using namespace Scene;

#if defined(_EDITOR)
namespace Editor
{
	extern bool IsPlaying(SceneUnitId id);
	extern bool IsPaused(SceneUnitId id);
}
using namespace Editor;
#endif


std::map<SceneUnitId, std::unique_ptr<SceneUnitScripting>> scenesScripts;
namespace Scripting
{
	static std::unique_ptr<Platform> platform;
	static Isolate* isolate = nullptr;

	void InitScripting(const char* path)
	{
		// Initialize V8.
		V8::InitializeICUDefaultLocation(path);
		V8::InitializeExternalStartupData(path);
		platform = platform::NewDefaultPlatform();
		V8::InitializePlatform(platform.get());
		V8::Initialize();

		Isolate::CreateParams create_params;
		create_params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();

		isolate = Isolate::New(create_params);
	}

	void ShutdownScripting()
	{
		V8::Dispose();
		V8::DisposePlatform();
	}

	Isolate* GetIsolate()
	{
		return isolate;
	}

	SceneUnitScripting* GetSceneUnitScripting(SceneUnitId id)
	{
		return scenesScripts.contains(id) ? scenesScripts.at(id).get() : nullptr;
	}

	void CreateScriptingSceneTemplate(SceneUnitId id)
	{
		scenesScripts[id] = std::make_unique<SceneUnitScripting>(id);
		scenesScripts[id]->Create(Scripting::GetIsolate());
	}

	std::string SafePath(std::string path)
	{
		if (!path.empty() && path[0] != '/')
		{
			path = "/" + path;
		}
		return path;
	}

	Local<Object> Scripting::WrapProxy(Isolate* isolate, SceneUnitScripting& script, JObject* owner, std::string path, size_t flag)
	{
		//EscapableHandleScope to allow the object create in this function to be alive outside this function
		EscapableHandleScope scope(isolate);
		Local<Context> context = isolate->GetCurrentContext();

		path = SafePath(path);

		//create the trace data using the proxies. this should be moved to SceneUnitScripting
		V8PropertyProxy* proxyData = new V8PropertyProxy();
		proxyData->owner = owner;
		proxyData->jsonPath = path;
		proxyData->dirtyFlag = flag;
		proxyData->script = &script;

		Local<ObjectTemplate> tmpl = script.proxyTemplate.Get(isolate);
		Local<Object> proxyInst = tmpl->NewInstance(context).ToLocalChecked();

		proxyInst->SetAlignedPointerInInternalField(0, proxyData);

		//Memory management (Weak Callback)
		//create a persistent handle so v8 can trace the lifetime of the specific object
		//use UniquePersistent to automatically clean proxyInst if the scope get's closed
		UniquePersistent<Object> persistent(isolate, proxyInst);

		persistent.SetWeak(proxyData, [](const WeakCallbackInfo<V8PropertyProxy>& data) {
			//this get's triggered by the V8 garbage collector
			V8PropertyProxy* p = data.GetParameter();
			delete p; // free the memory
			}, WeakCallbackType::kParameter);

		//Ensure proxyInst exists after this function
		return scope.Escape(proxyInst);
	}

	void RunScript(std::string script, SUUUID suuuid,
		std::function<void(Local<Context> context, Isolate* isolate, std::unique_ptr<SceneUnitScripting>& scriptData)> contextBinder
	)
	{
		if (script.empty()) return;

		SceneUnitId id = SUUUIDUNIT(suuuid);

		assert(scenesScripts.contains(id));

#if defined(_EDITOR)
		if (!IsPlaying(id) || IsPaused(id))
		{
			return;
		}
#endif

		//lock the isolate in this thread
		Locker locker(isolate);
		//mark the isolate for this stack as active
		Isolate::Scope isolate_scope(isolate);
		//make a HandleScope to manage garbage collection
		HandleScope handle_scope(isolate);

		//create the global template
		Local<ObjectTemplate> globalTmpl = ObjectTemplate::New(isolate);

		//create the context
		Local<Context> context = v8::Context::New(isolate, nullptr, globalTmpl);
		Context::Scope context_scope(context);
		Local<Object> global = context->Global();

		//Bind the console
		AddConsoleToContext(isolate, context);

		//get the SceneUnit
		auto& scene = GetSceneUnit(id);

		//get the script data
		auto& scriptData = scenesScripts.at(id);

		//instantiate the scene template and set the id of the scene unit as it's internal field 0
		Local<Object> sceneInst = scriptData->sceneTemplate.Get(isolate)->NewInstance(context).ToLocalChecked();
		sceneInst->SetAlignedPointerInInternalField(0, scene.get());

		for (auto const& [type, name] : SceneObjectTypeJsonContainer)
		{
			//create instances for the containers
			Local<ObjectTemplate> cTmpl = scriptData->containersTemplates[type].Get(isolate);
			Local<Object> cInst = cTmpl->NewInstance(context).ToLocalChecked();

			//set the sceneUnit pointer and the type of the container as internal fields
			cInst->SetAlignedPointerInInternalField(0, scene.get());
			cInst->SetInternalField(1, Integer::New(isolate, static_cast<int>(type)));

			//set the container instance as an attribute of the scene instance
			sceneInst->Set(context, v8_name(isolate, name), cInst).Check();
		}

		//attach the scene to the global
		global->Set(context, v8_name(isolate, "scene"), sceneInst).Check();

		//Bind the Controllers of the SceneObject
		std::set<JUUIDName> controllers = GetControllersUUIDNamesBySceneObjectUUID(suuuid);
		BindSceneObjectControllers(context, isolate, scriptData, controllers);

		//call the binder function for other bindings
		contextBinder(context, isolate, scriptData);

		//now we can execute the script
		ExecuteSource(context, script);
	}

	void BindSceneObjectControllers(Local<Context> context, Isolate* isolate, std::unique_ptr<SceneUnitScripting>& scriptData, std::set<JUUIDName> controllers)
	{
		for (auto [uuid, name] : controllers)
		{
			Controller* controller = GetController<Controller>(uuid);
			context->Global()->Set(
				context,
				v8_string(isolate, name),
				Scripting::WrapJObject<Controller>(isolate, *scriptData.get(), controller)
			);
			controller->BindNestedControllers(context, isolate, scriptData);
		}
	}

	void ExecuteSource(Local<Context>& context, std::string script)
	{
		Local<String> source = v8_string(isolate, script.c_str());
		Local<Script> runnable;

		TryCatch try_catch(isolate);

		if (!Script::Compile(context, source).ToLocal(&runnable))
		{
			v8_report_exception(isolate, &try_catch);
			return;
		}

		Local<Value> result;
		if (!runnable->Run(context).ToLocal(&result))
		{
			v8_report_exception(isolate, &try_catch);
			return;
		}
	}
}