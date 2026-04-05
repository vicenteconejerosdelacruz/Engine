#include "pch.h"
#include "Hero.h"

namespace Game::Brawler
{
#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include <Brawler/HeroAtt.h>
#include <JEnd.h>
#endif

	Hero::Hero(nlohmann::json& json) : BrawlerCharacter(json)
	{
#include <Attributes/JInit.h>
#include <Brawler/HeroAtt.h>
#include <JEnd.h>
#include <Attributes/JUpdate.h>
#include <Brawler/HeroAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void Hero::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <Brawler/HeroAtt.h>
#include <JEnd.h>
		BrawlerCharacter::WriteJson(j);
	}
#endif
};