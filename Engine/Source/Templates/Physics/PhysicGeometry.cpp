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
#if defined(_DEVELOPMENT)
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif
#include <Application.h>

using namespace physx;
extern PxPhysics* gPhysics;
extern PxCudaContextManager* gCudaContextManager;

#if defined(_EDITOR)
namespace Editor
{
	extern void MarkTemplatesPanelAssetsAsDirty();
};
#endif

void CookAssimpIntoPxTriangleMeshFile(Model3DJsonID model3D, bool sdf)
{
	//this was taken from Animated.cpp, assimp assets are mostly rotated at transformation level(skinning) so we apply the same transformation below
	static const XMMATRIX AssimpFlipYZ = XMMatrixSet(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	std::string filename = default3DModelsFolder + model3D->path();
	std::filesystem::path modelPath(filename);
	std::string cookedFilename = ((sdf) ? defaultPhysxCookingSDFFolder : defaultPhysxCookingFolder) + model3D->uuid() + ".cooked";

	std::vector<VertexPos> vertices;
	std::vector<unsigned int> indices;

	Assimp::Importer importer;
	const aiScene* aiModel = importer.ReadFile(modelPath.string(),
		aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_PreTransformVertices
	);

	unsigned int vertexOffset = 0U;
	unsigned int indicesOffset = 0U;
	//go through all the meshes in the model
	for (unsigned int meshIndex = 0; meshIndex < aiModel->mNumMeshes; meshIndex++)
	{
		auto aMesh = aiModel->mMeshes[meshIndex];

		for (unsigned int vertexIndex = 0; vertexIndex < aMesh->mNumVertices; vertexIndex++)
		{
			XMVECTOR pos{ .m128_f32 = {aMesh->mVertices[vertexIndex][0],aMesh->mVertices[vertexIndex][1],aMesh->mVertices[vertexIndex][2],1.0f} };
			pos = XMVector4Transform(pos, AssimpFlipYZ);
			VertexPos v;
			v.Position.x = pos.m128_f32[0];
			v.Position.y = pos.m128_f32[1];
			v.Position.z = pos.m128_f32[2];
			vertices.push_back(v);
		}

		std::vector<unsigned int> indicesData;
		LoadIndices(aMesh, indicesData);

		for (unsigned int i = 0; i < indicesData.size(); i++)
		{
			indices.push_back(indicesData.at(i) + indicesOffset);
		}

		vertexOffset += aMesh->mNumVertices;
		indicesOffset += static_cast<unsigned int>(indicesData.size());
	}

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
	meshDesc.points = PxBoundedData(vertices.data(), sizeof(VertexPos), static_cast<unsigned int>(vertices.size()));
	meshDesc.triangles = PxBoundedData(indices.data(), sizeof(indices[0]) * 3U, static_cast<unsigned int>(indices.size() / 3U));

	PxDefaultMemoryOutputStream writeBuffer;
	PxTriangleMeshCookingResult::Enum result;
	PxCookTriangleMesh(params, meshDesc, writeBuffer, &result);

	std::filesystem::path parent_dir = std::filesystem::path(cookedFilename).parent_path();
	if (!std::filesystem::exists(parent_dir))
		std::filesystem::create_directories(parent_dir);

	std::ofstream cookedFile(cookedFilename, std::ios::out | std::ios::binary | std::ios::trunc);
	cookedFile.write(reinterpret_cast<const char*>(writeBuffer.getData()), writeBuffer.getSize());
	cookedFile.close();
}

std::map<std::tuple<JUUID, bool>, PxTriangleMesh*> pxTrianglesMeshes;

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

PxTriangleMesh* LoadPxTriangleMeshFromCookedAssimp(bool sdf, Model3DJsonID model3D)
{
	using namespace physx;

	std::string cookedFilename = ((sdf) ? defaultPhysxCookingSDFFolder : defaultPhysxCookingFolder) + model3D->uuid() + ".cooked";
#if defined(_DEVELOPMENT)
	if (!std::filesystem::exists(cookedFilename))
	{
		CookAssimpIntoPxTriangleMeshFile(model3D, sdf);
	}
#endif

	std::ifstream file(cookedFilename, std::ios::binary | std::ios::ate);
	auto size = std::filesystem::file_size(cookedFilename);
	file.seekg(0, std::ios::beg);

	std::vector<PxU8> buffer(size);
	file.read(reinterpret_cast<char*>(buffer.data()), size);

	PxDefaultMemoryInputData readBuffer(buffer.data(), static_cast<PxU32>(buffer.size()));
	return gPhysics->createTriangleMesh(readBuffer);
}

template<typename T>
PxGeometryHolder LoadMeshIntoPxGeometry(JNAME uuid, XMFLOAT3 scale, bool sdf)
{
	if (!pxTrianglesMeshes.contains(std::make_tuple(uuid, sdf)))
	{
		T p;

		std::vector<uint32_t> indices = p.GetIndices();
		std::vector<Vertex<T::VertexClass>> vertices = p.GetVertices();

		pxTrianglesMeshes.insert_or_assign(std::make_tuple(uuid, sdf),
			LoadPxTriangleMeshFromPrimitive(
				sdf,
				vertices.data(), sizeof(T::VertexType), static_cast<unsigned int>(vertices.size()),
				indices.data(), sizeof(indices[0]) * 3U, static_cast<unsigned int>(indices.size() / 3U)
			)
		);
	}

	PxTriangleMesh* triangleMesh = pxTrianglesMeshes.at(std::make_tuple(uuid, sdf));
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
	{ "pyramid",[](auto r, bool sdf) { return LoadMeshIntoPxGeometry<Pentahedron>(GetMeshUUIDByName("pyramid"), r->scale(), sdf); }},
	{ "floor",[](auto r, bool sdf) { return LoadMeshIntoPxGeometry<Floor>(GetMeshUUIDByName("floor"), r->scale(), false); }},
	{ "sphere",[](auto r, bool sdf) { return LoadSphereIntoPxGeometry(r->scale()); } },
	{ "cone",[](auto r, bool sdf) { return LoadMeshIntoPxGeometry<Cone>(GetMeshUUIDByName("cone"), r->scale(), sdf); }},
};

PxGeometryHolder LoadAssimpIntoPxGeometry(RenderableID renderable, Model3DJsonID model3D, bool sdf)
{
	if (!pxTrianglesMeshes.contains(std::make_tuple(model3D(), sdf)))
	{
		pxTrianglesMeshes.insert_or_assign(
			std::make_tuple(model3D(), sdf),
			LoadPxTriangleMeshFromCookedAssimp(sdf, model3D)
		);
	}

	PxTriangleMesh* triangleMesh = pxTrianglesMeshes.at(std::make_tuple(model3D(), sdf));
	XMFLOAT3 scale = renderable->scale();
	PxMeshScale pxScale(PxVec3(scale.x, scale.y, scale.z));
	return PxTriangleMeshGeometry(triangleMesh, pxScale);
}

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

	PhysicGeometryInstance::PhysicGeometryInstance(PhysicGeometryJsonID geometryTemplate, RenderableID renderable, Model3DJsonID model3D, JUUID instance, PhysicsBehavior behavior)
	{
		this->geometryTemplate = geometryTemplate;
		this->renderable = renderable;
		this->model3D = model3D;
		this->instance = instance;

		geometry = LoadAssimpIntoPxGeometry(renderable, model3D, behavior == PB_Dynamic);
	}

	PhysicGeometryInstance::PhysicGeometryInstance(PhysicGeometryJsonID geometryTemplate, RenderableID renderable, JUUID mesh, JUUID instance, PhysicsBehavior behavior)
	{
		this->geometryTemplate = geometryTemplate;
		this->renderable = renderable;
		this->mesh = mesh;
		this->instance = instance;

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