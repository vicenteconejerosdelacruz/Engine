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
#include "Capsule.h"

extern std::unique_ptr<Renderer> renderer;
namespace Primitives
{
	using namespace Templates;
	using namespace Scene;

	template<typename T>
	void LoadPrimitiveIntoMesh(SceneUnitId id, const std::unique_ptr<MeshInstance>& mesh, nlohmann::json& json) {

		mesh->vertexClass = T::VertexClass;
		T p(json);

		p.PrepareMesh();
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

	template<
		typename T,
		std::map<std::string, JEdvEditorDrawerFunction> GetDrawers(),
		std::vector<std::pair<std::string, JsonToEditorValueType>> GetAttributes()
	>
	void DrawPrimitiveAttributes(nlohmann::json& json, std::set<std::string> enabledAtts, std::function<void(nlohmann::json)> update)
	{
		T s(json);
		std::vector<JObject*> jvec = { &s };

		nlohmann::json source = nlohmann::json::parse(s.dump());

		auto drawers = GetDrawers();
		auto attributes = GetAttributes();
		for (auto& [att, _] : attributes)
		{
			if (enabledAtts.size() > 0 && !enabledAtts.contains(att))
				continue;
			drawers.at(att)(att, jvec);
		}

		nlohmann::json target = nlohmann::json::parse(s.dump());
		nlohmann::json diff = nlohmann::json::diff(source, target);
		if (diff.size() > 0)
		{
			nlohmann::json patch = {};
			for (unsigned int i = 0; i < diff.size(); i++)
			{
				std::string att = diff.at(i).at("path");
				att.erase(0, 1);
				patch[att] = nullptr;
			}
			patch.patch_inplace(diff);
			update(patch);
		}
	}
};

using namespace Primitives;
static const std::map<std::string, std::function<void(SceneUnitId, const std::unique_ptr<Templates::MeshInstance>&, nlohmann::json&)>> LoadPrimitiveIntoMeshFunctions =
{
	{ "utahteapot", Primitives::LoadPrimitiveIntoMesh<UtahTeapot> },
	{ "cube", Primitives::LoadPrimitiveIntoMesh<Cube> },
	{ "pyramid", Primitives::LoadPrimitiveIntoMesh<Pentahedron> },
	{ "floor", Primitives::LoadPrimitiveIntoMesh<Floor> },
	{ "decal", Primitives::LoadPrimitiveIntoMesh<Decal> },
	{ "boxlines", Primitives::LoadPrimitiveIntoMesh<BoxLines> },
	{ "sphere", Primitives::LoadPrimitiveIntoMesh<Sphere> },
	{ "cone", Primitives::LoadPrimitiveIntoMesh<Cone> },
	{ "capsule", Primitives::LoadPrimitiveIntoMesh<Capsule> },
};

static const std::map<std::string, std::function<void(nlohmann::json&, std::set<std::string>, std::function<void(nlohmann::json)>)>> DrawPrimitiveAttributesFunctions =
{
	{ "sphere", Primitives::DrawPrimitiveAttributes<Sphere, Primitives::GetSphereDrawers, Primitives::GetSphereAttributes> },
	{ "cone", Primitives::DrawPrimitiveAttributes<Cone, Primitives::GetConeDrawers, Primitives::GetConeAttributes> },
	{ "capsule", Primitives::DrawPrimitiveAttributes<Capsule, Primitives::GetCapsuleDrawers, Primitives::GetCapsuleAttributes> },
};