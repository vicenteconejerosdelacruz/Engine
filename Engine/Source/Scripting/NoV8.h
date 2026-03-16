#include <v8.h>
#include <libplatform/libplatform.h>
#include <v8pp/context.hpp>
#include <v8pp/module.hpp>
#include <nlohmann/json.hpp>

struct MeshMaterial;
using namespace v8;
namespace nov8
{
	//console.log
	void ConsoleLog(const FunctionCallbackInfo<Value>& info);
	void AddConsoleToContext(Isolate* isolate, Local<Context> context);

	//utils
	Local<Name> v8_name(Isolate* isolate, std::string name);
	std::string v8_name(Isolate* isolate, Local<Name> name);
	Local<String> v8_string(Isolate* isolate, std::string str);
	Local<External> v8_external(Isolate* isolate, void* value);
	Local<Value> v8_json_parse(Isolate* isolate, nlohmann::json& json);

	//json
	using v8_to_json = std::function<void(const FunctionCallbackInfo<Value>&)>;
	using v8_att_to_jsons = std::map<std::string, v8_to_json>;

	void v8_toJSON(const FunctionCallbackInfo<Value>&);
	v8_to_json v8_toJSON(nlohmann::json* json);
	v8_to_json v8_toJSON(nlohmann::json* json, std::string attribute);
	void AddToJsonToTemplate(Isolate* isolate, Local<ObjectTemplate>& tmpl, std::string path, v8_att_to_jsons& att_to_jsons, nlohmann::json& json);
	void AddToJsonToTemplate(Isolate* isolate, Local<ObjectTemplate>& tmpl, std::string path, v8_att_to_jsons& att_to_jsons, nlohmann::json& json, std::string attribute);

	//indexed
	using v8_idx_get = std::function<void(uint32_t, const PropertyCallbackInfo<Value>&)>;
	using v8_idx_set = std::function<void(uint32_t, Local<Value>, const PropertyCallbackInfo<Value>&)>;
	using v8_idx_qry = std::function<void(uint32_t, const PropertyCallbackInfo<Integer>&)>;
	using v8_idx_enum = std::function<void(const PropertyCallbackInfo<Array>&)>;
	using v8_idx_handler = std::tuple<v8_idx_get, v8_idx_set, v8_idx_qry, v8_idx_enum>;
	using v8_att_idx_handlers = std::map<std::string, v8_idx_handler>;

	void v8_idx_getter(uint32_t index, const PropertyCallbackInfo<Value>& info);
	void v8_idx_setter(uint32_t index, Local<Value> value, const PropertyCallbackInfo<Value>& info);
	void v8_idx_query(uint32_t index, const PropertyCallbackInfo<Integer>& info);
	void v8_idx_enumerator(const PropertyCallbackInfo<Array>& info);

	template<typename T>
	v8_idx_get v8_idx_get_json(nlohmann::json* json, std::string attribute) { return nullptr; }
	template<typename T>
	v8_idx_set v8_idx_set_json(nlohmann::json* json, std::string attribute) { return nullptr; }
	template<typename T>
	v8_idx_qry v8_idx_query_json(nlohmann::json* json, std::string attribute) { return nullptr; }
	template<typename T>
	v8_idx_enum v8_idx_enumerator_json(nlohmann::json* json, std::string attribute) { return nullptr; }

	template<typename T>
	v8_idx_handler v8_create_idx_handler(nlohmann::json* json, std::string attribute)
	{
		return std::make_tuple(
			v8_idx_get_json<T>(json, attribute),
			v8_idx_set_json<T>(json, attribute),
			v8_idx_query_json<T>(json, attribute),
			v8_idx_enumerator_json<T>(json, attribute)
		);
	}

	//indexed XMFLOAT3
	template<>
	v8_idx_get v8_idx_get_json<XMFLOAT3>(nlohmann::json* json, std::string attribute);
	template<>
	v8_idx_set v8_idx_set_json<XMFLOAT3>(nlohmann::json* json, std::string attribute);
	template<>
	v8_idx_qry v8_idx_query_json<XMFLOAT3>(nlohmann::json* json, std::string attribute);
	template<>
	v8_idx_enum v8_idx_enumerator_json<XMFLOAT3>(nlohmann::json* json, std::string attribute);

	//accessors
	using v8_get = std::function<void(Local<Name>, const PropertyCallbackInfo<Value>&)>;
	using v8_set = std::function<void(Local<Name>, Local<Value>, const PropertyCallbackInfo<void>&)>;
	using v8_accessor = std::tuple<v8_get, v8_set, Local<Object>>;
	using v8_att_accessors = std::map<std::string, v8_accessor>;

	void v8_getter(Local<Name> property, const PropertyCallbackInfo<Value>& info);
	void v8_setter(Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info);

	template<typename T>
	v8_get v8_get_json(nlohmann::json* json, std::string attribute) { return nullptr; }
	template<typename T>
	v8_set v8_set_json(nlohmann::json* json, std::string attribute) { return nullptr; }

	template<typename T>
	v8_accessor v8_create_accessor(nlohmann::json* json, std::string attribute, Local<Object> object = Local<Object>())
	{
		return std::make_tuple(
			v8_get_json<T>(json, attribute),
			v8_set_json<T>(json, attribute),
			object
		);
	}

	//std::string
	template<>
	v8_get v8_get_json<std::string>(nlohmann::json* json, std::string attribute);
	template<>
	v8_set v8_set_json<std::string>(nlohmann::json* json, std::string attribute);
	//bool
	template<>
	v8_get v8_get_json<bool>(nlohmann::json* json, std::string attribute);
	template<>
	v8_set v8_set_json<bool>(nlohmann::json* json, std::string attribute);
	//int
	template<>
	v8_get v8_get_json<int>(nlohmann::json* json, std::string attribute);
	template<>
	v8_set v8_set_json<int>(nlohmann::json* json, std::string attribute);
	//unsigned int
	template<>
	v8_get v8_get_json<unsigned int>(nlohmann::json* json, std::string attribute);
	template<>
	v8_set v8_set_json<unsigned int>(nlohmann::json* json, std::string attribute);
	//float
	template<>
	v8_get v8_get_json<float>(nlohmann::json* json, std::string attribute);
	template<>
	v8_set v8_set_json<float>(nlohmann::json* json, std::string attribute);
	//accessors XMFLOAT3
	template<>
	v8_get v8_get_json<XMFLOAT3>(nlohmann::json* json, std::string attribute);
	template<>
	v8_set v8_set_json<XMFLOAT3>(nlohmann::json* json, std::string attribute);
	//accessors MeshMaterial
	template<>
	v8_get v8_get_json<MeshMaterial>(nlohmann::json* json, std::string attribute);
	template<>
	v8_set v8_set_json<MeshMaterial>(nlohmann::json* json, std::string attribute);

	using v8_att_templates = std::map<std::string, Local<ObjectTemplate>>;
	struct v8_att_context
	{
		v8_att_idx_handlers att_idx_handlers;
		v8_att_accessors att_accessors;
		v8_att_to_jsons att_to_jsons;
		v8_att_templates att_templates;
	};
};