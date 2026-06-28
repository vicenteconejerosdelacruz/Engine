#include "pch.h"
#include "Physics.h"
#include <unordered_map>
#include <cassert>
#include <SimpleMath.h>
#include <PhysicScene.h>
#include <NoMath.h>
//Physx
#include <PxPhysicsAPI.h>
#include <extensions/PxDefaultAllocator.h>
#include <characterkinematic/PxControllerManager.h>

using namespace physx;
#define PVD_HOST "127.0.0.1"
PxDefaultAllocator gAllocator;
PxDefaultErrorCallback gErrorCallback;
PxFoundation* gFoundation = nullptr;
PxPvd* gPvd = nullptr;
PxPhysics* gPhysics = nullptr;
PxDefaultCpuDispatcher* gDispatcher = nullptr;
PxCudaContextManager* gCudaContextManager = nullptr;

using namespace Scene;
using namespace DirectX;
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

		gDispatcher = PxDefaultCpuDispatcherCreate(4);

		PxCudaContextManagerDesc cudaContextManagerDesc;
		gCudaContextManager = PxCreateCudaContextManager(*gFoundation, cudaContextManagerDesc, PxGetProfilerCallback());
	}

	void DestroyPhysics()
	{
		//PX_RELEASE(gDispatcher);
//		PX_RELEASE(gCudaContextManager);
		PX_RELEASE(gPhysics);
		if (gPvd)
		{
			PxPvdTransport* transport = gPvd->getTransport();
			PX_RELEASE(gPvd);
			PX_RELEASE(transport);
		}
		PX_RELEASE(gFoundation);
	}

	PxFilterFlags BitmaskFilterShader(
		PxFilterObjectAttributes attributes0, PxFilterData fd0,
		PxFilterObjectAttributes attributes1, PxFilterData fd1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		// 1. Ignorar si ambos son triggers (opcional, según tu juego)
		if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlag::eDEFAULT;
		}

		// 2. Lógica de Bitmask: 
		// ¿El grupo de A está en la máscara de colisión de B? 
		// Y ¿El grupo de B está en la máscara de colisión de A?
		if ((fd0.word0 & fd1.word1) && (fd1.word0 & fd0.word1))
		{
			// Si ambos quieren chocar, habilitamos la resolución física y los eventos
			pairFlags = PxPairFlag::eCONTACT_DEFAULT; // Chocan y rebotan
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND; // Avisar al código (onContact)

			return PxFilterFlag::eDEFAULT;
		}

		// Si no pasan el AND binario, se ignoran por completo (atraviesan)
		return PxFilterFlag::eSUPPRESS;
	}

	//GPU
	physx::PxGpuDynamicsMemoryConfig gpuConfig;
	void CreatePhysicsScene(PhysicSceneID physicScene)
	{
		PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
		XMFLOAT3 gravity = physicScene->gravity();
		sceneDesc.gravity = PxVec3(gravity.x, gravity.y, gravity.z);
		sceneDesc.cpuDispatcher = gDispatcher;
		//sceneDesc.filterShader = PxDefaultSimulationFilterShader;
		sceneDesc.filterShader = BitmaskFilterShader;
		sceneDesc.cudaContextManager = gCudaContextManager;
		sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS | PxSceneFlag::eENABLE_PCM | PxSceneFlag::eENABLE_CCD;
		sceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;

		// Sube los límites iniciales para evitar que la GPU tenga que redimensionar en caliente
		gpuConfig.tempBufferCapacity = 16 * 1024 * 1024 * 4; // Buffer temporal (64MB)
		gpuConfig.heapCapacity = 64 * 1024 * 1024 * 4; // Heap de la GPU (256MB)
		gpuConfig.maxRigidContactCount = 1024 * 64;            // Capacidad de contactos rígidos
		gpuConfig.maxRigidPatchCount = 1024 * 16;
		// Si tu simulación incluye colisiones complejas, puedes ajustar también:
		gpuConfig.foundLostPairsCapacity = 1024 * 8;             // Pares nuevos/perdidos por frame
		// 3. Asígnalo a la descripción de la escena
		sceneDesc.gpuDynamicsConfig = gpuConfig;

		physicScene->contactCallback = std::make_unique<ContactCallback>(physicScene);
		sceneDesc.simulationEventCallback = physicScene->contactCallback.get();
		physicScene->pxScene = gPhysics->createScene(sceneDesc);
		physicScene->pxControllerManager = PxCreateControllerManager(*physicScene->pxScene);
		physicScene->pxControllerManager->setOverlapRecoveryModule(true);
		//physicScene->pxControllerManager->setTessellation(true, 2.0f);
	}

	//CPU
	/*
	void CreatePhysicsScene(PhysicSceneID physicScene)
	{
		PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
		XMFLOAT3 gravity = physicScene->gravity();
		sceneDesc.gravity = PxVec3(gravity.x, gravity.y, gravity.z);
		sceneDesc.cpuDispatcher = gDispatcher;

		sceneDesc.filterShader = BitmaskFilterShader;

		// --- CAMBIOS PARA CPU ---
		// 1. Quitamos la asignación del Cuda Context Manager
		sceneDesc.cudaContextManager = nullptr;

		// 2. Quitamos los flags de GPU (Aseguramos que no estén activos)
		sceneDesc.flags &= ~PxSceneFlag::eENABLE_GPU_DYNAMICS;

		// Opcional: eENABLE_PCM funciona en CPU, pero si quieres la simulación
		// de CPU más tradicional/clásica, puedes quitarlo también:
		// sceneDesc.flags &= ~PxSceneFlag::eENABLE_PCM;

		// 3. Cambiamos el Broad Phase a uno de CPU.
		// eSAP (Sweep-and-Prune) es el más común y robusto para escenas normales de CPU.
		sceneDesc.broadPhaseType = PxBroadPhaseType::eSAP;

		// 4. Ya no necesitas configurar ni asignar 'gpuConfig' porque la CPU
		// gestiona su memoria a través del asignador normal (AllocatorCallback)
		// --- FIN CAMBIOS PARA CPU ---

		physicScene->contactCallback = std::make_unique<ContactCallback>(physicScene);
		sceneDesc.simulationEventCallback = physicScene->contactCallback.get();

		physicScene->pxScene = gPhysics->createScene(sceneDesc);
		physicScene->pxControllerManager = PxCreateControllerManager(*physicScene->pxScene);
		physicScene->pxControllerManager->setOverlapRecoveryModule(true);
	}
	*/
};