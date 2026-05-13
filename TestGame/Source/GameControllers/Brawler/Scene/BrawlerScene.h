#pragma once
#include "../Camera/BrawlerCamera.h"
#include "../Rounds/BrawlerRound.h"
#include <Controller.h>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <unordered_map>

enum EnemiesAttackSides
{
	EAS_FarLeft,
	EAS_FarRight,
	EAS_NearLeft,
	EAS_NearRight,
};

static inline std::unordered_map<EnemiesAttackSides, std::string> EnemiesAttackSidesToString =
{
	{ EAS_FarLeft, "FarLeft" },
	{ EAS_FarRight, "FarRight" },
	{ EAS_NearLeft, "NearLeft" },
	{ EAS_NearRight, "NearRight" },
};

static inline std::unordered_map<std::string, EnemiesAttackSides> StringToEnemiesAttackSides =
{
	{ "FarLeft", EAS_FarLeft },
	{ "FarRight", EAS_FarRight },
	{ "NearLeft", EAS_NearLeft },
	{ "NearRight", EAS_NearRight },
};

struct AttackersQueue
{
	std::set<JUUID> attached;
	std::map<JUUID, unsigned int> attachedIndex;
	std::vector<XMVECTOR> offsets;
};

struct HeroAttackersQueues
{
	AttackersQueue farLeft;
	AttackersQueue farRight;
	AttackersQueue nearLeft;
	AttackersQueue nearRight;
};

// <Lado, DistanciaSq, PuedeAtacar, YaEstaAtachado, IndiceEnCola>
using EnemyAttackPoint = std::tuple<EnemiesAttackSides, float, bool, bool, int>;

struct EnemyAttackOption;

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

			XMVECTOR GetHeroCombatPositionInSide(JUUID heroID, XMVECTOR heroPos, EnemiesAttackSides side, int queueIndex);
			std::tuple<EnemiesAttackSides, float, bool, bool, int> GetNearHeroAttackPointInSide(JUUID heroID, XMVECTOR heroPos, JUUID enemyID, XMVECTOR enemyPos, bool lookLeft);
			std::tuple<EnemiesAttackSides, float, bool, bool, int> GetNearHeroAttackPoint(JUUID heroID, XMVECTOR heroPos, JUUID enemyID, XMVECTOR enemyPos);
			EnemyAttackOption PickHeroToFight(JUUID enemyController);
			XMVECTOR GetHeroCombatPositionInQueue(EnemyAttackOption& attack);
			void RegisterEnemyInAttackQueue(JUUID enemyID, EnemyAttackOption& attack);
			void UnregisterEnemyFromAttackQueue(JUUID enemyID, EnemyAttackOption& attack);
			void EnemyDeath(JUUID enemyID, EnemyAttackOption& attack);

			//HeroID, HeroAttackersQueue
			std::map<JUUID, HeroAttackersQueues> heroesAttackersQueues;
			//ThugId, AttackersQueue*
			std::map<JUUID, AttackersQueue*> enemiesAttackQueue;
			//ThudID, HeroID
			std::map<JUUID, JUUID> heroToAttackByEnemy;
			//ThuhgID, tuple<HeroID, HeroRenderableID, Offset, attackMode>
			std::map<JUUID, std::tuple<JUUID, RenderableID, XMVECTOR, bool>> enemiesCurrentOptions;
		};
	};
};

struct EnemyAttackOption {
	JUUID heroID;
	RenderableID heroRenderable;
	float heroRadius;
	EnemiesAttackSides side;
	bool canAttack;
	bool isAlreadyAttached;
	int queueIndex;

	// Constructor por defecto
	EnemyAttackOption();

	EnemyAttackOption(JUUID id, RenderableID renderable, float radius, EnemiesAttackSides s, bool can, bool attached, int idx);

	// Validez: El string no debe estar vacío Y el RenderableID debe ser válido
	explicit operator bool() const;
	bool operator==(const EnemyAttackOption& other) const;
};
