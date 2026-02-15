#include "pch.h"
#include "PhysicGeometry.h"
#include <Mesh/Mesh.h>
#include <Cube.h>
#include <Floor.h>
#include <Pentahedron.h>
#include <Sphere.h>
#include <Cone.h>
#include <PxPhysicsAPI.h>
#include <extensions/PxCudaHelpersExt.h>
#include <gpu/PxGpu.h>
#include <gpu/PxPhysicsGpu.h>

using namespace physx;
extern PxPhysics* gPhysics;
extern PxCudaContextManager* gCudaContextManager;

#if defined(_EDITOR)
namespace Editor
{
	extern void MarkTemplatesPanelAssetsAsDirty();
};
#endif
std::map<std::tuple<JNAME, bool>, PxTriangleMesh*> pxTrianglesMeshes;

PxTriangleMesh* LoadPxTriangleMeshFromPrimitive(bool sdf, void* vertices, unsigned int verticesStride, unsigned int numVertices, void* faces, unsigned int facesStride, unsigned int numFaces)
{
	PxSDFDesc sdfDesc;

	if (sdf)
	{
		sdfDesc.spacing = 0.05f;
		sdfDesc.subgridSize = 6;
		sdfDesc.bitsPerSubgridPixel = PxSdfBitsPerSubgridPixel::e16_BIT_PER_PIXEL;
		sdfDesc.numThreadsForSdfConstruction = 8;
		sdfDesc.sdfBuilder = PxGetPhysicsGpu()->createSDFBuilder(gCudaContextManager);
	}

	PxTolerancesScale tolerances;
	const PxCookingParams params(tolerances);
	PxTriangleMeshDesc meshDesc;
	meshDesc.sdfDesc = sdf ? &sdfDesc : nullptr;
	meshDesc.points = PxBoundedData(vertices, verticesStride, numVertices);
	meshDesc.triangles = PxBoundedData(faces, facesStride, numFaces);
	return PxCreateTriangleMesh(params, meshDesc);
}

template<typename T>
PxGeometryHolder LoadMeshIntoPxGeometry(JNAME primitiveName, XMFLOAT3 scale, bool sdf)
{
	if (!pxTrianglesMeshes.contains(std::make_tuple(primitiveName, sdf)))
	{
		T p;

		std::vector<uint32_t> indices = p.GetIndices();
		std::vector<Vertex<T::VertexClass>> vertices = p.GetVertices();

		pxTrianglesMeshes.insert_or_assign(std::make_tuple(primitiveName, sdf),
			LoadPxTriangleMeshFromPrimitive(
				sdf,
				vertices.data(), sizeof(T::VertexType), static_cast<unsigned int>(vertices.size()),
				indices.data(), sizeof(indices[0]) * 3U, static_cast<unsigned int>(indices.size() / 3U)
			)
		);
	}

	PxTriangleMesh* triangleMesh = pxTrianglesMeshes.at(std::make_tuple(primitiveName, sdf));
	PxMeshScale pxScale(PxVec3(scale.x, scale.y, scale.z));
	return PxTriangleMeshGeometry(triangleMesh, pxScale);
}

PxGeometryHolder LoadCubeIntoPxGeometry(XMFLOAT3 scale)
{
	return PxBoxGeometry(scale.x, scale.y, scale.z);
}

PxGeometryHolder LoadSphereIntoPxGeometry(XMFLOAT3 scale)
{
	return PxSphereGeometry(scale.x * 0.5f);
}

std::map<JNAME, std::function<PxGeometryHolder(RenderableID renderable, bool sdf)>> PxGeometryPrimitiveBuilder =
{
	{ "cube", [](auto r, bool sdf) { return LoadCubeIntoPxGeometry(r->scale()); } },
	{ "pyramid",[](auto r, bool sdf) { return LoadMeshIntoPxGeometry<Pentahedron>("pyramid", r->scale(), sdf); }},
	{ "floor",[](auto r, bool sdf) { return LoadMeshIntoPxGeometry<Floor>("floor", r->scale(), false); }},
	{ "sphere",[](auto r, bool sdf) { return LoadSphereIntoPxGeometry(r->scale()); } },
	{ "cone",[](auto r, bool sdf) { return LoadMeshIntoPxGeometry<Cone>("cone", r->scale(), sdf); }},
};

namespace Templates
{
#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#endif

	PhysicGeometryJson::PhysicGeometryJson(nlohmann::json& json) : JTemplate(json)
	{
#include <Attributes/JInit.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void PhysicGeometryJson::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <PhysicGeometryAtt.h>
#include <JEnd.h>
	}
#endif

	PhysicGeometryInstance::PhysicGeometryInstance(RenderableID renderable, Model3DJsonID model3D, JUUID instanceId, PhysicsBehavior behavior)
	{
		this->model3D = model3D;
	}

	PhysicGeometryInstance::PhysicGeometryInstance(RenderableID renderable, JUUID meshId, JUUID instanceId, PhysicsBehavior behavior)
	{
		mesh = meshId;
		this->renderable = renderable;

		JNAME name = GetMeshName(mesh);
		geometry = PxGeometryPrimitiveBuilder.at(name)(renderable, behavior == PB_Dynamic);
	}

	PhysicGeometryInstance::~PhysicGeometryInstance()
	{
	}

	TEMPDEF_FULL(PhysicGeometry);

	std::map<JUUID, std::unique_ptr<PhysicGeometryInstance>> physicGeometriesInstances;

	std::unique_ptr<PhysicGeometryInstance>& CreatePhysicGeometryInstance(JUUID instanceId, std::function<std::unique_ptr<PhysicGeometryInstance>()> newRefCallback)
	{
		physicGeometriesInstances.insert_or_assign(instanceId, std::move(newRefCallback()));
		return GetPhysicGeometryInstance(instanceId);
	}

	std::unique_ptr<PhysicGeometryInstance>& GetPhysicGeometryInstance(JUUID instanceId)
	{
		return physicGeometriesInstances.at(instanceId);
	}

	void ClearPhysicGeometryInstances()
	{
		physicGeometriesInstances.clear();
	}
};