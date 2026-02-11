#include "pch.h"
#include "Physics.h"
#include <unordered_map>
#include <cassert>
//Physx
#include <PxPhysicsAPI.h>
#include <extensions/PxDefaultAllocator.h>

using namespace physx;
#define PVD_HOST "127.0.0.1"
static PxDefaultAllocator gAllocator;
static PxDefaultErrorCallback gErrorCallback;
static PxFoundation* gFoundation = nullptr;
static PxPvd* gPvd = nullptr;
static PxPhysics* gPhysics = nullptr;
static PxDefaultCpuDispatcher* gDispatcher = nullptr;

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

		gDispatcher = PxDefaultCpuDispatcherCreate(2);
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

	void CreatePhysicsScene(PhysicSceneID physicScene)
	{
		PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
		XMFLOAT3 gravity = physicScene->gravity();
		sceneDesc.gravity = PxVec3(gravity.x, gravity.y, gravity.z);
		sceneDesc.cpuDispatcher = gDispatcher;
		sceneDesc.filterShader = PxDefaultSimulationFilterShader;
		physicScene->pxScene = gPhysics->createScene(sceneDesc);
	}
}