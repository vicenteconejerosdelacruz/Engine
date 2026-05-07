#pragma once
#include <Controller.h>

namespace Game
{
	namespace Test
	{
#if defined(_EDITOR)
#include <Attributes/JOrder.h>
#include <Test/SpinYawAtt.h>
#include <JEnd.h>
#include <Editor/JDrawersDecl.h>
#include <Test/SpinYawAtt.h>
#include <JEnd.h>
#endif

		struct SpinYaw : Controller
		{
#include <Attributes/JFlags.h>
#include <Test/SpinYawAtt.h>
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include <Test/SpinYawAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <Test/SpinYawAtt.h>
#include <JEnd.h>

			SpinYaw(nlohmann::json& json);
#if defined(_EDITOR)
			void WriteJson(nlohmann::json& j) override;
			DECL_CONTROLLER_DRAWER(SpinYaw, Controller);
#endif
			void Step(float delta) override;
		};
	}
}