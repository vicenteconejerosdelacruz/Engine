#include "pch.h"
#include "GamePhysics.h"

std::vector<std::string> Physics::GetCollisionMasks()
{
	return nostd::GetValuesFromFlagsMap(CollisionMasksToString);
}
