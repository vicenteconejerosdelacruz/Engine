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
	extern bool IsPlaying(SceneUnitId id);
	extern bool IsPaused(SceneUnitId id);
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

	Local<Context> CreateSceneObjectScriptContext(Isolate* isolate, SceneObject* so, v8_att_context& att_context)
	{
		Local<ObjectTemplate> global = ObjectTemplate::New(isolate);

		v8_att_functions& att_functions = att_context.att_functions;

		std::string suuuid_str = so->SUuuid_str();
		AddFunctionToTemplate(isolate, global, att_functions, suuuid_str, "toJSON", v8_toJSON(so));

		v8_templates_creators templateAttributeCreator = GetSceneObjectV8TemplatesCreators(so->SUuuid());
		v8_context_creators contextAttributeCreator = GetSceneObjectV8ContextCreators(so->SUuuid());

		AddTemplateJsonAttributes(isolate, global, att_context, templateAttributeCreator, *so, suuuid_str);
		Local<Context> context = Context::New(isolate, nullptr, global);
		v8::Context::Scope context_scope(context);
		AddConsoleToContext(isolate, context);
		AddContextJsonAttributes(isolate, context, att_context, contextAttributeCreator, *so, suuuid_str);

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
		Local<v8::String> source = v8::String::NewFromUtf8(isolate, script.c_str()).ToLocalChecked();
		Local<Script> runnable = Script::Compile(context, source).ToLocalChecked();
		//run the code and capture it's result
		v8::TryCatch try_catch(isolate);

		Local<Value> result;
		if (!runnable->Run(context).ToLocal(&result))
		{
#if defined(_DEVELOPMENT)
			v8::String::Utf8Value exception(isolate, try_catch.Exception());
			v8::Local<v8::Message> message = try_catch.Message();
			if (!message.IsEmpty()) {
				int line_number = message->GetLineNumber(context).FromMaybe(-1);
				v8::String::Utf8Value filename(isolate, message->GetScriptResourceName());

				std::string linenumstr = "Error en " + std::string(*filename) + ", linea " + std::to_string(line_number) + "\n";
				OutputDebugStringA(linenumstr.c_str());

				// Imprimir el Stack Trace completo si está disponible
				v8::Local<v8::Value> stack_trace;
				if (try_catch.StackTrace(context).ToLocal(&stack_trace)) {
					v8::String::Utf8Value stack_str(isolate, stack_trace);
					std::string stacktrace = "Stack Trace:\n" + std::string(*stack_str) + "\n";
					OutputDebugStringA(stacktrace.c_str());
				}
			}
#endif
		}
		else
		{
#if defined(_DEVELOPMENT)
			//print result if not undefined
			v8::String::Utf8Value utf8(isolate, result);
			if (std::string(*utf8) != "undefined")
			{
				std::string resultStr = std::string("result:") + *utf8 + "\n";
				OutputDebugStringA(resultStr.c_str());
			}
#endif
		}
	}
}