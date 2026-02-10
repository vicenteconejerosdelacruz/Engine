#pragma once
#include <SceneUnitId.h>

namespace Physics
{
	void InitializePhysics();
	void DestroyPhysics();
	void CreatePhysicsScene(SceneUnitId id, XMFLOAT3 gravity = XMFLOAT3(0.0f, -9.81f, 0.0f));
}