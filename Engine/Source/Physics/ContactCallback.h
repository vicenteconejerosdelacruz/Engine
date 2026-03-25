#pragma once 

#include <PxPhysicsAPI.h>
#include <UUID.h>

namespace Scene
{
	DEF_SCENEOBJECT_ID_DEP(PhysicScene);
};

using namespace Scene;

namespace Physics
{
	class ContactCallback : public PxSimulationEventCallback
	{
	public:
		ContactCallback(PhysicSceneID physicScene);
		~ContactCallback() {}

		void onConstraintBreak(PxConstraintInfo* /*constraints*/, PxU32 /*count*/)	PX_OVERRIDE;

		void onWake(PxActor** /*actors*/, PxU32 /*count*/)	PX_OVERRIDE;

		void onSleep(PxActor** /*actors*/, PxU32 /*count*/)	PX_OVERRIDE;

		void onTrigger(PxTriggerPair* pairs, PxU32 count)	PX_OVERRIDE;

		void onAdvance(const PxRigidBody* const*, const PxTransform*, const PxU32)	PX_OVERRIDE;

		void onContact(const PxContactPairHeader& /*pairHeader*/, const PxContactPair* pairs, PxU32 count)	PX_OVERRIDE;

		//bool isTriggerShape(PxShape* shape);

		PhysicSceneID physicScene;
	};
}
