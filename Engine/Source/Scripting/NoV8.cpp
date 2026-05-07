#include "pch.h"
#include "NoV8.h"
#include <Controller.h>
#include <PhysicObject.h>
#include <SceneObject.h>
#include "SceneUnitScripting.h"

using namespace Game;
using namespace v8;
namespace nov8
{
	void v8_console_log(const FunctionCallbackInfo<Value>& info)
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
		console->Set(context, v8::String::NewFromUtf8(isolate, "log").ToLocalChecked(), FunctionTemplate::New(isolate, v8_console_log)->GetFunction(context).ToLocalChecked()).Check();
		context->Global()->Set(context, v8::String::NewFromUtf8(isolate, "console").ToLocalChecked(), console).Check();
	}

	//utils
	Local<Value> v8_json_parse(Isolate* isolate, nlohmann::json& json)
	{
		Local<String> v8_json_str = v8_string(isolate, json.dump());
		Local<Value> json_object;
		if (JSON::Parse(isolate->GetCurrentContext(), v8_json_str).ToLocal(&json_object))
			return json_object;
		return Null(isolate);
	}
	Local<Name> v8_name(Isolate* isolate, std::string name)
	{
		return v8::String::NewFromUtf8(isolate, name.c_str()).ToLocalChecked();
	}

	std::string v8_name(Isolate* isolate, Local<Name> name)
	{
		Local<String> v8String = name->ToString(isolate->GetCurrentContext()).ToLocalChecked();
		v8::String::Utf8Value utf8(isolate, v8String);
		return *utf8;
	}

	Local<String> v8_string(Isolate* isolate, std::string str)
	{
		return v8::String::NewFromUtf8(isolate, str.c_str()).ToLocalChecked();
	}

	Local<External> v8_external(Isolate* isolate, void* value)
	{
		return External::New(isolate, value);
	}

	void v8_report_exception(Isolate* isolate, TryCatch* try_catch)
	{
		HandleScope handle_scope(isolate);
		Local<Context> context = isolate->GetCurrentContext();

		v8::String::Utf8Value exception(isolate, try_catch->Exception());
		const char* exception_str = *exception ? *exception : "Unknown exception";

		Local<Message> message = try_catch->Message();

		if (message.IsEmpty())
		{
			OutputDebugStringA(std::string("JS Error: " + std::string(exception_str) + "\n").c_str());
		}
		else
		{
			v8::String::Utf8Value filename(isolate, message->GetScriptOrigin().ResourceName());
			const char* filename_str = *filename ? *filename : "anonymous_script";

			int line_number = message->GetLineNumber(context).FromMaybe(-1);
			int start_column = message->GetStartColumn(context).FromMaybe(-1);

			OutputDebugStringA(std::string("JS Error en " + std::string(filename_str) + ":" + std::to_string(line_number) + ":" + std::to_string(start_column) + "\n").c_str());
			OutputDebugStringA(std::string("Message: " + std::string(exception_str) + "\n").c_str());

			// 3. Extraer la línea de código donde ocurrió el error (Source Line)
			v8::String::Utf8Value sourceline(isolate, message->GetSourceLine(context).ToLocalChecked());
			if (*sourceline)
			{
				OutputDebugStringA(std::string("Line: " + std::string(*sourceline) + "\n").c_str());;

				//Draw an arrow '^' pointing to the error
				for (int i = 0; i < start_column; i++) {
					OutputDebugStringA(" ");
				}
				OutputDebugStringA("^\n");
			}

			//Stack Trace
			Local<v8::Value> stack_trace_value;
			if (try_catch->StackTrace(context).ToLocal(&stack_trace_value) &&
				stack_trace_value->IsString())
			{
				v8::String::Utf8Value stack_trace(isolate, stack_trace_value);
				OutputDebugStringA(std::string("Stack Trace:\n" + std::string(*stack_trace) + "\n").c_str());
			}
		}
		OutputDebugStringA("--------------------------------------------------\n");
	}

	//container getters
	void v8_scene_container_getter(Local<Name> property, const PropertyCallbackInfo<Value>& info)
	{
		Isolate* isolate = info.GetIsolate();

		if (!property->IsString()) return;

		//get the scene unit pointer
		Local<Object> self = info.This();
		SceneUnit* scene = static_cast<SceneUnit*>(self->GetAlignedPointerFromInternalField(0));

		//get the type
		int typeInt = static_cast<int>(self->GetInternalField(1).As<v8::Integer>()->Value());
		SceneObjectType type = static_cast<SceneObjectType>(typeInt);

		//get the name
		v8::String::Utf8Value name(isolate, property);
		std::string objectName(*name);

		//get the scene object
		SceneObject* so = GetSceneObjectPointerByName(scene->Id(), type, objectName);

		if (so)
		{
			SceneUnitScripting* script = Scripting::GetSceneUnitScripting(scene->Id());
			info.GetReturnValue().Set(Scripting::WrapJObject(isolate, *script, so));
		}
	}

	void v8_scene_container_enumerator(const PropertyCallbackInfo<Array>& info)
	{
		Isolate* isolate = info.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();

		Local<Object> self = info.This();
		SceneUnit* scene = static_cast<SceneUnit*>(self->GetAlignedPointerFromInternalField(0));

		//get the type
		int typeInt = static_cast<int>(self->GetInternalField(1).As<v8::Integer>()->Value());
		SceneObjectType type = static_cast<SceneObjectType>(typeInt);

		auto names = GetSceneObjectsNames(scene->Id(), type);

		Local<Array> keys = Array::New(isolate, static_cast<int>(names.size()));
		int i = 0;
		for (auto& name : names)
		{
			keys->Set(context, i++, v8_string(isolate, name)).Check();
		}

		info.GetReturnValue().Set(keys);
	}

	//jobject getters/setters
	void v8_jobject_getter(Local<Name> property, const PropertyCallbackInfo<Value>& info)
	{
		v8::Isolate* isolate = info.GetIsolate();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		Local<External> data = info.Data().As<External>();
		SceneUnitScripting* script = static_cast<SceneUnitScripting*>(data->Value());

		//Get the JObject* from the field[0]
		JObject* jobj = static_cast<JObject*>(info.This()->GetAlignedPointerFromInternalField(0));
		std::string key = *v8::String::Utf8Value(isolate, property);

		if (!jobj) return;

		//if the jobject has metadata we use the metadata to return the getter/setter
		const JPropertyMeta* meta = jobj->GetMeta(key);
		if (meta) {
			nlohmann::json& jdata = (*jobj)[key];
			info.GetReturnValue().Set(meta->getter(isolate, script, jdata));
			return;
		}

		//if it's a function return the function
		v8::Local<v8::String> fnName = v8_string(isolate, key);
		v8::MaybeLocal<v8::Value> functionValue = info.This()->GetRealNamedProperty(context, fnName);
		if (!functionValue.IsEmpty()) {
			v8::Local<v8::Value> val = functionValue.ToLocalChecked();
			if (val->IsFunction()) {
				info.GetReturnValue().Set(val);
				return;
			}
		}

		//if the attribute can be obtained through nlohmann::json try to obtain it
		if (jobj->contains(key)) {
			nlohmann::json& node = (*jobj)[key];
			if (node.is_structured()) {
				//do a deep navigation creating the proxy, initial path is just the key
				info.GetReturnValue().Set(Scripting::WrapProxy(
					isolate, *script, jobj, key, 0 //0 as initial flag
				));
				return;
			}
			else {
				//simple value without metadata
				info.GetReturnValue().Set(V8Converter<nlohmann::json>::Read(isolate, script, node));
				return;
			}
		}

		info.GetReturnValue().SetUndefined();
	}

	void v8_jobject_setter(Local<Name> property, Local<Value> value, const PropertyCallbackInfo<Value>& info)
	{
		Local<External> data = info.Data().As<External>();
		SceneUnitScripting* script = static_cast<SceneUnitScripting*>(data->Value());

		Isolate* isolate = info.GetIsolate();
		JObject* jobj = static_cast<JObject*>(info.This()->GetAlignedPointerFromInternalField(0));
		std::string key = *v8::String::Utf8Value(isolate, property);

		//only set the value if the attribute exist on the meta
		const JPropertyMeta* meta = jobj->GetMeta(key);
		if (meta)
		{
			meta->setter(isolate, value, script, (*jobj)[key]);
			jobj->flag(meta->dirtyFlag);
			info.GetReturnValue().Set(value);
		}
	}

	void v8_jobject_enumerator(const PropertyCallbackInfo<Array>& info)
	{
		Isolate* isolate = info.GetIsolate();

		Local<Object> self = info.This();

		JObject* jobj = static_cast<JObject*>(self->GetAlignedPointerFromInternalField(0));

		if (!jobj)
			return;

		Local<Context> context = isolate->GetCurrentContext();
		Local<Array> keys = Array::New(isolate, static_cast<int>(jobj->size()));

		//iterate through all objects defined in the nlohmann::json iterators
		int i = 0;
		for (auto it = jobj->begin(); it != jobj->end(); it++)
		{
			Local<v8::String> v8Key = v8_string(isolate, it.key());
			keys->Set(context, i++, v8Key).Check();
		}
		info.GetReturnValue().Set(keys);
	}

	//nlohmann::json proxy getters/setters
	void v8_proxy_getter(Local<Name> property, const PropertyCallbackInfo<Value>& info)
	{
		Isolate* isolate = info.GetIsolate();

		//get the sceneUnitScripting for this
		Local<External> data = info.Data().As<External>();
		SceneUnitScripting* script = static_cast<SceneUnitScripting*>(data->Value());

		V8PropertyProxy* proxy = static_cast<V8PropertyProxy*>(info.This()->GetAlignedPointerFromInternalField(0));
		if (!proxy || !proxy->owner) return;

		std::string key = *v8::String::Utf8Value(isolate, property);

		//normalize the path so it correspond with a json_ptr from nlohmann::
		std::string currentPathSafe = Scripting::SafePath(proxy->jsonPath);
		nlohmann::json::json_pointer currentPtr(currentPathSafe);

		if (!proxy->owner->contains(currentPtr)) {
			info.GetReturnValue().SetUndefined();
			return;
		}
		nlohmann::json& currentNode = proxy->owner->at(currentPtr);

		//length it's a special case for the array types
		if (currentNode.is_array() && key == "length") {
			info.GetReturnValue().Set(v8::Integer::New(isolate, static_cast<int>(currentNode.size())));
			return;
		}

		//concatenate the key to the path creating the next pointer
		std::string nextPathSafe = currentPathSafe + "/" + key;
		nlohmann::json::json_pointer nextPtr(nextPathSafe);

		//verify and return the data
		if (proxy->owner->contains(nextPtr)) {
			nlohmann::json& node = proxy->owner->at(nextPtr);

			if (node.is_structured()) {
				//return a new proxy if it's an array or an object
				info.GetReturnValue().Set(Scripting::WrapProxy(
					isolate,
					*script,
					proxy->owner,
					nextPathSafe, // pass the path
					proxy->dirtyFlag
				));
			}
			else {
				//use the static converter for atomic values
				info.GetReturnValue().Set(V8Converter<nlohmann::json>::Read(isolate, script, node));
			}
		}
		else {
			//no attribute in the json
			info.GetReturnValue().SetUndefined();
		}
	}

	void v8_proxy_setter(Local<Name> property, Local<Value> value, const PropertyCallbackInfo<Value>& info)
	{
		Isolate* isolate = info.GetIsolate();
		V8PropertyProxy* proxy = static_cast<V8PropertyProxy*>(info.This()->GetAlignedPointerFromInternalField(0));

		std::string key = *v8::String::Utf8Value(isolate, property);
		nlohmann::json::json_pointer ptr(proxy->jsonPath + "/" + key);

		//write in the json
		V8Converter<nlohmann::json>::Update(isolate, value, nullptr, (*proxy->owner)[ptr]);

		//notify to the engine
		proxy->owner->flag(proxy->dirtyFlag);
	}

	void v8_proxy_enumerator(const PropertyCallbackInfo<Array>& info)
	{
		Isolate* isolate = info.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();

		V8PropertyProxy* proxy = static_cast<V8PropertyProxy*>(info.This()->GetAlignedPointerFromInternalField(0));
		if (!proxy || !proxy->owner) return;

		nlohmann::json::json_pointer ptr(proxy->jsonPath);
		if (!proxy->owner->contains(ptr)) return;

		nlohmann::json& currentNode = proxy->owner->at(ptr);

		//create the array with the size of the json node
		Local<v8::Array> keys = v8::Array::New(isolate, static_cast<int>(currentNode.size()));

		int i = 0;
		if (currentNode.is_array())
		{
			//for arrays we return the numeric values as strings
			for (i = 0; i < static_cast<int>(currentNode.size()); ++i)
			{
				keys->Set(context, i, v8_string(isolate, std::to_string(i).c_str())).Check();
			}
		}
		else if (currentNode.is_object())
		{
			//but for objects we return the json keys
			for (auto it = currentNode.begin(); it != currentNode.end(); ++it)
			{
				keys->Set(context, i++, v8_string(isolate, it.key().c_str())).Check();
			}
		}

		info.GetReturnValue().Set(keys);
	}

	Local<Value> V8Converter<nlohmann::json>::Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& j) {
		if (j.is_null()) return v8::Null(isolate);
		if (j.is_boolean()) return v8::Boolean::New(isolate, j.get<bool>());
		if (j.is_number_integer()) return v8::Integer::New(isolate, j.get<int>());
		if (j.is_number_float()) return v8::Number::New(isolate, j.get<double>());
		if (j.is_string()) return v8_string(isolate, j.get<std::string>().c_str());

		//because this is the converter a copy of the array will be returned
		if (j.is_array()) {
			Local<Context> context = isolate->GetCurrentContext();
			Local<v8::Array> v8Arr = v8::Array::New(isolate, (int)j.size());
			for (int i = 0; i < (int)j.size(); ++i) {
				v8Arr->Set(context, i, Read(isolate, script, j[i])).Check();
			}
			return v8Arr;
		}
		//same goes for the objects
		if (j.is_object()) {
			Local<Context> context = isolate->GetCurrentContext();
			Local<v8::Object> v8Obj = v8::Object::New(isolate);
			for (auto it = j.begin(); it != j.end(); ++it) {
				v8Obj->Set(context, v8_string(isolate, it.key().c_str()), Read(isolate, script, it.value())).Check();
			}
			return v8Obj;
		}

		return v8::Undefined(isolate);
	}

	Local<Value> V8Converter<DirectX::XMFLOAT3>::Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& jdata) {
		Local<Context> ctx = isolate->GetCurrentContext();
		Local<Array> arr = Array::New(isolate, 3);

		for (uint32_t i = 0; i < 3; ++i) {
			float val = jdata.is_array() ? jdata[i].get<float>() : 0.0f;
			arr->Set(ctx, i, Number::New(isolate, val)).Check();
		}
		return arr;
	}

	void V8Converter<DirectX::XMFLOAT3>::Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata) {
		if (!value->IsArray()) return;
		Local<Array> arr = value.As<Array>();
		Local<Context> ctx = isolate->GetCurrentContext();
		for (uint32_t i = 0; i < 3 && i < arr->Length(); i++) {
			jdata[i] = static_cast<float>(arr->Get(ctx, i).ToLocalChecked()->NumberValue(ctx).FromMaybe(0.0f));
		}
	}

	Local<Value> V8Converter<DirectX::XMFLOAT4>::Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& jdata) {
		Local<Context> ctx = isolate->GetCurrentContext();
		Local<Array> arr = Array::New(isolate, 4);

		for (uint32_t i = 0; i < 4; ++i) {
			float val = jdata.is_array() ? jdata[i].get<float>() : 0.0f;
			arr->Set(ctx, i, Number::New(isolate, val)).Check();
		}
		return arr;
	}

	void V8Converter<DirectX::XMFLOAT4>::Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata) {
		if (!value->IsArray()) return;
		Local<Array> arr = value.As<Array>();
		Local<Context> ctx = isolate->GetCurrentContext();
		for (uint32_t i = 0; i < 4 && i < arr->Length(); i++) {
			jdata[i] = static_cast<float>(arr->Get(ctx, i).ToLocalChecked()->NumberValue(ctx).FromMaybe(0.0f));
		}
	}

	Local<Value> V8Converter<float>::Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& jdata) {
		return Number::New(isolate, jdata.get<float>());
	}

	void V8Converter<float>::Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata) {
		if (value->IsNumber()) jdata = value->NumberValue(isolate->GetCurrentContext()).FromMaybe(0.0);
	}

	Local<Value> V8Converter<int>::Read(Isolate* isolate, SceneUnitScripting* script, nlohmann::json& jdata)
	{
		return Integer::New(isolate, jdata.get<int>());
	}

	void V8Converter<int>::Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata) {
		if (value->IsInt32()) jdata = value->Int32Value(isolate->GetCurrentContext()).FromMaybe(0);
	}

	Local<Value> V8Converter<std::set<int>>::Read(Isolate* isolate, SceneUnitScripting* script, nlohmann::json& jdata)
	{
		Local<Context> ctx = isolate->GetCurrentContext();
		if (!jdata.is_array()) return Array::New(isolate, 0);

		Local<Array> v8Arr = Array::New(isolate, (int)jdata.size());
		int i = 0;
		for (auto& item : jdata) {
			v8Arr->Set(ctx, i++, Integer::New(isolate, item.get<int>())).Check();
		}
		return v8Arr;
	}

	void V8Converter<std::set<int>>::Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata)
	{
		if (!value->IsArray()) return;
		Local<Array> v8Arr = value.As<Array>();
		Local<Context> ctx = isolate->GetCurrentContext();

		std::set<int> tempSet;
		for (uint32_t i = 0; i < v8Arr->Length(); ++i) {
			tempSet.insert(v8Arr->Get(ctx, i).ToLocalChecked()->Int32Value(ctx).FromMaybe(0));
		}
		jdata = tempSet;
	}

	Local<Value> V8Converter<unsigned int>::Read(Isolate* isolate, SceneUnitScripting* script, nlohmann::json& jdata)
	{
		return Integer::NewFromUnsigned(isolate, jdata.get<unsigned int>());
	}

	void V8Converter<unsigned int>::Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata) {
		if (value->IsUint32()) jdata = value->Uint32Value(isolate->GetCurrentContext()).FromMaybe(0U);
	}

	Local<Value> V8Converter<std::set<unsigned int>>::Read(Isolate* isolate, SceneUnitScripting* script, nlohmann::json& jdata)
	{
		Local<Context> ctx = isolate->GetCurrentContext();
		if (!jdata.is_array()) return Array::New(isolate, 0);

		Local<Array> v8Arr = Array::New(isolate, (int)jdata.size());
		int i = 0;
		for (auto& item : jdata) {
			v8Arr->Set(ctx, i++, Integer::NewFromUnsigned(isolate, item.get<unsigned int>())).Check();
		}
		return v8Arr;
	}

	void V8Converter<std::set<unsigned int>>::Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata)
	{
		if (!value->IsArray()) return;
		Local<Array> v8Arr = value.As<Array>();
		Local<Context> ctx = isolate->GetCurrentContext();

		std::set<unsigned int> tempSet;
		for (uint32_t i = 0; i < v8Arr->Length(); ++i) {
			tempSet.insert(v8Arr->Get(ctx, i).ToLocalChecked()->Uint32Value(ctx).FromMaybe(0U));
		}
		jdata = tempSet;
	}

	Local<Value> V8Converter<bool>::Read(Isolate* isolate, SceneUnitScripting* script, nlohmann::json& jdata)
	{
		return Boolean::New(isolate, jdata.get<bool>());
	}

	void V8Converter<bool>::Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata) {
		if (value->IsBoolean()) jdata = value->BooleanValue(isolate);
	}

	Local<Value> V8Converter<std::string>::Read(Isolate* isolate, SceneUnitScripting* script, nlohmann::json& jdata)
	{
		return v8::String::NewFromUtf8(isolate, std::string(jdata.get<std::string>()).c_str()).ToLocalChecked();
	}

	void V8Converter<std::string>::Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata) {
		if (value->IsString())
		{
			v8::String::Utf8Value utf8(isolate, value);
			jdata = std::string(*utf8);
		}
	}

	Local<Value> V8Converter<std::vector<std::string>>::Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& jdata) {
		Local<Context> ctx = isolate->GetCurrentContext();

		// non array -> return empty array
		if (!jdata.is_array()) return Array::New(isolate, 0);

		Local<Array> v8Arr = Array::New(isolate, (int)jdata.size());
		for (uint32_t i = 0; i < jdata.size(); ++i) {
			std::string val = jdata[i].get<std::string>();
			v8Arr->Set(ctx, i, v8_string(isolate, val.c_str())).Check();
		}
		return v8Arr;
	}

	void V8Converter<std::vector<std::string>>::Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata) {
		if (!value->IsArray()) return;
		Local<Array> v8Arr = value.As<Array>();
		Local<Context> ctx = isolate->GetCurrentContext();

		//clean the array before inserting the new elements
		jdata = nlohmann::json::array();
		for (uint32_t i = 0; i < v8Arr->Length(); ++i) {
			v8::String::Utf8Value utf8(isolate, v8Arr->Get(ctx, i).ToLocalChecked());
			jdata.push_back(std::string(*utf8));
		}
	}

	Local<Value> V8Converter<std::set<std::string>>::Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& jdata) {
		Local<Context> ctx = isolate->GetCurrentContext();

		// non array -> return empty array
		if (!jdata.is_array()) return Array::New(isolate, 0);

		Local<Array> v8Arr = Array::New(isolate, (int)jdata.size());
		for (uint32_t i = 0; i < jdata.size(); ++i) {
			std::string val = jdata[i].get<std::string>();
			v8Arr->Set(ctx, i, v8_string(isolate, val.c_str())).Check();
		}
		return v8Arr;
	}

	void V8Converter<std::set<std::string>>::Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata) {
		if (!value->IsArray()) return;
		Local<Array> v8Arr = value.As<Array>();
		Local<Context> ctx = isolate->GetCurrentContext();

		//clean the array before inserting the new elements
		jdata = nlohmann::json::array();
		std::set<std::string> elements;
		for (uint32_t i = 0; i < v8Arr->Length(); ++i) {
			v8::String::Utf8Value utf8(isolate, v8Arr->Get(ctx, i).ToLocalChecked());
			elements.insert(std::string(*utf8));
		}
		jdata = elements;
	}

	Local<Value> V8Converter<MeshMaterial>::Read(Isolate* isolate, SceneUnitScripting* script, nlohmann::json& jdata)
	{
		Local<Context> ctx = isolate->GetCurrentContext();
		Local<Object> obj = Object::New(isolate);

		//read the material
		std::string mat = jdata.value("material", "");
		//set the material into the object
		obj->Set(ctx, v8_name(isolate, "material"), v8_string(isolate, mat.c_str())).Check();

		//if the mesh exists and it's and object
		if (jdata.contains("mesh") && jdata["mesh"].is_object())
		{
			//create the mesh object
			Local<Object> meshObj = Object::New(isolate);
			//read the primitive
			std::string prim = jdata["mesh"].value("primitive", "");
			//set the primitive into the mesh
			meshObj->Set(ctx, v8_name(isolate, "primitive"), v8_string(isolate, prim.c_str())).Check();
			//set the mesh into the object
			obj->Set(ctx, v8_name(isolate, "mesh"), meshObj).Check();
		}
		else
		{
			//as it doesn't exist just set the mesh as null
			obj->Set(ctx, v8_name(isolate, "mesh"), Null(isolate)).Check();
		}
		return obj;
	}

	void V8Converter<MeshMaterial>::Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata)
	{
		if (!value->IsObject()) return;
		Local<Object> v8Obj = value.As<Object>();
		Local<Context> ctx = isolate->GetCurrentContext();

		//update the material
		Local<Value> matVal = v8Obj->Get(ctx, v8_name(isolate, "material")).ToLocalChecked();
		if (matVal->IsString()) {
			v8::String::Utf8Value utf8(isolate, matVal);
			jdata["material"] = std::string(*utf8);
		}
		else if (matVal->IsNull() || matVal->IsUndefined()) {
			jdata["material"] = ""; //empty if is an undefined value
		}

		//update the mesh
		Local<Value> meshVal = v8Obj->Get(ctx, v8_name(isolate, "mesh")).ToLocalChecked();
		if (meshVal->IsObject()) {
			Local<Object> mObj = meshVal.As<Object>();
			Local<Value> primVal = mObj->Get(ctx, v8_name(isolate, "primitive")).ToLocalChecked();

			if (primVal->IsString()) {
				v8::String::Utf8Value utf8(isolate, primVal);
				jdata["mesh"]["primitive"] = std::string(*utf8);
			}
		}
		else if (meshVal->IsNull()) {
			jdata["mesh"] = nullptr;
		}
	}

	Local<Value> V8Converter<Game::Controller>::Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& j)
	{
		Local<Context> context = isolate->GetCurrentContext();
		Local<v8::Object> v8Controllers = v8::Object::New(isolate);

		for (auto it = j.begin(); it != j.end(); ++it)
		{
			std::string uuid = it.value().get<std::string>();

			//get the controller by it's uuid
			Game::Controller* controller = Game::GetController(uuid).get();

			if (controller) {
				//if the controller exists we set it as a wraped object
				v8Controllers->Set(context,
					v8_string(isolate, it.key().c_str()),
					Scripting::WrapJObject(isolate, *script, controller)
				).Check();
			}
			else {
				//if the controller doesn't exists just put the UUID
				v8Controllers->Set(context,
					v8_string(isolate, it.key().c_str()),
					v8_string(isolate, uuid.c_str())
				).Check();
			}
		}

		return v8Controllers;
	}

	void V8Converter<Game::Controller>::Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata)
	{
		return V8Converter<nlohmann::json>::Update(isolate, value, script, jdata);
	}

	Local<Value> V8Converter<std::vector<PhysicObject*>>::Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& j)
	{
		EscapableHandleScope scope(isolate);
		Local<Context> context = isolate->GetCurrentContext();

		if (!j.is_array()) return scope.Escape(v8::Null(isolate));

		//create the array
		Local<v8::Array> v8Array = v8::Array::New(isolate, static_cast<int>(j.size()));

		for (int i = 0; i < (int)j.size(); ++i)
		{
			PhysicObject* physObj = Physics::GetPhysicObject(j.at(i)).get();

			if (physObj) {
				//wrap the physic object in the array slot
				v8Array->Set(context, i, Scripting::WrapJObject(isolate, *script, physObj)).Check();
			}
			else {
				v8Array->Set(context, i, v8::Null(isolate)).Check();
			}
		}

		return scope.Escape(v8Array);
	}

	Local<Value> V8Converter<DepthStencilDesc>::Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& j)
	{
		//the generic converter will work as to_json already exists. nlohmann::json(d) will do it automatically
		return V8Converter<nlohmann::json>::Read(isolate, script, j);
	}

	void V8Converter<DepthStencilDesc>::Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata)
	{
		if (!value->IsObject()) return;

		//convert the V8 object to nlohmann::json
		nlohmann::json jTemp;
		V8Converter<nlohmann::json>::Update(isolate, value, script, jTemp);

		//use from_json to automatically validate the DepthStencilDesc
		DepthStencilDesc desc;
		from_json(jTemp, desc);

		//write the jdata with the DepthStencilDesc data
		to_json(jdata, desc);
	}

	Local<Value> V8Converter<Perspective>::Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& j)
	{
		//the generic converter will work as to_json already exists. nlohmann::json(d) will do it automatically
		return V8Converter<nlohmann::json>::Read(isolate, script, j);
	}

	void V8Converter<Perspective>::Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata)
	{
		if (!value->IsObject()) return;

		//convert the V8 object to nlohmann::json
		nlohmann::json jTemp;
		V8Converter<nlohmann::json>::Update(isolate, value, script, jTemp);

		//use from_json to automatically validate the Perspective
		Perspective p;
		from_json(jTemp, p);

		//write the jdata with the Perspective data
		to_json(jdata, p);
	}

	Local<Value> V8Converter<Orthographic>::Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& j)
	{
		//the generic converter will work as to_json already exists. nlohmann::json(d) will do it automatically
		return V8Converter<nlohmann::json>::Read(isolate, script, j);
	}

	void V8Converter<Orthographic>::Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata)
	{
		if (!value->IsObject()) return;

		//convert the V8 object to nlohmann::json
		nlohmann::json jTemp;
		V8Converter<nlohmann::json>::Update(isolate, value, script, jTemp);

		//use from_json to automatically validate the Orthographic
		Orthographic p;
		from_json(jTemp, p);

		//write the jdata with the Orthographic data
		to_json(jdata, p);
	}

	Local<Value> V8Converter<ControllerBinding>::Read(Isolate* isolate, SceneUnitScripting* script, const nlohmann::json& j)
	{
		//the generic converter will work as to_json already exists. nlohmann::json(d) will do it automatically
		return V8Converter<nlohmann::json>::Read(isolate, script, j);
	}

	void V8Converter<ControllerBinding>::Update(Isolate* isolate, Local<Value> value, SceneUnitScripting* script, nlohmann::json& jdata)
	{
		if (!value->IsObject()) return;

		//convert the V8 object to nlohmann::json
		nlohmann::json jTemp;
		V8Converter<nlohmann::json>::Update(isolate, value, script, jTemp);

		//use from_json to automatically validate the ControllerBinding
		ControllerBinding p;
		from_json(jTemp, p);

		//write the jdata with the ControllerBinding data
		to_json(jdata, p);
	}
}