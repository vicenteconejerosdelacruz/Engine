#pragma once

#include <UUID.h>
#include <characterkinematic/PxController.h>
#include "PhysicObject.h"

namespace Scene
{
	DEF_SCENEOBJECT_ID_DEP(PhysicScene);
};

using namespace Scene;
using namespace physx;

class CharacterHitCallback : public PxUserControllerHitReport
{
public:
	CharacterHitCallback(PhysicObjectID object);
	~CharacterHitCallback() {}

	void onShapeHit(const PxControllerShapeHit& hit) override;
	// Obligatorios por la interfaz
	void onControllerHit(const PxControllersHit& hit) override {}
	void onObstacleHit(const PxControllerObstacleHit& hit) override {}

	PhysicObjectID physicObject;
};