#include "pch.h"
#include "Thug.h"
#include "../../Scene/BrawlerScene.h"
#include <StepTimer.h>
#include <NoStd.h>
#include <SimpleMath.h>
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
	/*
	BrawlerScene* Thug::GetBrawlerSceneController()
	{
		return GetController<BrawlerScene>(unit, sceneController());
	}
	*/

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
		//v8_register_method<Thug>(isolate, tpl, "EvaluateNextFollowMovement", script, [](Thug* self) { if (self) self->EvaluateNextFollowMovement(); });
		v8_register_method<Thug>(isolate, tpl, "CombatIdleNextState", script, [](Thug* self) { if (self) self->CombatIdleNextState(); });
		v8_register_method<Thug>(isolate, tpl, "OnCombatPunchAnimationEnd", script, [](Thug* self) { if (self) self->OnCombatPunchAnimationEnd(); });
		v8_register_method<Thug>(isolate, tpl, "TakeHit", script, [](Thug* self, int damage) { if (self) self->TakeHit(damage); });
		v8_register_method<Thug>(isolate, tpl, "OnDeathAnimationEnd", script, [](Thug* self) { if (self) self->OnDeathAnimationEnd(); });
		v8_register_method<Thug>(isolate, tpl, "PlayPunchSound", script, [](Thug* self) { if (self) self->PlayPunchSound(); });
	}

	void Thug::SetInitialConditions()
	{
		BrawlerCharacter::SetInitialConditions();
		tsm.currentState = TS_None;
		health(initialHealth);
		pickedHero = EnemyAttackOption();
		combatEnabled(false);
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
#include <GamePhysics.h>

	void Thug::Map(SUUUID so)
	{
		using namespace Scene;
		BrawlerCharacter::Map(so);
		SceneObjectType type = GetSceneObjectType(FROMSUUUID(so));

		if (type == SO_Renderables)
		{
			renderable = so;
		}

		physicScene = MAKESUUUID(unit, *GetPhysicScenes(unit).begin());
		physicObject = renderable->at("physicObject").at(0);

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
		BrawlerScene* brawler = GetBrawlerScene(this);
		EnemyAttackOption attack = brawler->PickHeroToFight(uuid());
		if (pickedHero != attack)
		{
			brawler->UnregisterEnemyFromAttackQueue(uuid(), pickedHero);
			brawler->RegisterEnemyInAttackQueue(uuid(), attack);
		}
		pickedHero = attack;
	}

	//Step
	void Thug::Step(float delta)
	{
#if defined(_EDITOR)
		if (!Editor::IsPlaying(unit) || Editor::IsPaused(unit))
			return;
#endif

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
		if (!pickedHero)
			return;

		float dx = pickedHero.heroRenderable->position().x - renderable->position().x;
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
		if (!pickedHero)
			return false;

		BrawlerScene* brawler = GetBrawlerScene(this);
		XMVECTOR heroPos = brawler->GetHeroCombatPositionInQueue(pickedHero);
		XMVECTOR myPos = renderable->positionV();

		// 1. Vector diferencia en el plano XZ
		XMVECTOR diff = XMVectorSubtract(heroPos, myPos);
		static const XMVECTORF32 MaskXZ = { 1.0f, 0.0f, 1.0f, 0.0f };
		diff = XMVectorMultiply(diff, MaskXZ);

		// 2. Extraer componentes
		float diffX = fabsf(XMVectorGetX(diff));
		float diffZ = fabsf(XMVectorGetZ(diff));

		// 3. Validación de Distancia Circular Máxima
		float lenSq = XMVectorGetX(XMVector3LengthSq(diff));
		float combatDist = combatMinDistanceToAttack();

		if (lenSq > (combatDist * combatDist))
			return false;

		// 4. Validación de Alineación (Ángulo + Tolerancia fija)
		// tan(10°) ≈ 0.1763
		const float tan10deg = 0.176326f;

		// Tolerancia fija en Z (ejemplo: 0.1 unidades). 
		// Esto permite atacar si estás muy cerca aunque el ángulo sea "grande".
		const float zTolerance = 0.1f;

		// Es válido si:
		// a) Está dentro del ángulo de 10 grados respecto a la X
		// b) O si la diferencia en Z es menor a la tolerancia fija (está en el mismo carril)
		bool isAlignedZ = (diffZ <= (diffX * tan10deg)) || (diffZ <= zTolerance);

		return isAlignedZ;
	}

	//Idle
	//bool ThugController::ShouldIdle()
	//{
	//	return false;
	//}

	void Thug::EnterIdle()
	{
		renderable->SetCurrentAnimation(idleAnimation(), 0.0f, 1.0f, true, true);
	}

	void Thug::LeaveIdle()
	{
	}

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
		//EvaluateNextFollowMovement();
	}

	void Thug::CombatFollow()
	{
		if (IsInAttackRange())
		{
			tsm.ChangeState(TS_CombatPunch);
		}
		else
		{
			PickHeroToFight();

			UpdateLookTo();

			// 1. Obtenemos el vector de movimiento calculado
			XMVECTOR currentMoveDir = CalculateSteeringDirection();

			// 2. Evaluamos qué animación tocar y obtenemos la velocidad asociada
			float adaptiveSpeed = EvaluateNextFollowMovement(currentMoveDir);

			// 3. Movemos al personaje con la velocidad adaptada
			CharacterMoveXZPlane(currentMoveDir, gameUpdateFrequency, adaptiveSpeed, physicScene->gravity());

		}
	}

	XMVECTOR Thug::CalculateSteeringDirection()
	{
		BrawlerScene* brawler = GetBrawlerScene(this);
		XMVECTOR targetPos = brawler->GetHeroCombatPositionInQueue(pickedHero);
		XMVECTOR myPos = renderable->positionV();
		XMVECTOR actualHeroPos = pickedHero.heroRenderable->positionV();

		XMVECTOR toTarget = XMVectorMultiply({ 1.0f, 0.0f, 1.0f, 0.0f }, XMVectorSubtract(targetPos, myPos));
		XMVECTOR toHero = XMVectorMultiply({ 1.0f, 0.0f, 1.0f, 0.0f }, XMVectorSubtract(actualHeroPos, myPos));

		float distToHero = XMVectorGetX(XMVector3Length(toHero));
		float distToTarget = XMVectorGetX(XMVector3Length(toTarget));
		const float avoidRadius = pickedHero.heroRadius * 2.0f + 0.2f;

		if (distToHero < distToTarget && distToHero < avoidRadius * 2.0f)
		{
			XMVECTOR cross = XMVector3Cross(toTarget, toHero);
			float side = XMVectorGetY(cross);

			XMVECTOR avoidanceDir;
			if (side > 0)
				avoidanceDir = XMVectorSet(-XMVectorGetZ(toHero), 0.0f, XMVectorGetX(toHero), 0.0f);
			else
				avoidanceDir = XMVectorSet(XMVectorGetZ(toHero), 0.0f, -XMVectorGetX(toHero), 0.0f);

			float weight = 1.0f - (distToHero / (avoidRadius * 2.0f));
			return XMVector3Normalize(XMVectorLerp(XMVector3Normalize(toTarget), XMVector3Normalize(avoidanceDir), weight));
		}

		return XMVector3Normalize(toTarget);
	}

	void Thug::MoveTowardHero(float speed)
	{
		if (!pickedHero) return;

		BrawlerScene* brawler = GetBrawlerScene(this);
		XMVECTOR targetPos = brawler->GetHeroCombatPositionInQueue(pickedHero);
		XMVECTOR myPos = renderable->positionV();

		// Posición actual del héroe (no el punto de la cola)
		XMVECTOR actualHeroPos = pickedHero.heroRenderable->positionV();

		// 1. Vector hacia el objetivo y vector hacia el centro del héroe
		XMVECTOR toTarget = XMVectorMultiply({ 1.0f, 0.0f, 1.0f, 0.0f }, XMVectorSubtract(targetPos, myPos));
		XMVECTOR toHero = XMVectorMultiply({ 1.0f, 0.0f, 1.0f, 0.0f }, XMVectorSubtract(actualHeroPos, myPos));

		float distToHero = XMVectorGetX(XMVector3Length(toHero));
		float distToTarget = XMVectorGetX(XMVector3Length(toTarget));

		// 2. Definir radio de evitación (Radio de la cápsula del héroe + margen)
		// Supongamos que el radio es 1.0f, añadimos 0.5f de margen
		const float avoidRadius = pickedHero.heroRadius * 2.0f + 0.2f;

		XMVECTOR finalDisp;

		// 3. ¿El héroe está en medio? 
		// Si la distancia al héroe es menor que la del objetivo, y estamos lo suficientemente cerca para chocar
		if (distToHero < distToTarget && distToHero < avoidRadius * 2.0f)
		{
			// Calculamos el producto cruz para saber si el héroe está a la izquierda o derecha de nuestro camino
			XMVECTOR cross = XMVector3Cross(toTarget, toHero);
			float side = XMVectorGetY(cross);

			// Generamos un vector perpendicular (Normal) para rodear
			// Si side > 0, el héroe está a la derecha, nos movemos a la izquierda y viceversa
			XMVECTOR avoidanceDir;
			if (side > 0)
				avoidanceDir = XMVectorSet(-XMVectorGetZ(toHero), 0.0f, XMVectorGetX(toHero), 0.0f);
			else
				avoidanceDir = XMVectorSet(XMVectorGetZ(toHero), 0.0f, -XMVectorGetX(toHero), 0.0f);

			avoidanceDir = XMVector3Normalize(avoidanceDir);

			// Mezclamos el movimiento hacia el objetivo con el de evitación (Curva)
			// A más cerca del héroe, más fuerza de evitación
			float weight = 1.0f - (distToHero / (avoidRadius * 2.0f));
			finalDisp = XMVector3Normalize(XMVectorLerp(XMVector3Normalize(toTarget), avoidanceDir, weight));
		}
		else
		{
			finalDisp = XMVector3Normalize(toTarget);
		}

		CharacterMoveXZPlane(finalDisp, gameUpdateFrequency, speed, physicScene->gravity());
	}
	/*
	void Thug::MoveTowardHero(float speed)
	{
		if (!pickedHero)
			return;

		BrawlerScene* brawler = GetBrawlerScene(this);
		XMVECTOR heroPos = brawler->GetHeroCombatPositionInQueue(pickedHero);
		XMVECTOR myPos = renderable->positionV();
		XMVECTOR diff = XMVectorMultiply({ 1.0f,0.0f,1.0f,0.0f }, XMVectorSubtract(heroPos, myPos));
		XMVECTOR disp = XMVector3Normalize(diff);
		CharacterMoveXZPlane(disp, gameUpdateFrequency, speed, physicScene->gravity());
	}
	*/

	float Thug::EvaluateNextFollowMovement(XMVECTOR actualMovementDir)
	{
		if (!pickedHero) return walkSpeed(); // Velocidad base por defecto

		XMVECTOR movementDir = XMVector3Normalize(XMVectorMultiply({ 1.0f, 0.0f, 1.0f, 0.0f }, actualMovementDir));
		XMVECTOR right = { 1.0f, 0.0f, 0.0f, 0.0f };
		XMVECTOR radians = XMVector3AngleBetweenVectors(movementDir, right);
		float degree = XMConvertToDegrees(XMVectorGetX(radians));

		float nearFarThreshold = combatMoveNearFarAngle();
		bool isHorizontal = (degree <= nearFarThreshold) || (degree >= 180.0f - nearFarThreshold);

		if (isHorizontal)
		{
			if (!followAnimationPlaying() || renderable->animationSequence() != combatMoveFwAnimation())
			{
				renderable->SetCurrentAnimation(combatMoveFwAnimation(), 0.0f, combatMoveFwTimeFactor(), true, false);
				followAnimationPlaying(true);
			}
			return walkSpeed(); // Velocidad estándar para caminar de frente
		}
		else if (XMVectorGetZ(movementDir) > 0.0f) // Fondo
		{
			if (!followAnimationPlaying() || renderable->animationSequence() != combatMoveFarAnimation())
			{
				renderable->SetCurrentAnimation(combatMoveFarAnimation(), 0.0f, combatMoveNearFarTimeFactor(), true, false);
				followAnimationPlaying(true);
			}
			return combatMoveFarSpeed(); // Nueva función/variable que definas
		}
		else // Cámara
		{
			if (!followAnimationPlaying() || renderable->animationSequence() != combatMoveNearAnimation())
			{
				renderable->SetCurrentAnimation(combatMoveNearAnimation(), 0.0f, combatMoveNearFarTimeFactor(), true, false);
				followAnimationPlaying(true);
			}
			return combatMoveNearSpeed(); // Nueva función/variable que definas
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
		GetBrawlerScene(this)->EnemyDeath(uuid(), pickedHero);
		renderable->SetCurrentAnimation(deathAnimation(), 0.0f, deathTimeFactor());
	}

	void Thug::OnDeathAnimationEnd()
	{
		renderable->markedForDelete = true;
	}
};