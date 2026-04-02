#pragma once
#include <Controller.h>
#include <set>
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>
#include "../Camera/BrawlerCameraController.h"

namespace Game
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <Brawler/BrawlerSceneControllerAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <Brawler/BrawlerSceneControllerAtt.h>
#include <JEnd.h>

#endif

	struct BrawlerSceneController : Controller
	{
#include <Attributes/JFlags.h>
#include <Brawler/BrawlerSceneControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <Brawler/BrawlerSceneControllerAtt.h>
#include <JEnd.h>

		//Constructor and Binding
		BrawlerSceneController(nlohmann::json& json);
		virtual void SetInitialConditions();
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
		DECL_CONTROLLER_DRAWER(BrawlerSceneController, Controller);
#endif
		virtual void Map(SUUUID so);
		virtual void Unmap();

		//Step
		virtual void Step(float delta);
		//Rendering
		virtual void Render(SceneUnitId id);

		//UI
		void CreateVenomUI(SceneUnitId id);
		void UpdateVenomUI(SceneUnitId id);

		//Camera
		void RegisterCamera(JUUID camController);

		//Heroes
		void RegisterHero(JUUID heroController);

		//Enemies
		void RegisterEnemy(JUUID enemyController);
		bool HeroesReadyToFight();
		std::tuple<JUUID, XMFLOAT3> PickHeroToFight(JUUID enemyController);

		//Camera
		BrawlerCameraController* GetCameraController();

		HtmlUIInstanceID venomUIInstance;

		std::set<JUUID> heroesControllers;
		std::set<JUUID> enemiesControllers;
		JUUID cameraController;

		JUUID leftSlot;
		JUUID rightSlot;
	};
};