#include "pch.h"
#include "Scripting.h"
#include <JObject.h>

#if defined(_EDITOR)
namespace Editor
{
	extern bool IsPlaying();
	extern bool IsPaused();
}
#endif

namespace Scripting
{
	static std::unique_ptr<v8::Platform> platform;
	static v8::Isolate* isolate = nullptr;

	void InitScripting(const char* path)
	{
		// Initialize V8.
		v8::V8::InitializeICUDefaultLocation(path);
		v8::V8::InitializeExternalStartupData(path);
		platform = v8::platform::NewDefaultPlatform();
		v8::V8::InitializePlatform(platform.get());
		v8::V8::Initialize();

		isolate = v8pp::context::create_isolate();
	}

	void ShutdownScripting()
	{
		v8::V8::Dispose();
		v8::V8::DisposePlatform();
	}

	v8::Isolate* GetIsolate()
	{
		return isolate;
	}

	void BindModule(std::function<void(v8::Isolate*)> binder)
	{
		binder(isolate);
	}

	void RunScript(std::string script, RenderableUUID renderable)
	{
#if defined(_EDITOR)
		if (!Editor::IsPlaying() || Editor::IsPaused())
		{
			return;
		}
#endif
		if (script.empty()) return;

		//create a context using the stored isolate
		v8pp::context context(isolate);

		// Create a stack-allocated handle scope.
		v8::HandleScope handle_scope(context.isolate());

		//bind the object for running the script
		renderable->BindToV8Context(context);

		//run the script
		context.run_script(script);
	}
}