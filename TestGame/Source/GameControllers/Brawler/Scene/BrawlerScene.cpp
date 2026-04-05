#include "pch.h"
#include "BrawlerScene.h"
#include "../Characters/Heroes/Venom.h"
#if defined(_EDITOR)
#include <Editor.h>
#endif

namespace Game::Brawler
{

#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include <Brawler/BrawlerSceneAtt.h>
#include <JEnd.h>
#endif

	//Constructor and Binding
	BrawlerScene::BrawlerScene(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include <Brawler/BrawlerSceneAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <Brawler/BrawlerSceneAtt.h>
#include <JEnd.h>
		SetInitialConditions();
	}

	void BrawlerScene::SetInitialConditions()
	{
	}

#if defined(_EDITOR)
	void BrawlerScene::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <Brawler/BrawlerSceneAtt.h>
#include <JEnd.h>
		Controller::WriteJson(j);
	}
#endif

	void BrawlerScene::Map(SUUUID so)
	{
		using namespace Scene;
		Controller::Map(so);


		SetInitialConditions();
	}

	void BrawlerScene::Unmap()
	{
		Controller::Unmap();
	}

	//Step
	void BrawlerScene::Step(float delta)
	{
	}

	//Rendering
	void BrawlerScene::Render(SceneUnitId id)
	{
#if defined(_EDITOR)
		if (!Editor::IsPlaying(id) || Editor::IsPaused(id))
			return;
#endif
		venomUIInstance.empty() ? CreateVenomUI(id) : UpdateVenomUI(id);
	}

	//UI
	void BrawlerScene::CreateVenomUI(SceneUnitId id)
	{
		venomUIInstance = venomUI() + "-" + getUUID();
		CreateHtmlUIInstance(venomUIInstance(), [&]()
			{
				return std::make_unique<HtmlUIInstance>(id, venomUIInstance(), venomUI());
			}
		);
	}

	void BrawlerScene::UpdateVenomUI(SceneUnitId id)
	{
		venomUIInstance->UpdateTexture(id);
		venomUIInstance->Resolve(id);
	}

	void BrawlerScene::RegisterCamera(JUUID camController)
	{
		cameraController = camController;
	}

	void BrawlerScene::RegisterHero(JUUID heroController)
	{
		heroesControllers.insert(heroController);
	}

	void BrawlerScene::RegisterEnemy(JUUID enemyController)
	{
		enemiesControllers.insert(enemyController);
	}

	const std::set<VenomStates> venomNonAttackStates({ VS_None, VS_Intro });
	bool BrawlerScene::HeroesReadyToFight()
	{
		auto* venom = GetController<Venom>(*heroesControllers.begin());
		return !venomNonAttackStates.contains(venom->GetState());
	}

	std::tuple<JUUID, XMFLOAT3> BrawlerScene::PickHeroToFight(JUUID enemyController)
	{
		if (!leftSlot.empty() && !rightSlot.empty())
			return std::make_tuple("", XMFLOAT3());

		return std::make_tuple(*heroesControllers.begin(), rightSlot.empty() ? thugAttackOffsetVector() : (-1.0f * thugAttackOffsetVector()));
	}

	BrawlerCamera* BrawlerScene::GetCameraController()
	{
		return GetController<BrawlerCamera>(cameraController);
	}
};