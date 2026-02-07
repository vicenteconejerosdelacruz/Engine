#include "pch.h"
#include "IndexBuffer.h"
#include <DirectXHelper.h>
#include "../Resources/Resources.h"

namespace DeviceUtils
{
	void InitializeIndexBufferView(CComPtr<ID3D12Device2>& d3dDevice, CComPtr<ID3D12GraphicsCommandList2>& commandList, const void* indices, unsigned int indicesCount, IndexBufferViewData& ibvData)
	{
		UpdateBufferResource(d3dDevice, commandList, ibvData.indexBuffer, ibvData.indexBufferUpload, indicesCount, sizeof(unsigned int), indices);
		ibvData.indexBufferView.BufferLocation = ibvData.indexBuffer->GetGPUVirtualAddress();
		ibvData.indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		ibvData.indexBufferView.SizeInBytes = sizeof(unsigned int) * indicesCount;
		ibvData.indexBuffer->SetName(nostd::StringToWString(std::string("indexBuffer:(" + std::to_string(indicesCount) + ")")).c_str());
		LogCComPtrAddress(std::string("indexBuffer:(" + std::to_string(indicesCount) + ")"), ibvData.indexBuffer);
	}
};