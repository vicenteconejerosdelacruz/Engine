#pragma once
#include <Renderable/Renderable.h>

struct BillboardRegistry
{
	std::map<JUUID, RenderableID> billboardRegistry; //scene object -> renderable billboard
	std::set<RenderableID> billboardsToDestroy;
};
