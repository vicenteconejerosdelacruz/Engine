#include "pch.h"
#include "SceneObject.h"

#if defined(_EDITOR)
namespace Editor
{
	//extern bool levelModified;
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
		//Editor::levelModified = true;
#endif
		JObject::JUpdate(p);
	}

	void SceneObject::JPatch(nlohmann::json p)
	{
#if defined(_EDITOR)
		//Editor::levelModified = true;
#endif
		JObject::JPatch(p);
	}

	void SceneObject::BindToV8Context(v8pp::context& context)
	{
		Game::BindToV8Context(context, SUuuid());
	}
}