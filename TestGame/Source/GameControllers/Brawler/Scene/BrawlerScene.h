#pragma once
#include <Controller.h>
#include <set>
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>
#include "../Camera/BrawlerCamera.h"

namespace Game
{
	namespace Brawler
	{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <Brawler/BrawlerSceneAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <Brawler/BrawlerSceneAtt.h>
#include <JEnd.h>

#endif

		struct BrawlerScene : Controller
		{
#include <Attributes/JFlags.h>
#include <Brawler/BrawlerSceneAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <Brawler/BrawlerSceneAtt.h>
#include <JEnd.h>

			//Constructor and Binding
			BrawlerScene(nlohmann::json& json);
			void SetInitialConditions() override;
#if defined(_EDITOR)
			void WriteJson(nlohmann::json& j) override;
			DECL_CONTROLLER_DRAWER(BrawlerScene, Controller);
#endif
			void Map(SUUUID so) override;
			void Unmap() override;

			//Step
			void Step(float delta) override;
			//Rendering
			void Render(SceneUnitId id) override;

			//UI
			void CreateVenomUI(SceneUnitId id);
			void UpdateVenomUI(SceneUnitId id);
			void HeroTookHit(JUUID enemy, int newHealth);
			void UpdateEnemy(JUUID enemy);
			void UpdateHeroHealthUI();
			void UpdateEnemyUI();

			//Camera
			void RegisterCamera(JUUID camController);

			//Heroes
			void RegisterHero(JUUID heroController);

			//Enemies
			void RegisterEnemy(JUUID enemyController);
			void UnRegisterEnemy(JUUID enemyController);
			bool HeroesReadyToFight();
			std::tuple<JUUID, XMFLOAT3> PickHeroToFight(JUUID enemyController);

			//Camera
			BrawlerCamera* GetCameraController();

			/*
			HtmlUIInstanceID venomUIInstance;

			std::set<JUUID> heroesControllers;
			std::set<JUUID> enemiesControllers;
			JUUID cameraController;

			JUUID leftSlot;
			JUUID rightSlot;

			//UI
			bool heroHealthChanged = true;
			int heroHealth;

			JUUID lastAttacker;
			bool newAttacker = false;
			bool lastAttackerHealthChanged = false;

			int lastAttackerHealth;
			std::string lastAttackerName;
			*/
		};
	};
};