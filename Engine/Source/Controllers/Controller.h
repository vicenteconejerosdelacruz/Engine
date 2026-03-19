#pragma once

#include <memory>
#include <string>
#include <vector>
#include <UUID.h>
#include <JObject.h>
#include <JTypes.h>
#include <nlohmann/json.hpp>
#include <StepTimer.h>
#include <NoV8.h>

#define DECL_CONTROLLER_DRAWER(JClass,ParentJClass)\
virtual std::map<std::string, JEdvEditorDrawerFunction> GetControllerDrawers()\
{\
	std::map<std::string, JEdvEditorDrawerFunction> drawers = ParentJClass::GetControllerDrawers();\
	drawers.merge(Get##JClass##Drawers());\
	return drawers;\
}\
virtual std::vector<std::pair<std::string, JsonToEditorValueType>> GetControllerAttributes()\
{\
	std::vector<std::pair<std::string, JsonToEditorValueType>> attributes = ParentJClass::GetControllerAttributes();\
	auto atts = Get##JClass##Attributes();\
	attributes.insert(attributes.end(), atts.begin(), atts.end());\
	return attributes;\
}

using namespace nov8;
namespace Game
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <ControllerAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <ControllerAtt.h>
#include <JEnd.h>

#endif
	struct Controller : JObject
	{
#include <Attributes/JFlags.h>
#include <ControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <ControllerAtt.h>
#include <JEnd.h>

		virtual ~Controller() = default;
		Controller(nlohmann::json& json);
		virtual void JUpdate(nlohmann::json p);
		virtual void JPatch(nlohmann::json p);
		virtual void SetInitialConditions() {};
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
		virtual std::map<std::string, JEdvEditorDrawerFunction> GetControllerDrawers() { return Game::GetControllerDrawers(); }
		virtual std::vector<std::pair<std::string, JsonToEditorValueType>> GetControllerAttributes() { return Game::GetControllerAttributes(); }
#endif
		virtual void Map(SUUUID so);
		virtual void Unmap();
		virtual void Step(float delta) {};
		//virtual void BindToV8Context(v8pp::context& context) {}
		//virtual void MapControllerToV8Object(Isolate* isolate, Local<Object>& object) {}
		//Scripting
		virtual v8_templates_creators GetV8TemplatesCreators();
		virtual v8_context_creators GetV8ContextCreators();

		JUUID controller;
		SceneUnitId unit;
		SUUUID sceneObject;
	};

	JUUID RegisterController(std::string controllerName, SUUUID sceneObject, std::unique_ptr<Controller>& controller);
	void MapControllers(SceneUnitId id);
	std::unique_ptr<Controller>& GetController(JUUID uuid);
	std::set<JUUID> GetControllersBySceneObjectUUID(SUUUID uuid);
	void DestroyControllers();
	void DestroyController(JUUID uuid);
	void StepControllers(DX::StepTimer& timer);
	//void BindToV8Context(v8pp::context& context, SUUUID uuid);

	extern std::vector<std::string> GetControllers();
	extern JUUID CreateController(std::string name, SUUUID sceneObject, nlohmann::json& json);

	//template<typename T>
	//T* ContextController()
	//{
	//	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	//	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	//	v8::Context::Scope context_scope(context);
	//	v8::HandleScope handle_scope(isolate);

	//	v8::Local<v8::Object> global = context->Global();

	//	v8::Local<v8::String> key = v8::String::NewFromUtf8(isolate, "uuid", v8::NewStringType::kNormal).ToLocalChecked();
	//	v8::Local<v8::Value> value_js = global->Get(context, key).ToLocalChecked();
	//	v8::String::Utf8Value utf8_value(isolate, value_js);
	//	std::string uuid(*utf8_value);

	//	T* controller = static_cast<T*>(GetController(uuid).get());
	//	return controller;
	//}
};
