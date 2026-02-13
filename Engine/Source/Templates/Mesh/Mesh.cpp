#include "pch.h"
#include "Mesh.h"
#include <Primitives.h>
#include <RefTracker.h>
#include <UUID.h>

namespace Templates {

	std::map<JUUID, JNAME> meshes;

	namespace Mesh
	{
		static RefTracker<JUUID, std::unique_ptr<MeshInstance>> refTracker;
	}

	//CREATE
	void CreatePrimitiveMeshTemplate(JUUID uuid, JNAME name)
	{
		meshes.insert_or_assign(uuid, name);
	}

	std::unique_ptr<MeshInstance>& GetMeshInstance(SceneUnitId id, JUUID uuid)
	{
		using namespace Mesh;

		if (meshes.contains(uuid))
		{
			JNAME& name = meshes.at(uuid);
			return refTracker.AddRef(uuid, [&]()
				{
					std::unique_ptr<MeshInstance> instance = std::make_unique<MeshInstance>();
					instance->uuid = uuid;
					LoadPrimitiveIntoMeshFunctions.at(name)(id, instance, nullptr);
					return instance;
				}
			);
		}
		else
		{
			return  refTracker.FindValue(uuid);
		}
	}

	std::unique_ptr<MeshInstance>& GetMeshInstance(JUUID uuid)
	{
		using namespace Mesh;
		return  refTracker.FindValue(uuid);
	}

	std::unique_ptr<MeshInstance>& GetMeshInstance(SceneUnitId id, JUUID uuid, VertexClass vertexClass, void* vertexData, unsigned int vertexSize, unsigned int verticesCount, const void* indices, unsigned int indicesCount)
	{
		using namespace Mesh;
		using namespace Scene;
		return refTracker.AddRef(uuid, [&]()
			{
				std::unique_ptr<MeshInstance> instance = std::make_unique<MeshInstance>();
				instance->uuid = uuid;
				instance->vertexClass = vertexClass;
				auto& commandList = GetSceneUnit(id)->GetLoadingCommandList();
				InitializeVertexBufferView(renderer->d3dDevice, commandList, vertexData, vertexSize, verticesCount, instance->vbvData);
				InitializeIndexBufferView(renderer->d3dDevice, commandList, indices, indicesCount, instance->ibvData);
				return instance;
			}
		);
	}

	JNAME GetMeshName(JUUID uuid)
	{
		return meshes.at(uuid);
	}

	std::vector<JUUIDName> GetMeshesUUIDsNames()
	{
		std::vector<JUUIDName> uuidNames;
		std::transform(meshes.begin(), meshes.end(), std::back_inserter(uuidNames), [](auto& pair)
			{
				JUUIDName uuidname;
				JUUID& uuid = std::get<0>(uuidname);
				JNAME& name = std::get<1>(uuidname);
				uuid = pair.first;
				name = pair.second;
				return uuidname;
			}
		);
		return uuidNames;
	}

	JUUID GetMeshUUIDByName(JNAME name)
	{
		for (auto& [meshUUID, meshName] : meshes)
		{
			if (meshName == name) return meshUUID;
		}
		assert(!!!"mesh not found");
		return "";
	}

	bool MeshInstanceExists(JUUID uuid)
	{
		using namespace Mesh;
		return refTracker.Has(uuid);
	}

	//UPDATE

	//DESTROY

	void DestroyMeshInstance(JUUID uuid)
	{
		using namespace Mesh;
		refTracker.RemoveRef(uuid);
	}

	void MeshInstance::CreateVerticesShaderResourceView(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
	{
		AllocCSUDescriptor(cpuHandle, gpuHandle);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{
			.Format = DXGI_FORMAT_UNKNOWN, .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Buffer = {
				.FirstElement = 0, .NumElements = vbvData.vertexBufferView.SizeInBytes / vbvData.vertexBufferView.StrideInBytes,
				.StructureByteStride = vbvData.vertexBufferView.StrideInBytes, .Flags = D3D12_BUFFER_SRV_FLAG_NONE
			}
		};

		renderer->d3dDevice->CreateShaderResourceView(vbvData.vertexBuffer, &srvDesc, cpuHandle);
	}

	void MeshInstance::ExtendBoundingBox(BoundingBox& outBB, bool extend)
	{
		BoundingBox out;
		BoundingBox::CreateMerged(out, extend ? outBB : boundingBox, boundingBox);
		outBB = out;
	}
};
