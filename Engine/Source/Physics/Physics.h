#pragma once
#include <SceneUnitId.h>
#include <UUID.h>

namespace Scene
{
	DEF_SCENEOBJECT_ID_DEP(PhysicScene);
};

namespace Physics
{
	void InitializePhysics();
	void DestroyPhysics();
	void CreatePhysicsScene(PhysicSceneID physicScene);
};