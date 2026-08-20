#pragma once
#include <Controller.h>

namespace Game
{
	namespace Brawler
	{
#if defined(_EDITOR)
#include <Attributes/JOrder.h>
#include "PumpkinBombAtt.h"
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include "PumpkinBombAtt.h"
#include <JEnd.h>
#endif

		struct PumpkinBomb : Controller
		{
#include <Attributes/JFlags.h>
#include "PumpkinBombAtt.h"
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include "PumpkinBombAtt.h"
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include "PumpkinBombAtt.h"
#include <JEnd.h>

			DEF_STRING2FLAGS_FUNC(PumpkinBomb, Controller);

			PumpkinBomb(nlohmann::json& json);
#if defined(_EDITOR)
			void WriteJson(nlohmann::json& j) override;
			DECL_CONTROLLER_DRAWER(PumpkinBomb, Controller);
#endif
			void Map(SUUUID so) override;
			void Unmap() override;
			void Step(float delta) override;

			//Physics
			void OnStaticContactEvent(JUUID phO, unsigned int event);

			RenderableID renderable;
			PhysicObjectID physicObject;
		};
	}
}