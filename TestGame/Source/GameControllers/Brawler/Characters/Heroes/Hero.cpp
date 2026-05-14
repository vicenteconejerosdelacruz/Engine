#include "pch.h"
#include "Hero.h"

namespace Game::Brawler
{
#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include "HeroAtt.h"
#include <JEnd.h>
#endif

	Hero::Hero(nlohmann::json& json) : BrawlerCharacter(json)
	{
#include <Attributes/JInit.h>
#include "HeroAtt.h"
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include "HeroAtt.h"
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include "HeroAtt.h"
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void Hero::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include "HeroAtt.h"
#include <JEnd.h>
		BrawlerCharacter::WriteJson(j);
	}
#endif
	void Hero::Map(SUUUID so)
	{
		BrawlerCharacter::Map(so);

		SceneObjectType type = GetSceneObjectType(so);

		if (type == SO_Renderables)
		{
			RenderableID r = so;
			if (r->physicObject().size() > 0)
			{
				capsuleRadius = static_cast<float>(r->physicObject().at(0).get()->at("radius"));
			}
		}
	}
};