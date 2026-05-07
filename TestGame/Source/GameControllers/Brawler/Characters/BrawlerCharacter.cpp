#include "pch.h"
#include "BrawlerCharacter.h"

namespace Game::Brawler
{
#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include <Brawler/BrawlerCharacterAtt.h>
#include <JEnd.h>
#endif

	//Constructor and Binding
	BrawlerCharacter::BrawlerCharacter(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include <Brawler/BrawlerCharacterAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <Brawler/BrawlerCharacterAtt.h>
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include <Brawler/BrawlerCharacterAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)
		initialLookingTo = lookingTo();
#endif
	}

	void BrawlerCharacter::SetInitialConditions()
	{
#if defined(_EDITOR)
		lookingTo(initialLookingTo);
#endif
	}

#if defined(_EDITOR)
	void BrawlerCharacter::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <Brawler/BrawlerSceneAtt.h>
#include <JEnd.h>
		Controller::WriteJson(j);
	}
#endif
};