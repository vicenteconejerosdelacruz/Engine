#include "pch.h"
#include "Scripting.h"
#include <SceneObject.h>
#include <NoV8.h>

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
	extern bool IsPlaying(SceneUnitId unit);
	extern bool IsPaused(SceneUnitId unit);
}
using namespace Editor;
#endif

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

	void BindModule(std::function<void(Isolate*)> binder)
	{
		binder(isolate);
	}

	void AddTemplateJsonAttributes(Isolate* isolate, Local<ObjectTemplate>& tmpl, v8_att_context& att_context, SceneObject& json, std::string path)
	{
		v8_templates_creators attributeCreator = GetSceneObjectV8TemplatesCreators(json.SUuuid());

		for (auto& elem : json.items())
		{
			if (!attributeCreator.contains(elem.key())) continue;
			attributeCreator.at(elem.key())(isolate, tmpl, att_context, json, path, elem.key());
		}
	}

	void AddContextJsonAttributes(Isolate* isolate, Local<Context> context, v8_att_context& att_context, SceneObject& json, std::string path)
	{
		v8_context_creators attributeCreator = GetSceneObjectV8ContextCreators(json.SUuuid());

		for (auto& elem : json.items())
		{
			if (!attributeCreator.contains(elem.key())) continue;
			attributeCreator.at(elem.key())(isolate, att_context, json, path, elem.key());
		}
	}

	Local<Context> CreateSceneObjectScriptContext(Isolate* isolate, SceneObject* so, v8_att_context& att_context)
	{
		Local<ObjectTemplate> global = ObjectTemplate::New(isolate);

		v8_att_functions& att_functions = att_context.att_functions;

		std::string suuuid_str = so->SUuuid_str();
		AddFunctionToTemplate(isolate, global, att_functions, suuuid_str, "toJSON", v8_toJSON(so));

		AddTemplateJsonAttributes(isolate, global, att_context, *so, suuuid_str);

		Local<Context> context = Context::New(isolate, nullptr, global);
		v8::Context::Scope context_scope(context);
		AddConsoleToContext(isolate, context);
		AddContextJsonAttributes(isolate, context, att_context, *so, suuuid_str);

		return context;
	}

	void RunScript(std::string script, SUUUID suuuid)
	{
#if defined(_EDITOR)
		if (!IsPlaying(std::get<0>(suuuid)) || IsPaused(std::get<0>(suuuid)))
		{
			return;
		}
#endif
		if (script.empty()) return;

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);

		v8_att_context att_context;
		Local<Context> context = CreateSceneObjectScriptContext(isolate, GetSceneObjectPointer(suuuid), att_context);

		Context::Scope context_scope(context);

		//create the source code as a local string
		Local<String> source = String::NewFromUtf8(isolate, script.c_str()).ToLocalChecked();
		Local<Script> runnable = Script::Compile(context, source).ToLocalChecked();
		//run the code and capture it's result
		Local<Value> result = runnable->Run(context).ToLocalChecked();

#if defined(_DEVELOPMENT)
		//print result if not undefined
		String::Utf8Value utf8(isolate, result);
		if (*utf8 != "undefined")
		{
			std::string resultStr = std::string("result:") + *utf8 + "\n";
			OutputDebugStringA(resultStr.c_str());
		}
#endif
	}
}