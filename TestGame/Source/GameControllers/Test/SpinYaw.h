#pragma once
#include <Controller.h>

namespace Game
{
	namespace Test
	{
#if defined(_EDITOR)
#include <Attributes/JOrder.h>
#include "SpinYawAtt.h"
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include "SpinYawAtt.h"
#include <JEnd.h>
#endif

		struct SpinYaw : Controller
		{
#include <Attributes/JFlags.h>
#include "SpinYawAtt.h"
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include "SpinYawAtt.h"
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include "SpinYawAtt.h"
#include <JEnd.h>

			DEF_STRING2FLAGS_FUNC(SpinYaw, Controller);

			SpinYaw(nlohmann::json& json);
#if defined(_EDITOR)
			void WriteJson(nlohmann::json& j) override;
			static void GatherFiles(nlohmann::json& json, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream);
			DECL_CONTROLLER_DRAWER(SpinYaw, Controller);
#endif
			void Step(float delta) override;
		};
	}
}