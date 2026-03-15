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
		v8::Local<v8::String> v8String = name->ToString(isolate->GetCurrentContext()).ToLocalChecked();
		v8::String::Utf8Value utf8(isolate, v8String);
		return *utf8;
	}

	Local<External> v8_external(Isolate* isolate, void* value)
	{
		return External::New(isolate, value);
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

	//indexed XMFLOAT3
	template<>
	v8_idx_get v8_idx_get_json<XMFLOAT3>(nlohmann::json* json, std::string attribute)
	{
		return [=](uint32_t index, const PropertyCallbackInfo<Value>& info)
			{
				if (index > 3) return;
				info.GetReturnValue().Set(Number::New(info.GetIsolate(), static_cast<float>(json->at(attribute).at(index))));
			};
	}
	template<>
	v8_idx_set v8_idx_set_json<XMFLOAT3>(nlohmann::json* json, std::string attribute)
	{
		return [=](uint32_t index, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info)
			{
				if (index >= 3) return;
				json->at(attribute).at(index) = static_cast<float>(value->NumberValue(info.GetIsolate()->GetCurrentContext()).ToChecked());
				info.GetReturnValue().Set(value);
			};
	}
	template<>
	v8_idx_qry v8_idx_query_json<XMFLOAT3>(nlohmann::json* json, std::string attribute)
	{
		return [=](uint32_t index, const PropertyCallbackInfo<Integer>& info)
			{
				if (index > 3) return;
				info.GetReturnValue().Set(None);
			};
	}
	template<>
	v8_idx_enum v8_idx_enumerator_json<XMFLOAT3>(nlohmann::json* json, std::string attribute) {
		return [=](const v8::PropertyCallbackInfo<Array>& info)
			{
				Isolate* isolate = info.GetIsolate();
				Local<Array> arr = Array::New(isolate, 3);
				for (uint32_t i = 0; i < 3; i++) {
					arr->Set(isolate->GetCurrentContext(), i, Integer::New(isolate, i)).Check();
				}
				info.GetReturnValue().Set(arr);
			};
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

	//accessors std::string
	template<>
	v8_get v8_get_json<std::string>(nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, const PropertyCallbackInfo<Value>& info)
			{
				info.GetReturnValue().Set(String::NewFromUtf8(info.GetIsolate(), std::string(json->at(attribute)).c_str()).ToLocalChecked());
			};
	}
	template<>
	v8_set v8_set_json<std::string>(nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info)
			{
				if (value->IsString()) {
					v8::String::Utf8Value utf8(info.GetIsolate(), value);
					json->at(attribute) = std::string(*utf8);
				}
				info.GetReturnValue().Set(value);
			};
	}
	//accessors XMFLOAT3
	template<>
	v8_get v8_get_json<XMFLOAT3>(nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, const PropertyCallbackInfo<Value>& info)
			{
				Local<Object>& data = std::get<2>(*static_cast<v8_accessor*>(Local<External>::Cast(info.Data())->Value()));
				info.GetReturnValue().Set(data);
			};
	}
	template<>
	v8_set v8_set_json<XMFLOAT3>(nlohmann::json* json, std::string attribute)
	{
		return [=](v8::Local<v8::Name> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
			{
				if (!value->IsArray()) return;

				nlohmann::json& jarr = json->at(attribute);
				Local<Array> arr = value.As<Array>();
				for (uint32_t i = 0; i < 3 && i < arr->Length(); i++) {
					v8::Local<v8::Value> item;
					if (arr->Get(info.GetIsolate()->GetCurrentContext(), i).ToLocal(&item)) {
						jarr.at(i) = static_cast<float>(item->NumberValue(info.GetIsolate()->GetCurrentContext()).ToChecked());
					}
				}
			};
	}
}