#pragma once
#include "../BrawlerCharacter.h"

namespace Game
{
	namespace Brawler
	{
#if defined(_EDITOR)
#include <Attributes/JOrder.h>
#include <Brawler/HeroAtt.h>
#include <JEnd.h>
#include <Editor/JDrawersDecl.h>
#include <Brawler/HeroAtt.h>
#include <JEnd.h>
#endif

		struct Hero : BrawlerCharacter
		{
#include <Attributes/JFlags.h>
#include <Brawler/HeroAtt.h>
#include <JEnd.h>
#include <Attributes/JDecl.h>
#include <Brawler/HeroAtt.h>
#include <JEnd.h>

			Hero(nlohmann::json& json);
#if defined(_EDITOR)
			void WriteJson(nlohmann::json& j) override;
			DECL_CONTROLLER_DRAWER(Hero, BrawlerCharacter);
#endif
		};
	};
};