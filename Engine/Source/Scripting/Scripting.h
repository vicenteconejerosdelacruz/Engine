#pragma once

namespace Scripting
{
	void InitScripting(const char* path);
	void ShutdownScripting();
	v8::Isolate* GetIsolate();
	void BindModule(std::function<void(v8::Isolate*)>);
	void RunScript(std::string script, RenderableSUUUID renderable);
}