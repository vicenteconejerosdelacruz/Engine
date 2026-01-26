#include "pch.h"
#include "SceneObject.h"

#if defined(_EDITOR)
namespace Editor
{
	void MarkSceneUnitAsModified(SceneUnitId id);
};
#endif

namespace Game
{
	extern void BindToV8Context(v8pp::context& context, SUUUID uuid);
};

namespace Scene
{


	void SceneObject::JUpdate(nlohmann::json p)
	{
#if defined(_EDITOR)
		Editor::MarkSceneUnitAsModified(unit);
#endif
		JObject::JUpdate(p);
	}

	void SceneObject::JPatch(nlohmann::json p)
	{
#if defined(_EDITOR)
		Editor::MarkSceneUnitAsModified(unit);
#endif
		JObject::JPatch(p);
	}

	void SceneObject::BindToV8Context(v8pp::context& context)
	{
		Game::BindToV8Context(context, SUuuid());
	}
}