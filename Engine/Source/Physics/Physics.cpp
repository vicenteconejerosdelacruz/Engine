#include "pch.h"
#include "Physics.h"

//Physx
using namespace physx;
#define PVD_HOST "127.0.0.1"
static PxDefaultAllocator gAllocator;
static PxDefaultErrorCallback gErrorCallback;
static PxFoundation* gFoundation = nullptr;
static PxPvd* gPvd = nullptr;
static PxPhysics* gPhysics = nullptr;

namespace Physics
{
	void InitializePhysics()
	{
		gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
		gPvd = PxCreatePvd(*gFoundation);
		PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
		gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

		gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
		PxInitExtensions(*gPhysics, gPvd);
	}

	void DestroyPhysics()
	{
		//PX_RELEASE(gDispatcher);
		PX_RELEASE(gPhysics);
		if (gPvd)
		{
			PxPvdTransport* transport = gPvd->getTransport();
			PX_RELEASE(gPvd);
			PX_RELEASE(transport);
		}
		PX_RELEASE(gFoundation);
	}
}