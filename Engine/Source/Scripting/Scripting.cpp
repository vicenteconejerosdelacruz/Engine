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

	struct v8_att_context
	{
		v8_att_idx_handlers att_idx_handlers;
		v8_att_accessors att_accessors;
	};

	Local<Context> CreateSceneObjectScriptContext(Isolate* isolate, SceneObject* so, v8_att_context& att_context)
	{
		Local<ObjectTemplate> global = ObjectTemplate::New(isolate);

		Local<Context> context = Context::New(isolate, nullptr, global);
		v8::Context::Scope context_scope(context);
		AddConsoleToContext(isolate, context);

		v8_att_idx_handlers& att_idx_handlers = att_context.att_idx_handlers;
		v8_att_accessors& att_accessors = att_context.att_accessors;

		att_accessors.insert_or_assign("name", v8_create_accessor<std::string>(so, "name"));
		context->Global()->SetAccessor(context, v8_name(isolate, "name"), v8_getter, v8_setter,
			v8_external(isolate, &att_accessors.at("name"))
		);

		att_idx_handlers.insert_or_assign("position", v8_create_idx_handler<XMFLOAT3>(so, "position"));
		Local<ObjectTemplate> position_idx_tmpl = ObjectTemplate::New(isolate);
		position_idx_tmpl->SetIndexedPropertyHandler(v8_idx_getter, v8_idx_setter, v8_idx_query, nullptr, v8_idx_enumerator, v8_external(isolate, &att_idx_handlers.at("position")));
		position_idx_tmpl->SetAccessor(v8_name(isolate, "length"),
			[](v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value>& info) { info.GetReturnValue().Set(3); }
		);
		//position_idx_tmpl->Set(isolate, "toJSON", v8::FunctionTemplate::New(isolate, PositionToJSON));

		Local<Object> position_inst = position_idx_tmpl->NewInstance(context).ToLocalChecked();
		Local<Array> position_dummyArray = Array::New(isolate, 0);
		position_inst->SetPrototype(context, position_dummyArray);

		att_accessors.insert_or_assign("position", v8_create_accessor<XMFLOAT3>(so, "position", position_inst));
		context->Global()->SetAccessor(context, v8_name(isolate, "position"), v8_getter, v8_setter, v8_external(isolate, &att_accessors.at("position")));

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