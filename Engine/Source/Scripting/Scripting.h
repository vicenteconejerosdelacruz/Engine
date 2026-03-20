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
	void BindModule(std::function<void(Isolate*)>);
	void RunScript(std::string script, SUUUID suuuid);
}