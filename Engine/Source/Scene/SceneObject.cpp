#include "pch.h"
#include "SceneObject.h"
#include <Controller.h>

#if defined(_EDITOR)
namespace Editor
{
	extern bool levelModified;
};
#endif

namespace Scene
{
	void SceneObject::Initialize()
	{
	}

	void SceneObject::BindToScene()
	{
	}

	void SceneObject::JUpdate(nlohmann::json p)
	{
#if defined(_EDITOR)
		Editor::levelModified = true;
#endif
		JObject::JUpdate(p);
	}

	void SceneObject::JPatch(nlohmann::json p)
	{
#if defined(_EDITOR)
		Editor::levelModified = true;
#endif
		JObject::JPatch(p);
	}

	void SceneObject::BindControllers()
	{
		using namespace Game;

		if (!contains("controllers")) return;

		auto& ctrls = at("controllers");
		for (auto it = ctrls.begin(); it != ctrls.end(); it++)
		{
			std::unique_ptr<Controller> controller = GetGameController(*it);
			if (!controller) continue;

			controllers.insert(Game::RegisterController(controller, *it, Juuid()));
		}
	}

	void SceneObject::UnbindControllers()
	{
		for (auto c : controllers)
		{
			Game::UnregisterController(c);
		}
		controllers.clear();
	}

	void SceneObject::BindToV8Context(v8pp::context& context)
	{
		Game::BindToV8Context(context, Juuid());
	}
}