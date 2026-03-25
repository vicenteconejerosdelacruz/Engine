#include "pch.h"
#include "CharacterHitCallback.h"
#include <PxShape.h>
using namespace Physics;

CharacterHitCallback::CharacterHitCallback(PhysicObjectID object)
{
	physicObject = object;
}

void CharacterHitCallback::onShapeHit(const PxControllerShapeHit& hit) {
	CallCharacterHitCallback(physicObject(), hit.shape->getSimulationFilterData());
}
