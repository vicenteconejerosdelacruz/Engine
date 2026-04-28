#pragma once

#include <v8.h>
#include <libplatform/libplatform.h>
#include <nlohmann/json.hpp>
#include <set>
#include <tuple>
#include <type_traits>

struct JObject;
namespace Game
{
	struct Controller;
};
struct MeshMaterial;
struct ScriptBinding;
using namespace v8;
using namespace Game;
namespace nov8
{
	using v8_get = std::function<void(Local<Name>, const PropertyCallbackInfo<Value>&)>;
	using v8_set = std::function<void(Local<Name>, Local<Value>, const PropertyCallbackInfo<void>&)>;
	using v8_idx_get = std::function<void(uint32_t, const PropertyCallbackInfo<Value>&)>;
	using v8_idx_set = std::function<void(uint32_t, Local<Value>, const PropertyCallbackInfo<Value>&)>;
	using v8_idx_qry = std::function<void(uint32_t, const PropertyCallbackInfo<Integer>&)>;
	using v8_idx_enum = std::function<void(const PropertyCallbackInfo<Array>&)>;
	using v8_accessor = std::tuple<v8_get, v8_set, Global<Object>>;
	using v8_function = std::function<void(const FunctionCallbackInfo<Value>&)>;
	using v8_idx_handler = std::tuple<v8_idx_get, v8_idx_set, v8_idx_qry, v8_idx_enum>;

	using v8_att_templates = std::map<std::string, Global<ObjectTemplate>>;
	using v8_att_accessors = std::map<std::string, std::unique_ptr<v8_accessor>>;
	using v8_att_functions = std::map<std::string, v8_function>;
	using v8_att_idx_handlers = std::map<std::string, std::unique_ptr<v8_idx_handler>>;
	struct v8_att_context
	{
		v8_att_templates att_templates;
		v8_att_accessors att_accessors;
		v8_att_functions att_functions;
		v8_att_idx_handlers att_idx_handlers;
	};
	using v8_template_attribute = std::function<void(Isolate*, Global<ObjectTemplate>&, v8_att_context&, JObject&, std::string, std::string)>;
	using v8_context_attribute = std::function<void(Isolate* isolate, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)>;
	using v8_templates_creators = std::map<std::string, v8_template_attribute>;
	using v8_functions_creators = std::map<std::string, v8_function>;
	using v8_context_creators = std::map<std::string, v8_context_attribute>;

	// thanks google, keep the vibes alive
	// Estructura base para extraer tipos de una función/lambda
	template <typename T>
	struct v8_lambda_traits : v8_lambda_traits<decltype(&T::operator())> {};

	// Especialización para el operador de llamada del lambda
	template <typename C, typename R, typename... Args>
	struct v8_lambda_traits<R(C::*)(Args...) const> {
		using ReturnType = R;
		using ArgsTuple = std::tuple<Args...>; // Los tipos de los argumentos guardados en una tupla
		static constexpr size_t ArgCount = sizeof...(Args); // Cuántos argumentos hay
	};

	// Versión no-const (por si usas lambdas mutables)
	template <typename C, typename R, typename... Args>
	struct v8_lambda_traits<R(C::*)(Args...)> {
		using ReturnType = R;
		using ArgsTuple = std::tuple<Args...>;
		static constexpr size_t ArgCount = sizeof...(Args);
	};

	//console.log
	void ConsoleLog(const FunctionCallbackInfo<Value>& info);
	void AddConsoleToContext(Isolate* isolate, Local<Context> context);
	void AddTemplateJsonAttributes(Isolate* isolate, Global<ObjectTemplate>& tmpl, v8_att_context& att_context, v8_templates_creators& attributeCreator, JObject& json, std::string path);
	void AddTemplateFunctions(Isolate* isolate, Global<ObjectTemplate>& tmpl, v8_att_context& att_context, v8_functions_creators& functionsCreator, JObject& json, std::string objectName, std::string path);
	void AddContextJsonAttributes(Isolate* isolate, Local<Context> context, v8_att_context& att_context, v8_context_creators& attributeCreator, JObject& json, std::string path);

