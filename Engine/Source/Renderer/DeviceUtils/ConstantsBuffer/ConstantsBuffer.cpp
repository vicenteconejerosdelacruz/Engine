#include "pch.h"
#include <map>
#include "ConstantsBuffer.h"
#include <DeviceUtils/D3D12Device/Builder.h>
#include <DeviceUtils/DescriptorHeap/DescriptorHeap.h>
#include <Renderer.h>
#include <DirectXHelper.h>

extern std::unique_ptr<Renderer> renderer;

namespace DeviceUtils
{
	std::unique_ptr<DescriptorHeap> csuDescriptorHeap;
	static std::unordered_map<JUUID, std::unique_ptr<ConstantsBuffer>> constantsBuffers;

	void CreateCSUDescriptorHeap(unsigned int numFrames) {
		csuDescriptorHeap = std::make_unique<DescriptorHeap>();
		csuDescriptorHeap->CreateDescriptorHeap(renderer->d3dDevice, numFrames * csuNumDescriptorsInFrame, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	}

	void DestroyCSUDescriptorHeap()
	{
		csuDescriptorHeap->DestroyDescriptorHeap();
	}

	unsigned int GetCSUDescriptorSize()
	{
		return csuDescriptorHeap->descriptorSize;
	}

	CComPtr<ID3D12DescriptorHeap>& GetCSUDescriptorHeap()
	{
		return csuDescriptorHeap->descriptorHeap;
	}

	void AllocCSUDescriptor(::CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, ::CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
	{
		csuDescriptorHeap->AllocDescriptor(cpuHandle, gpuHandle);
	}

	void FreeCSUDescriptor(::CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, ::CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
	{
		csuDescriptorHeap->FreeDescriptor(cpuHandle, gpuHandle);
	}

	JUUID CreateConstantsBuffer(size_t bufferSize, unsigned int numDescriptors, std::string cbName)
	{
		JUUID constantsBufferUUID = getUUID();
		std::unique_ptr<ConstantsBuffer> cbvData = std::make_unique<ConstantsBuffer>(bufferSize, cbName);

		//create the d3d12 cbuffer
		CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(numDescriptors * cbvData->alignedConstantBufferSize);
		CD3DX12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		DX::ThrowIfFailed(renderer->d3dDevice->CreateCommittedResource(
			&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
			&constantBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&cbvData->constantBuffer)
		));

		CCNAME_D3D12_OBJECT_N(cbvData->constantBuffer, cbName);
		LogCComPtrAddress(cbName, cbvData->constantBuffer);

		//create the views for the constants buffer and get the handles to the views
		D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = cbvData->constantBuffer->GetGPUVirtualAddress();
		for (UINT n = 0; n < numDescriptors; n++) {
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
			desc.BufferLocation = cbvGpuAddress;
			desc.SizeInBytes = cbvData->alignedConstantBufferSize;

			CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_xhandle;
			CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_xhandle;
			AllocCSUDescriptor(cpu_xhandle, gpu_xhandle);

			renderer->d3dDevice->CreateConstantBufferView(&desc, cpu_xhandle);
			cbvGpuAddress += desc.SizeInBytes;

			cbvData->cpu_xhandle.push_back(cpu_xhandle);
			cbvData->gpu_xhandle.push_back(gpu_xhandle);
		}

		//map the CPU memory to the GPU and then mapped memory
		//mapea la memoria de la PCU con la GPU y luego vacia la memoria mapeada
		CD3DX12_RANGE readRange(0, 0);
		DX::ThrowIfFailed(cbvData->constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&cbvData->mappedConstantBuffer)));
		ZeroMemory(cbvData->mappedConstantBuffer, numDescriptors * cbvData->alignedConstantBufferSize);

		constantsBuffers.insert_or_assign(constantsBufferUUID, std::move(cbvData));
		return constantsBufferUUID;
	}

	void DestroyConstantsBuffer(JUUID constantsBufferUUID)
	{
		constantsBuffers.erase(constantsBufferUUID);
	}

	void DestroyConstantsBuffer()
	{
		constantsBuffers.clear();
	}

	::CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(JUUID constantsBufferUUID, unsigned int index)
	{
		return constantsBuffers.at(constantsBufferUUID)->cpu_xhandle[index];
	}

	::CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(JUUID constantsBufferUUID, unsigned int index)
	{
		return constantsBuffers.at(constantsBufferUUID)->gpu_xhandle[index];
	}

	std::unique_ptr<ConstantsBuffer>& GetConstantsBuffer(JUUID constantsBufferUUID)
	{
		return constantsBuffers.at(constantsBufferUUID);
	}

	void ConstantsBuffer::SetRootDescriptorTable(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& cbvSlot, unsigned int backBufferIndex)
	{
		commandList->SetGraphicsRootDescriptorTable(cbvSlot, gpu_xhandle[backBufferIndex]);
		cbvSlot++;
	}

	void ConstantsBuffer::Destroy()
	{
		constantBuffer = nullptr;
		for (unsigned int i = 0; i < cpu_xhandle.size(); i++)
		{
			FreeCSUDescriptor(cpu_xhandle[i], gpu_xhandle[i]);
		}
		mappedConstantBuffer = nullptr;
		cpu_xhandle.clear();
		gpu_xhandle.clear();
	}
}