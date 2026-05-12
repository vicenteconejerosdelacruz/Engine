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
		heroesAttackersQueues.clear();
		enemiesAttackQueue.clear();
		heroToAttackByEnemy.clear();
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
		for (auto& cb : cround.enemies)
		{
			if (!cb) continue;

			Thug* thug = GetController<Thug>(unit, cb);
			thug->combatEnabled(true);
		}
	}

	//Heroes
	void BrawlerScene::RegisterHero(JUUID heroController)
	{
		heroes_insert(heroController);
		Hero* hero = GetController<Hero>(heroController);
		heroHealth(hero->health());
		heroesAttackersQueues.insert_or_assign(heroController, HeroAttackersQueues{});
	}

	void BrawlerScene::HeroReady(JUUID heroUUID)
	{
		ready_heroes_insert(heroUUID);
		if (heroes_size() == ready_heroes_size())
		{
			OnStartRound();
		}
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
		lastAttackerPicture(bc->picture());
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

	std::string BuildEvalScript(std::string type, nlohmann::json data)
	{
		nlohmann::json event =
		{

		};

		return event;
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
			//nlohmann::json event =
			//{ {
			//	"details",
			//	{
			//		{ "type", "NEW_ENEMY" },
			//		{ "name", lastAttackerName() },
			//		{ "picture", lastAttackerPicture() }
			//	}
			//} };
			std::string js = "window.dispatchEvent(new CustomEvent('engineUpdate', { detail: { type: 'NEW_ENEMY', name:'" + lastAttackerName() + "', picture:'" + lastAttackerPicture() + "' } })); ";
			//std::string update = event.dump();
			//std::replace(update.begin(), update.end(), '\"', '\'');
			//std::string js = "window.dispatchEvent(new CustomEvent('engineUpdate',";
			//js += update;
			//js += "))";
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

	XMVECTOR BrawlerScene::GetHeroCombatPositionInSide(JUUID heroID, XMVECTOR heroPos, EnemiesAttackSides side, int queueIndex)
	{
		Hero* heroC = GetController<Hero>(heroID);
		if (!heroC) return heroPos;

		XMFLOAT3 rawOffset;
		// 1. Obtener el offset base según el enum solicitado
		switch (side)
		{
		case EAS_FarLeft:   rawOffset = heroC->farLeftAttackOffset();   break;
		case EAS_FarRight:  rawOffset = heroC->farRightAttackOffset();  break;
		case EAS_NearLeft:  rawOffset = heroC->nearLeftAttackOffset();  break;
		case EAS_NearRight: rawOffset = heroC->nearRightAttackOffset(); break;
		default:            rawOffset = { 0, 0, 0 }; break;
		}

		XMVECTOR baseOffset = XMVectorSet(rawOffset.x, 0.0f, rawOffset.z, 0.0f);

		// 2. Aplicar el Caso Especial: Si la cola está vacía, aplicamos la suma de Z (Near + Far)
		// Para saber si está vacía en esta consulta "teórica", verificamos si el queueIndex es 0
		// y si la cola hermana está vacía.
		auto& heroQueues = heroesAttackersQueues.at(heroID);
		bool isLoneInSide = false;

		if (queueIndex == 0)
		{
			if (side == EAS_FarLeft || side == EAS_NearLeft)
				isLoneInSide = (heroQueues.farLeft.attached.empty() && heroQueues.nearLeft.attached.empty());
			else
				isLoneInSide = (heroQueues.farRight.attached.empty() && heroQueues.nearRight.attached.empty());
		}

		if (isLoneInSide)
		{
			float combinedZ = 0.0f;
			if (side == EAS_FarLeft || side == EAS_NearLeft)
				combinedZ = heroC->farLeftAttackOffset().z + heroC->nearLeftAttackOffset().z;
			else
				combinedZ = heroC->farRightAttackOffset().z + heroC->nearRightAttackOffset().z;

			baseOffset = XMVectorSetZ(baseOffset, combinedZ);
		}
		else if (queueIndex > 0)
		{
			// 3. Si ya hay gente, aplicar profundidad en X (fila)
			XMVECTOR xAxis = XMVectorMultiply(baseOffset, g_XMIdentityR0);
			XMVECTOR depthScale = XMVectorScale(xAxis, static_cast<float>(queueIndex));
			baseOffset = XMVectorAdd(baseOffset, depthScale);
		}

		return XMVectorAdd(heroPos, baseOffset);
	}

	std::tuple<EnemiesAttackSides, float, bool, bool, int>
		BrawlerScene::GetNearHeroAttackPointInSide(JUUID heroID, XMVECTOR heroPos, JUUID enemyID, XMVECTOR enemyPos, bool lookLeft)
	{
		// 1. Definir los dos lados a evaluar según el flanco solicitado
		EnemiesAttackSides sideA = lookLeft ? EAS_NearLeft : EAS_NearRight;
		EnemiesAttackSides sideB = lookLeft ? EAS_FarLeft : EAS_FarRight;

		// 2. Obtener las colas del héroe
		auto& heroQueues = heroesAttackersQueues.at(heroID);

		// 3. Evaluar Side A (Near)
		AttackersQueue& queueA = (sideA == EAS_NearLeft) ? heroQueues.nearLeft : heroQueues.nearRight;
		XMVECTOR posA = GetHeroCombatPositionInSide(heroID, heroPos, sideA, (int)queueA.attached.size());
		float distSqA = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(posA, enemyPos)));
		bool canA = queueA.attached.size() < 3; // Ejemplo: límite de 3 por cola

		// 4. Evaluar Side B (Far)
		AttackersQueue& queueB = (sideB == EAS_FarLeft) ? heroQueues.farLeft : heroQueues.farRight;
		XMVECTOR posB = GetHeroCombatPositionInSide(heroID, heroPos, sideB, (int)queueB.attached.size());
		float distSqB = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(posB, enemyPos)));
		bool canB = queueB.attached.size() < 3;

		// 5. Lógica de selección:
		// Priorizamos el que tenga espacio y sea más cercano.
		EnemiesAttackSides finalSide;
		float finalDistSq;
		int finalIdx;
		bool finalCan;

		if (canA && (!canB || distSqA < distSqB))
		{
			finalSide = sideA;
			finalDistSq = distSqA;
			finalCan = canA;
			finalIdx = (int)queueA.attached.size();
		}
		else
		{
			finalSide = sideB;
			finalDistSq = distSqB;
			finalCan = canB;
			finalIdx = (int)queueB.attached.size();
		}

		// El cuarto valor del tuple (isAlreadyAttached) se suele verificar externamente,
		// aquí lo ponemos en false por defecto para la búsqueda de nueva posición.
		return { finalSide, finalDistSq, finalCan, false, finalIdx };
	}

	std::tuple<EnemiesAttackSides, float, bool, bool, int>
		BrawlerScene::GetNearHeroAttackPoint(JUUID heroID, XMVECTOR heroPos, JUUID enemyID, XMVECTOR enemyPos)
	{
		auto it = heroesAttackersQueues.find(heroID);
		if (it == heroesAttackersQueues.end())
			return { EAS_NearLeft, FLT_MAX, false, false, 0 };

		// --- NUEVA LÓGICA DE DETECCIÓN DE FLANCO REAL ---
		// Calculamos si el enemigo está físicamente a la derecha o izquierda del héroe
		// Usamos el mundo (X positivo es derecha, X negativo es izquierda) 
		// o el vector "Right" del héroe si tienes rotación.

		XMVECTOR diff = XMVectorSubtract(enemyPos, heroPos);
		float relativeX = XMVectorGetX(diff);

		// Si relativeX > 0, el enemigo está a la derecha del héroe.
		bool enemyIsAtRightSide = (relativeX > 0);

		// Priorizamos el lado donde ya se encuentra el enemigo
		auto primaryData = GetNearHeroAttackPointInSide(heroID, heroPos, enemyID, enemyPos, !enemyIsAtRightSide); // lookLeft si NO está a la derecha
		auto secondaryData = GetNearHeroAttackPointInSide(heroID, heroPos, enemyID, enemyPos, enemyIsAtRightSide);

		// Si puede atacar en su propio lado, devolvemos ese sin mirar el otro
		if (std::get<2>(primaryData))
		{
			return primaryData;
		}

		// Si su lado está lleno, intentamos el otro (esto provocará el flanqueo)
		return secondaryData;
	}

	EnemyAttackOption BrawlerScene::PickHeroToFight(JUUID enemyController)
	{
		Thug* enemy = GetController<Thug>(enemyController);
		if (!enemy) return EnemyAttackOption();

		RenderableID enemyR = static_cast<RenderableID>(enemy->sceneObject);
		XMVECTOR enemyPos = enemyR->positionV();

		EnemyAttackOption bestOption;
		float minDistanceSq = FLT_MAX;

		for (auto& [heroID, attackQueues] : heroesAttackersQueues)
		{
			Hero* heroC = GetController<Hero>(heroID);
			if (!heroC) continue;

			RenderableID heroR = static_cast<RenderableID>(heroC->sceneObject);
			XMVECTOR heroPos = heroR->positionV();

			// 1. DETERMINAR EL FLANCO NATURAL (Evita cruzar por el medio)
			// Calculamos la posición relativa del enemigo respecto al héroe
			XMVECTOR diff = XMVectorSubtract(enemyPos, heroPos);
			float relativeX = XMVectorGetX(diff);

			// Si relativeX > 0, el enemigo está a la derecha. 
			// lookLeft será true solo si el enemigo está a la izquierda (relativeX < 0)
			bool enemyIsAtLeftSide = (relativeX < 0);

			// 2. OBTENER LA MEJOR OPCIÓN EN SU PROPIO LADO PRIMERO
			auto naturalData = GetNearHeroAttackPointInSide(heroID, heroPos, enemyController, enemyPos, enemyIsAtLeftSide);

			EnemiesAttackSides side = std::get<0>(naturalData);
			float distSq = std::get<1>(naturalData);
			bool can = std::get<2>(naturalData);
			bool attached = std::get<3>(naturalData);
			int idx = std::get<4>(naturalData);

			// 3. LÓGICA DE BALANCEO (FLANQUEO TÁCTICO)
			// Solo evaluamos el lado opuesto si:
			// a) El lado natural está lleno (can == false)
			// b) O si hay más de 1 enemigo en el lado natural y el opuesto está vacío
			size_t leftCount = attackQueues.farLeft.attached.size() + attackQueues.nearLeft.attached.size();
			size_t rightCount = attackQueues.farRight.attached.size() + attackQueues.nearRight.attached.size();

			bool shouldTryFlank = false;
			if (enemyIsAtLeftSide)
				shouldTryFlank = (!can || (leftCount > 1 && rightCount == 0));
			else
				shouldTryFlank = (!can || (rightCount > 1 && leftCount == 0));

			if (shouldTryFlank)
			{
				auto flankData = GetNearHeroAttackPointInSide(heroID, heroPos, enemyController, enemyPos, !enemyIsAtLeftSide);
				bool canFlank = std::get<2>(flankData);

				// Si el flanco opuesto es viable, lo tomamos
				if (canFlank) {
					side = std::get<0>(flankData);
					distSq = std::get<1>(flankData);
					can = canFlank;
					attached = std::get<3>(flankData);
					idx = std::get<4>(flankData);
				}
			}

			// 4. SELECCIÓN FINAL (Comparar contra otros héroes si los hay)
			if (can && distSq < minDistanceSq) {
				minDistanceSq = distSq;
				bestOption = EnemyAttackOption(heroID, heroR, heroC->capsuleRadius, side, can, attached, idx);
			}
		}

		return bestOption;
	}

	XMVECTOR BrawlerScene::GetHeroCombatPositionInQueue(EnemyAttackOption& attack)
	{
		if (!attack || !attack.heroRenderable) return XMVectorZero();

		Hero* heroC = GetController<Hero>(attack.heroID);
		if (!heroC) return attack.heroRenderable->positionV();

		// 1. Obtener las colas del héroe
		auto& heroQueues = heroesAttackersQueues.at(attack.heroID);

		XMVECTOR heroPos = attack.heroRenderable->positionV();
		XMFLOAT3 rawOffset;
		bool isLoneCombatant = false;

		// 2. Determinar offset y verificar si es el único enemigo en ese flanco (Left o Right)
		switch (attack.side)
		{
		case EAS_FarLeft:
			rawOffset = heroC->farLeftAttackOffset();
			// Es el único si FarLeft tiene 1 (él) y NearLeft tiene 0
			isLoneCombatant = (heroQueues.farLeft.attached.size() == 1 && heroQueues.nearLeft.attached.size() == 0);
			break;

		case EAS_NearLeft:
			rawOffset = heroC->nearLeftAttackOffset();
			isLoneCombatant = (heroQueues.nearLeft.attached.size() == 1 && heroQueues.farLeft.attached.size() == 0);
			break;

		case EAS_FarRight:
			rawOffset = heroC->farRightAttackOffset();
			isLoneCombatant = (heroQueues.farRight.attached.size() == 1 && heroQueues.nearRight.attached.size() == 0);
			break;

		case EAS_NearRight:
			rawOffset = heroC->nearRightAttackOffset();
			isLoneCombatant = (heroQueues.nearRight.attached.size() == 1 && heroQueues.farRight.attached.size() == 0);
			break;
		}

		XMVECTOR finalOffset = XMVectorSet(rawOffset.x, 0.0f, rawOffset.z, 0.0f);

		// 3. Aplicar Caso Especial: Si es el único en todo el flanco, sumar las Z
		if (isLoneCombatant)
		{
			float combinedZ = 0.0f;
			if (attack.side == EAS_FarLeft || attack.side == EAS_NearLeft)
			{
				combinedZ = heroC->farLeftAttackOffset().z + heroC->nearLeftAttackOffset().z;
			}
			else
			{
				combinedZ = heroC->farRightAttackOffset().z + heroC->nearRightAttackOffset().z;
			}
			finalOffset = XMVectorSetZ(finalOffset, combinedZ);
		}
		else if (attack.queueIndex > 0)
		{
			// 4. Si no es el único (hay gente en la otra cola o él es el 2º+), aplicar profundidad en X
			XMVECTOR xAxis = XMVectorMultiply(finalOffset, g_XMIdentityR0);
			XMVECTOR depthOffset = XMVectorScale(xAxis, static_cast<float>(attack.queueIndex));
			finalOffset = XMVectorAdd(finalOffset, depthOffset);
		}

		return XMVectorAdd(heroPos, finalOffset);
	}

	void BrawlerScene::RegisterEnemyInAttackQueue(JUUID enemyID, EnemyAttackOption& attack)
	{
		// 1. Validaciones de seguridad
		if (!attack || enemyID.empty()) return;

		// 2. Obtener las colas del héroe objetivo
		auto& heroQueues = heroesAttackersQueues.at(attack.heroID);
		AttackersQueue* targetQueue = nullptr;

		// 3. Seleccionar la cola específica según el lado de la opción de ataque
		switch (attack.side)
		{
		case EAS_FarLeft:   targetQueue = &heroQueues.farLeft;   break;
		case EAS_FarRight:  targetQueue = &heroQueues.farRight;  break;
		case EAS_NearLeft:  targetQueue = &heroQueues.nearLeft;  break;
		case EAS_NearRight: targetQueue = &heroQueues.nearRight; break;
		}

		if (!targetQueue) return;

		// 4. Verificar si el enemigo ya está registrado en ESTA cola
		if (targetQueue->attached.contains(enemyID)) return;

		// 5. Registrar al enemigo
		// El índice actual será el tamaño actual del vector de offsets (0-based)
		unsigned int newIndex = static_cast<unsigned int>(targetQueue->offsets.size());

		// Insertar en el set de IDs (para búsquedas rápidas)
		targetQueue->attached.insert(enemyID);

		// Mapear el ID al índice que ocupará
		targetQueue->attachedIndex[enemyID] = newIndex;

		// Calcular y guardar el offset específico para este índice
		// Usamos la lógica de desplazamiento en X para crear la fila
		Hero* heroC = GetController<Hero>(attack.heroID);
		XMFLOAT3 rawOffset;

		// Obtenemos el offset base definido en el héroe para este lado
		switch (attack.side)
		{
		case EAS_FarLeft:   rawOffset = heroC->farLeftAttackOffset();   break;
		case EAS_FarRight:  rawOffset = heroC->farRightAttackOffset();  break;
		case EAS_NearLeft:  rawOffset = heroC->nearLeftAttackOffset();  break;
		case EAS_NearRight: rawOffset = heroC->nearRightAttackOffset(); break;
		}

		XMVECTOR baseOffset = XMVectorSet(rawOffset.x, 0.0f, rawOffset.z, 0.0f);

		// Si no es el primero, escalamos el eje X según su posición en la fila
		if (newIndex > 0)
		{
			XMVECTOR xAxis = XMVectorMultiply(baseOffset, g_XMIdentityR0);
			XMVECTOR depthScale = XMVectorScale(xAxis, static_cast<float>(newIndex));
			baseOffset = XMVectorAdd(baseOffset, depthScale);
		}

		targetQueue->offsets.push_back(baseOffset);

		// 6. Actualizar la opción de ataque con el estado de registro
		attack.isAlreadyAttached = true;
		attack.queueIndex = newIndex;
	}

	void BrawlerScene::UnregisterEnemyFromAttackQueue(JUUID enemyID, EnemyAttackOption& attack)
	{
		// 1. Validaciones básicas y de estado
		if (enemyID.empty() || !attack || !attack.isAlreadyAttached) return;

		// 2. Obtener la cola específica usando los datos del AttackOption
		auto itHero = heroesAttackersQueues.find(attack.heroID);
		if (itHero == heroesAttackersQueues.end()) return;

		AttackersQueue* targetQueue = nullptr;
		switch (attack.side)
		{
		case EAS_FarLeft:   targetQueue = &itHero->second.farLeft;   break;
		case EAS_FarRight:  targetQueue = &itHero->second.farRight;  break;
		case EAS_NearLeft:  targetQueue = &itHero->second.nearLeft;  break;
		case EAS_NearRight: targetQueue = &itHero->second.nearRight; break;
		}

		if (!targetQueue || !targetQueue->attached.contains(enemyID)) return;

		// 3. Obtener el índice del que se va para saber a quiénes desplazar
		unsigned int oldIndex = targetQueue->attachedIndex[enemyID];

		// 4. Limpieza de estructuras
		targetQueue->attached.erase(enemyID);
		targetQueue->attachedIndex.erase(enemyID);

		// Al borrar del vector, los elementos posteriores se desplazan automáticamente
		if (oldIndex < targetQueue->offsets.size()) {
			targetQueue->offsets.erase(targetQueue->offsets.begin() + oldIndex);
		}

		// 5. Re-indexar a los enemigos restantes en el mapa
		for (auto& [id, index] : targetQueue->attachedIndex)
		{
			if (index > oldIndex)
			{
				index--;
			}
		}

		// 6. Resetear el estado de la opción de ataque pasada por referencia
		attack.isAlreadyAttached = false;
		attack.queueIndex = 0;
	}
};

// Constructor por defecto
EnemyAttackOption::EnemyAttackOption()
	: heroID(""), heroRenderable(), heroRadius(0.0f), side(EAS_NearLeft),
	canAttack(false), isAlreadyAttached(false), queueIndex(0) {
}

EnemyAttackOption::EnemyAttackOption(JUUID id, RenderableID renderable, float radius, EnemiesAttackSides s, bool can, bool attached, int idx)
	: heroID(id), heroRenderable(renderable), heroRadius(radius), side(s),
	canAttack(can), isAlreadyAttached(attached), queueIndex(idx) {
}

EnemyAttackOption::operator bool() const {
	return !heroID.empty() && heroRenderable;
}

bool EnemyAttackOption::operator==(const EnemyAttackOption& other) const {
	return heroID == other.heroID && side == other.side &&
		canAttack == other.canAttack && canAttack == other.canAttack &&
		isAlreadyAttached == other.isAlreadyAttached && queueIndex == other.queueIndex;
}