#pragma once

#include <SceneUnitId.h>
#include <UUID.h>
#include <JObject.h>
#include <PhysicScene.h>

namespace Physics
{
	void InitializePhysics();
	void DestroyPhysics();
	void CreatePhysicsScene(PhysicSceneID physicScene);
};
