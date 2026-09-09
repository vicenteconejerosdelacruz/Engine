#pragma once
#include <set>
#include <PhysicObject.h>

struct PhysicsDrawState
{
	PhysicsDrawState()
	{
		draw = true;
		drawPlayState = true;
	}

	void PlayMode()
	{
		drawPlayState = draw;
		draw = false;
	}

	void EditorMode()
	{
		draw = drawPlayState;
	}

	void SwitchDraw()
	{
		bool value = !draw;
		draw = value;
		for (auto phO : levelPhysicObjects)
		{
			phO->visible(value);
		}
	}

	bool draw;
	bool drawPlayState;
	std::set<PhysicObjectID> levelPhysicObjects;
};