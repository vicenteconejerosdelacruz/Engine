#include "pch.h"
#include "Thug.h"
#include "../../Scene/BrawlerScene.h"
#include <StepTimer.h>
#include <NoStd.h>
#include <SimpleMath.h>
#include <GamePhysics.h>
#include <Brawler/Characters/Heroes/Hero.h>
#if defined(_EDITOR)
#include <Editor.h>
#endif

//Timer
extern DX::StepTimer timer;
extern float gameUpdateFrequency;
namespace Game::Brawler
{
#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include "ThugAtt.h"
#include <JEnd.h>
#endif

	static BrawlerScene* GetBrawlerScene(Thug* thug)
	{
		return GetController<BrawlerScene>(thug->unit, thug->sceneController());
	}

	//Constructor and Binding
	Thug::Thug(nlohmann::json& json) : BrawlerCharacter(json)
	{
#include <Attributes/JInit.h>
#include "ThugAtt.h"
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include "ThugAtt.h"
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include "ThugAtt.h"
#include <JEnd.h>

		tsm = {
			.currentState = TS_None,
			.onEnter = {
				{ TS_Idle, [&](auto* sm, ThugStates prevState) { EnterIdle(); }},
				{ TS_CombatIdle, [&](auto* sm, ThugStates prevState) { EnterCombatIdle(); }},
				{ TS_CombatFollow, [&](auto* sm, ThugStates prevState) { EnterCombatFollow(); }},
				{ TS_CombatPunch, [&](auto* sm, ThugStates prevState) { EnterCombatPunch(); }},
				{ TS_Death, [&](auto* sm, ThugStates prevState) { EnterDeath(); }},
			},
			.onLeave = {
				{ TS_Idle,[&](auto* sm, ThugStates prevState) { LeaveIdle(); }},
			},
			.onStep = {
				{ TS_None, [&](auto* sm) { thugScale = renderable->scale(); tsm.ChangeState(TS_Idle); }},
				{ TS_Idle, [&](auto* sm) { Idle(); }},
				{ TS_CombatIdle, [&](auto* sm) { CombatIdle(); }},
				{ TS_CombatFollow, [&](auto* sm) { CombatFollow(); }},
				{ TS_CombatPunch, [&](auto* sm) { CombatPunch(); }},
			}
		};

		initialHealth = health();
		SetInitialConditions();
	}

	void Thug::RegisterScript(Isolate* isolate, Local<ObjectTemplate> tpl, SceneUnitScripting* script)
	{
		v8_register_method<Thug>(isolate, tpl, "CombatIdleNextState", script, [](Thug* self) { if (self) self->CombatIdleNextState(); });
		v8_register_method<Thug>(isolate, tpl, "OnCombatPunchAnimationEnd", script, [](Thug* self) { if (self) self->OnCombatPunchAnimationEnd(); });
		v8_register_method<Thug>(isolate, tpl, "TakeHit", script, [](Thug* self, int damage) { if (self) self->TakeHit(damage); });
		v8_register_method<Thug>(isolate, tpl, "OnDeathAnimationEnd", script, [](Thug* self) { if (self) self->OnDeathAnimationEnd(); });
		v8_register_method<Thug>(isolate, tpl, "PlayPunchSound", script, [](Thug* self) { if (self) self->PlayPunchSound(); });
	}

	void Thug::SetInitialConditions()
	{
		tsm.currentState = TS_None;
		health(initialHealth);
		pickedHeroID.clear();
		combatEnabled(false);
		BrawlerCharacter::SetInitialConditions();
	}

#if defined(_EDITOR)
	void Thug::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include "ThugAtt.h"
#include <JEnd.h>
		BrawlerCharacter::WriteJson(j);
	}
