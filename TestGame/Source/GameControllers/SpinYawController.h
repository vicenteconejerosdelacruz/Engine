#pragma once
#include <Controller.h>

namespace Game
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <SpinYawControllerAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <SpinYawControllerAtt.h>
#include <JEnd.h>

#endif

	struct SpinYawController : Controller
	{
#include <Attributes/JFlags.h>
#include <SpinYawControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <SpinYawControllerAtt.h>
#include <JEnd.h>

		SpinYawController(nlohmann::json& json);
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
		virtual std::map<std::string, JEdvEditorDrawerFunction> GetControllerDrawers() { return GetSpinYawControllerDrawers(); }
		virtual std::vector<std::pair<std::string, JsonToEditorValueType>> GetControllerAttributes() { return GetSpinYawControllerAttributes(); }
#endif

		virtual void Step(float delta);
	};
}