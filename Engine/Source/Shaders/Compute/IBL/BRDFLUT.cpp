#include "pch.h"
#include "BRDFLUT.h"
#include <Renderer.h>
#include <DeviceUtils/Resources/Resources.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <DirectXHelper.h>
#include <Textures/Texture.h>
#include <DirectXTex.h>
#include <DXTypes.h>

extern std::unique_ptr<JRenderer> renderer;

using namespace DeviceUtils;
using namespace Templates;

namespace ComputeShader
{
	BRDFLUT::BRDFLUT(std::filesystem::path iblBRDFLUTPath) : ComputeInterface("IBLBRDFLUT_cs")
	{
		outputFile = iblBRDFLUTPath;

		//create the uav resource for the calculation results (U0)
		D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(dataFormat, faceWidth, faceHeight, 1U, 1U, 1U, 0U, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		renderer->d3dDevice->CreateCommittedResource(
			&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resource)
		);
		CCNAME_D3D12_OBJECT_N(resource, std::string("BRDFLUT"));
		LogCComPtrAddress("BRDFLUT", resource);
		DeviceUtils::AllocCSUDescriptor(resultCpuHandle, resultGpuHandle);

		//create a uav desc/view for writing the diffuse ibl cube texture
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc =
		{
			.Format = dataFormat, .ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
			.Texture2D = {.MipSlice = 0, .PlaneSlice = 0}
		};
		//renderer->d3dDevice->CreateUnorderedAccessView(resource, nullptr, &uavDesc, resultCpuHandle);
		renderer->d3dDevice->CreateUnorderedAccessView(resource, nullptr, nullptr, resultCpuHandle);
	}

	BRDFLUT::~BRDFLUT()
	{
		DeviceUtils::FreeCSUDescriptor(resultCpuHandle, resultGpuHandle);
		resource = nullptr;
		readBackResource = nullptr;
	}

	void BRDFLUT::Compute(SceneUnitId unit)
	{
		auto& scene = GetSceneUnit(unit);
		CComPtr<ID3D12GraphicsCommandList2>& commandList = scene->GetComputeCommandList();

#if defined(_DEVELOPMENT)
		{
			PIXScopedEvent(commandList.p, 0, L"BRDFLUT Compute");
#endif

			shader.SetComputeState(unit);

			commandList->SetComputeRootDescriptorTable(0, resultGpuHandle);
			commandList->Dispatch(8, 8, 1);

#if defined(_DEVELOPMENT)
		}
#endif
	}

	void BRDFLUT::Solution(SceneUnitId unit)
	{
		DeviceUtils::CaptureTexture(
			renderer->d3dDevice,
			renderer->commandQueue,
			resource,
			faceWidth * pixelSize,
			resourceDesc,
			readBackResource,
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COMMON
		);

		XMFLOAT4* mem{};
		D3D12_RANGE range{};
		range.Begin = 0;
		range.End = dataSize;

		readBackResource->Map(0, &range, reinterpret_cast<void**>(&mem));

		WriteFile(mem);

		D3D12_RANGE emptyRange{ 0, 0 };
		readBackResource->Unmap(0, &emptyRange);
		readBackResource = nullptr;

		nlohmann::json json =
		{
			{ "format", DXGI_FORMATToString.at(dataFormat) },
			{ "height", faceHeight },
			{ "images", { outputFile.string() } },
			{ "mipLevels", 1 },
			{ "name", outputFile.string() },
			{ "numFrames",  1 },
			{ "type", TextureTypeToString.at(TextureType_2D) },
			{ "uuid", getUUID() },
			{ "width", faceWidth }
		};
		CreateTexture(json);
	}

	void BRDFLUT::WriteFile(XMFLOAT4* data) const
	{
		using namespace DirectX;

		std::vector<Image> imgs;
		imgs.push_back(
			{
				.width = faceWidth,
				.height = faceHeight,
				.format = dataFormat,
				.rowPitch = static_cast<size_t>(faceWidth * pixelSize),
				.slicePitch = static_cast<size_t>(faceWidth * faceHeight * pixelSize),
				.pixels = reinterpret_cast<uint8_t*>(data)
			}
		);

		TexMetadata meta =
		{
			.width = faceWidth,
			.height = faceHeight,
			.depth = 1,
			.arraySize = 1,
			.mipLevels = 1,
			.miscFlags = 0,
			.miscFlags2 = 0,
			.format = dataFormat,
			.dimension = TEX_DIMENSION_TEXTURE2D
		};

		DX::ThrowIfFailed(SaveToDDSFile(imgs.data(), imgs.size(), meta, DDS_FLAGS_NONE, outputFile.wstring().c_str()));
	}
};