	//utils
	Local<Name> v8_name(Isolate* isolate, std::string name);
	std::string v8_name(Isolate* isolate, Local<Name> name);
	Local<String> v8_string(Isolate* isolate, std::string str);
	Local<External> v8_external(Isolate* isolate, void* value);
	Local<Value> v8_json_parse(Isolate* isolate, nlohmann::json& json);
	v8_get v8_fixed_size(int size);
	// thanks google, keep the vibes alive
	template <typename T>
	T v8_to_cpp(v8::Isolate* isolate, v8::Local<v8::Value> val) {
		auto context = isolate->GetCurrentContext();

		if constexpr (std::is_same_v<T, int>) {
			return val->Int32Value(context).FromMaybe(0);
		}
		else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
			return static_cast<T>(val->NumberValue(context).FromMaybe(0.0));
		}
		else if constexpr (std::is_same_v<T, bool>) {
			return val->BooleanValue(isolate);
		}
		else if constexpr (std::is_same_v<T, std::string>) {
			v8::String::Utf8Value utf8(isolate, val);
			return std::string(*utf8 ? *utf8 : "");
		}
		// Si necesitas tus propios objetos (ej. JObject*), añádelos aquí
		return T{};
	}
	template <typename Tuple, size_t... Is>
	Tuple v8_to_tuple(const v8::FunctionCallbackInfo<v8::Value>& info, std::index_sequence<Is...>) {
		v8::Isolate* isolate = info.GetIsolate();

		// std::tuple_element_t<Is, Tuple> obtiene el tipo exacto (int, string, etc.) 
		// que el lambda espera en la posición Is.
		return std::make_tuple(
			v8_to_cpp<std::tuple_element_t<Is, Tuple>>(isolate, info[Is])...
		);
	}

	//accessors
	void v8_getter(Local<Name> property, const PropertyCallbackInfo<Value>& info);
	void v8_setter(Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info);

	//functions
	void v8_call_function(const FunctionCallbackInfo<Value>&);
	// thanks google, keep the vibes alive
	template <typename F>
	v8_function v8_wrap_call(F&& func) {
		// 1. Deducir tipos del lambda (usando los Traits que definimos antes)
		using Traits = v8_lambda_traits<std::decay_t<F>>;
		using ArgsTuple = typename Traits::ArgsTuple;

		// 2. Retornar el std::function que V8 espera
		// Capturamos el lambda por valor [func] para que viva dentro del std::function
		return [func = std::forward<F>(func)](const FunctionCallbackInfo<Value>& info) {
			Isolate* isolate = info.GetIsolate();

			// 3. Validar argumentos (opcional pero recomendado)
			if (info.Length() < Traits::ArgCount) {
				isolate->ThrowException(String::NewFromUtf8Literal(isolate, "Faltan argumentos"));
				return;
			}

			// 4. Convertir argumentos de V8 a Tupla y ejecutar
			auto args = v8_to_tuple<ArgsTuple>(info, std::make_index_sequence<Traits::ArgCount>{});
			std::apply(func, args);
			};
	}
	void AddFunctionToTemplate(Isolate* isolate, Global<ObjectTemplate>& tmpl, v8_att_functions& att_functions, std::string path, std::string functionName, v8_function func);
	void AddFunctionToTemplate(Isolate* isolate, Global<ObjectTemplate>& tmpl, v8_att_functions& att_functions, std::string path, std::string attribute, std::string functionName, v8_function func);
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
	v8_get v8_get_json_controller(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute);
	v8_get v8_get_json_scriptbinding(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, unsigned int idx);

	//std::set
	template<typename T>
	inline v8_function v8_set_erase(size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute) { return [=](const FunctionCallbackInfo<Value>&) {}; }
	template<typename T>
	inline v8_function v8_set_insert(size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute) { return [=](const FunctionCallbackInfo<Value>&) {}; }
	template<typename T>
	inline v8_function v8_set_clear(size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](const FunctionCallbackInfo<Value>& args)
			{
				v8::Isolate* isolate = args.GetIsolate();

				if (!json->contains(attribute)) return;
				json->at(attribute).clear();
				jobject->dirty(flag);
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
	inline v8_function v8_set_erase<std::set<int>>(size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
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
				jobject->dirty(flag);
			};
	}
	template<>
	inline v8_function v8_set_insert<std::set<int>>(size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
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
				jobject->dirty(flag);
			};
	}
	//std::vector
	template<typename T>
	inline v8_function v8_vector_erase(size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute) { return [=](const FunctionCallbackInfo<Value>&) {}; }
	template<typename T>
	inline v8_function v8_vector_push_back(size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute) { return [=](const FunctionCallbackInfo<Value>&) {}; }
	template<typename T>
	inline v8_function v8_vector_clear(size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](const FunctionCallbackInfo<Value>& args)
			{
				v8::Isolate* isolate = args.GetIsolate();

				if (!json->contains(attribute)) return;
				json->at(attribute).clear();
				jobject->dirty(flag);
			};
	}
	template<typename T>
	inline v8_function v8_vector_size(nlohmann::json* json, std::string attribute)
	{
		return [=](const FunctionCallbackInfo<Value>& args)
			{
				if (!json->contains(attribute)) return;
				v8::Isolate* isolate = args.GetIsolate();
				args.GetReturnValue().Set(Integer::New(isolate, static_cast<int>(json->at(attribute).size())));
			};
	}
	template<>
	inline v8_function v8_vector_erase<std::vector<std::string>>(size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](const FunctionCallbackInfo<Value>& args)
			{
				v8::Isolate* isolate = args.GetIsolate();

				if (args.Length() == 0 || !args[0]->IsInt32())
					return;

				int index = args[0]->Int32Value(isolate->GetCurrentContext()).FromJust();

				if (!json->contains(attribute)) return;
				json->at(attribute).erase(index);
				jobject->dirty(flag);
			};
	}
	template<>
	inline v8_function v8_vector_push_back<std::vector<std::string>>(size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](const FunctionCallbackInfo<Value>& args)
			{
				v8::Isolate* isolate = args.GetIsolate();

				if (args.Length() == 0 || !args[0]->IsString())
					return;
				if (!json->contains(attribute)) return;
				std::string value = v8_name(isolate, args[0]->ToString(isolate->GetCurrentContext()).ToLocalChecked());
				json->at(attribute).push_back(value);
				jobject->dirty(flag);
			};
	}
	//indexed
	void v8_idx_getter(uint32_t index, const PropertyCallbackInfo<Value>& info);
	void v8_idx_setter(uint32_t index, Local<Value> value, const PropertyCallbackInfo<Value>& info);
	void v8_idx_query(uint32_t index, const PropertyCallbackInfo<Integer>& info);
	void v8_idx_enumerator(const PropertyCallbackInfo<Array>& info);

	//accessors
	template<typename T>
	inline v8_get v8_get_json(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute) { return nullptr; }
	template<typename T>
	inline v8_set v8_set_json(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute) { return nullptr; }
	template<typename T>
	inline v8_get v8_get_json(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, unsigned int idx) { return nullptr; }
	template<typename T>
	inline v8_set v8_set_json(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, unsigned int idx) { return nullptr; }
	//accessors std::string
	template<>
	inline v8_get v8_get_json<std::string>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, const PropertyCallbackInfo<Value>& info)
			{
				info.GetReturnValue().Set(String::NewFromUtf8(info.GetIsolate(), std::string(json->at(attribute)).c_str()).ToLocalChecked());
			};
	}
	template<>
	inline v8_set v8_set_json<std::string>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info)
			{
				if (value->IsString()) {
					String::Utf8Value utf8(info.GetIsolate(), value);
					json->at(attribute) = std::string(*utf8);
				}
				info.GetReturnValue().Set(value);
				jobject->dirty(flag);
			};
	}
	//accessors std::vector<std::string>
	template<>
	inline v8_get v8_get_json<std::vector<std::string>>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=, &att_context](Local<Name> property, const PropertyCallbackInfo<Value>& info)
			{
				v8_att_functions& att_functions = att_context.att_functions;

				Isolate* isolate = info.GetIsolate();
				Local<Context> context = isolate->GetCurrentContext();

				Local<Array> my_array = Array::New(isolate, 0);

				AddFunctionToObject(isolate, context, att_functions, path, "erase", v8_vector_erase<std::vector<std::string>>(flag, jobject, json, attribute), my_array);
				AddFunctionToObject(isolate, context, att_functions, path, "push_back", v8_vector_push_back<std::vector<std::string>>(flag, jobject, json, attribute), my_array);
				AddFunctionToObject(isolate, context, att_functions, path, "clear", v8_vector_clear<std::vector<std::string>>(flag, jobject, json, attribute), my_array);
				AddFunctionToObject(isolate, context, att_functions, path, "size", v8_vector_size<std::vector<std::string>>(json, attribute), my_array);

				info.GetReturnValue().Set(my_array);
			};
	}
	template<>
	inline v8_set v8_set_json<std::vector<std::string>>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info)
			{
				if (!value->IsArray()) return;

				nlohmann::json& jarr = json->at(attribute);
				jarr.clear();
				Local<Array> arr = value.As<Array>();
				for (uint32_t i = 0; i < arr->Length(); i++) {
					Local<Value> item;
					if (arr->Get(info.GetIsolate()->GetCurrentContext(), i).ToLocal(&item)) {
						std::string s = v8_name(info.GetIsolate(), item->ToString(info.GetIsolate()->GetCurrentContext()).ToLocalChecked());
						jarr.push_back(s);
					}
				}
				jobject->dirty(flag);
			};
	}
	//accessors bool
	template<>
	inline v8_get v8_get_json<bool>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, const PropertyCallbackInfo<Value>& info)
			{
				info.GetReturnValue().Set(v8::Boolean::New(info.GetIsolate(), static_cast<bool>(json->at(attribute))));
			};
	}
	template<>
	inline v8_set v8_set_json<bool>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info)
			{
				json->at(attribute) = value->BooleanValue(info.GetIsolate());
				jobject->dirty(flag);
			};
	}
	//accessors int
	template<>
	inline v8_get v8_get_json<int>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, const PropertyCallbackInfo<Value>& info)
			{
				info.GetReturnValue().Set(Integer::New(info.GetIsolate(), static_cast<int>(json->at(attribute))));
			};
	}
	template<>
	inline v8_set v8_set_json<int>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info)
			{
				json->at(attribute) = value->Int32Value(info.GetIsolate()->GetCurrentContext()).ToChecked();
				jobject->dirty(flag);
			};
	}
	//accessors std::set<int>
	template<>
	inline v8_get v8_get_json<std::set<int>>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=, &att_context](Local<Name> property, const PropertyCallbackInfo<Value>& info)
			{
				v8_att_functions& att_functions = att_context.att_functions;

				Isolate* isolate = info.GetIsolate();
				Local<Context> context = isolate->GetCurrentContext();

				Local<Array> my_array = Array::New(isolate, 0);

				AddFunctionToObject(isolate, context, att_functions, path, "erase", v8_set_erase<std::set<int>>(flag, jobject, json, attribute), my_array);
				AddFunctionToObject(isolate, context, att_functions, path, "insert", v8_set_insert<std::set<int>>(flag, jobject, json, attribute), my_array);
				AddFunctionToObject(isolate, context, att_functions, path, "clear", v8_set_clear<std::set<int>>(flag, jobject, json, attribute), my_array);
				AddFunctionToObject(isolate, context, att_functions, path, "size", v8_set_size<std::set<int>>(json, attribute), my_array);

				info.GetReturnValue().Set(my_array);
			};
	}
	template<>
	inline v8_set v8_set_json<std::set<int>>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
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
				jobject->dirty(flag);
			};
	}
	//accessors unsigned int
	template<>
	inline v8_get v8_get_json<unsigned int>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, const PropertyCallbackInfo<Value>& info)
			{
				info.GetReturnValue().Set(Integer::NewFromUnsigned(info.GetIsolate(), static_cast<int>(json->at(attribute))));
			};
	}
	template<>
	inline v8_set v8_set_json<unsigned int>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info)
			{
				json->at(attribute) = value->Uint32Value(info.GetIsolate()->GetCurrentContext()).ToChecked();
				jobject->dirty(flag);
			};
	}
	//accessors float
	template<>
	inline v8_get v8_get_json<float>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, const PropertyCallbackInfo<Value>& info)
			{
				info.GetReturnValue().Set(Number::New(info.GetIsolate(), static_cast<float>(json->at(attribute))));
			};
	}
	template<>
	inline v8_set v8_set_json<float>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info)
			{
				json->at(attribute) = static_cast<float>(value->NumberValue(info.GetIsolate()->GetCurrentContext()).FromMaybe(0.0));
				jobject->dirty(flag);
			};
	}
	//accessors XMFLOAT3
	template<>
	inline v8_get v8_get_json<XMFLOAT3>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, const PropertyCallbackInfo<Value>& info)
			{
				Isolate* isolate = info.GetIsolate();
				v8_accessor* accessor = static_cast<v8_accessor*>(Local<External>::Cast(info.Data())->Value());
				Global<Object>& global_data = std::get<2>(*accessor);

				if (!global_data.IsEmpty()) {
					// Esta es la conversión que le faltaba a tu código:
					Local<Object> local_data = Local<Object>::New(isolate, global_data);
					info.GetReturnValue().Set(local_data);
				}
				else {
					// Si el global está vacío, devolvemos null o undefined
					info.GetReturnValue().SetNull();
				}
				//Local<Object>& data = std::get<2>(*static_cast<v8_accessor*>(Local<External>::Cast(info.Data())->Value()));
				//info.GetReturnValue().Set(data);
			};
	}
	template<>
	inline v8_set v8_set_json<XMFLOAT3>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
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
				jobject->dirty(flag);
			};
	}
	//accessors MeshMaterial
	template<>
	inline v8_get v8_get_json<MeshMaterial>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
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
	inline v8_set v8_set_json<MeshMaterial>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info)
			{

			};
	}
	//accessor Controller
	template<>
	inline v8_get v8_get_json<Game::Controller>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return v8_get_json_controller(att_context, path, flag, jobject, json, attribute);
	}
	template<>
	inline v8_set v8_set_json<Game::Controller>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute) { return nullptr; }
	//accessor ScriptBinding
	template<>
	inline v8_get v8_get_json<ScriptBinding>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, unsigned int idx)
	{
		return v8_get_json_scriptbinding(att_context, path, flag, jobject, json, idx);
	}
	template<>
	inline v8_set v8_set_json<ScriptBinding>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, unsigned int idx) { return nullptr; }

	//accessors creator
	template<typename T>
	inline v8_accessor v8_create_accessor(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute, Local<Object> object = Local<Object>())
	{
		v8::Isolate* isolate = v8::Isolate::GetCurrent();

		// 2. Creamos el Global vacío
		v8::Global<v8::Object> global_obj;

		// 3. Si nos pasaron un objeto local, lo hacemos persistente
		if (!object.IsEmpty()) {
			global_obj.Reset(isolate, object);
		}

		// 4. Devolvemos la tupla con el Global (coincidiendo con el nuevo alias)
		return std::make_tuple(
			v8_get_json<T>(att_context, path, flag, jobject, json, attribute),
			v8_set_json<T>(att_context, path, flag, jobject, json, attribute),
			std::move(global_obj) // Movemos el global a la tupla
		);
	}

	template<typename T>
	inline v8_accessor v8_create_accessor(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, unsigned int idx, Local<Object> object = Local<Object>())
	{
		v8::Isolate* isolate = v8::Isolate::GetCurrent();

		// 2. Creamos el Global vacío
		v8::Global<v8::Object> global_obj;

		// 3. Si nos pasaron un objeto local, lo hacemos persistente
		if (!object.IsEmpty()) {
			global_obj.Reset(isolate, object);
		}

		// 4. Devolvemos la tupla con el Global (coincidiendo con el nuevo alias)
		return std::make_tuple(
			v8_get_json<T>(att_context, path, flag, jobject, json, idx),
			v8_set_json<T>(att_context, path, flag, jobject, json, idx),
			std::move(global_obj) // Movemos el global a la tupla
		);
	}

	template<typename T>
	inline v8_idx_get v8_idx_get_json(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute) { return nullptr; }
	template<typename T>
	inline v8_idx_set v8_idx_set_json(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute) { return nullptr; }
	template<typename T>
	inline v8_idx_qry v8_idx_query_json(v8_att_context& att_context, std::string path, nlohmann::json* json, std::string attribute) { return nullptr; }
	template<typename T>
	inline v8_idx_enum v8_idx_enumerator_json(v8_att_context& att_context, std::string path, nlohmann::json* json, std::string attribute) { return nullptr; }

	//indexed XMFLOAT3
	template<>
	inline v8_idx_get v8_idx_get_json<XMFLOAT3>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return [=](uint32_t index, const PropertyCallbackInfo<Value>& info)
			{
				if (index > 3) return;
				info.GetReturnValue().Set(Number::New(info.GetIsolate(), static_cast<float>(json->at(attribute).at(index))));
			};
	}
	template<>
	inline v8_idx_set v8_idx_set_json<XMFLOAT3>(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
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
	inline v8_idx_handler v8_create_idx_handler(v8_att_context& att_context, std::string path, size_t flag, JObject* jobject, nlohmann::json* json, std::string attribute)
	{
		return std::make_tuple(
			v8_idx_get_json<T>(att_context, path, flag, jobject, json, attribute),
			v8_idx_set_json<T>(att_context, path, flag, jobject, json, attribute),
			v8_idx_query_json<T>(att_context, path, json, attribute),
			v8_idx_enumerator_json<T>(att_context, path, json, attribute)
		);
	}

	//these functions are crafted for JObject derivations for the Local<ObjectTemplate>
	template<typename T, size_t flag>
	struct v8_template
	{
		inline static void json_attribute(Isolate* isolate, Global<ObjectTemplate>& tmpl, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
		{
			v8_att_accessors& att_accessors = att_context.att_accessors;
			std::string jptr = path + "/" + attribute;

			att_accessors.insert_or_assign(jptr, std::make_unique<v8_accessor>(
				v8_create_accessor<T>(att_context, jptr, flag, &json, &json, attribute)
			));

			Local<ObjectTemplate> jtmpl = Local<ObjectTemplate>::New(isolate, tmpl);
			jtmpl->SetAccessor(v8_name(isolate, attribute), v8_getter, v8_setter, v8_external(isolate, att_accessors.at(jptr).get()));
		};
		inline static void json_enum_attribute(Isolate* isolate, Global<ObjectTemplate>& tmpl, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
		{
			v8_template<std::string, flag>::json_attribute(isolate, tmpl, att_context, json, path, attribute);
		}
		inline static void json_set_attribute(Isolate* isolate, Global<ObjectTemplate>& tmpl, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
		{
			v8_att_accessors& att_accessors = att_context.att_accessors;
			std::string jptr = path + "/" + attribute;

			att_accessors.insert_or_assign(jptr, std::make_unique<v8_accessor>(
				v8_create_accessor<std::set<T>>(att_context, jptr, flag, &json, &json, attribute))
			);
			Local<ObjectTemplate> jtmpl = Local<ObjectTemplate>::New(isolate, tmpl);
			jtmpl->SetAccessor(v8_name(isolate, attribute), v8_getter, v8_setter, v8_external(isolate, att_accessors.at(jptr).get()));
		}
		inline static void json_vector_attribute(Isolate* isolate, Global<ObjectTemplate>& tmpl, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
		{
			v8_att_accessors& att_accessors = att_context.att_accessors;
			std::string jptr = path + "/" + attribute;

			att_accessors.insert_or_assign(jptr, std::make_unique<v8_accessor>(
				v8_create_accessor<std::vector<T>>(att_context, jptr, flag, &json, &json, attribute))
			);
			Local<ObjectTemplate> jtmpl = Local<ObjectTemplate>::New(isolate, tmpl);
			jtmpl->SetAccessor(v8_name(isolate, attribute), v8_getter, v8_setter, v8_external(isolate, att_accessors.at(jptr).get()));
		}
		inline static void json_object_attribute(Isolate* isolate, Global<ObjectTemplate>& tmpl, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
		{
			v8_att_accessors& att_accessors = att_context.att_accessors;
			std::string jptr = path + "/" + attribute;
			att_accessors.insert_or_assign(jptr, std::make_unique<v8_accessor>(
				v8_create_accessor<T>(att_context, jptr, flag, &json, &json, attribute))
			);
			Local<ObjectTemplate> jtmpl = Local<ObjectTemplate>::New(isolate, tmpl);
			jtmpl->SetAccessor(v8_name(isolate, attribute), v8_getter, v8_setter, v8_external(isolate, att_accessors.at(jptr).get()));
		}
	};

	template<size_t flag>
	struct v8_template<XMFLOAT3, flag>
	{
		inline static void json_attribute(Isolate* isolate, Global<ObjectTemplate>& tmpl, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
		{
			HandleScope handle_scope(isolate);

			v8_att_idx_handlers& att_idx_handlers = att_context.att_idx_handlers;
			v8_att_functions& att_functions = att_context.att_functions;
			v8_att_templates& att_templates = att_context.att_templates;
			v8_att_accessors& att_accessors = att_context.att_accessors;

			std::string jptr = path + "/" + attribute;

			att_idx_handlers.insert_or_assign(jptr, std::make_unique<v8_idx_handler>(
				v8_create_idx_handler<XMFLOAT3>(att_context, jptr, flag, &json, &json, attribute))
			);

			Local<ObjectTemplate> xmf3_idx_tmpl = ObjectTemplate::New(isolate);
			xmf3_idx_tmpl->SetIndexedPropertyHandler(v8_idx_getter, v8_idx_setter, v8_idx_query, nullptr, v8_idx_enumerator, v8_external(isolate, att_idx_handlers.at(jptr).get()));

			std::string jptr_length = jptr + "/length";
			att_accessors.insert_or_assign(jptr_length, std::make_unique<v8_accessor>(
				std::make_tuple(v8_fixed_size(3), nullptr, Global<Object>()))
			);
			xmf3_idx_tmpl->SetAccessor(v8_name(isolate, "length"), v8_getter, v8_setter, v8_external(isolate, att_accessors.at(jptr_length).get()));

			att_templates[jptr].Reset(isolate, xmf3_idx_tmpl);
			AddFunctionToTemplate(isolate, att_templates.at(jptr), att_functions, path, attribute, "toJSON", v8_toJSON(&json, attribute));

			//Local<ObjectTemplate> parent_tmpl = Local<ObjectTemplate>::New(isolate, tmpl);
			//parent_tmpl->SetAccessor(
			//	v8_name(isolate, attribute),
			//	v8_getter,
			//	v8_setter,
			//	v8_external(isolate, att_accessors.at(jptr).get()) // Necesitas haber creado un accessor para 'attribute' antes
			//);
		}
	};

	template<size_t flag>
	struct v8_template<MeshMaterial, flag>
	{
		inline static void json_attribute(Isolate* isolate, Global<ObjectTemplate>& tmpl, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
		{
			v8_att_templates& att_templates = att_context.att_templates;
			v8_att_accessors& att_accessors = att_context.att_accessors;
			v8_att_functions& att_functions = att_context.att_functions;

			//:/meshMaterial
			std::string jptr = path + "/" + attribute;
			Local<ObjectTemplate> meshMaterial_tmpl = Local<ObjectTemplate>::New(isolate, tmpl);
			att_templates[jptr].Reset(isolate, meshMaterial_tmpl);
			att_accessors.insert_or_assign(jptr, std::make_unique<v8_accessor>(
				v8_create_accessor<MeshMaterial>(att_context, jptr, flag, &json, &json, attribute))
			);

			//:/meshMaterial/material
			std::string material_jptr = jptr + "/material";
			att_accessors.insert_or_assign(material_jptr, std::make_unique<v8_accessor>(
				v8_create_accessor<MeshMaterial>(att_context, material_jptr, flag, &json, &json.at(attribute), "material"))
			);
			meshMaterial_tmpl->SetAccessor(v8_name(isolate, "material"), v8_getter, v8_setter, v8_external(isolate, att_accessors.at(material_jptr).get()));
			AddFunctionToTemplate(isolate, att_templates.at(jptr), att_functions, jptr, "material", "toJSON", v8_toJSON(&json, "material"));

			//:/meshMaterial/mesh
			std::string mesh_jptr = jptr + "/mesh";
			Local<ObjectTemplate> meshMaterial_mesh_tmpl = ObjectTemplate::New(isolate);
			att_templates[mesh_jptr].Reset(isolate, meshMaterial_mesh_tmpl);
			att_accessors.insert_or_assign(mesh_jptr, std::make_unique<v8_accessor>(
				v8_create_accessor<MeshMaterial>(att_context, mesh_jptr, flag, &json, &json.at(attribute), "mesh"))
			);

			//:/meshMaterial/mesh/primitive
			std::string primitive_jptr = mesh_jptr + "/primitive";
			att_accessors.insert_or_assign(primitive_jptr, std::make_unique<v8_accessor>(
				v8_create_accessor<MeshMaterial>(att_context, primitive_jptr, flag, &json, &json.at(attribute).at("mesh"), "primitive"))
			);
			meshMaterial_mesh_tmpl->SetAccessor(v8_name(isolate, "primitive"), v8_getter, v8_setter, v8_external(isolate, att_accessors.at(primitive_jptr).get()));
			AddFunctionToTemplate(isolate, att_templates.at(jptr), att_functions, mesh_jptr, "primitive", "toJSON", v8_toJSON(&json.at(attribute).at("mesh"), "primitive"));
		}
	};

	template<size_t flag>
	struct v8_template<Game::Controller, flag>
	{
		inline static void json_object_attribute(Isolate* isolate, Global<ObjectTemplate>& tmpl, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
		{
			v8_att_templates& att_templates = att_context.att_templates;
			v8_att_accessors& att_accessors = att_context.att_accessors;
			v8_att_functions& att_functions = att_context.att_functions;

			//:/controllers
			std::string jptr = path + "/" + attribute;
			Local<ObjectTemplate> controllers_tmpl = ObjectTemplate::New(isolate);
			att_templates[jptr].Reset(isolate, controllers_tmpl);
			att_accessors.insert_or_assign(jptr, std::make_unique<v8_accessor>(
				v8_create_accessor<Game::Controller>(att_context, jptr, flag, &json, &json, attribute))
			);

			//:/controllers/[controller_name]
			for (auto& [name, uuid] : json.at(attribute).items())
			{
				std::string controller_jptr = jptr + "/" + name;
				Local<ObjectTemplate> controller_inst_tmpl = ObjectTemplate::New(isolate);
				att_templates[controller_jptr].Reset(isolate, controller_inst_tmpl);
				att_accessors.insert_or_assign(controller_jptr, std::make_unique<v8_accessor>(
					v8_create_accessor<Game::Controller>(att_context, controller_jptr, flag, &json, &json.at(attribute), name))
				);
			}
		}
	};

	template<size_t flag>
	struct v8_template<ScriptBinding, flag>
	{
		inline static void json_vector_attribute(Isolate* isolate, Global<ObjectTemplate>& tmpl, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
		{
			v8_att_templates& att_templates = att_context.att_templates;
			v8_att_accessors& att_accessors = att_context.att_accessors;

			//:[bindingName]
			for (unsigned int i = 0; i < json.at(attribute).size(); i++)
			{
				ScriptBinding sb(json.at(attribute).at(i));
				std::string binding_jptr = path + "/" + attribute + "/" + sb.bindingName;
				Local<ObjectTemplate> binding_tmpl = ObjectTemplate::New(isolate);
				att_templates[binding_jptr].Reset(isolate, binding_tmpl);
				att_accessors.insert_or_assign(binding_jptr, std::make_unique<v8_accessor>(
					v8_create_accessor<ScriptBinding>(att_context, binding_jptr, flag, &json, &json.at(attribute), i))
				);
			}
		}
	};


	//these functions are crafted for JObject derivations for the Local<Object>
	template<typename T, size_t flag>
	struct v8_context
	{
		inline static void json_attribute(Isolate* isolate, v8_att_context& att_context, JObject& json, std::string path, std::string attribute) {}
		inline static void json_enum_attribute(Isolate* isolate, v8_att_context& att_context, JObject& json, std::string path, std::string attribute) {}
		inline static void json_set_attribute(Isolate* isolate, v8_att_context& att_context, JObject& json, std::string path, std::string attribute) {}
		inline static void json_vector_attribute(Isolate* isolate, v8_att_context& att_context, JObject& json, std::string path, std::string attribute) {}
		inline static void json_object_attribute(Isolate* isolate, v8_att_context& att_context, JObject& json, std::string path, std::string attribute) {}
	};

	template<size_t flag>
	struct v8_context<XMFLOAT3, flag>
	{
		inline static void json_attribute(Isolate* isolate, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
		{
			v8_att_templates& att_templates = att_context.att_templates;
			v8_att_accessors& att_accessors = att_context.att_accessors;
			std::string jptr = path + "/" + attribute;

			v8::Local<v8::ObjectTemplate> local_tpl = v8::Local<v8::ObjectTemplate>::New(isolate, att_templates.at(jptr));
			Local<Object> inst = local_tpl->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
			Local<Array> dummyArray = Array::New(isolate, 0);
			inst->SetPrototype(isolate->GetCurrentContext(), dummyArray);

			att_accessors.insert_or_assign(jptr, std::make_unique<v8_accessor>(
				v8_create_accessor<XMFLOAT3>(att_context, jptr, flag, &json, &json, attribute, inst))
			);
			isolate->GetCurrentContext()->Global()->SetAccessor(isolate->GetCurrentContext(), v8_name(isolate, attribute), v8_getter, v8_setter, v8_external(isolate, att_accessors.at(jptr).get()));
		}
	};

	template<size_t flag>
	struct v8_context<MeshMaterial, flag>
	{
		inline static void json_attribute(Isolate* isolate, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
		{
			v8_att_templates& att_templates = att_context.att_templates;
			v8_att_accessors& att_accessors = att_context.att_accessors;

			//:/meshMaterial
			std::string jptr = path + "/" + attribute;
			Local<ObjectTemplate> local_jtpl = Local<ObjectTemplate>::New(isolate, att_templates.at(jptr));
			Local<Object> inst = local_jtpl->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
			att_accessors.insert_or_assign(jptr, std::make_unique<v8_accessor>(
				v8_create_accessor<MeshMaterial>(att_context, jptr, flag, &json, &json, attribute, inst))
			);
			isolate->GetCurrentContext()->Global()->SetAccessor(isolate->GetCurrentContext(), v8_name(isolate, attribute), v8_getter, v8_setter, v8_external(isolate, att_accessors.at(jptr).get()));

			//:/meshMaterial/mesh
			std::string mesh_jptr = jptr + "/mesh";
			Local<ObjectTemplate> local_mesh_jtpl = Local<ObjectTemplate>::New(isolate, att_templates.at(mesh_jptr));
			Local<Object> mesh_inst = local_mesh_jtpl->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
			att_accessors.insert_or_assign(mesh_jptr, std::make_unique<v8_accessor>(
				v8_create_accessor<MeshMaterial>(att_context, mesh_jptr, flag, &json, &json, attribute, mesh_inst))
			);
			inst->SetAccessor(isolate->GetCurrentContext(), v8_name(isolate, "mesh"), v8_getter, v8_setter, v8_external(isolate, att_accessors.at(mesh_jptr).get()));
		}
	};

	template<size_t flag>
	struct v8_context<Game::Controller, flag>
	{
		inline static void json_object_attribute(Isolate* isolate, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
		{
			v8_att_templates& att_templates = att_context.att_templates;
			v8_att_accessors& att_accessors = att_context.att_accessors;

			//:/controllers
			std::string jptr = path + "/" + attribute;
			Local<ObjectTemplate> local_jtpl = Local<ObjectTemplate>::New(isolate, att_templates.at(jptr));
			Local<Object> inst = local_jtpl->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
			att_accessors.insert_or_assign(jptr, std::make_unique<v8_accessor>(
				v8_create_accessor<Game::Controller>(att_context, jptr, flag, &json, &json, attribute, inst))
			);
			isolate->GetCurrentContext()->Global()->SetAccessor(isolate->GetCurrentContext(), v8_name(isolate, attribute), v8_getter, v8_setter, v8_external(isolate, att_accessors.at(jptr).get()));

			//:/controllers/[controller_name]
			for (auto& [name, uuid] : json.at(attribute).items())
			{
				std::string controller_jptr = jptr + "/" + name;
				Local<ObjectTemplate> controller_jtpl = Local<ObjectTemplate>::New(isolate, att_templates.at(controller_jptr));
				Local<Object> controller_inst = controller_jtpl->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
				att_accessors.insert_or_assign(controller_jptr, std::make_unique<v8_accessor>(
					v8_create_accessor<Game::Controller>(att_context, controller_jptr, flag, &json, &json, attribute, controller_inst))
				);
				inst->SetAccessor(isolate->GetCurrentContext(), v8_name(isolate, name), v8_getter, v8_setter, v8_external(isolate, att_accessors.at(controller_jptr).get()));
			}
		}
	};

	template<size_t flag>
	struct v8_context<ScriptBinding, flag>
	{
		inline static void json_vector_attribute(Isolate* isolate, v8_att_context& att_context, JObject& json, std::string path, std::string attribute)
		{
			v8_att_templates& att_templates = att_context.att_templates;
			v8_att_accessors& att_accessors = att_context.att_accessors;

			//:/[bindingName]
			for (unsigned int i = 0; i < json.at(attribute).size(); i++)
			{
				ScriptBinding sb(json.at(attribute).at(i));
				std::string binding_jptr = path + "/" + attribute + "/" + sb.bindingName;

				Local<ObjectTemplate> binding_jtpl = Local<ObjectTemplate>::New(isolate, att_templates.at(binding_jptr));
				Local<Object> inst = binding_jtpl->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
				att_accessors.insert_or_assign(binding_jptr, std::make_unique<v8_accessor>(
					v8_create_accessor<ScriptBinding>(att_context, binding_jptr, flag, &json, &json.at(attribute), i, inst))
				);
				isolate->GetCurrentContext()->Global()->SetAccessor(isolate->GetCurrentContext(), v8_name(isolate, sb.bindingName), v8_getter, v8_setter, v8_external(isolate, att_accessors.at(binding_jptr).get()));
			}
		}
	};
}