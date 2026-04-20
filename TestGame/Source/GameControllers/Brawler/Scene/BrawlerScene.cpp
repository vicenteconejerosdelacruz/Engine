#include "BrawlerScene.h"
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
		if (!venomUIInstance().empty())
		{
			DeleteHtmlUIInstance(venomUIInstance());
			venomUIInstance("");
		}
		heroes_clear();
		enemies_clear();
		camera("");
		leftSlot("");
		rightSlot("");
		heroHealthChanged(true);
		heroHealth(100);
		lastAttacker("");
		newAttacker(false);
		lastAttackerHealthChanged(false);
		lastAttackerHealth(100);
		lastAttackerName("");
		heroScore(0);
	}

#if defined(_EDITOR)
	void BrawlerScene::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <Brawler/BrawlerSceneAtt.h>
#include <JEnd.h>
		Controller::WriteJson(j);
		j.at("camera") = "";
		j.at("heroes") = nlohmann::json::array({});
		j.at("enemies") = nlohmann::json::array({});
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
		venomUIInstance().empty() ? CreateVenomUI(id) : UpdateVenomUI(id);
	}

	//UI
	void BrawlerScene::CreateVenomUI(SceneUnitId id)
	{
		venomUIInstance(venomUI() + "-" + getUUID());
		CreateHtmlUIInstance(venomUIInstance(), [&]()
			{
				return std::make_unique<HtmlUIInstance>(id, venomUIInstance(), venomUI());
			}
		);
	}

	void BrawlerScene::UpdateVenomUI(SceneUnitId id)
	{
		UpdateHeroHealthUI();
		UpdateEnemyUI();
		HtmlUIInstanceID instance = venomUIInstance();
		instance->UpdateTexture(id);
		instance->Resolve(id);
	}

	void BrawlerScene::HeroTookHit(JUUID enemy, int newHealth)
	{
		heroHealth(newHealth);
		heroHealthChanged(true);
		UpdateEnemy(enemy);
	}

	void BrawlerScene::UpdateEnemy(JUUID enemy)
	{
		newAttacker(lastAttacker() != enemy);
		lastAttacker(enemy);

		auto* bc = GetController<BrawlerCharacter>(lastAttacker());
		lastAttackerName(bc->name());
		lastAttackerHealth(bc->health());
		lastAttackerHealthChanged(true);
	}

	void BrawlerScene::AddScore(int scoreToAdd)
	{
		heroScore(scoreToAdd + heroScore());
		std::string js = "window.dispatchEvent(new CustomEvent('engineUpdate', { detail: { type: 'SCORE_UPDATE', value: " + std::to_string(heroScore()) + " } })); ";
		HtmlUIInstanceID instance = venomUIInstance();
		instance->EvaluateScript(js);
	}

	void BrawlerScene::UpdateHeroHealthUI()
	{
		if (heroHealthChanged())
		{
			std::string js = "window.dispatchEvent(new CustomEvent('engineUpdate', { detail: { type: 'HERO_HP', value: " + std::to_string(heroHealth()) + " } })); ";
			HtmlUIInstanceID instance = venomUIInstance();
			instance->EvaluateScript(js);
			heroHealthChanged(false);
		}
	}

	void BrawlerScene::UpdateEnemyUI()
	{
		if (lastAttackerDied())
		{
			std::string js = "window.dispatchEvent(new CustomEvent('engineUpdate', { detail: { type: 'REMOVE_ENEMY' } })); ";
			HtmlUIInstanceID instance = venomUIInstance();
			instance->EvaluateScript(js);
			lastAttackerDied(false);
		}

		if (newAttacker())
		{
			std::string js = "window.dispatchEvent(new CustomEvent('engineUpdate', { detail: { type: 'NEW_ENEMY', name:'" + lastAttackerName() + "' } })); ";
			HtmlUIInstanceID instance = venomUIInstance();
			instance->EvaluateScript(js);
			newAttacker(false);
		}
		if (lastAttackerHealthChanged())
		{
			std::string js = "window.dispatchEvent(new CustomEvent('engineUpdate', { detail: { type: 'ENEMY_HP', value: " + std::to_string(lastAttackerHealth()) + " } })); ";
			HtmlUIInstanceID instance = venomUIInstance();
			instance->EvaluateScript(js);
			lastAttackerHealthChanged(false);
		}
	}

	void BrawlerScene::RegisterCamera(JUUID camController)
	{
		camera(camController);
	}

	void BrawlerScene::RegisterHero(JUUID heroController)
	{
		heroes_insert(heroController);
		heroHealth(GetController<BrawlerCharacter>(heroController)->health());
	}

	void BrawlerScene::RegisterEnemy(JUUID enemyController)
	{
		enemies_insert(enemyController);
	}

	void BrawlerScene::UnRegisterEnemy(JUUID enemyController)
	{
		if (enemies_contains(enemyController))
			enemies_erase(enemyController);
		if (lastAttacker() == enemyController)
		{
			lastAttacker("");
			lastAttackerDied(true);
		}
	}

	const std::set<VenomStates> venomNonAttackStates({ VS_None, VS_Intro });
	bool BrawlerScene::HeroesReadyToFight()
	{
		auto* venom = GetController<Venom>(*heroes().begin());
		return !venomNonAttackStates.contains(venom->GetState());
	}

	std::tuple<JUUID, XMFLOAT3> BrawlerScene::PickHeroToFight(JUUID enemyController)
	{
		if (!leftSlot().empty() && !rightSlot().empty())
			return std::make_tuple("", XMFLOAT3());

		return std::make_tuple(*heroes().begin(), rightSlot().empty() ? thugAttackOffsetVector() : (-1.0f * thugAttackOffsetVector()));
	}

	BrawlerCamera* BrawlerScene::GetCameraController()
	{
		return GetController<BrawlerCamera>(camera());
	}
};