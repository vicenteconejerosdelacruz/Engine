#include "pch.h"
#include "NoV8.h"
#include <Controller.h>
#include <SceneObject.h>

using namespace Game;
namespace nov8
{
	void ConsoleLog(const FunctionCallbackInfo<Value>& info)
	{
#if defined(_DEVELOPMENT)
		Isolate* isolate = info.GetIsolate();
		HandleScope handle_scope(isolate);

		std::vector<std::string> strvec;
		for (int i = 0; i < info.Length(); i++) {
			v8::String::Utf8Value str(isolate, info[i]);
			if (*str)
			{
				strvec.push_back(*str);
			}
		}
		strvec.push_back("\n");
		OutputDebugStringA(nostd::join(strvec, " ").c_str());
#endif
	}

	void AddConsoleToContext(Isolate* isolate, Local<Context> context)
	{
		Local<Object> console = Object::New(isolate);
		console->Set(context, v8::String::NewFromUtf8(isolate, "log").ToLocalChecked(), FunctionTemplate::New(isolate, ConsoleLog)->GetFunction(context).ToLocalChecked()).Check();;
		context->Global()->Set(context, v8::String::NewFromUtf8(isolate, "console").ToLocalChecked(), console).Check();
	}
	void AddTemplateJsonAttributes(Isolate* isolate, Local<ObjectTemplate>& tmpl, v8_att_context& att_context, v8_templates_creators& attributeCreator, JObject& json, std::string path)
	{
		for (auto& [attribute, creator] : attributeCreator)
		{
			creator(isolate, tmpl, att_context, json, path, attribute);
		}
	}
	void AddTemplateFunctions(Isolate* isolate, Local<ObjectTemplate>& tmpl, v8_att_context& att_context, v8_functions_creators& functionsCreator, JObject& json, std::string objectName, std::string path)
	{
		v8_att_functions& att_functions = att_context.att_functions;
		for (auto& [funcName, func] : functionsCreator)
		{
			AddFunctionToTemplate(isolate, tmpl, att_functions, path, objectName, funcName, func);

		}
	}
	void AddContextJsonAttributes(Isolate* isolate, Local<Context> context, v8_att_context& att_context, v8_context_creators& attributeCreator, JObject& json, std::string path)
	{
		for (auto& [attribute, creator] : attributeCreator)
		{
			creator(isolate, att_context, json, path, attribute);
		}
	}

	Local<Name> v8_name(Isolate* isolate, std::string name)
	{
		return v8::String::NewFromUtf8(isolate, name.c_str()).ToLocalChecked();
	}

	std::string v8_name(Isolate* isolate, Local<Name> name)
	{
		Local<v8::String> v8String = name->ToString(isolate->GetCurrentContext()).ToLocalChecked();
		v8::String::Utf8Value utf8(isolate, v8String);
		return *utf8;
	}

	Local<v8::String> v8_string(Isolate* isolate, std::string str)
	{
		return v8::String::NewFromUtf8(isolate, str.c_str()).ToLocalChecked();
	}

	Local<External> v8_external(Isolate* isolate, void* value)
	{
		return External::New(isolate, value);
	}

	Local<Value> v8_json_parse(Isolate* isolate, nlohmann::json& json)
	{
		Local<String> v8_json_str = v8_string(isolate, json.dump());
		Local<Value> json_object;
		if (JSON::Parse(isolate->GetCurrentContext(), v8_json_str).ToLocal(&json_object))
			return json_object;
		return Null(isolate);
	}

	v8_get v8_fixed_size(int size)
	{
		return [=](Local<Name> property, const PropertyCallbackInfo<Value>& info)
			{
				info.GetReturnValue().Set(size);
			};
	}

	//json
	void v8_call_function(const FunctionCallbackInfo<Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		Local<Value> data = args.Data();
		if (!data.IsEmpty() && data->IsExternal()) {
			Local<External> external = data.As<External>();
			auto& func = *static_cast<v8_function*>(external->Value());
			func(args);
		}
	}
	v8_function v8_toJSON(nlohmann::json* json)
	{
		return [=](const FunctionCallbackInfo<Value>& args)
			{
				args.GetReturnValue().Set(v8_json_parse(args.GetIsolate(), *json));
			};
	}

	v8_function v8_toJSON(nlohmann::json* json, std::string attribute) {
		return [=](const FunctionCallbackInfo<Value>& args)
			{
				args.GetReturnValue().Set(v8_json_parse(args.GetIsolate(), json->at(attribute)));
			};
	}

	v8_get v8_get_json_controller(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return[=](Local<Name> property, const PropertyCallbackInfo<Value>& info) mutable
			{
				Isolate* isolate = info.GetIsolate();
				Local<Context> context = isolate->GetCurrentContext();

				Local<Object> controller_map_obj = Object::New(isolate);

				for (auto& [name, uuid] : json->at(attribute).items())
				{
					auto& controller = GetController(uuid);
					v8_templates_creators template_creators = controller->GetV8TemplatesCreators();
					v8_functions_creators functions_creators = controller->GetV8FunctionsCreators();

					Local<ObjectTemplate> controller_tmpl = ObjectTemplate::New(isolate);
					//Add Attributes
					AddTemplateJsonAttributes(isolate, controller_tmpl, att_context, template_creators, *controller.get(), path);
					//Add Functions
					AddTemplateFunctions(isolate, controller_tmpl, att_context, functions_creators, *controller.get(), name, path);

					Local<Object> controller_obj = controller_tmpl->NewInstance(context).ToLocalChecked();
					controller_map_obj->Set(context, v8_name(isolate, name), controller_obj);
				}

				info.GetReturnValue().Set(controller_map_obj);
			};
	}

