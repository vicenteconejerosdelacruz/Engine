#pragma once
#include <map>
#include <atlbase.h>
#include <d3dx12.h>
#include <UUID.h>

namespace DeviceUtils
{
	struct ConstantsBuffer;

	static constexpr unsigned int csuNumDescriptorsInFrame = 10000;

	inline static const std::string CameraConstantBufferName = "camera";
	inline static const std::string LightConstantBufferName = "lights";
	inline static const std::string ShadowMapConstantBufferName = "shadowMaps";
	inline static const std::string AnimationConstantBufferName = "animation";

	struct ConstantsBuffer {
		ConstantsBuffer(size_t size, std::string name) : alignedConstantBufferSize((size + 255) & ~255), name(name) {}
		~ConstantsBuffer() { Destroy(); }
		std::string name;
		unsigned int alignedConstantBufferSize;

		CComPtr<ID3D12Resource> constantBuffer;
		byte* mappedConstantBuffer = nullptr;
		std::vector<::CD3DX12_CPU_DESCRIPTOR_HANDLE> cpu_xhandle;
		std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> gpu_xhandle;

		template<typename T> size_t push(T& data, unsigned int index, size_t offset = 0ULL) {
			memcpy(mappedConstantBuffer + alignedConstantBufferSize * index + offset, &data, sizeof(T));
			return offset + sizeof(T);
		}

		void SetRootDescriptorTable(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& cbvSlot, unsigned int backBufferIndex);
		void Destroy();
	};

	void CreateCSUDescriptorHeap(unsigned int numFrames);
	void DestroyCSUDescriptorHeap();
	unsigned int GetCSUDescriptorSize();
	CComPtr<ID3D12DescriptorHeap>& GetCSUDescriptorHeap();
	void AllocCSUDescriptor(::CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, ::CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle);
	void FreeCSUDescriptor(::CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, ::CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle);

	std::unique_ptr<ConstantsBuffer>& GetConstantsBuffer(JUUID ConstantsBufferID);
	DEF_TEMPLATE_ID(ConstantsBuffer, GetConstantsBuffer);

	ConstantsBufferID CreateConstantsBuffer(size_t bufferSize, unsigned int numDescriptors, std::string cbName = "");
	void DestroyConstantsBuffer(ConstantsBufferID ConstantsBufferID);
	void DestroyConstantsBuffer();
	::CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(ConstantsBufferID constantsBuffer, unsigned int index);
	::CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(ConstantsBufferID constantsBuffer, unsigned int index);
};

//using namespace DeviceUtils;
//DEF_TEMPLATE_ID_HASH(ConstantsBuffer);