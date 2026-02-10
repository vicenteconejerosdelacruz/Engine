#pragma once
#include <UUID.h>

namespace Scene
{
	DEF_SCENEOBJECT_ID_DEP(Renderable);
};
using namespace Scene;
namespace Scripting
{
	void InitScripting(const char* path);
	void ShutdownScripting();
	v8::Isolate* GetIsolate();
	void BindModule(std::function<void(v8::Isolate*)>);
	void RunScript(std::string script, RenderableID renderable);
}