#include "pch.h"
#include "PhysicGeometry.h"
#include <Primitives.h>
/*
#include <Mesh/Mesh.h>
#include <Cube.h>
#include <Floor.h>
#include <Pentahedron.h>
#include <Sphere.h>
#include <Cone.h>
#include <Capsule.h>
*/
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
	std::string filename = default3DModelsFolder + model3D->path();
	std::filesystem::path modelPath(filename);
	std::string cookedFilename = ((sdf) ? defaultPhysxCookingSDFFolder : defaultPhysxCookingFolder) + model3D->uuid() + ".cooked";

	std::vector<VertexPos> vertices;
	std::vector<unsigned int> indices;

	Assimp::Importer importer;
	const aiScene* aiModel = importer.ReadFile(modelPath.string(),
		aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded
	);

	unsigned int vertexOffset = 0U;
	unsigned int indicesOffset = 0U;
	//go through all the meshes in the model
	for (unsigned int meshIndex = 0; meshIndex < aiModel->mNumMeshes; meshIndex++)
	{
		auto aMesh = aiModel->mMeshes[meshIndex];

		for (unsigned int vertexIndex = 0; vertexIndex < aMesh->mNumVertices; vertexIndex++)
		{
			VertexPos v;
			v.Position.x = aMesh->mVertices[vertexIndex][0];
			v.Position.y = aMesh->mVertices[vertexIndex][1];
			v.Position.z = aMesh->mVertices[vertexIndex][2];
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
		nlohmann::json ph;
		T p(ph);
		p.PrepareMesh();

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

PxGeometryHolder LoadCapsuleIntoPxGeometry(nlohmann::json& atts)
{
	using namespace Primitives;
	Capsule capsule(atts);
	return PxCapsuleGeometry(capsule.at("radius"), capsule.at("halfHeight"));
}

using namespace Primitives;
std::map<JNAME, std::function<PxGeometryHolder(XMFLOAT3 scale, nlohmann::json& attributes, bool sdf)>> PxGeometryPrimitiveBuilder =
{
	{ "cube", [](XMFLOAT3 s, nlohmann::json& atts, bool sdf) { return LoadCubeIntoPxGeometry(s); } },
	{ "pyramid",[](XMFLOAT3 s, nlohmann::json& atts, bool sdf) { return LoadMeshIntoPxGeometry<Pentahedron>(GetMeshUUIDByName("pyramid"), s, sdf); }},
	{ "floor",[](XMFLOAT3 s, nlohmann::json& atts, bool sdf) { return LoadMeshIntoPxGeometry<Floor>(GetMeshUUIDByName("floor"), s, false); }},
	{ "sphere",[](XMFLOAT3 s, nlohmann::json& atts, bool sdf) { return LoadSphereIntoPxGeometry(s); } },
	{ "cone",[](XMFLOAT3 s, nlohmann::json& atts, bool sdf) { return LoadMeshIntoPxGeometry<Cone>(GetMeshUUIDByName("cone"), s, sdf); }},
	{ "capsule",[](XMFLOAT3 s, nlohmann::json& atts, bool sdf) { return LoadCapsuleIntoPxGeometry(atts); }},
};

PxGeometryHolder LoadAssimpIntoPxGeometry(XMFLOAT3 scale, Model3DJsonID model3D, bool sdf)
{
	if (!pxTrianglesMeshes.contains(std::make_tuple(model3D(), sdf)))
	{
		pxTrianglesMeshes.insert_or_assign(
			std::make_tuple(model3D(), sdf),
			LoadPxTriangleMeshFromCookedAssimp(sdf, model3D)
		);
	}

	PxTriangleMesh* triangleMesh = pxTrianglesMeshes.at(std::make_tuple(model3D(), sdf));
	PxMeshScale pxScale(ToPxVec3(scale));
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

	nlohmann::json GetCubeAttributes()
	{
		using namespace Primitives;

		std::set<std::string> atts = { "halfDimensions" };

		nlohmann::json ph;
		Cube cube(ph);

		nlohmann::json ret;

		for (auto att : atts)
		{
			if (!cube.contains(att))
			{
				assert(!!!"attribute not present");
				continue;
			}

			nlohmann::json patch = { {att,cube.at(att)} };
			ret.merge_patch(patch);
		}

		return ret;
	}

	nlohmann::json GetCapsuleAttributes()
	{
		using namespace Primitives;

		std::set<std::string> atts = { "radius", "halfHeight" };

		nlohmann::json ph;
		Capsule capsule(ph);

		nlohmann::json ret;

		for (auto att : atts)
		{
			if (!capsule.contains(att))
			{
				assert(!!!"attribute not present");
				continue;
			}

			nlohmann::json patch = { {att,capsule.at(att)} };
			ret.merge_patch(patch);
		}

		return ret;
	}

	std::vector<JUUIDName> GetPhysicGeometrysTriggerUUIDsNames()
	{
		static std::vector<std::string> geometries = { "cube", "sphere", "capsule" };
		std::vector<JUUIDName> uuidNames;

		std::transform(geometries.begin(), geometries.end(), std::back_inserter(uuidNames), [](auto& g)
			{
				return std::make_tuple(GetPhysicGeometryUUIDByName(g), g);
			}
		);
		return uuidNames;
	}

	std::vector<JUUIDName> GetPhysicGeometrysCharacterUUIDsNames()
	{
		static std::vector<std::string> geometries = { "cube", "capsule" };
		std::vector<JUUIDName> uuidNames;

		std::transform(geometries.begin(), geometries.end(), std::back_inserter(uuidNames), [](auto& g)
			{
				return std::make_tuple(GetPhysicGeometryUUIDByName(g), g);
			}
		);
		return uuidNames;
	}

#endif

	PhysicGeometryInstance::PhysicGeometryInstance(PhysicGeometryJsonID geometryTemplate, RenderableID renderable, Model3DJsonID model3D, JUUID instance, PhysicsBehavior behavior)
	{
		this->geometryTemplate = geometryTemplate;
		this->renderable = renderable;
		this->model3D = model3D;
		this->instance = instance;

		geometry = LoadAssimpIntoPxGeometry(renderable->scale(), model3D, behavior == PB_Dynamic);
	}

	PhysicGeometryInstance::PhysicGeometryInstance(PhysicGeometryJsonID geometryTemplate, TriggerID trigger, Model3DJsonID model3D, JUUID instance, PhysicsBehavior behavior)
	{
		this->geometryTemplate = geometryTemplate;
		this->trigger = trigger;
		this->model3D = model3D;
		this->instance = instance;

		geometry = LoadAssimpIntoPxGeometry(trigger->scale(), model3D, false);
	}

	PhysicGeometryInstance::PhysicGeometryInstance(PhysicGeometryJsonID geometryTemplate, RenderableID renderable, nlohmann::json& attributes, JUUID mesh, JUUID instance, PhysicsBehavior behavior)
	{
		this->geometryTemplate = geometryTemplate;
		this->renderable = renderable;
		this->mesh = mesh;
		this->instance = instance;

		JNAME name = GetMeshName(mesh);
		geometry = PxGeometryPrimitiveBuilder.at(name)(renderable->scale(), attributes, behavior == PB_Dynamic);
	}

	PhysicGeometryInstance::PhysicGeometryInstance(PhysicGeometryJsonID geometryTemplate, TriggerID trigger, nlohmann::json& attributes, JUUID mesh, JUUID instance, PhysicsBehavior behavior)
	{
		this->geometryTemplate = geometryTemplate;
		this->trigger = trigger;
		this->mesh = mesh;
		this->instance = instance;

		JNAME name = GetMeshName(mesh);
		geometry = PxGeometryPrimitiveBuilder.at(name)(trigger->scale(), attributes, false);
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