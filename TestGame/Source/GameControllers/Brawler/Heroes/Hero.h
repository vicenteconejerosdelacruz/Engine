#pragma once
#include <Controller.h>

namespace Game
{
	struct Hero : Controller
	{
		Hero(nlohmann::json& json);
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif
		virtual void Step(float delta);
	};
};