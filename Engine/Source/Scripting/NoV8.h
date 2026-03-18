#pragma once

#include <v8.h>
#include <libplatform/libplatform.h>
#include <v8pp/context.hpp>
#include <v8pp/module.hpp>
#include <nlohmann/json.hpp>
#include <set>

struct MeshMaterial;
struct JObject;
using namespace v8;
namespace nov8
{
	using v8_get = std::function<void(Local<Name>, const PropertyCallbackInfo<Value>&)>;
	using v8_set = std::function<void(Local<Name>, Local<Value>, const PropertyCallbackInfo<void>&)>;
	using v8_idx_get = std::function<void(uint32_t, const PropertyCallbackInfo<Value>&)>;
	using v8_idx_set = std::function<void(uint32_t, Local<Value>, const PropertyCallbackInfo<Value>&)>;
	using v8_idx_qry = std::function<void(uint32_t, const PropertyCallbackInfo<Integer>&)>;
	using v8_idx_enum = std::function<void(const PropertyCallbackInfo<Array>&)>;
	using v8_accessor = std::tuple<v8_get, v8_set, Local<Object>>;
	using v8_function = std::function<void(const FunctionCallbackInfo<Value>&)>;
	using v8_idx_handler = std::tuple<v8_idx_get, v8_idx_set, v8_idx_qry, v8_idx_enum>;

	using v8_att_templates = std::map<std::string, Local<ObjectTemplate>>;
	using v8_att_accessors = std::map<std::string, v8_accessor>;
	using v8_att_functions = std::map<std::string, v8_function>;
	using v8_att_idx_handlers = std::map<std::string, v8_idx_handler>;
	struct v8_att_context
	{
		v8_att_templates att_templates;
		v8_att_accessors att_accessors;
		v8_att_functions att_functions;
		v8_att_idx_handlers att_idx_handlers;
	};
	using v8_template_attribute = std::function<void(Isolate*, Local<ObjectTemplate>&, v8_att_context&, JObject&, std::string, std::string)>;
	using v8_context_attribute = std::function<void(Isolate* isolate, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)>;
	using v8_templates_creators = std::map<std::string, v8_template_attribute>;
	using v8_context_creators = std::map<std::string, v8_context_attribute>;

	//console.log
	void ConsoleLog(const FunctionCallbackInfo<Value>& info);
	void AddConsoleToContext(Isolate* isolate, Local<Context> context);

	//utils
	Local<Name> v8_name(Isolate* isolate, std::string name);
	std::string v8_name(Isolate* isolate, Local<Name> name);
	Local<String> v8_string(Isolate* isolate, std::string str);
	Local<External> v8_external(Isolate* isolate, void* value);
	Local<Value> v8_json_parse(Isolate* isolate, nlohmann::json& json);
	v8_get v8_fixed_size(int size);

	//accessors
	void v8_getter(Local<Name> property, const PropertyCallbackInfo<Value>& info);
	void v8_setter(Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info);

