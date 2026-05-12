#include "pch.h"
#include "BrawlerCharacter.h"

namespace Game::Brawler
{
#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include "BrawlerCharacterAtt.h"
#include <JEnd.h>
#endif

	//Constructor and Binding
	BrawlerCharacter::BrawlerCharacter(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include "BrawlerCharacterAtt.h"
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include "BrawlerCharacterAtt.h"
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include "BrawlerCharacterAtt.h"
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
		followAnimationPlaying(false);
	}

	void BrawlerCharacter::Unmap()
	{
		renderable.clear();
	}

#if defined(_EDITOR)
	void BrawlerCharacter::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include "BrawlerCharacterAtt.h"
#include <JEnd.h>
		Controller::WriteJson(j);
	}
#endif
	void BrawlerCharacter::BindNestedControllers(Local<Context> context, Isolate* isolate, std::unique_ptr<SceneUnitScripting>& scriptData)
	{
		std::set<JNAME> names = { sceneController().name };
		std::set<JUUIDName> soControllers = GetControllersUUIDNamesBySceneObjectUUID(MAKESUUUID(unit, sceneController().uuid));
		std::set<JUUIDName> controllers;
		std::copy_if(soControllers.begin(), soControllers.end(), std::inserter(controllers, controllers.begin()), [&](JUUIDName un)
			{
				return names.contains(std::get<1>(un));
			}
		);

		Scripting::BindSceneObjectControllers(context, isolate, scriptData, controllers);
	}
};