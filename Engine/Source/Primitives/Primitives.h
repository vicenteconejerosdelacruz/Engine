#pragma once
#include <Mesh/Mesh.h>
#include <Renderer.h>
#include <Scene.h>
#include "Cube.h"
#include "Decal.h"
#include "Floor.h"
#include "Pentahedron.h"
#include "UtahTeapot.h"
#include "BoxLines.h"
#include "Sphere.h"
#include "Cone.h"

extern std::unique_ptr<Renderer> renderer;
namespace Primitives {

	using namespace Templates;
	using namespace Scene;

	template<typename T>
	void LoadPrimitiveIntoMesh(SceneUnitId id, const std::unique_ptr<MeshInstance>& mesh, void* params) {

		mesh->vertexClass = T::VertexClass;
		T p(params);

		std::vector<uint32_t> indices = p.GetIndices();
		std::vector<Vertex<T::VertexClass>> vertices = p.GetVertices();

		BoundingBox::CreateFromPoints(mesh->boundingBox, vertices.size(), &vertices.at(0).Position, sizeof(T::VertexType));

		auto& scene = GetSceneUnit(id);
		auto& commandList = scene->GetLoadingCommandList();

		//upload the vertex buffer to the GPU and create the vertex buffer view
		InitializeVertexBufferView(renderer->d3dDevice, commandList, vertices.data(), sizeof(T::VertexType), static_cast<unsigned int>(vertices.size()), mesh->vbvData);

		//upload the index buffer to the GPU and create the index buffer view
		InitializeIndexBufferView(renderer->d3dDevice, commandList, indices.data(), static_cast<unsigned int>(indices.size()), mesh->ibvData);
	}

	template<typename T>
	void LoadPrimitiveIntoPxGeometry(PhysicObject& physicObject)
	{
	}

	template<>
	inline void LoadPrimitiveIntoPxGeometry<Cube>(PhysicObject& physicObject)
	{
		RenderableID renderable = physicObject.sceneObject;
		XMFLOAT3 scale = renderable->scale();
		physicObject.geometry = PxBoxGeometry(scale.x, scale.y, scale.z);
	}

	template<>
	inline void LoadPrimitiveIntoPxGeometry<Floor>(PhysicObject& physicObject)
	{
		RenderableID renderable = physicObject.sceneObject;
		Floor p;

		std::vector<uint32_t> indices = p.GetIndices();
		std::vector<Vertex<Floor::VertexClass>> vertices = p.GetVertices();

		PxTolerancesScale tolerances;
		const PxCookingParams params(tolerances);
		PxTriangleMeshDesc meshDesc;
		meshDesc.points = PxBoundedData(vertices.data(), sizeof(Floor::VertexType), static_cast<unsigned int>(vertices.size()));
		meshDesc.triangles = PxBoundedData(indices.data(), sizeof(indices[0]) * 3U, static_cast<unsigned int>(indices.size() / 3U));

		PxTriangleMesh* triangleMesh = PxCreateTriangleMesh(params, meshDesc);
		XMFLOAT3 scale = renderable->scale();
		PxMeshScale pxScale(PxVec3(scale.x, scale.y, scale.z));
		physicObject.geometry = PxTriangleMeshGeometry(triangleMesh, pxScale);
	}
}

static const std::map<std::string, std::function<void(SceneUnitId, const std::unique_ptr<Templates::MeshInstance>&, void* params)>> LoadPrimitiveIntoMeshFunctions =
{
	{ "utahteapot", Primitives::LoadPrimitiveIntoMesh<UtahTeapot> },
	{ "cube", Primitives::LoadPrimitiveIntoMesh<Cube> },
	{ "pyramid", Primitives::LoadPrimitiveIntoMesh<Pentahedron> },
	{ "floor", Primitives::LoadPrimitiveIntoMesh<Floor> },
	{ "decal", Primitives::LoadPrimitiveIntoMesh<Decal> },
	{ "boxlines", Primitives::LoadPrimitiveIntoMesh<BoxLines> },
	{ "sphere", Primitives::LoadPrimitiveIntoMesh<Sphere> },
	{ "cone", Primitives::LoadPrimitiveIntoMesh<Cone> },
};

static const std::map<std::string, std::function<void(PhysicObject& physicObject)>> LoadPrimitiveIntoPxGeometryFunctions =
{
	{ "utahteapot", Primitives::LoadPrimitiveIntoPxGeometry<UtahTeapot> },
	{ "cube", Primitives::LoadPrimitiveIntoPxGeometry<Cube> },
	{ "pyramid", Primitives::LoadPrimitiveIntoPxGeometry<Pentahedron> },
	{ "floor", Primitives::LoadPrimitiveIntoPxGeometry<Floor> },
	{ "decal", Primitives::LoadPrimitiveIntoPxGeometry<Decal> },
	{ "boxlines", Primitives::LoadPrimitiveIntoPxGeometry<BoxLines> },
	{ "sphere", Primitives::LoadPrimitiveIntoPxGeometry<Sphere> },
	{ "cone", Primitives::LoadPrimitiveIntoPxGeometry<Cone> },
};

static const std::map<std::string, bool> PrimitiveCanBeMadeDynamic =
{
	{ "utahteapot", false },
	{ "cube", true },
	{ "pyramid", false },
	{ "floor", false },
	{ "decal", false },
	{ "boxlines", true },
	{ "sphere", true },
	{ "cone", false },
};