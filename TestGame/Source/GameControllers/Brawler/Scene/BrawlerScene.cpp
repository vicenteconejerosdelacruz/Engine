#include "pch.h"
#include "BrawlerScene.h"
#include "BrawlerScene.h"
#include <Brawler/Characters/Enemies/Thug.h>
#include <Brawler/Characters/Heroes/Venom.h>
#if defined(_EDITOR)
#include <Editor.h>
#endif

namespace Game::Brawler
{

#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include "BrawlerSceneAtt.h"
#include <JEnd.h>
#endif

	//Constructor and Binding
	BrawlerScene::BrawlerScene(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include "BrawlerSceneAtt.h"
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include "BrawlerSceneAtt.h"
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include "BrawlerSceneAtt.h"
#include <JEnd.h>

		SetInitialConditions();
	}

	void BrawlerScene::RegisterScript(Isolate* isolate, Local<ObjectTemplate> tpl, SceneUnitScripting* script)
	{
		v8_register_method<BrawlerScene>(isolate, tpl, "HeroReady", script, [](BrawlerScene* self, JUUID heroUUID) { if (self) self->HeroReady(heroUUID); });
		v8_register_method<BrawlerScene>(isolate, tpl, "OnStartRound", script, [](BrawlerScene* self, unsigned int round) { if (self) self->OnStartRound(round); });
		v8_register_method<BrawlerScene>(isolate, tpl, "ShowLeftArrowSign", script, [](BrawlerScene* self) { if (self) self->ShowLeftArrowSign(); });
		v8_register_method<BrawlerScene>(isolate, tpl, "HideLeftArrowSign", script, [](BrawlerScene* self) { if (self) self->HideLeftArrowSign(); });
		v8_register_method<BrawlerScene>(isolate, tpl, "ShowRightArrowSign", script, [](BrawlerScene* self) { if (self) self->ShowRightArrowSign(); });
		v8_register_method<BrawlerScene>(isolate, tpl, "HideRightArrowSign", script, [](BrawlerScene* self) { if (self) self->HideRightArrowSign(); });
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
		ready_heroes_clear();
		currentRound(0);
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
#include "BrawlerSceneAtt.h"
#include <JEnd.h>
		Controller::WriteJson(j);
		j.at("camera") = "";
		j.at("heroes") = nlohmann::json::array({});
		j.at("ready_heroes") = nlohmann::json::array({});
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

	//States
	void BrawlerScene::OnStartRound(unsigned int round)
	{
		currentRound(round);
		BrawlerRound cround = rounds().at(round);
		enemiesInCurrentRound(static_cast<unsigned int>(cround.enemies.size()));
		for (auto& cb : cround.enemies)
		{
			if (!cb) continue;

			Thug* thug = GetController<Thug>(unit, cb);
			thug->combatEnabled(true);
		}
		Scripting::RunScript(cround.onStart, sceneObject);
	}

	void BrawlerScene::OnEndRound()
	{
		BrawlerRound cround = rounds().at(currentRound());
		Scripting::RunScript(cround.onEnd, sceneObject);
	}

	//Heroes
	void BrawlerScene::RegisterHero(JUUID heroController)
	{
		heroes_insert(heroController);
		Hero* hero = GetController<Hero>(heroController);
		heroHealth(hero->health());
	}

	void BrawlerScene::HeroReady(JUUID heroUUID)
	{
		ready_heroes_insert(heroUUID);
		if (heroes_size() == ready_heroes_size())
		{
			OnStartRound();
		}
	}

	void BrawlerScene::DecreaseEnemiesInRound(int count)
	{
		int numEnemies = enemiesInCurrentRound();
		numEnemies += count;
		numEnemies = std::max(numEnemies, 0);
		enemiesInCurrentRound(static_cast<unsigned int>(numEnemies));
		if (numEnemies == 0)
		{
			OnEndRound();
		}
	}

	//Step
	void BrawlerScene::Step(float delta)
	{}

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
	std::string BrawlerScene::BuildEvalScript(std::string type, nlohmann::json data)
	{
		nlohmann::json event =
		{
			{ "detail", {{ "type", type }}}
		};
		for (auto it = data.begin(); it != data.end(); ++it) {
			event["detail"][it.key()] = it.value();
		}
		std::string eventStr = event.dump();
		std::string js = "window.dispatchEvent(new CustomEvent('engineUpdate', " + eventStr + "));";
		return js;
	}

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
		lastAttackerPicture(bc->picture());
		lastAttackerHealth(bc->health());
		lastAttackerHealthChanged(true);
	}

