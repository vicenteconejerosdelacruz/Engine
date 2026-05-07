#include "pch.h"
#include "SceneUnitScripting.h"

void SceneUnitScripting::Create(Isolate* isolate)
{
	Locker locker(isolate);
	Isolate::Scope isolate_scope(isolate);
	HandleScope handle_scope(isolate);

	//create the scene template
	Local<ObjectTemplate> tmpl = ObjectTemplate::New(isolate);
	tmpl->SetInternalFieldCount(1);

	//set the containers as placeholders
	for (auto& [type, name] : SceneObjectTypeJsonContainer)
	{
		tmpl->Set(v8_string(isolate, name), Null(isolate));
	}

	//store the scene template
	sceneTemplate.Reset(isolate, tmpl);

	//then we can create the containers templates(not the same as the placeholders)
	CreateContainers(isolate);
	//the proxies
	CreateProxyTemplate(isolate);
	//and the member functions
	CreateMemberFunctionTemplates(isolate);
}

void SceneUnitScripting::CreateContainers(Isolate* isolate)
{
	for (auto& [type, name] : SceneObjectTypeJsonContainer)
	{
		//create a template for the container and assign 2 internal fields(sceneUnit and container type)
		Local<ObjectTemplate> ctmpl = ObjectTemplate::New(isolate);
		ctmpl->SetInternalFieldCount(2);

		//assign the getter for the scene container
		ctmpl->SetHandler(NamedPropertyHandlerConfiguration(
			v8_scene_container_getter,
			nullptr, nullptr, nullptr,
			v8_scene_container_enumerator
		));

		//reset the global template of the container
		containersTemplates[type].Reset(isolate, ctmpl);
	}
}

void SceneUnitScripting::CreateProxyTemplate(Isolate* isolate)
{
	Local<ObjectTemplate> tmpl = ObjectTemplate::New(isolate);
	tmpl->SetInternalFieldCount(1); // V8PropertyProxy*

	Local<External> scriptExternal = External::New(isolate, this);
	tmpl->SetHandler(NamedPropertyHandlerConfiguration(
		v8_proxy_getter,
		v8_proxy_setter,
		nullptr, nullptr,
		v8_proxy_enumerator,
		scriptExternal
	));

	proxyTemplate.Reset(isolate, tmpl);
}

void SceneUnitScripting::CreateMemberFunctionTemplates(Isolate* isolate)
{
	//this is from the engine
	CreateSceneObjectsMemberFunctionTemplates(isolate, id);
	//this is from the game
	CreateControllersMemberFunctionTemplates(isolate, id);
	//also engine
	CreatePhysicObjectMemberFunctionTemplates(isolate, id);
}

//jobject derived classes to object template
std::map<std::tuple<SceneUnitId, std::string>, Global<ObjectTemplate>> templateRegistry;
Local<ObjectTemplate> SceneUnitScripting::GetOrCreateTemplate(
	Isolate* isolate, SceneUnitId id,
	const std::string& className,
	std::function<void(Isolate*, Local<ObjectTemplate>, SceneUnitScripting*)> setupFunc
) {
	std::tuple key = std::make_tuple(id, className);

	//if the template is already stored just return it
	auto it = templateRegistry.find(key);
	if (it != templateRegistry.end()) { return it->second.Get(isolate); }

	//NEEDED: so the handle doesn't die after leaving this function
	EscapableHandleScope scope(isolate);

	//create the template
	Local<ObjectTemplate> tpl = ObjectTemplate::New(isolate);
	tpl->SetInternalFieldCount(1);

	//register the JObject interceptors
	tpl->SetHandler(NamedPropertyHandlerConfiguration(
		v8_jobject_getter,     // El que decide si devolver un valor o un Proxy
		v8_jobject_setter,     // El que actualiza el JSON y marca el dirty flag
		nullptr,
		nullptr,
		v8_jobject_enumerator, // El que ya probamos y funciona para JSON.stringify
		External::New(isolate, Scripting::GetSceneUnitScripting(id)) // Pasamos 'this' para que el getter encuentre WrapProxy
	));

	// call the setup function for registering the function templates
	if (setupFunc)
	{
		setupFunc(isolate, tpl, Scripting::GetSceneUnitScripting(id));
	}

	//construct the template inplace
	templateRegistry.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(isolate, tpl));

	return scope.Escape(tpl); // the handle is valid outside the function
}