	//functions
	void v8_call_function(const FunctionCallbackInfo<Value>&);
	void AddFunctionToTemplate(Isolate* isolate, Local<ObjectTemplate>& tmpl, v8_att_functions& att_functions, std::string path, std::string functionName, v8_function func);
	void AddFunctionToTemplate(Isolate* isolate, Local<ObjectTemplate>& tmpl, v8_att_functions& att_functions, std::string path, std::string attribute, std::string functionName, v8_function func);
	template<typename T>
	inline void AddFunctionToObject(Isolate* isolate, Local<Context>& context, v8_att_functions& att_functions, std::string path, std::string name, v8_function func, T& object)
	{
		std::string jptr_erase = path + "." + name + "()";
		att_functions.insert_or_assign(jptr_erase, func);
		v8::Local<v8::FunctionTemplate> func_tpl = v8::FunctionTemplate::New(isolate, v8_call_function, v8_external(isolate, &att_functions.at(jptr_erase)));
		v8::Local<v8::Function> func_inst = func_tpl->GetFunction(context).ToLocalChecked();
		object->Set(context, v8_name(isolate, name), func_inst);
	}
	v8_function v8_toJSON(nlohmann::json* json);
	v8_function v8_toJSON(nlohmann::json* json, std::string attribute);
	template<typename T>
	inline v8_function v8_set_erase(nlohmann::json* json, std::string attribute) { return [=](const FunctionCallbackInfo<Value>&) {}; }
	template<typename T>
	inline v8_function v8_set_insert(nlohmann::json* json, std::string attribute) { return [=](const FunctionCallbackInfo<Value>&) {}; }
	template<typename T>
	inline v8_function v8_set_clear(nlohmann::json* json, std::string attribute)
	{
		return [=](const FunctionCallbackInfo<Value>& args)
			{
				v8::Isolate* isolate = args.GetIsolate();

				if (!json->contains(attribute)) return;
				json->at(attribute).clear();
			};
	}
	template<typename T>
	inline v8_function v8_set_size(nlohmann::json* json, std::string attribute)
	{
		return [=](const FunctionCallbackInfo<Value>& args)
			{
				if (!json->contains(attribute)) return;
				v8::Isolate* isolate = args.GetIsolate();
				args.GetReturnValue().Set(Integer::New(isolate, static_cast<int>(json->at(attribute).size())));
			};
	}
	template<>
	inline v8_function v8_set_erase<std::set<int>>(nlohmann::json* json, std::string attribute)
	{
		return [=](const FunctionCallbackInfo<Value>& args)
			{
				v8::Isolate* isolate = args.GetIsolate();

				if (args.Length() == 0 || !args[0]->IsInt32())
					return;

				int value = args[0]->Int32Value(isolate->GetCurrentContext()).FromJust();

				if (!json->contains(attribute)) return;
				for (int i = 0; i < json->at(attribute).size(); i++)
				{
					if (json->at(attribute).at(i) == value)
					{
						json->at(attribute).erase(i);
						return;
					}
				}
			};
	}
	template<>
	inline v8_function v8_set_insert<std::set<int>>(nlohmann::json* json, std::string attribute)
	{
		return [=](const FunctionCallbackInfo<Value>& args)
			{
				v8::Isolate* isolate = args.GetIsolate();

				if (args.Length() == 0 || !args[0]->IsInt32())
					return;

				int value = args[0]->Int32Value(isolate->GetCurrentContext()).FromJust();

				if (!json->contains(attribute)) return;
				for (int i = 0; i < json->at(attribute).size(); i++)
				{
					if (json->at(attribute).at(i) == value)
						return;
				}
				json->at(attribute).push_back(value);
			};
	}

	//indexed
	void v8_idx_getter(uint32_t index, const PropertyCallbackInfo<Value>& info);
	void v8_idx_setter(uint32_t index, Local<Value> value, const PropertyCallbackInfo<Value>& info);
	void v8_idx_query(uint32_t index, const PropertyCallbackInfo<Integer>& info);
	void v8_idx_enumerator(const PropertyCallbackInfo<Array>& info);

