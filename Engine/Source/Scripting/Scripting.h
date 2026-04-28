#pragma once
#include <UUID.h>
#include <v8.h>
#include <libplatform/libplatform.h>

using namespace v8;

namespace Scene
{
	DEF_SCENEOBJECT_ID_DEP(Renderable);
};
using namespace Scene;
namespace Scripting
{
	void InitScripting(const char* path);
	void ShutdownScripting();
	Isolate* GetIsolate();
	void CreateSceneObjectScriptTemplate(Isolate* isolate, Global<ObjectTemplate>& tmpl, SceneObject* so, v8_att_context& att_context);
	Local<Context> CreateSceneObjectScriptContext(Isolate* isolate, Local<ObjectTemplate>& tmpl, SceneObject* so, v8_att_context& att_context);
	void RunScript(std::string script, SUUUID suuuid);
}