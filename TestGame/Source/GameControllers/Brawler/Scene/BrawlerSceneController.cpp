#include "pch.h"
#include "BrawlerSceneController.h"
#include "../Heroes/VenomController.h"
#if defined(_EDITOR)
#include <Editor.h>
#endif

namespace Game
{
#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <Brawler/BrawlerSceneControllerAtt.h>
#include <JEnd.h>

#endif

	//Constructor and Binding
	BrawlerSceneController::BrawlerSceneController(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include <Brawler/BrawlerSceneControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <Brawler/BrawlerSceneControllerAtt.h>
#include <JEnd.h>
		SetInitialConditions();
	}

	void BrawlerSceneController::SetInitialConditions()
	{
	}

#if defined(_EDITOR)
	void BrawlerSceneController::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <Brawler/BrawlerSceneControllerAtt.h>
#include <JEnd.h>
		j.erase("uuid");
	}
#endif

	void BrawlerSceneController::Map(SUUUID so)
	{
		using namespace Scene;
		Controller::Map(so);


		SetInitialConditions();
	}

	void BrawlerSceneController::Unmap()
	{
		Controller::Unmap();
	}

	//Step
	void BrawlerSceneController::Step(float delta)
	{
	}

	//Rendering
	void BrawlerSceneController::Render(SceneUnitId id)
	{
#if defined(_EDITOR)
		if (!Editor::IsPlaying(id) || Editor::IsPaused(id))
			return;
#endif
		venomUIInstance.empty() ? CreateVenomUI(id) : UpdateVenomUI(id);
	}

	//UI
	void BrawlerSceneController::CreateVenomUI(SceneUnitId id)
	{
		venomUIInstance = venomUI() + "-" + getUUID();
		CreateHtmlUIInstance(venomUIInstance(), [&]()
			{
				return std::make_unique<HtmlUIInstance>(id, venomUIInstance(), venomUI());
			}
		);
	}

	void BrawlerSceneController::UpdateVenomUI(SceneUnitId id)
	{
		venomUIInstance->UpdateTexture(id);
		venomUIInstance->Resolve(id);
	}

	void BrawlerSceneController::RegisterCamera(JUUID camController)
	{
		cameraController = camController;
	}

	void BrawlerSceneController::RegisterHero(JUUID heroController)
	{
		heroesControllers.insert(heroController);
	}

	void BrawlerSceneController::RegisterEnemy(JUUID enemyController)
	{
		enemiesControllers.insert(enemyController);
	}

	const std::set<VenomStates> venomNonAttackStates({ VS_None, VS_Intro });
	bool BrawlerSceneController::HeroesReadyToFight()
	{
		auto* venom = GetController<VenomController>(*heroesControllers.begin());
		return !venomNonAttackStates.contains(venom->GetState());
	}

	std::tuple<JUUID, XMFLOAT3> BrawlerSceneController::PickHeroToFight(JUUID enemyController)
	{
		if (!leftSlot.empty() && !rightSlot.empty())
			return std::make_tuple("", XMFLOAT3());

		return std::make_tuple(*heroesControllers.begin(), rightSlot.empty() ? thugAttackOffsetVector() : (-1.0f * thugAttackOffsetVector()));
	}

	BrawlerCameraController* BrawlerSceneController::GetCameraController()
	{
		return GetController<BrawlerCameraController>(cameraController);
	}
};