#include "pch.h"
#include "GamePhysics.h"

std::vector<std::string> Physics::GetCollisionMasks()
{
	std::vector<std::string> masks;
	std::transform(StringToCollisionFlags.begin(), StringToCollisionFlags.end(), std::back_inserter(masks), [](auto& pair) { return pair.first; });
	return masks;
}
