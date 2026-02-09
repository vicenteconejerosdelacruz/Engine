#include "pch.h"
#include "RenderableBoundingBox.h"
#include <Renderable/Renderable.h>
#include <Renderer.h>
#include <DeviceUtils/Resources/Resources.h>

extern std::unique_ptr<Renderer> renderer;

namespace ComputeShader
{
	static std::unordered_map<JUUID, std::unique_ptr<RenderableBoundingBox>> renderableBoundingBoxCompute;

	JUUID CreateRenderableBoundingBox(RenderableID renderable)
	{
		JUUID compUUID = getUUID();
		std::unique_ptr<RenderableBoundingBox> rbbComp = std::make_unique<RenderableBoundingBox>(renderable);
		renderableBoundingBoxCompute.insert_or_assign(compUUID, std::move(rbbComp));
		return compUUID;
	}

	std::unique_ptr<RenderableBoundingBox>& GetRenderableBoundingBox(JUUID compUUID)
	{
		return renderableBoundingBoxCompute.at(compUUID);
	}

	void DeleteRenderableBoundingBox(JUUID compUUID)
	{
		renderableBoundingBoxCompute.erase(compUUID);
	}

	RenderableBoundingBox::RenderableBoundingBox(RenderableID renderable) : ComputeInterface("BoundingBox_cs")
	{
		using namespace Animation;
		bonesCbv = GetAnimatedConstantsBuffer(renderable);
		auto& shaderInstance = GetShaderInstance(shader.shader);

		auto createComputeResource = [this, &renderable](size_t numResources)
			{
				size_t dataSize = sizeof(XMFLOAT4) * 2ULL;
				D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
				D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {
					.Format = DXGI_FORMAT_UNKNOWN, .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
					.Buffer = {
						.FirstElement = 0ULL, .NumElements = 1U, .StructureByteStride = sizeof(XMFLOAT4) * 2,
						.CounterOffsetInBytes = 0ULL, .Flags = D3D12_BUFFER_UAV_FLAG_NONE
					}
				};

				D3D12_HEAP_PROPERTIES readBackHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
				D3D12_RESOURCE_DESC bufferDescReadBack = CD3DX12_RESOURCE_DESC::Buffer(dataSize);

				for (unsigned int i = 0; i < numResources; i++)
				{
					// Create UAV for the result
					resources.push_back(CComPtr<ID3D12Resource>());
					auto& resource = resources.back();
					renderer->d3dDevice->CreateCommittedResource(
						&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resource)
					);
					CCNAME_D3D12_OBJECT_N(resource, std::string(renderable->name() + ":" + std::to_string(i)));
					LogCComPtrAddress(std::string(renderable->name() + ":" + std::to_string(i)), resource);

					resultCpuHandle.push_back(::CD3DX12_CPU_DESCRIPTOR_HANDLE());
					resultGpuHandle.push_back(::CD3DX12_GPU_DESCRIPTOR_HANDLE());
					::CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle = resultCpuHandle.back();
					::CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle = resultGpuHandle.back();
					DeviceUtils::AllocCSUDescriptor(cpuHandle, gpuHandle);
					renderer->d3dDevice->CreateUnorderedAccessView(resource, nullptr, &uavDesc, cpuHandle);

					//Create ReadBack
					readBackResources.push_back(CComPtr<ID3D12Resource>());
					auto& readBackResource = readBackResources.back();
					renderer->d3dDevice->CreateCommittedResource(
						&readBackHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDescReadBack,
						D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&readBackResource)
					);
					CCNAME_D3D12_OBJECT_N(readBackResource, std::string(renderable->name() + "readback:" + std::to_string(i)));
					LogCComPtrAddress(std::string(renderable->name() + ":" + std::to_string(i)), readBackResource);
				}
			};

		auto createMeshVerticesShaderResourceView = [this](auto mesh)
			{
				verticesCpuHandles.push_back(CD3DX12_CPU_DESCRIPTOR_HANDLE());
				verticesGpuHandles.push_back(CD3DX12_GPU_DESCRIPTOR_HANDLE());
				mesh->CreateVerticesShaderResourceView(verticesCpuHandles.back(), verticesGpuHandles.back());
			};

		auto createNumVerticesConstantsBuffer = [this, &shaderInstance](auto mesh)
			{
				JUUID cbvUUID = CreateConstantsBuffer(shaderInstance->cbufferSize[0], 1U, "bboxCS." + mesh->uuid);
				unsigned int numVertices = mesh->vbvData.vertexBufferView.SizeInBytes / mesh->vbvData.vertexBufferView.StrideInBytes;
				auto& cbv = GetConstantsBuffer(cbvUUID);
				cbv->push<unsigned int>(numVertices, 0);
				constantsBuffers.push_back(cbvUUID);
			};

		//Create the bounding box compute resource
		createComputeResource(renderable->meshes.size());

		bool extend = false;
		for (auto& mesh : renderable->meshes)
		{
			//Create the SRV for the vertex buffers of each mesh
			createMeshVerticesShaderResourceView(mesh);

			createNumVerticesConstantsBuffer(mesh);

			mesh->ExtendBoundingBox(boundingBox, extend);
			extend = true;
		}
	}

	RenderableBoundingBox::~RenderableBoundingBox()
	{
		bonesCbv.clear();
		resources.clear();
		readBackResources.clear();
		for (auto& constantBuffer : constantsBuffers)
		{
			DestroyConstantsBuffer(constantBuffer());
		}
		constantsBuffers.clear();
		for (unsigned int i = 0; i < verticesCpuHandles.size(); i++)
		{
			DeviceUtils::FreeCSUDescriptor(verticesCpuHandles[i], verticesGpuHandles[i]);
		}
		for (unsigned int i = 0; i < resultCpuHandle.size(); i++)
		{
			DeviceUtils::FreeCSUDescriptor(resultCpuHandle[i], resultGpuHandle[i]);
		}
	}

	void RenderableBoundingBox::Compute(SceneUnitId unit)
	{
		using namespace DeviceUtils;
		using namespace Scene;

		auto& scene = GetSceneUnit(unit);
		CComPtr<ID3D12GraphicsCommandList2>& commandList = scene->GetComputeCommandList();

#if defined(_DEVELOPMENT)
		PIXBeginEvent(commandList.p, 0, L"RenderableBoundingBox Compute");
#endif

		shader.SetComputeState(unit);

		//0 : the number of vertices
		//1 : UAV for the bones transformation <- as all the meshes shares the same matrices we can just set once
		//2 : bounding box center and extents <- this is the output UAV
		//3 : UAV for the vertices of the mesh
		unsigned int backBufferIndex = scene->Frame();
		commandList->SetComputeRootDescriptorTable(1, bonesCbv->gpu_xhandle[backBufferIndex]);

		for (unsigned int i = 0; i < verticesCpuHandles.size(); i++)
		{
			auto& cbv = constantsBuffers[i];
			commandList->SetComputeRootDescriptorTable(0, cbv->gpu_xhandle[0]);
			commandList->SetComputeRootDescriptorTable(2, resultGpuHandle[i]);
			commandList->SetComputeRootDescriptorTable(3, verticesGpuHandles[i]);
			commandList->Dispatch(1, 1, 1);
		}

#if defined(_DEVELOPMENT)
		PIXEndEvent(commandList.p);
#endif
	}

	void RenderableBoundingBox::Solution(SceneUnitId unit)
	{
		using namespace Scene;

		XMFLOAT4* mem{};
		D3D12_RANGE range{};
		range.Begin = 0;
		range.End = sizeof(XMFLOAT4) * 2ULL;

		CComPtr<ID3D12GraphicsCommandList2>& commandList = GetSceneUnit(unit)->GetComputeCommandList();

#if defined(_DEVELOPMENT)
		PIXBeginEvent(commandList.p, 0, L"RenderableBoundingBox Solution");
#endif

		for (unsigned int i = 0; i < verticesCpuHandles.size(); i++)
		{
			DeviceUtils::TransitionResource(commandList, resources[i], D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE);
			DeviceUtils::TransitionResource(commandList, readBackResources[i], D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
			commandList->CopyResource(readBackResources[i], resources[i]);

			readBackResources[i]->Map(0, &range, reinterpret_cast<void**>(&mem));

			XMFLOAT3 center = { mem[0].x, mem[0].y , mem[0].z };
			XMFLOAT3 extents = { mem[1].x, mem[1].y , mem[1].z };

			BoundingBox gpuBBox = BoundingBox(center, extents);

			if (i == 0)
			{
				boundingBox = gpuBBox;
			}
			else
			{
				BoundingBox out;
				BoundingBox::CreateMerged(out, boundingBox, gpuBBox);
				boundingBox = out;
			}

			D3D12_RANGE emptyRange{ 0, 0 };
			readBackResources[i]->Unmap(0, &emptyRange);

			DeviceUtils::TransitionResource(commandList, resources[i], D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
			DeviceUtils::TransitionResource(commandList, readBackResources[i], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
		}

#if defined(_DEVELOPMENT)
		PIXEndEvent(commandList.p);
#endif
	}
}
