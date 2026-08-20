#pragma once
#include "../BrawlerCharacter.h"

namespace Game
{
	namespace Brawler
	{
#if defined(_EDITOR)
#include <Attributes/JOrder.h>
#include "HeroAtt.h"
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include "HeroAtt.h"
#include <JEnd.h>
#endif

		struct Hero : BrawlerCharacter
		{
#include <Attributes/JFlags.h>
#include "HeroAtt.h"
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include "HeroAtt.h"
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include "HeroAtt.h"
#include <JEnd.h>

			DEF_STRING2FLAGS_FUNC(Hero, BrawlerCharacter);

			Hero(nlohmann::json& json);
#if defined(_EDITOR)
			void WriteJson(nlohmann::json& j) override;
			DECL_CONTROLLER_DRAWER(Hero, BrawlerCharacter);
#endif
			void Map(SUUUID so) override;

			float capsuleRadius;
		};
	};
};