#pragma once

#include <v8.h>
#include <libplatform/libplatform.h>
#include <nlohmann/json.hpp>
#include <set>
#include <tuple>
#include <type_traits>
#include <Material/DepthStencilDesc.h>
#include <Camera/Projections/Orthographic.h>
#include <Camera/Projections/Perspective.h>

struct JObject;
struct ControllerBinding;
namespace Game
{
	struct Controller;
};
namespace Scene
{
	class SceneUnit;
	struct SceneObject;
};
namespace Physics
{
	struct PhysicObject;
};
struct SceneUnitScripting;
struct MeshMaterial;
struct ScriptBinding;
using namespace v8;
using namespace Game;
using namespace Scene;
using namespace Physics;
using namespace Scene::CameraProjections;
namespace nov8
{
	using v8_function = std::function<void(const FunctionCallbackInfo<Value>&)>;

	template <typename T>
	struct v8_lambda_traits : v8_lambda_traits<decltype(&T::operator())> {};

	template <typename ClassType, typename ReturnType, typename... Args>
	struct v8_lambda_traits<ReturnType(ClassType::*)(Args...) const> {
		//decrease by 1 as the first argument is a T* self
		static constexpr size_t arg_count = sizeof...(Args) - 1;
		using args_tuple = std::tuple<Args...>;
	};

	template <typename ClassType, typename ReturnType, typename... Args>
	struct v8_lambda_traits<ReturnType(ClassType::*)(Args...) > {
		//decrease by 1 as the first argument is a T* self
		static constexpr size_t arg_count = sizeof...(Args) - 1;
		using args_tuple = std::tuple<Args...>;
	};

	//console.log
	void v8_console_log(const FunctionCallbackInfo<Value>& info);
	void AddConsoleToContext(Isolate* isolate, Local<Context> context);
	//require
	void v8_native_require(const v8::FunctionCallbackInfo<v8::Value>& args);
	std::string ReadScriptFile(std::filesystem::path filename);
	void AddRequireToContext(Isolate* isolate, Local<Context> context);

	//utils
	Local<Value> v8_json_parse(Isolate* isolate, nlohmann::json& json);
	Local<Name> v8_name(Isolate* isolate, std::string name);
	std::string v8_name(Isolate* isolate, Local<Name> name);
	Local<String> v8_string(Isolate* isolate, std::string str);
	Local<External> v8_external(Isolate* isolate, void* value);
	void v8_report_exception(Isolate* isolate, TryCatch* try_catch);