#endif

	void Thug::Map(SUUUID so)
	{
		using namespace Scene;
		BrawlerCharacter::Map(so);
		SceneObjectType type = GetSceneObjectType(FROMSUUUID(so));

		if (type == SO_Renderables)
		{
			renderable = so;
		}

		if (GetCountFromPhysicScenes(unit) > 0ULL)
		{
			physicScene = MAKESUUUID(unit, *GetPhysicScenes(unit).begin());
			if (renderable->at("physicObject").size() > 0ULL)
			{
				physicObject = renderable->at("physicObject").at(0);
			}
		}

		SetInitialConditions();
	}

	void Thug::Unmap()
	{
		BrawlerCharacter::Unmap();
	}

	void Thug::TakeHit(int damage)
	{
		if (tsm.currentState == TS_Death) return;
		health(std::max(0, health() - damage));
		auto* brawler = GetBrawlerScene(this);
		brawler->UpdateEnemy(uuid());
		if (health() == 0)
		{
			brawler->AddScore(score());
		}
	}

	void Thug::PickHeroToFight()
	{
		BrawlerScene* scene = GetBrawlerScene(this);
		const auto& heroIDs = scene->heroes();

		if (heroIDs.empty()) return;

		// Buscamos al héroe más cercano que tenga slots libres
		JUUID bestHero;
		float closestDist = FLT_MAX;

		for (const auto& hID : heroIDs)
		{
			Hero* hero = GetController<Hero>(hID);
			if (!hero) continue;

			// Permitimos un máximo de 4 atacantes repartidos (2 por lado)
			if (scene->CanJoinCombat(hID, 4))
			{
				RenderableID heroR = hero->sceneObject;
				float dist = XMVectorGetX(XMVector3Length(XMVectorSubtract(heroR->positionV(), renderable->positionV())));
				if (dist < closestDist)
				{
					closestDist = dist;
					bestHero = hID;
				}
			}
		}

		if (!bestHero.empty())
		{
			// Guardamos nuestro objetivo
			pickedHeroID = bestHero;
			// IMPORTANTE: Nos registramos en la escena para obtener un slot
			scene->RegisterThugInCombat(bestHero, uuid());
		}
	}

	void Thug::UnregisterFromCombat()
	{
		auto* scene = GetBrawlerScene(this);
		scene->UnregisterThugFromCombat(uuid());
		pickedHeroID.clear();
	}

	//Step
	void Thug::Step(float delta)
	{
#if defined(_EDITOR)
		if (!Editor::IsPlaying(unit) || Editor::IsPaused(unit))
			return;
#endif
		if (physicObject.empty())
			return;

		float dt = static_cast<float>(timer.GetElapsedSeconds());
		std::string Rname = renderable->name();

		tsm.Step();
		if (ShouldDie())
		{
			tsm.ChangeState(TS_Death);
		}
	}

	class BrawlerCCTFilter : public PxControllerFilterCallback {
	public:
		virtual bool filter(const PxController& a, const PxController& b) override {
			// Obtenemos nuestros PhysicObject guardados en el userData
			PhysicObject* objA = static_cast<PhysicObject*>(a.getUserData());
			PhysicObject* objB = static_cast<PhysicObject*>(b.getUserData());

			if (!objA || !objB) return true;

			// Si AMBOS son enemigos, permitimos que se atraviesen
			if ((objA->objectMask() & CM_Enemy) && (objB->objectMask() & CM_Enemy)) {
				return false; // Retornar falso significa "ignorar colisión"
			}

			return true; // Colisionar normalmente (Enemigo vs Hero, Enemigo vs Static, etc.)
		}
	};
	static BrawlerCCTFilter gCCTFilter;
	void Thug::CharacterMoveXZPlane(XMVECTOR displacement, float dt, float sideSpeed, XMFLOAT3 gravity)
	{
		PxControllerFilters filters;
		filters.mCCTFilterCallback = &gCCTFilter;
		XMVECTOR move = XMVector3Normalize(displacement) * sideSpeed * dt;
		physicObject->MoveCharacter(move, dt, filters);
	}

	void Thug::UpdateLookTo()
	{
		if (pickedHeroID.empty())
			return;

		RenderableID heroRenderable = GetController<Hero>(pickedHeroID)->sceneObject;
		float dx = heroRenderable->position().x - renderable->position().x;
		if (dx > 0.0f)
		{
			if (lookingTo() == CLT_Left)
			{
				lookingTo(CLT_Right);
				XMFLOAT3 scl = renderable->scale() * lookToSwapVector();
				renderable->scale(scl);
			}
		}
		else if (dx < 0.0f)
		{
			if (lookingTo() == CLT_Right)
			{
				lookingTo(CLT_Left);
				XMFLOAT3 scl = renderable->scale() * lookToSwapVector();
				renderable->scale(scl);
			}
		}
	}

	bool Thug::IsInAttackRange()
	{
		// 1. Verificación de seguridad
		if (pickedHeroID.empty())
			return false;

		Hero* hero = GetController<Hero>(pickedHeroID);
		if (!hero)
			return false;

		// 2. Acceso a Renderables (Arquitectura Engine)
		RenderableID myR = this->sceneObject;
		RenderableID heroR = hero->sceneObject;

		XMVECTOR myPos = myR->positionV();
		XMVECTOR actualHeroPos = heroR->positionV();

		// 3. Vector diferencia en el plano XZ
		XMVECTOR diff = XMVectorSubtract(actualHeroPos, myPos);
		static const XMVECTORF32 MaskXZ = { 1.0f, 0.0f, 1.0f, 0.0f };
		XMVECTOR diffXZ = XMVectorMultiply(diff, MaskXZ);

		// 4. Validación de Distancia (Rango de ataque)
		float lenSq = XMVectorGetX(XMVector3LengthSq(diffXZ));
		float combatDist = combatMinDistanceToAttack();

		// Si está más lejos de lo que permiten sus puños, fuera
		if (lenSq > (combatDist * combatDist))
			return false;

		// 5. Validación de Alineación (El "Carril" del Brawler)
		// Extraemos componentes para validar la profundidad (Z)
		float diffX = fabsf(XMVectorGetX(diffXZ));
		float diffZ = fabsf(XMVectorGetZ(diffXZ));

		// Tan(10°) ≈ 0.1763. Define un cono de ataque frontal.
		const float tan10deg = 0.176326f;
		// Tolerancia fija en Z para ataques muy cercanos.
		const float zTolerance = 0.1f;

		// Es válido si el enemigo está alineado en el carril Z respecto al héroe
		bool isAlignedZ = (diffZ <= (diffX * tan10deg)) || (diffZ <= zTolerance);

		// 6. Validación de altura (Y) - Failsafe para saltos
		float diffY = fabsf(XMVectorGetY(actualHeroPos) - XMVectorGetY(myPos));
		if (diffY > 1.0f)
			return false;

		return isAlignedZ;
	}

	//Idle
	//bool ThugController::ShouldIdle()
	//{
	//	return false;
	//}

	void Thug::EnterIdle()
	{
		renderable->SetCurrentAnimation(idleAnimation(), 0.0f, idleTimeFactor(), true, true);
		UnregisterFromCombat();
	}

	void Thug::LeaveIdle()
	{}

	void Thug::Idle()
	{
		if (!combatEnabled())
			return;

		PickHeroToFight();

		bool inAttackRange = IsInAttackRange();
		if (inAttackRange)
		{
			tsm.ChangeState(TS_CombatIdle);
		}
		else
		{
			tsm.ChangeState(TS_CombatFollow);
		}
	}

	//CombatIdle
	void Thug::EnterCombatIdle()
	{
		renderable->SetCurrentAnimation(combatIdleAnimation(), 0.0f, combatIdleTimeFactor(), true, true);
	}

	void Thug::CombatIdle()
	{
		UpdateLookTo();
		bool inAttackRange = IsInAttackRange();
		if (!inAttackRange)
		{
			tsm.ChangeState(TS_CombatFollow);
		}
	}

	void Thug::CombatIdleNextState()
	{
		bool inAttackRange = IsInAttackRange();
		if (!inAttackRange)
		{
			tsm.ChangeState(TS_CombatFollow);
		}
		else
		{
			tsm.ChangeState(TS_CombatPunch);
		}
	}

	void Thug::EnterCombatFollow()
	{
		PickHeroToFight();
	}

	void Thug::CombatFollow()
	{
		// Si no tenemos héroe o el que teníamos ya no es válido, buscamos uno
		if (pickedHeroID.empty())
		{
			PickHeroToFight();
			if (pickedHeroID.empty())
				return; // No hay nadie a quien pelear
		}

		if (IsInAttackRange())
		{
			tsm.ChangeState(TS_CombatPunch);
		}
		else
		{
			UpdateLookTo();

			// 1. Calculamos la dirección usando el Steering que ya tenemos
			XMVECTOR currentMoveDir = CalculateSteeringDirection();

			// 2. Evaluamos animación y velocidad (Near/Far/Fw)
			float adaptiveSpeed = EvaluateNextFollowMovement(currentMoveDir);

			// 3. Movemos al personaje
			CharacterMoveXZPlane(currentMoveDir, gameUpdateFrequency, adaptiveSpeed, physicScene->gravity());
		}
	}

	XMVECTOR Thug::CalculateSteeringDirection()
	{
		BrawlerScene* brawler = GetBrawlerScene(this);

		// 1. Obtenemos el punto exacto que nos toca según nuestro índice de slot
		// Esta función ya nos devuelve la posición (Frente, Espalda, etc.)
		XMVECTOR targetPos = brawler->GetHeroCombatPositionForThug(pickedHeroID, uuid());

		XMVECTOR myPos = renderable->positionV();
		Hero* hero = GetController<Hero>(pickedHeroID);
		RenderableID heroR = hero->sceneObject;
		XMVECTOR actualHeroPos = heroR->positionV();

		// 2. Proyectamos vectores en el plano XZ (ignoramos altura para el steering)
		XMVECTOR maskXZ = { 1.0f, 0.0f, 1.0f, 0.0f };
		XMVECTOR toTarget = XMVectorMultiply(maskXZ, XMVectorSubtract(targetPos, myPos));
		XMVECTOR toHero = XMVectorMultiply(maskXZ, XMVectorSubtract(actualHeroPos, myPos));

		float distToHero = XMVectorGetX(XMVector3Length(toHero));
		float distToTarget = XMVectorGetX(XMVector3Length(toTarget));

		// El radio de evitación (el cuerpo del héroe)
		const float avoidRadius = hero->capsuleRadius * 2.0f + 0.2f;

		// 3. LÓGICA DE EVITACIÓN (Steering)
		// Si el héroe está entre nosotros y el target, y estamos cerca de su radio
		if (distToHero < distToTarget && distToHero < avoidRadius * 2.0f)
		{
			// Calculamos hacia qué lado rodear
			XMVECTOR cross = XMVector3Cross(toTarget, toHero);
			float side = XMVectorGetY(cross);

			XMVECTOR avoidanceDir;
			if (side > 0) // El héroe está a nuestra derecha, rodeamos por la izquierda
				avoidanceDir = XMVectorSet(-XMVectorGetZ(toHero), 0.0f, XMVectorGetX(toHero), 0.0f);
			else         // El héroe está a nuestra izquierda, rodeamos por la derecha
				avoidanceDir = XMVectorSet(XMVectorGetZ(toHero), 0.0f, -XMVectorGetX(toHero), 0.0f);

			// Mezclamos el camino directo al target con el camino de evitación
			// A menor distancia del héroe, más peso tiene la evitación (curva más cerrada)
			float weight = 1.0f - std::clamp(distToHero / (avoidRadius * 2.0f), 0.0f, 1.0f);

			XMVECTOR normTarget = XMVector3Normalize(toTarget);
			XMVECTOR normAvoid = XMVector3Normalize(avoidanceDir);

			return XMVector3Normalize(XMVectorLerp(normTarget, normAvoid, weight));
		}

		// Si el camino está despejado, vamos directo al target del slot
		return XMVector3Normalize(toTarget);
	}

	float Thug::EvaluateNextFollowMovement(XMVECTOR actualMovementDir)
	{
		if (pickedHeroID.empty()) return walkSpeed(); // Velocidad base por defecto

		// 1. Normalizamos la dirección de movimiento en el plano XZ
		XMVECTOR movementDir = XMVector3Normalize(XMVectorMultiply({ 1.0f, 0.0f, 1.0f, 0.0f }, actualMovementDir));

		// 2. Usamos el vector derecha (1,0,0) como referencia para el ángulo
		XMVECTOR right = { 1.0f, 0.0f, 0.0f, 0.0f };
		XMVECTOR radians = XMVector3AngleBetweenVectors(movementDir, right);
		float degree = XMConvertToDegrees(XMVectorGetX(radians));

		// 3. Umbral para decidir si el movimiento es "Horizontal" o "Profundidad"
		// combatMoveNearFarAngle suele ser unos 45-60 grados
		float nearFarThreshold = combatMoveNearFarAngle();

		// Es horizontal si el ángulo está cerca de 0° (derecha) o 180° (izquierda)
		bool isHorizontal = (degree <= nearFarThreshold) || (degree >= 180.0f - nearFarThreshold);

		if (isHorizontal)
		{
			// CAMINAR DE FRENTE (Hacia los lados del héroe)
			if (!followAnimationPlaying() || renderable->animationSequence() != combatMoveFwAnimation())
			{
				renderable->SetCurrentAnimation(combatMoveFwAnimation(), 0.0f, combatMoveFwTimeFactor(), true, false);
				followAnimationPlaying(true);
			}
			return walkSpeed();
		}
		else if (XMVectorGetZ(movementDir) > 0.0f)
		{
			// CAMINAR HACIA EL FONDO (Z positivo)
			if (!followAnimationPlaying() || renderable->animationSequence() != combatMoveFarAnimation())
			{
				renderable->SetCurrentAnimation(combatMoveFarAnimation(), 0.0f, combatMoveNearFarTimeFactor(), true, false);
				followAnimationPlaying(true);
			}
			return combatMoveFarSpeed();
		}
		else
		{
			// CAMINAR HACIA LA CÁMARA (Z negativo)
			if (!followAnimationPlaying() || renderable->animationSequence() != combatMoveNearAnimation())
			{
				renderable->SetCurrentAnimation(combatMoveNearAnimation(), 0.0f, combatMoveNearFarTimeFactor(), true, false);
				followAnimationPlaying(true);
			}
			return combatMoveNearSpeed();
		}
	}

	//CombatPunch
	void Thug::EnterCombatPunch()
	{
		renderable->SetCurrentAnimation(combatPunchAnimation(), 0.0f, combatPunchTimeFactor(), true, false);
	}

	void Thug::CombatPunch()
	{

	}

	void Thug::OnCombatPunchAnimationEnd()
	{
		tsm.ChangeState(TS_CombatIdle);
	}

	void Thug::PlayPunchSound()
	{
		//don't play the sound if in gameover or level complete
		if (GetBrawlerScene(this)->IsGameOver() || GetBrawlerScene(this)->IsLevelComplete())
			return;

		if (punchSounds().size() == 0ULL) return;
		SoundFXID sfx = SoundFXID(unit, punchSounds().at(0));
		sfx->Stop();
		sfx->Play();
	}

	bool Thug::ShouldDie()
	{
		return health() <= 0 && tsm.currentState != TS_Death;
	}

	void Thug::EnterDeath()
	{
		renderable->SetCurrentAnimation(deathAnimation(), 0.0f, deathTimeFactor());
		UnregisterFromCombat();
		GetBrawlerScene(this)->DecreaseEnemiesInRound(-1);
	}

	void Thug::OnDeathAnimationEnd()
	{
		renderable->markedForDelete = true;
	}
};