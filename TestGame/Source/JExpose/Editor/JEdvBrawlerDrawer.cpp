#include "pch.h"
#include "JEdvBrawlerDrawer.h"
#include "../../GameControllers/Effects/AnimatedDecal.h"

using namespace Effects;

void DrawAnimatedDecal(std::string attribute, std::vector<JObject*>& json)
{
	if (json.size() > 1) return;

	AnimatedDecal* decal = static_cast<AnimatedDecal*>(json.at(0));
	decal->DrawPlayer();
}