	void BrawlerScene::AddScore(int scoreToAdd)
	{
		heroScore(scoreToAdd + heroScore());
		std::string js = BuildEvalScript("SCORE_UPDATE",
			{
				{ "value", std::to_string(heroScore()) }
			}
		);
		HtmlUIInstanceID instance = venomUIInstance();
		instance->EvaluateScript(js);
	}

	void BrawlerScene::UpdateHeroHealthUI()
	{
		if (heroHealthChanged())
		{
			std::string js = BuildEvalScript("HERO_HP",
				{
					{ "value", std::to_string(heroHealth()) }
				}
			);
			HtmlUIInstanceID instance = venomUIInstance();
			instance->EvaluateScript(js);
			heroHealthChanged(false);
		}
	}

	void BrawlerScene::UpdateEnemyUI()
	{
		if (lastAttackerDied())
		{
			std::string js = BuildEvalScript("REMOVE_ENEMY", {});
			HtmlUIInstanceID instance = venomUIInstance();
			instance->EvaluateScript(js);
			lastAttackerDied(false);
		}

		if (newAttacker())
		{
			std::string js = BuildEvalScript("NEW_ENEMY",
				{
					{ "name", lastAttackerName() },
					{ "picture", lastAttackerPicture() }
				}
			);
			HtmlUIInstanceID instance = venomUIInstance();
			instance->EvaluateScript(js);
			newAttacker(false);
		}
		if (lastAttackerHealthChanged())
		{
			std::string js = BuildEvalScript("ENEMY_HP",
				{
					{ "value", std::to_string(lastAttackerHealth()) }
				}
			);
			HtmlUIInstanceID instance = venomUIInstance();
			instance->EvaluateScript(js);
			lastAttackerHealthChanged(false);
		}
	}

	void BrawlerScene::ShowLeftArrowSign()
	{
		std::string js = BuildEvalScript("ARROW_LEFT",
			{
				{ "value", true }
			}
		);
		HtmlUIInstanceID instance = venomUIInstance();
		instance->EvaluateScript(js);
	}

	void BrawlerScene::HideLeftArrowSign()
	{
		std::string js = BuildEvalScript("ARROW_LEFT",
			{
				{ "value", false }
			}
		);
		HtmlUIInstanceID instance = venomUIInstance();
		instance->EvaluateScript(js);
	}

	void BrawlerScene::ShowRightArrowSign()
	{
		std::string js = BuildEvalScript("ARROW_RIGHT",
			{
				{ "value", true }
			}
		);
		HtmlUIInstanceID instance = venomUIInstance();
		instance->EvaluateScript(js);
	}

	void BrawlerScene::HideRightArrowSign()
	{
		std::string js = BuildEvalScript("ARROW_RIGHT",
			{
				{ "value", false }
			}
		);
		HtmlUIInstanceID instance = venomUIInstance();
		instance->EvaluateScript(js);
	}

	//Combat system
	void BrawlerScene::RegisterThugInCombat(JUUID heroID, JUUID thugID)
	{
		// 1. Seguridad: Si el Thug ya estaba intentando atacar a otro héroe,
		// lo sacamos de esa cola primero para mantener la integridad.
		UnregisterThugFromCombat(thugID);

		// 2. Accedemos a la cola del héroe (si no existe, std::map la crea)
		auto& attackers = m_activeCombats[heroID].attackers;

		// 3. Verificamos si por alguna razón ya está en esta lista 
		// (aunque Unregister debería haberlo limpiado)
		auto it = std::find(attackers.begin(), attackers.end(), thugID);

		if (it == attackers.end())
		{
			// 4. Lo añadimos al final de la cola. 
			// Su índice será attackers.size() - 1, lo que definirá su posición física.
			attackers.push_back(thugID);
		}
	}