	v8_get v8_get_json_scriptbinding(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, unsigned int idx)
	{
		return[=](Local<Name> property, const PropertyCallbackInfo<Value>& info) mutable
			{
				using namespace Scene;
				SceneObject* so = static_cast<SceneObject*>(jobject);
				ScriptBinding sb(json->at(idx));

				Isolate* isolate = info.GetIsolate();
				Local<Context> context = isolate->GetCurrentContext();
				Local<ObjectTemplate> binding_map_tmpl = ObjectTemplate::New(isolate);

				v8_att_functions& att_functions = att_context.att_functions;

				switch (sb.bindingType)
				{
				case BT_SceneObject:
				{
					SceneObject* binded = GetSceneObjectPointer(so->unit, sb.uuid);
					v8_templates_creators template_creators = so->GetV8TemplatesCreators();
					v8_functions_creators functions_creators = so->GetV8FunctionsCreators();

					//Add toJSON
					AddFunctionToTemplate(isolate, binding_map_tmpl, att_functions, binded->SUuuid_str(), "toJSON", v8_toJSON(so));
					//Add Attributes
					AddTemplateJsonAttributes(isolate, binding_map_tmpl, att_context, template_creators, *binded, path);
					//Add Functions
					AddTemplateFunctions(isolate, binding_map_tmpl, att_context, functions_creators, *binded, sb.bindingName, path);
				}
				break;
				case BT_Controller:
				{
					SceneObject* binded = GetSceneObjectPointer(so->unit, sb.uuid);
					auto& controller = GetController(binded->at("controllers").at(sb.controllerName));
					v8_templates_creators template_creators = controller->GetV8TemplatesCreators();
					v8_functions_creators functions_creators = controller->GetV8FunctionsCreators();

					//Add toJSON
					AddFunctionToTemplate(isolate, binding_map_tmpl, att_functions, controller->uuid(), "toJSON", v8_toJSON(controller.get()));
					//Add Attributes
					AddTemplateJsonAttributes(isolate, binding_map_tmpl, att_context, template_creators, *controller.get(), path);
					//Add Functions
					AddTemplateFunctions(isolate, binding_map_tmpl, att_context, functions_creators, *controller.get(), sb.bindingName, path);
				}
				break;
				case BT_PhysicObject:
				{
					SceneObject* binded = GetSceneObjectPointer(so->unit, sb.uuid);
					auto& phO = GetPhysicObject(binded->at("physicObject").at(sb.physicObjectIndex));
					v8_templates_creators template_creators = phO->GetV8TemplatesCreators();
					v8_functions_creators functions_creators = phO->GetV8FunctionsCreators();

					//Add Attributes
					AddTemplateJsonAttributes(isolate, binding_map_tmpl, att_context, template_creators, *phO.get(), path);
					//Add Functions
					AddTemplateFunctions(isolate, binding_map_tmpl, att_context, functions_creators, *phO.get(), sb.bindingName, path);
				}
				break;
				}

				Local<Object> binding_map_obj = binding_map_tmpl->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
				info.GetReturnValue().Set(binding_map_obj);
			};
	}

	v8_function v8_set_size(nlohmann::json* json)
	{
		return [=](const FunctionCallbackInfo<Value>& args)
			{
				args.GetReturnValue().Set(Integer::New(args.GetIsolate(), static_cast<int>(json->size())));
			};
	}

	void AddFunctionToTemplate(Isolate* isolate, Local<ObjectTemplate>& tmpl, v8_att_functions& att_functions, std::string path, std::string functionName, v8_function func)
	{
		std::string jptr = path + "." + functionName + "()";
		att_functions.insert_or_assign(jptr, func);
		tmpl->Set(isolate, functionName.c_str(), FunctionTemplate::New(isolate, v8_call_function, v8_external(isolate, &att_functions.at(jptr))));
	}

	void AddFunctionToTemplate(Isolate* isolate, Local<ObjectTemplate>& tmpl, v8_att_functions& att_functions, std::string path, std::string attribute, std::string functionName, v8_function func)
	{
		std::string jptr = path + "/" + attribute + "." + functionName + "()";
		att_functions.insert_or_assign(jptr, func);
		tmpl->Set(isolate, functionName.c_str(), FunctionTemplate::New(isolate, v8_call_function, v8_external(isolate, &att_functions.at(jptr))));
	}

	//indexed
	void v8_idx_getter(uint32_t index, const PropertyCallbackInfo<Value>& info)
	{
		auto& func = std::get<0>(*static_cast<v8_idx_handler*>(Local<External>::Cast(info.Data())->Value()));
		func(index, info);
	}
	void v8_idx_setter(uint32_t index, Local<Value> value, const PropertyCallbackInfo<Value>& info)
	{
		auto& func = std::get<1>(*static_cast<v8_idx_handler*>(Local<External>::Cast(info.Data())->Value()));
		func(index, value, info);
	}
	void v8_idx_query(uint32_t index, const PropertyCallbackInfo<Integer>& info)
	{
		auto& func = std::get<2>(*static_cast<v8_idx_handler*>(Local<External>::Cast(info.Data())->Value()));
		func(index, info);
	}
	void v8_idx_enumerator(const PropertyCallbackInfo<Array>& info)
	{
		auto& func = std::get<3>(*static_cast<v8_idx_handler*>(Local<External>::Cast(info.Data())->Value()));
		func(info);
	}

	//accessors
	void v8_getter(Local<Name> property, const PropertyCallbackInfo<Value>& info)
	{
		auto& func = std::get<0>(*static_cast<v8_accessor*>(Local<External>::Cast(info.Data())->Value()));
		func(property, info);
	}
	void v8_setter(Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info)
	{
		auto& func = std::get<1>(*static_cast<v8_accessor*>(Local<External>::Cast(info.Data())->Value()));
		func(property, value, info);
	}
}