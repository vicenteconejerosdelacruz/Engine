#include "pch.h"
#include "DescriptorHeap.h"
#include <DirectXHelper.h>

namespace DeviceUtils
{
	void DescriptorHeap::CreateDescriptorHeap(CComPtr<ID3D12Device2>& device, unsigned int sz, D3D12_DESCRIPTOR_HEAP_TYPE heapType, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
	{
		size = sz;
		descriptorHeap = DeviceUtils::CreateDescriptorHeap(device, size, heapType, flags);
		descriptorSize = device->GetDescriptorHandleIncrementSize(heapType);
		std::string heapName = "descriptorHeap-" + D3D12_DESCRIPTOR_HEAP_TYPEToString.at(heapType);

		CCNAME_D3D12_OBJECT_N(descriptorHeap, heapName);
		LogCComPtrAddress(heapName, descriptorHeap);
	}

	void DescriptorHeap::DestroyDescriptorHeap()
	{
		descriptorHeap.Release();
		descriptorHeap = nullptr;
	}

	unsigned int DescriptorHeap::GetDescriptorSize()
	{
		return descriptorSize;
	}

	void DescriptorHeap::AllocDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
	{
		AllocCPUDescriptor(cpuHandle);
		AllocGPUDescriptor(gpuHandle);
	}

	static std::mutex allocCPUMutex;
	void DescriptorHeap::AllocCPUDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		std::lock_guard<std::mutex> lock(allocCPUMutex);
		UINT slotIndex = 0U;
		for (; slotIndex < size; slotIndex++) {
			if (!usedCpuDescriptorHandle.contains(slotIndex)) break;
		}
		assert(slotIndex != size);

		usedCpuDescriptorHandle.insert(slotIndex);

		cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart());
		cpuHandle.Offset(slotIndex, GetDescriptorSize());
	}

	static std::mutex allocGPUMutex;
	void DescriptorHeap::AllocGPUDescriptor(CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
	{
		std::lock_guard<std::mutex> lock(allocGPUMutex);
		UINT slotIndex = 0U;
		for (; slotIndex < size; slotIndex++) {
			if (!usedGpuDescriptorHandle.contains(slotIndex)) break;
		}
		assert(slotIndex != size);

		usedGpuDescriptorHandle.insert(slotIndex);

		gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart());
		gpuHandle.Offset(slotIndex, GetDescriptorSize());
	}

	void DescriptorHeap::FreeDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
	{
		FreeCPUDescriptor(cpuHandle);
		FreeGPUDescriptor(gpuHandle);
	}

	void DescriptorHeap::FreeCPUDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		if (!descriptorHeap) return;

		CD3DX12_CPU_DESCRIPTOR_HANDLE startHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart());

		UINT slotIndex = static_cast<UINT>(cpuHandle.ptr - startHandle.ptr) / GetDescriptorSize();
		usedCpuDescriptorHandle.erase(slotIndex);

		cpuHandle.ptr = 0;
	}

	void DescriptorHeap::FreeGPUDescriptor(CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
	{
		if (!descriptorHeap) return;

		CD3DX12_GPU_DESCRIPTOR_HANDLE startHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart());

		UINT slotIndex = static_cast<UINT>(gpuHandle.ptr - startHandle.ptr) / GetDescriptorSize();
		usedGpuDescriptorHandle.erase(slotIndex);

		gpuHandle.ptr = 0;
	}

};