	// thanks google, keep the vibes alive
	template <typename T>
	T v8_to_cpp(Isolate* isolate, Local<Value> val) {
		auto context = isolate->GetCurrentContext();

		if constexpr (std::is_same_v<T, int>) {
			return val->Int32Value(context).FromMaybe(0);
		}
		else if constexpr (std::is_same_v<T, uint32_t>) {
			return val->Uint32Value(context).FromMaybe(0);
		}
		else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
			return static_cast<T>(val->NumberValue(context).FromMaybe(0.0));
		}
		else if constexpr (std::is_same_v<T, bool>) {
			return val->BooleanValue(isolate);
		}
		else if constexpr (std::is_same_v<T, std::string>) {
			String::Utf8Value utf8(isolate, val);
			return std::string(*utf8 ? *utf8 : "");
		}
		else if constexpr (std::is_pointer_v<T>) {
			if (val->IsObject()) {
				Local<Object> obj = val.As<Object>();
				//return the internal field if it exists
				if (obj->InternalFieldCount() > 0) {
					return static_cast<T>(obj->GetAlignedPointerFromInternalField(0));
				}
			}
			return nullptr;
		}
		else
		{
			//add other types here if needed
			static_assert(sizeof(T) == 0, "v8_to_cpp: unsupported type");
			return T{};
		}
	}

	template <typename T, typename Tuple, size_t... Is>
	Tuple v8_to_tuple_with_self(T* self, const FunctionCallbackInfo<Value>& info, std::index_sequence<Is...>) {
		Isolate* isolate = info.GetIsolate();

		// std::make_tuple gets:
		// 0: as 'self' pointer
		// 1...N: the result of v8_to_cpp' info[0...N-1]
		return std::make_tuple(
			self,
			v8_to_cpp<std::tuple_element_t<Is + 1, Tuple>>(isolate, info[Is])...
		);
	}

	template <typename Tuple, size_t... Is>
	Tuple v8_to_tuple(const FunctionCallbackInfo<Value>& info, std::index_sequence<Is...>) {
		Isolate* isolate = info.GetIsolate();

		// std::tuple_element_t<Is, Tuple> obtains the exact type(int, string, etc.)
		// which the lambda needs in the position Is.
		return std::make_tuple(
			v8_to_cpp<std::tuple_element_t<Is, Tuple>>(isolate, info[Is])...
		);
	}

	using PropertyGetterFn = std::function<Local<Value>(Isolate* isolate, SceneUnitScripting* script, nlohmann::json& jdata)>;
	using PropertySetterFn = std::function<void(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata)>;

	template<typename T>
	struct V8Converter
	{
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, nlohmann::json& jdata) { return v8::Null(isolate); }
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata) {}
	};

	template<typename E>
	struct V8ConverterEnum
	{
		static PropertyGetterFn Read(std::unordered_map<std::string, E>& StoE)
		{
			return [=](Isolate* isolate, SceneUnitScripting* script, nlohmann::json& jdata) -> Local<Value>
				{
					if (jdata.is_string()) return v8_string(isolate, jdata.get<std::string>().c_str());
					return v8::Null(isolate);
				};
		}
		static PropertySetterFn Update(std::unordered_map<std::string, E>& StoE)
		{
			return [=](Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata)
				{
					if (!value->IsString())
						return;

					v8::String::Utf8Value utf8(isolate, value);
					std::string s(*utf8);
					if (StoE.contains(s))
					{
						jdata = s;
					}
				};
		}
	};

	template<>
	struct V8Converter<nlohmann::json> {
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& j);
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata) {}
	};

	//XMFLOAT3
	template<>
	struct V8Converter<DirectX::XMFLOAT3> {
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& jdata);
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata);
	};

	//XMFLOAT4
	template<>
	struct V8Converter<DirectX::XMFLOAT4> {
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& jdata);
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata);
	};

	//float
	template<>
	struct V8Converter<float> {
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& jdata);
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata);
	};

	//int
	template<>
	struct V8Converter<int> {
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, nlohmann::json& jdata);
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata);
	};

	//std::set<int>
	template<>
	struct V8Converter<std::set<int>>
	{
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, nlohmann::json& jdata);
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata);
	};

	//unsigned int
	template<>
	struct V8Converter<unsigned int> {
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, nlohmann::json& jdata);
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata);
	};

	//std::set<unsigned int>
	template<>
	struct V8Converter<std::set<unsigned int>>
	{
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, nlohmann::json& jdata);
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata);
	};

	//bool
	template<>
	struct V8Converter<bool> {
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, nlohmann::json& jdata);
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata);
	};

	//std::string
	template<>
	struct V8Converter<std::string> {
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, nlohmann::json& jdata);
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata);
	};

	//std::vector<std::string>
	template<>
	struct V8Converter<std::vector<std::string>> {
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& jdata);
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata);
	};

	//std::set<std::string>
	template<>
	struct V8Converter<std::set<std::string>> {
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& jdata);
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata);
	};

	//MeshMaterial
	template<>
	struct V8Converter<MeshMaterial>
	{
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, nlohmann::json& jdata);
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata);
	};

	template<>
	struct V8Converter<Game::Controller>
	{
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& j);
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata);
	};

	template<>
	struct V8Converter<std::vector<PhysicObject*>>
	{
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& j);
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata) {}
	};

	template<>
	struct V8Converter<DepthStencilDesc> {
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& j);
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata);
	};

	template<>
	struct V8Converter<Perspective> {
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& j);
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata);
	};

	template<>
	struct V8Converter<Orthographic> {
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& j);
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata);
	};

	template<>
	struct V8Converter<ControllerBinding> {
		static Local<Value> Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& j);
		static void Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata);
	};

	struct JPropertyMeta
	{
		PropertyGetterFn getter;
		PropertySetterFn setter;
		size_t dirtyFlag;
	};

	struct V8PropertyProxy {
		JObject* owner;            // Renderable/Light/Controller/etc.
		std::string jsonPath;      // Ex:"/meshMaterial/mesh" (JSON pointer format)
		size_t dirtyFlag;          // T::Update* flag
		SceneUnitScripting* script;
	};

	template <typename Func>
	struct MethodPayload {
		Func callback;
		SceneUnitScripting* script;
	};

	template <typename F>
	v8_function v8_wrap_call(F&& func)
	{
		using Traits = v8_lambda_traits<std::decay_t<F>>;
		using ArgsTuple = typename Traits::ArgsTuple;
		using ReturnType = typename Traits::ReturnType;

		return [func = std::forward<F>(func)](const FunctionCallbackInfo<Value>& info)
			{
				Isolate* isolate = info.GetIsolate();

				if (info.Length() < Traits::ArgCount) {
					isolate->ThrowException(v8_string(isolate, "Invalid number of arguments"));
					return;
				}

				auto args = v8_to_tuple<ArgsTuple>(info, std::make_index_sequence<Traits::ArgCount>{});

				if constexpr (std::is_same_v<ReturnType, void>) {
					std::apply(func, args);
				}
				else
				{
					//Use the V8Converter to return the result to JS
					ReturnType result = std::apply(func, args);
					//Use the V8Converter based from ReturnType
					info.GetReturnValue().Set(V8Converter<ReturnType>::Read(isolate, nullptr, result));
				}
			};
	}

	template <typename T, typename Func, typename Traits, size_t... Is>
	void v8_call_lambda(Func* func, T* self, const v8::FunctionCallbackInfo<v8::Value>& info, std::index_sequence<Is...>, SceneUnitScripting* script) {
		//get the whole arguments tuple
		using FullTuple = typename Traits::args_tuple;

		//call the lambda
		(*func)(
			self, //argument 0 is always a pointer to self
			//for each Is (0, 1, 2... N-1), get the corresponding JS argument
			([&](auto index_const) {
				//conver the index into a compile time constant
				constexpr size_t JS_Index = decltype(index_const)::value;

				//the argument in the lambda is in the JS_Index+1(0 is T*)
				using ArgType = std::decay_t<std::tuple_element_t<JS_Index + 1, FullTuple>>;

				nlohmann::json j;
				//use the system to extract the V8 data to a temporal JSON
				nov8::V8Converter<ArgType>::Update(info.GetIsolate(), info[JS_Index], script, j);

				//return the converted value from the JSON to the type of the lambda
				return j.get<ArgType>();
				}(std::integral_constant<size_t, Is>{}))...
			);
	}

	template <typename T, typename Func>
	void v8_register_method(Isolate* isolate, Local<ObjectTemplate> tpl, const char* name, SceneUnitScripting* script, Func&& callback) {
		using Traits = v8_lambda_traits<Func>;

		// pack the lambda and the script
		auto* payload = new MethodPayload<Func>{ std::forward<Func>(callback), script };

		tpl->Set(isolate, name, FunctionTemplate::New(isolate, [](const FunctionCallbackInfo<Value>& info) {
			auto* p = static_cast<MethodPayload<Func>*>(info.Data().As<External>()->Value());
			T* self = static_cast<T*>(info.This()->GetAlignedPointerFromInternalField(0));

			//Validate that in JS the passed arguments are the expected by the lambda(without the self)
			if (info.Length() < (int)Traits::arg_count) {
				info.GetIsolate()->ThrowException(v8::String::NewFromUtf8Literal(info.GetIsolate(), "Missing arguments from JS"));
				return;
			}

			v8_call_lambda<T, Func, Traits>(
				&p->callback,
				self,
				info,
				std::make_index_sequence<Traits::arg_count>{},
				p->script
			);
			}, External::New(isolate, payload)));
	}

	//container getters
	void v8_scene_container_getter(Local<Name> property, const PropertyCallbackInfo<Value>& info);
	void v8_scene_container_enumerator(const PropertyCallbackInfo<Array>& info);
	//jobject getters/setters
	void v8_jobject_getter(Local<Name> property, const PropertyCallbackInfo<Value>& info);
	void v8_jobject_setter(Local<Name> property, Local<Value> value, const PropertyCallbackInfo<Value>& info);
	void v8_jobject_enumerator(const PropertyCallbackInfo<Array>& info);
	//nlohmann::json proxy getters/setters
	void v8_proxy_getter(Local<Name> property, const PropertyCallbackInfo<Value>& info);
	void v8_proxy_setter(Local<Name> property, Local<Value> value, const PropertyCallbackInfo<Value>& info);
	void v8_proxy_enumerator(const PropertyCallbackInfo<Array>& info);

	template <typename T, typename F>
	v8_function v8_wrap_method_call(F&& func) {
		using Traits = v8_lambda_traits<std::decay_t<F>>;
		using ArgsTuple = typename Traits::ArgsTuple;
		using ReturnType = typename Traits::ReturnType;

		return [func = std::forward<F>(func)](const FunctionCallbackInfo<Value>& info)
			{
				Isolate* isolate = info.GetIsolate();
				Local<Context> context = isolate->GetCurrentContext();

				//get the real instance from C++. 'this' from javascript
				T* self = static_cast<T*>(info.This()->GetAlignedPointerFromInternalField(0));
				if (!self) {
					isolate->ThrowException(v8_string(isolate, "Internal pointer is null"));
					return;
				}

				//validate the arguments to match with JS(ArgCount-1)
				if (info.Length() < (Traits::ArgCount - 1)) {
					isolate->ThrowException(v8_string(isolate, "Faltan argumentos en la función"));
					return;
				}

				//inject the arguments to a tuple (self:0, info[Is-1]:1...N-1)
				auto args = v8_to_tuple_with_self<T, ArgsTuple>(
					self,
					info,
					std::make_index_sequence<Traits::ArgCount - 1>{}
				);

				//run the func with the arguments and return
				if constexpr (std::is_same_v<ReturnType, void>)
				{
					std::apply(func, args);
				}
				else
				{
					ReturnType result = std::apply(func, args);
					//use V8Converter to return the value to JS
					info.GetReturnValue().Set(V8Converter<ReturnType>::Read(isolate, nullptr, result));
				}
			};
	}
}