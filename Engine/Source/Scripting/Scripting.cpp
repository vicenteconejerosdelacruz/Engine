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

	void AddTemplateJsonAttributes(Isolate* isolate, Local<ObjectTemplate>& tmpl, v8_att_context& att_context, nlohmann::json& json, std::string path)
	{
		std::map<std::string, v8_template_attribute> attributeCreator =
		{
			{ "uuid", v8_template_json_attribute<std::string> },
			{ "name", v8_template_json_attribute<std::string> },
			{ "castShadows" , v8_template_json_attribute<bool> },
			{ "shadowed" , v8_template_json_attribute<bool> },
			{ "hidden", v8_template_json_attribute<bool> },
			{ "ibl", v8_template_json_attribute<bool> },
			{ "meshMaterial", v8_template_json_attribute<MeshMaterial> },
			{ "model", v8_template_json_attribute<std::string> },
			{ "animationSequence", v8_template_json_attribute<std::string> },
			{ "animation", v8_template_json_attribute<std::string> },
			{ "animationTime", v8_template_json_attribute<float> },
			{ "animationTimeFactor", v8_template_json_attribute<float> },
			{ "animationLoop", v8_template_json_attribute<bool> },
			{ "animationPlay" , v8_template_json_attribute<bool> },
			{ "animationFrame", v8_template_json_attribute<int> },
			{ "animationUseTransformation", v8_template_json_attribute<bool> },
			{ "position", v8_template_json_attribute<XMFLOAT3> },
			{ "rotation", v8_template_json_attribute<XMFLOAT3> },
			{ "scale", v8_template_json_attribute<XMFLOAT3> },
			{ "checkBoundingBox" , v8_template_json_attribute<bool> },
			{ "uniqueMaterialInstance", v8_template_json_attribute<bool> },
			{ "visible", v8_template_json_attribute<bool> },
		};

		for (auto& elem : json.items())
		{
			if (!attributeCreator.contains(elem.key())) continue;
			attributeCreator.at(elem.key())(isolate, tmpl, att_context, json, path, elem.key());
		}
	}

	void AddContextJsonAttributes(Isolate* isolate, Local<Context> context, v8_att_context& att_context, nlohmann::json& json, std::string path)
	{

		std::map<std::string, v8_context_attribute> attributeCreator =
		{
			{ "meshMaterial", v8_context_json_attribute<MeshMaterial> },
			{ "position", v8_context_json_attribute<XMFLOAT3> },
			{ "rotation", v8_context_json_attribute<XMFLOAT3> },
			{ "scale", v8_context_json_attribute<XMFLOAT3> },
		};

		for (auto& elem : json.items())
		{
			if (!attributeCreator.contains(elem.key())) continue;
			attributeCreator.at(elem.key())(isolate, att_context, json, path, elem.key());
		}
	}

	Local<Context> CreateSceneObjectScriptContext(Isolate* isolate, SceneObject* so, v8_att_context& att_context)
	{
		Local<ObjectTemplate> global = ObjectTemplate::New(isolate);

		v8_att_to_jsons& att_to_jsons = att_context.att_to_jsons;

		std::string suuuid_str = so->SUuuid_str();
		AddToJsonToTemplate(isolate, global, suuuid_str, att_to_jsons, *so);

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