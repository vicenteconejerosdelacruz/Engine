#ifndef _PHYSICS_PHYSIC_H
#define _PHYSICS_PHYSIC_H

#include <SceneUnitId.h>
#include <UUID.h>
#include <JObject.h>
#include <Physics/PhysicScene.h>

namespace Physics
{
	void InitializePhysics();
	void DestroyPhysics();
	void CreatePhysicsScene(PhysicSceneID physicScene);
};

#endif