	void BrawlerScene::UnregisterThugFromCombat(JUUID thugID)
	{
		// Recorremos todos los combates activos para encontrar dónde está el Thug
		for (auto& [heroID, combat] : m_activeCombats)
		{
			auto& list = combat.attackers;
			auto it = std::find(list.begin(), list.end(), thugID);

			if (it != list.end())
			{
				list.erase(it);
				// Opcional: si la lista queda vacía, podrías borrar la entrada del mapa
				// pero para brawlers con pocos héroes, dejar el vector vacío es más eficiente.
				break;
			}
		}
	}

	int BrawlerScene::GetThugCombatSlotIndex(JUUID heroID, JUUID thugID)
	{
		auto it = m_activeCombats.find(heroID);
		if (it == m_activeCombats.end()) return -1;

		const auto& list = it->second.attackers;
		for (int i = 0; i < (int)list.size(); ++i)
		{
			if (list[i] == thugID) return i;
		}

		return -1;
	}

	XMVECTOR BrawlerScene::GetHeroCombatPositionForThug(JUUID heroID, JUUID thugID)
	{
		Hero* hero = GetController<Hero>(heroID);
		if (!hero) return XMVectorZero();

		RenderableID heroR = hero->sceneObject;
		int slotIndex = GetThugCombatSlotIndex(heroID, thugID);

		// Failsafe
		if (slotIndex == -1) return heroR->positionV();

		XMVECTOR heroPos = heroR->positionV();

		// Eje X global para alineación horizontal de arcade clásico 2.5D
		XMVECTOR worldRight = { 1.0f, 0.0f, 0.0f, 0.0f };

		// La distancia fija a la que se posicionarán TODOS los enemigos de los lados
		float baseSideDist = 1.4f;

		// Repartición de flancos: pares a la derecha, impares a la izquierda
		bool isRightSide = (slotIndex % 2 == 0);

		XMVECTOR targetPos;
		if (isRightSide)
		{
			// Todos los del lado derecho van exactamente al mismo punto X
			targetPos = XMVectorAdd(heroPos, XMVectorScale(worldRight, baseSideDist));
		}
		else
		{
			// Todos los del lado izquierdo van exactamente al mismo punto X
			targetPos = XMVectorSubtract(heroPos, XMVectorScale(worldRight, baseSideDist));
		}

		// OPCIONAL PARA EL OVERLAP: 
		// Si van exactamente a la misma coordenada X y Z, se taparán de forma idéntica.
		// Si quieres que se sobrepongan pero con un levísimo desfase en el carril (Z) 
		// para que se vean ambas siluetas en pantalla (muy común en Streets of Rage), 
		// puedes descomentar las siguientes líneas:
		/*
		int depthInQueue = slotIndex / 2; // 0 para el primero, 1 para el segundo del mismo lado...
		if (depthInQueue > 0)
		{
			float tinyZOffset = (slotIndex % 4 == 2) ? 0.15f : -0.15f;
			targetPos = XMVectorAdd(targetPos, XMVectorSet(0.0f, 0.0f, tinyZOffset, 0.0f));
		}
		*/

		// Forzamos que compartan la altura Y del suelo del héroe
		return XMVectorSetY(targetPos, XMVectorGetY(heroPos));
	}

	bool BrawlerScene::CanJoinCombat(JUUID heroID, int maxAttackers)
	{
		// Buscamos si existe una cola para este héroe
		auto it = m_activeCombats.find(heroID);

		// Si no existe, es que no hay nadie atacando, por lo tanto el slot está libre
		if (it == m_activeCombats.end())
			return true;

		// Comprobamos si la cantidad de atacantes actuales es menor al límite
		return (int)it->second.attackers.size() < maxAttackers;
	}

}