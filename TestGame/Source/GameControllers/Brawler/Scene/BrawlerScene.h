#pragma once
#include "../Camera/BrawlerCamera.h"
#include "../Rounds/BrawlerRound.h"
#include <Controller.h>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <unordered_map>

namespace Game
{
	namespace Brawler
	{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include "BrawlerSceneAtt.h"
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include "BrawlerSceneAtt.h"
#include <JEnd.h>

#endif

		struct BrawlerScene : Controller
		{
#include <Attributes/JFlags.h>
#include "BrawlerSceneAtt.h"
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include "BrawlerSceneAtt.h"
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include "BrawlerSceneAtt.h"
#include <JEnd.h>

			//Constructor and Binding
			BrawlerScene(nlohmann::json& json);
			static void RegisterScript(Isolate* isolate, Local<ObjectTemplate> tpl, SceneUnitScripting* script);
			void SetInitialConditions() override;
#if defined(_EDITOR)
			void WriteJson(nlohmann::json& j) override;
			DECL_CONTROLLER_DRAWER(BrawlerScene, Controller);
#endif
			void Map(SUUUID so) override;
			void Unmap() override;

			//States
			void OnStartRound(unsigned int round = 0U);
			void OnEndRound();

			//Heroes
			void RegisterHero(JUUID heroController);
			void HeroReady(JUUID heroUUID);

			//Step
			void Step(float delta) override;

			//Rendering
			void Render(SceneUnitId id) override;

			//UI
			std::string BuildEvalScript(std::string type, nlohmann::json data);
			void CreateVenomUI(SceneUnitId id);
			void UpdateVenomUI(SceneUnitId id);
			void HeroTookHit(JUUID enemy, int newHealth);
			void UpdateEnemy(JUUID enemy);
			void AddScore(int scoreToAdd);
			void UpdateHeroHealthUI();
			void UpdateEnemyUI();
			void ShowLeftArrowSign();
			void HideLeftArrowSign();
			void ShowRightArrowSign();
			void HideRightArrowSign();

			//Combat system
			void RegisterThugInCombat(JUUID heroID, JUUID thugID);
			void UnregisterThugFromCombat(JUUID thugID);
			int GetThugCombatSlotIndex(JUUID heroID, JUUID thugID);
			XMVECTOR GetHeroCombatPositionForThug(JUUID heroID, JUUID thugID);
			bool CanJoinCombat(JUUID heroID, int maxAttackers = 4);

			struct CombatQueue {
				std::vector<JUUID> attackers;
			};
			std::map<JUUID, CombatQueue> m_activeCombats;
		};
	};
};

struct EnemyAttackOption {
	JUUID heroID;
	RenderableID heroRenderable;
	float heroRadius;
	bool canAttack; // ¿El slot está lleno? (Ej: máximo 4 enemigos por héroe)

	explicit operator bool() const { return !heroID.empty(); } // Según tu implementación de JUUID
};