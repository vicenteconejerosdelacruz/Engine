#include "pch.h"
#include "NoV8.h"

namespace nov8
{
	void ConsoleLog(const FunctionCallbackInfo<Value>& info)
	{
#if defined(_DEVELOPMENT)
		Isolate* isolate = info.GetIsolate();
		HandleScope handle_scope(isolate);

		std::vector<std::string> strvec;
		for (int i = 0; i < info.Length(); i++) {
			String::Utf8Value str(isolate, info[i]);
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
		console->Set(context, String::NewFromUtf8(isolate, "log").ToLocalChecked(), FunctionTemplate::New(isolate, ConsoleLog)->GetFunction(context).ToLocalChecked()).Check();;
		context->Global()->Set(context, String::NewFromUtf8(isolate, "console").ToLocalChecked(), console).Check();
	}

	Local<Name> v8_name(Isolate* isolate, std::string name)
	{
		return String::NewFromUtf8(isolate, name.c_str()).ToLocalChecked();
	}

	std::string v8_name(Isolate* isolate, Local<Name> name)
	{
		Local<String> v8String = name->ToString(isolate->GetCurrentContext()).ToLocalChecked();
		String::Utf8Value utf8(isolate, v8String);
		return *utf8;
	}

	Local<String> v8_string(Isolate* isolate, std::string str)
	{
		return String::NewFromUtf8(isolate, str.c_str()).ToLocalChecked();
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

	//json
	void v8_toJSON(const FunctionCallbackInfo<Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		Local<Value> data = args.Data();
		if (!data.IsEmpty() && data->IsExternal()) {
			Local<External> external = data.As<External>();
			auto& toJSON = *static_cast<v8_to_json*>(external->Value());
			toJSON(args);
		}
	}

	v8_to_json v8_toJSON(nlohmann::json* json)
	{
		return [=](const FunctionCallbackInfo<Value>& args)
			{
				Isolate* isolate = args.GetIsolate();

				args.GetReturnValue().Set(v8_json_parse(isolate, *json));
			};
	}

	v8_to_json v8_toJSON(nlohmann::json* json, std::string attribute) {
		return [=](const FunctionCallbackInfo<Value>& args)
			{
				Isolate* isolate = args.GetIsolate();

				args.GetReturnValue().Set(v8_json_parse(isolate, json->at(attribute)));
			};
	}

	void AddToJsonToTemplate(Isolate* isolate, Local<ObjectTemplate>& tmpl, std::string path, v8_att_to_jsons& att_to_jsons, nlohmann::json& json)
	{
		std::string jptr = path;
		att_to_jsons.insert_or_assign(jptr, v8_toJSON(&json));
		tmpl->Set(isolate, "toJSON", FunctionTemplate::New(isolate, v8_toJSON, v8_external(isolate, &att_to_jsons.at(jptr))));
	}

	void AddToJsonToTemplate(Isolate* isolate, Local<ObjectTemplate>& tmpl, std::string path, v8_att_to_jsons& att_to_jsons, nlohmann::json& json, std::string attribute)
	{
		std::string jptr = path + "/" + attribute;
		att_to_jsons.insert_or_assign(jptr, v8_toJSON(&json, attribute));
		tmpl->Set(isolate, "toJSON", FunctionTemplate::New(isolate, v8_toJSON, v8_external(isolate, &att_to_jsons.at(jptr))));
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