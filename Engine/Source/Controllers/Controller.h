#pragma once
#include <memory>
#include <string>
#include <vector>
#include <UUID.h>
#include <JObject.h>
#include <JTypes.h>
#include <nlohmann/json.hpp>
#include <StepTimer.h>

namespace Game
{
	struct Controller : JObject
	{
		virtual ~Controller() = default;
		Controller(nlohmann::json& json);
		virtual void Map(SUUUID so);
		virtual void Unmap();
		virtual void Step(float delta) {};
		virtual void BindToV8Context(v8pp::context& context) {}
#if defined(_EDITOR)
		virtual std::map<std::string, JEdvEditorDrawerFunction> GetControllerDrawers() { return {}; }
		virtual std::vector<std::pair<std::string, JsonToEditorValueType>> GetControllerAttributes() { return {}; }
#endif

		JUUID controller;
		SceneUnitId unit;
		SUUUID sceneObject;
	};

	JUUID RegisterController(std::string controllerName, SUUUID sceneObject, std::unique_ptr<Controller>& controller);
	void MapControllers(SceneUnitId id);
	std::unique_ptr<Controller>& GetController(JUUID uuid);
	std::unique_ptr<Controller>& GetControllerBySceneObjectUUID(SUUUID uuid);
	std::unique_ptr<Controller>& GetControllerByName(std::string name);
	//void DestroyControllers();
	void DestroyController(JUUID uuid);
	void StepControllers(DX::StepTimer& timer);
	void BindToV8Context(v8pp::context& context, SUUUID uuid);

	extern std::vector<std::string> GetControllers();
	extern JUUID CreateController(std::string name, SUUUID sceneObject, nlohmann::json& json);

	template<typename T>
	T* ContextController()
	{
		v8::Isolate* isolate = v8::Isolate::GetCurrent();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();

		v8::Context::Scope context_scope(context);
		v8::HandleScope handle_scope(isolate);

		v8::Local<v8::Object> global = context->Global();

		v8::Local<v8::String> key = v8::String::NewFromUtf8(isolate, "uuid", v8::NewStringType::kNormal).ToLocalChecked();
		v8::Local<v8::Value> value_js = global->Get(context, key).ToLocalChecked();
		v8::String::Utf8Value utf8_value(isolate, value_js);
		std::string uuid(*utf8_value);

		T* controller = static_cast<T*>(GetController(uuid).get());
		return controller;
	}
};
