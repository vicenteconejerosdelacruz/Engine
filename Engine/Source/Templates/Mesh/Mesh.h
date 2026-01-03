#pragma once

#include <VertexFormats.h>
#include <DeviceUtils/VertexBuffer/VertexBuffer.h>
#include <DeviceUtils/IndexBuffer/IndexBuffer.h>
#include <DirectXCollision.h>
//#include <Application.h>

namespace Templates {

	using namespace DeviceUtils;

	struct MeshInstance
	{
		JUUID uuid;
		VertexClass vertexClass;
		VertexBufferViewData vbvData;
		IndexBufferViewData	ibvData;
		BoundingBox boundingBox;
		void CreateVerticesShaderResourceView(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle);
		void ExtendBoundingBox(BoundingBox& outBB, bool extend);
	};

	//CREATE
	void CreatePrimitiveMeshTemplate(JUUID uuid, JNAME name);
	std::unique_ptr<MeshInstance>& GetMeshInstance(SceneUnitId id, JUUID uuid);
	std::unique_ptr<MeshInstance>& GetMeshInstance(JUUID uuid);
	std::unique_ptr<MeshInstance>& GetMeshInstance(SceneUnitId id, JUUID uuid, VertexClass vertexClass, void* vertexData, unsigned int vertexSize, unsigned int verticesCount, const void* indices, unsigned int indicesCount);

	//READ&GET
	JNAME GetMeshName(JUUID uuid);
	std::vector<JUUIDName> GetMeshesUUIDsNames();
	JUUID GetMeshUUIDByName(JNAME name);
	bool MeshInstanceExists(JUUID uuid);

	//UPDATE

	//DESTROY
	void DestroyMeshInstance(JUUID uuid);

	//EDITOR
};
