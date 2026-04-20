#pragma once
#include <Controller.h>
#include <set>
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>
#include "BrawlerCharacter.h"

namespace Game
{
	namespace Brawler
	{
		enum CharacterLookingTo
		{
			CLT_Right,
			CLT_Left,
		};

		inline static std::unordered_map<CharacterLookingTo, std::string> CharacterLookingToToString =
		{
			{ CLT_Right, "Right" },
			{ CLT_Left, "Left" },
		};

		inline static std::unordered_map<std::string, CharacterLookingTo> StringToCharacterLookingTo =
		{
			{ "Right", CLT_Right },
			{ "Left", CLT_Left },
		};

#if defined(_EDITOR)
#include <Attributes/JOrder.h>
#include <Brawler/BrawlerCharacterAtt.h>
#include <JEnd.h>
#include <Editor/JDrawersDecl.h>
#include <Brawler/BrawlerCharacterAtt.h>
#include <JEnd.h>
#endif

		struct BrawlerCharacter : Controller
		{
#include <Attributes/JFlags.h>
#include <Brawler/BrawlerCharacterAtt.h>
#include <JEnd.h>
#include <Attributes/JDecl.h>
#include <Brawler/BrawlerCharacterAtt.h>
#include <JEnd.h>

			//Constructor and Binding
			BrawlerCharacter(nlohmann::json& json);
			void SetInitialConditions() override;

#if defined(_EDITOR)
			void WriteJson(nlohmann::json& j) override;
			DECL_CONTROLLER_DRAWER(BrawlerCharacter, Controller);
			CharacterLookingTo initialLookingTo;
#endif
		};
	};
};