	//accessors
	template<typename T>
	inline v8_get v8_get_json(v8_att_context& att_context, std::string path, nlohmann::json* json, std::string attribute) { return nullptr; }
	template<typename T>
	inline v8_set v8_set_json(v8_att_context& att_context, std::string path, JObject* jobject, nlohmann::json* json, std::string attribute) { return nullptr; }
	//accessors std::string
	template<>
	inline v8_get v8_get_json<std::string>(v8_att_context& att_context, std::string path, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, const PropertyCallbackInfo<Value>& info)
			{
				info.GetReturnValue().Set(String::NewFromUtf8(info.GetIsolate(), std::string(json->at(attribute)).c_str()).ToLocalChecked());
			};
	}
	template<>
	inline v8_set v8_set_json<std::string>(v8_att_context& att_context, std::string path, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info)
			{
				if (value->IsString()) {
					String::Utf8Value utf8(info.GetIsolate(), value);
					json->at(attribute) = std::string(*utf8);
				}
				info.GetReturnValue().Set(value);
			};
	}
	//accessors bool
	template<>
	inline v8_get v8_get_json<bool>(v8_att_context& att_context, std::string path, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, const PropertyCallbackInfo<Value>& info)
			{
				info.GetReturnValue().Set(v8::Boolean::New(info.GetIsolate(), static_cast<bool>(json->at(attribute))));
			};
	}
	template<>
	inline v8_set v8_set_json<bool>(v8_att_context& att_context, std::string path, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info)
			{
				json->at(attribute) = value->BooleanValue(info.GetIsolate());
			};
	}
	//accessors int
	template<>
	inline v8_get v8_get_json<int>(v8_att_context& att_context, std::string path, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, const PropertyCallbackInfo<Value>& info)
			{
				info.GetReturnValue().Set(Integer::New(info.GetIsolate(), static_cast<int>(json->at(attribute))));
			};
	}
	template<>
	inline v8_set v8_set_json<int>(v8_att_context& att_context, std::string path, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info)
			{
				json->at(attribute) = value->Int32Value(info.GetIsolate()->GetCurrentContext()).ToChecked();
			};
	}

	//accessors std::set<int>
	template<>
	inline v8_get v8_get_json<std::set<int>>(v8_att_context& att_context, std::string path, nlohmann::json* json, std::string attribute)
	{
		return [=, &att_context](Local<Name> property, const PropertyCallbackInfo<Value>& info)
			{
				v8_att_functions& att_functions = att_context.att_functions;

				Isolate* isolate = info.GetIsolate();
				Local<Context> context = isolate->GetCurrentContext();

				Local<Array> my_array = Array::New(isolate, 0);

				AddFunctionToObject(isolate, context, att_functions, path, "erase", v8_set_erase<std::set<int>>(json, attribute), my_array);
				AddFunctionToObject(isolate, context, att_functions, path, "insert", v8_set_insert<std::set<int>>(json, attribute), my_array);
				AddFunctionToObject(isolate, context, att_functions, path, "clear", v8_set_clear<std::set<int>>(json, attribute), my_array);
				AddFunctionToObject(isolate, context, att_functions, path, "size", v8_set_size<std::set<int>>(json, attribute), my_array);

				info.GetReturnValue().Set(my_array);
			};
	}
	template<>
	inline v8_set v8_set_json<std::set<int>>(v8_att_context& att_context, std::string path, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info)
			{
				if (!value->IsArray()) return;

				nlohmann::json& jarr = json->at(attribute);
				jarr.clear();
				Local<Array> arr = value.As<Array>();
				std::set<int> values;
				for (uint32_t i = 0; i < arr->Length(); i++) {
					Local<Value> item;
					if (arr->Get(info.GetIsolate()->GetCurrentContext(), i).ToLocal(&item)) {
						int value = static_cast<int>(item->Int32Value(info.GetIsolate()->GetCurrentContext()).ToChecked());
						if (!values.contains(value))
						{
							jarr.at(i) = value;
							values.insert(value);
						}
					}
				}
			};
	}
	//accessors unsigned int
	template<>
	inline v8_get v8_get_json<unsigned int>(v8_att_context& att_context, std::string path, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, const PropertyCallbackInfo<Value>& info)
			{
				info.GetReturnValue().Set(Integer::NewFromUnsigned(info.GetIsolate(), static_cast<int>(json->at(attribute))));
			};
	}
	template<>
	inline v8_set v8_set_json<unsigned int>(v8_att_context& att_context, std::string path, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info)
			{
				json->at(attribute) = value->Uint32Value(info.GetIsolate()->GetCurrentContext()).ToChecked();
			};
	}
	//accessors float
	template<>
	inline v8_get v8_get_json<float>(v8_att_context& att_context, std::string path, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, const PropertyCallbackInfo<Value>& info)
			{
				info.GetReturnValue().Set(Number::New(info.GetIsolate(), static_cast<float>(json->at(attribute))));
			};
	}
	template<>
	inline v8_set v8_set_json<float>(v8_att_context& att_context, std::string path, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info)
			{
				json->at(attribute) = static_cast<float>(value->NumberValue(info.GetIsolate()->GetCurrentContext()).FromMaybe(0.0));
			};
	}
	//accessors XMFLOAT3
	template<>
	inline v8_get v8_get_json<XMFLOAT3>(v8_att_context& att_context, std::string path, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, const PropertyCallbackInfo<Value>& info)
			{
				Local<Object>& data = std::get<2>(*static_cast<v8_accessor*>(Local<External>::Cast(info.Data())->Value()));
				info.GetReturnValue().Set(data);
			};
	}
	template<>
	inline v8_set v8_set_json<XMFLOAT3>(v8_att_context& att_context, std::string path, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info)
			{
				if (!value->IsArray()) return;

				nlohmann::json& jarr = json->at(attribute);
				Local<Array> arr = value.As<Array>();
				for (uint32_t i = 0; i < 3 && i < arr->Length(); i++) {
					Local<Value> item;
					if (arr->Get(info.GetIsolate()->GetCurrentContext(), i).ToLocal(&item)) {
						jarr.at(i) = static_cast<float>(item->NumberValue(info.GetIsolate()->GetCurrentContext()).ToChecked());
					}
				}
			};
	}
	//accessors MeshMaterial
	template<>
	inline v8_get v8_get_json<MeshMaterial>(v8_att_context& att_context, std::string path, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, const PropertyCallbackInfo<Value>& info)
			{
				Isolate* isolate = info.GetIsolate();
				Local<String> v8_json_str = v8_string(isolate, json->at(attribute).dump());
				Local<Value> json_object;
				if (JSON::Parse(isolate->GetCurrentContext(), v8_json_str).ToLocal(&json_object))
				{
					info.GetReturnValue().Set(json_object);
				}
			};
	}
	template<>
	inline v8_set v8_set_json<MeshMaterial>(v8_att_context& att_context, std::string path, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info)
			{

			};
	}

	//accessors creator
	template<typename T>
	inline v8_accessor v8_create_accessor(v8_att_context& att_context, std::string path, JObject* jobject, nlohmann::json* json, std::string attribute, Local<Object> object = Local<Object>())
	{
		return std::make_tuple(
			v8_get_json<T>(att_context, path, json, attribute),
			v8_set_json<T>(att_context, path, jobject, json, attribute),
			object
		);
	}

	template<typename T>
	inline v8_idx_get v8_idx_get_json(v8_att_context& att_context, std::string path, nlohmann::json* json, std::string attribute) { return nullptr; }
	template<typename T>
	inline v8_idx_set v8_idx_set_json(v8_att_context& att_context, std::string path, JObject* jobject, nlohmann::json* json, std::string attribute) { return nullptr; }
	template<typename T>
	inline v8_idx_qry v8_idx_query_json(v8_att_context& att_context, std::string path, nlohmann::json* json, std::string attribute) { return nullptr; }
	template<typename T>
	inline v8_idx_enum v8_idx_enumerator_json(v8_att_context& att_context, std::string path, nlohmann::json* json, std::string attribute) { return nullptr; }

	//indexed XMFLOAT3
	template<>
	inline v8_idx_get v8_idx_get_json<XMFLOAT3>(v8_att_context& att_context, std::string path, nlohmann::json* json, std::string attribute)
	{
		return [=](uint32_t index, const PropertyCallbackInfo<Value>& info)
			{
				if (index > 3) return;
				info.GetReturnValue().Set(Number::New(info.GetIsolate(), static_cast<float>(json->at(attribute).at(index))));
			};
	}
	template<>
	inline v8_idx_set v8_idx_set_json<XMFLOAT3>(v8_att_context& att_context, std::string path, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](uint32_t index, Local<Value> value, const PropertyCallbackInfo<Value>& info)
			{
				if (index >= 3) return;
				json->at(attribute).at(index) = static_cast<float>(value->NumberValue(info.GetIsolate()->GetCurrentContext()).ToChecked());
				info.GetReturnValue().Set(value);
			};
	}
	template<>
	inline v8_idx_qry v8_idx_query_json<XMFLOAT3>(v8_att_context& att_context, std::string path, nlohmann::json* json, std::string attribute)
	{
		return [=](uint32_t index, const PropertyCallbackInfo<Integer>& info)
			{
				if (index > 3) return;
				info.GetReturnValue().Set(None);
			};
	}
	template<>
	inline v8_idx_enum v8_idx_enumerator_json<XMFLOAT3>(v8_att_context& att_context, std::string path, nlohmann::json* json, std::string attribute) {
		return [=](const PropertyCallbackInfo<Array>& info)
			{
				Isolate* isolate = info.GetIsolate();
				Local<Array> arr = Array::New(isolate, 3);
				for (uint32_t i = 0; i < 3; i++) {
					arr->Set(isolate->GetCurrentContext(), i, Integer::New(isolate, i)).Check();
				}
				info.GetReturnValue().Set(arr);
			};
	}

	//indexed creator
	template<typename T>
	inline v8_idx_handler v8_create_idx_handler(v8_att_context& att_context, std::string path, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return std::make_tuple(
			v8_idx_get_json<T>(att_context, path, json, attribute),
			v8_idx_set_json<T>(att_context, path, jobject, json, attribute),
			v8_idx_query_json<T>(att_context, path, json, attribute),
			v8_idx_enumerator_json<T>(att_context, path, json, attribute)
		);
	}

	//these functions are crafted for JObject derivations for the Local<ObjectTemplate>
	template<typename T>
	inline void v8_template_json_attribute(Isolate* isolate, Local<ObjectTemplate>& tmpl, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
	{
		v8_att_accessors& att_accessors = att_context.att_accessors;
		std::string jptr = path + "/" + attribute;
		att_accessors.insert_or_assign(jptr, v8_create_accessor<T>(att_context, jptr, &json, &json, attribute));
		tmpl->SetAccessor(v8_name(isolate, attribute), v8_getter, v8_setter, v8_external(isolate, &att_accessors.at(jptr)));
	};
	template<>
	inline void v8_template_json_attribute<XMFLOAT3>(Isolate* isolate, Local<ObjectTemplate>& tmpl, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
	{
		v8_att_idx_handlers& att_idx_handlers = att_context.att_idx_handlers;
		v8_att_functions& att_functions = att_context.att_functions;
		v8_att_templates& att_templates = att_context.att_templates;
		v8_att_accessors& att_accessors = att_context.att_accessors;

		std::string jptr = path + "/" + attribute;

		att_idx_handlers.insert_or_assign(jptr, v8_create_idx_handler<XMFLOAT3>(att_context, jptr, &json, &json, attribute));
		Local<ObjectTemplate> xmf3_idx_tmpl = ObjectTemplate::New(isolate);
		xmf3_idx_tmpl->SetIndexedPropertyHandler(v8_idx_getter, v8_idx_setter, v8_idx_query, nullptr, v8_idx_enumerator, v8_external(isolate, &att_idx_handlers.at(jptr)));

		std::string jptr_length = jptr + "/length";
		att_accessors.insert_or_assign(jptr_length, std::make_tuple(v8_fixed_size(3), nullptr, Local<Object>()));
		xmf3_idx_tmpl->SetAccessor(v8_name(isolate, "length"), v8_getter, v8_setter, v8_external(isolate, &att_accessors.at(jptr_length)));

		AddFunctionToTemplate(isolate, xmf3_idx_tmpl, att_functions, path, attribute, "toJSON", v8_toJSON(&json, attribute));
		att_templates.insert_or_assign(jptr, xmf3_idx_tmpl);
	}
	template<>
	inline void v8_template_json_attribute<MeshMaterial>(Isolate* isolate, Local<ObjectTemplate>& tmpl, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
	{
		v8_att_templates& att_templates = att_context.att_templates;
		v8_att_accessors& att_accessors = att_context.att_accessors;
		v8_att_functions& att_functions = att_context.att_functions;

		//:/meshMaterial
		std::string jptr = path + "/" + attribute;
		Local<ObjectTemplate> meshMaterial_tmpl = ObjectTemplate::New(isolate);
		att_templates.insert_or_assign(jptr, meshMaterial_tmpl);
		att_accessors.insert_or_assign(jptr, v8_create_accessor<MeshMaterial>(att_context, jptr, &json, &json, attribute));

		//:/meshMaterial/material
		std::string material_jptr = jptr + "/material";
		att_accessors.insert_or_assign(material_jptr, v8_create_accessor<MeshMaterial>(att_context, material_jptr, &json, &json.at(attribute), "material"));
		meshMaterial_tmpl->SetAccessor(v8_name(isolate, "material"), v8_getter, v8_setter, v8_external(isolate, &att_accessors.at(material_jptr)));
		AddFunctionToTemplate(isolate, meshMaterial_tmpl, att_functions, jptr, "material", "toJSON", v8_toJSON(&json, "material"));

		//:/meshMaterial/mesh
		std::string mesh_jptr = jptr + "/mesh";
		Local<ObjectTemplate> meshMaterial_mesh_tmpl = ObjectTemplate::New(isolate);
		att_templates.insert_or_assign(mesh_jptr, meshMaterial_mesh_tmpl);
		att_accessors.insert_or_assign(mesh_jptr, v8_create_accessor<MeshMaterial>(att_context, mesh_jptr, &json, &json.at(attribute), "mesh"));

		//:/meshMaterial/mesh/primitive
		std::string primitive_jptr = mesh_jptr + "/primitive";
		att_accessors.insert_or_assign(primitive_jptr, v8_create_accessor<MeshMaterial>(att_context, primitive_jptr, &json, &json.at(attribute).at("mesh"), "primitive"));
		meshMaterial_mesh_tmpl->SetAccessor(v8_name(isolate, "primitive"), v8_getter, v8_setter, v8_external(isolate, &att_accessors.at(primitive_jptr)));
		AddFunctionToTemplate(isolate, meshMaterial_tmpl, att_functions, mesh_jptr, "primitive", "toJSON", v8_toJSON(&json.at(attribute).at("mesh"), "primitive"));
	}
	template<typename T>
	inline void v8_template_json_enum_attribute(Isolate* isolate, Local<ObjectTemplate>& tmpl, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
	{
		v8_template_json_attribute<std::string>(isolate, tmpl, att_context, json, path, attribute);
	}
	template<typename T>
	inline void v8_template_json_set_attribute(Isolate* isolate, Local<ObjectTemplate>& tmpl, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
	{
		v8_att_accessors& att_accessors = att_context.att_accessors;
		v8_att_functions& att_functions = att_context.att_functions;

		std::string jptr = path + "/" + attribute;
		att_accessors.insert_or_assign(jptr, v8_create_accessor<std::set<T>>(att_context, jptr, &json, &json, attribute));
		tmpl->SetAccessor(v8_name(isolate, attribute), v8_getter, v8_setter, v8_external(isolate, &att_accessors.at(jptr)));
	}

	//these functions are crafted for JObject derivations for the Local<Object>
	template<typename T>
	inline void v8_context_json_attribute(Isolate* isolate, v8_att_context& att_context, JObject& json, std::string path, std::string attribute) {}
	template<>
	inline void v8_context_json_attribute<XMFLOAT3>(Isolate* isolate, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
	{
		v8_att_templates& att_templates = att_context.att_templates;
		v8_att_accessors& att_accessors = att_context.att_accessors;
		std::string jptr = path + "/" + attribute;

		Local<Object> inst = att_templates.at(jptr)->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Array> dummyArray = Array::New(isolate, 0);
		inst->SetPrototype(isolate->GetCurrentContext(), dummyArray);

		att_accessors.insert_or_assign(jptr, v8_create_accessor<XMFLOAT3>(att_context, jptr, &json, &json, attribute, inst));
		isolate->GetCurrentContext()->Global()->SetAccessor(isolate->GetCurrentContext(), v8_name(isolate, attribute), v8_getter, v8_setter, v8_external(isolate, &att_accessors.at(jptr)));
	}
	template<>
	inline void v8_context_json_attribute<MeshMaterial>(Isolate* isolate, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
	{
		v8_att_templates& att_templates = att_context.att_templates;
		v8_att_accessors& att_accessors = att_context.att_accessors;

		//:/meshMaterial
		std::string jptr = path + "/" + attribute;
		Local<Object> inst = att_templates.at(jptr)->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		att_accessors.insert_or_assign(jptr, v8_create_accessor<MeshMaterial>(att_context, jptr, &json, &json, attribute, inst));
		isolate->GetCurrentContext()->Global()->SetAccessor(isolate->GetCurrentContext(), v8_name(isolate, attribute), v8_getter, v8_setter, v8_external(isolate, &att_accessors.at(jptr)));

		//:/meshMaterial/mesh
		std::string mesh_jptr = jptr + "/mesh";
		Local<Object> mesh_inst = att_templates.at(mesh_jptr)->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		att_accessors.insert_or_assign(mesh_jptr, v8_create_accessor<MeshMaterial>(att_context, mesh_jptr, &json, &json, attribute, mesh_inst));
		inst->SetAccessor(isolate->GetCurrentContext(), v8_name(isolate, "mesh"), v8_getter, v8_setter, v8_external(isolate, &att_accessors.at(mesh_jptr)));
	}
};