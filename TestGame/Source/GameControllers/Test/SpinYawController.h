#pragma once
#include <Controller.h>

namespace Game
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <Test/SpinYawControllerAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <Test/SpinYawControllerAtt.h>
#include <JEnd.h>

#endif

	struct SpinYawController : Controller
	{
#include <Attributes/JFlags.h>
#include <Test/SpinYawControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <Test/SpinYawControllerAtt.h>
#include <JEnd.h>

		SpinYawController(nlohmann::json& json);
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
		DECL_CONTROLLER_DRAWER(SpinYawController, Controller);
#endif
		virtual void Step(float delta);
	};
}