#pragma once
#include <v8.h>
#include <map>
#include <SceneObject.h>

using namespace v8;
using namespace nov8;

struct SceneUnitScripting
{
	SceneUnitId id;
	//object template for the scene
	Global<ObjectTemplate> sceneTemplate;
	//object template for the containers(renderables, lights, cameras, etc)
	std::map<SceneObjectType, Global<ObjectTemplate>> containersTemplates;
	//object template for the proxies (concting A->B)
	Global<ObjectTemplate> proxyTemplate;

	SceneUnitScripting(SceneUnitId unit) :id(unit) {}
	void Create(Isolate* isolate);
	void CreateContainers(Isolate* isolate);
	void CreateProxyTemplate(Isolate* isolate);
	void CreateMemberFunctionTemplates(Isolate* isolate);
	static Local<ObjectTemplate> GetOrCreateTemplate(Isolate* isolate, SceneUnitId id,
		const std::string& className, std::function<void(Isolate*, Local<ObjectTemplate>, SceneUnitScripting*)> setupFunc
	);
};