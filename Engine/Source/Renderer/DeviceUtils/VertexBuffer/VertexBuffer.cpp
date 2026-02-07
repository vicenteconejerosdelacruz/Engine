#include "pch.h"
#include "VertexBuffer.h"
#include <DirectXHelper.h>
#include "../Resources/Resources.h"

namespace DeviceUtils
{
	void InitializeVertexBufferView(CComPtr<ID3D12Device2> d3dDevice, CComPtr<ID3D12GraphicsCommandList2> commandList, const void* vertices, unsigned int vertexSize, unsigned int verticesCount, VertexBufferViewData& vbvData)
	{
		UpdateBufferResource(d3dDevice, commandList, vbvData.vertexBuffer, vbvData.vertexBufferUpload, verticesCount, vertexSize, vertices);
		vbvData.vertexBufferView.BufferLocation = vbvData.vertexBuffer->GetGPUVirtualAddress();
		vbvData.vertexBufferView.SizeInBytes = vertexSize * verticesCount;
		vbvData.vertexBufferView.StrideInBytes = vertexSize;
		vbvData.vertexBuffer->SetName(nostd::StringToWString(std::string("vertexBuffer:(" + std::to_string(vertexSize) + "," + std::to_string(verticesCount) + ")")).c_str());
		LogCComPtrAddress(std::string("vertexBuffer:(" + std::to_string(vertexSize) + "," + std::to_string(verticesCount) + ")"), vbvData.vertexBuffer);
	}
}