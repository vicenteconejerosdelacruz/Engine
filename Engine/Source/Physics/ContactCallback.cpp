#include "pch.h"
#include "ContactCallback.h"
#include "PhysicObject.h"

namespace Physics
{
	ContactCallback::ContactCallback(PhysicSceneID physicScene) : PxSimulationEventCallback()
	{
		this->physicScene = physicScene;
	}

	void ContactCallback::onConstraintBreak(PxConstraintInfo* /*constraints*/, PxU32 /*count*/)
	{
		OutputDebugStringA("onConstraintBreak\n");
	}

	void ContactCallback::onWake(PxActor** /*actors*/, PxU32 /*count*/)
	{
		OutputDebugStringA("onWake\n");
	}

	void ContactCallback::onSleep(PxActor** /*actors*/, PxU32 /*count*/)
	{
		OutputDebugStringA("onSleep\n");
	}

	void ContactCallback::onTrigger(PxTriggerPair* pairs, PxU32 count)
	{
		//		printf("onTrigger: %d trigger pairs\n", count);
		while (count--)
		{
			const PxTriggerPair& current = *pairs++;
			if (current.status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				PhysicObject* trigger = (PhysicObject*)current.triggerShape->userData;
				PhysicObject* other = (PhysicObject*)current.otherActor->userData;
				//OutputDebugStringA("Shape is entering trigger volume\n");
				CallRegisteredCallbacks(PB_Trigger, other->uuid(), trigger->uuid(), PxPairFlag::eNOTIFY_TOUCH_FOUND);
				CallTriggerContactCallback(trigger->trigger, MAKESUUUID(other->unit(), other->uuid()), PxPairFlag::eNOTIFY_TOUCH_FOUND);
			}
			if (current.status & PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				PhysicObject* trigger = (PhysicObject*)current.triggerShape->userData;
				PhysicObject* other = (PhysicObject*)current.otherActor->userData;
				//OutputDebugStringA("Shape is leaving trigger volume\n");
				CallRegisteredCallbacks(PB_Trigger, other->uuid(), trigger->uuid(), PxPairFlag::eNOTIFY_TOUCH_LOST);
				CallTriggerContactCallback(trigger->trigger, MAKESUUUID(other->unit(), other->uuid()), PxPairFlag::eNOTIFY_TOUCH_LOST);
			}
		}
	}

	void ContactCallback::onAdvance(const PxRigidBody* const*, const PxTransform*, const PxU32)
	{
		OutputDebugStringA("onAdvance\n");
	}

	void ContactCallback::onContact(const PxContactPairHeader& /*pairHeader*/, const PxContactPair* pairs, PxU32 count)
	{
		//		printf("onContact: %d pairs\n", count);

		while (count--)
		{
			const PxContactPair& current = *pairs++;

			// The reported pairs can be trigger pairs or not. We only enabled contact reports for
			// trigger pairs in the filter shader, so we don't need to do further checks here. In a
			// real-world scenario you would probably need a way to tell whether one of the shapes
			// is a trigger or not. You could e.g. reuse the PxFilterData like we did in the filter
			// shader, or maybe use the shape's userData to identify triggers, or maybe put triggers
			// in a hash-set and test the reported shape pointers against it. Many options here.

			if (current.events & (PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_CCD))
				OutputDebugStringA("Shape is entering trigger volume\n");
			if (current.events & PxPairFlag::eNOTIFY_TOUCH_LOST)
				OutputDebugStringA("Shape is leaving trigger volume\n");

			//if (isTriggerShape(current.shapes[0]) && isTriggerShape(current.shapes[1]))
			//	OutputDebugStringA("Trigger-trigger overlap detected\n");
		}
	}

	/*
	bool isTriggerShape(PxShape* shape)
	{
		const TriggerImpl impl = getImpl();

		// Detects native built-in triggers.
		if (impl == REAL_TRIGGERS && (shape->getFlags() & PxShapeFlag::eTRIGGER_SHAPE))
			return true;

		// Detects our emulated triggers using the simulation filter data. See createTriggerShape() function.
		if (impl == FILTER_SHADER && ::isTrigger(shape->getSimulationFilterData()))
			return true;

		// Detects our emulated triggers using the simulation filter callback. See createTriggerShape() function.
		if (impl == FILTER_CALLBACK && shape->userData)
			return true;
		return false;
	}
	*/
}