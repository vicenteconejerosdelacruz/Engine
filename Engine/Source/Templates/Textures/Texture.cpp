#include "pch.h"
#include "Texture.h"
#include <Renderer.h>
#include <Scene.h>
#include <DirectXHelper.h>
#include <ImageConvert.h>

extern std::unique_ptr<Renderer> renderer;

namespace Editor {
	extern SceneUnitId currentSceneUnitId;
	extern void MarkTemplatesPanelAssetsAsDirty();
};

namespace Templates
{
#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <TextureAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <TextureAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <TextureAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <TextureAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <TextureAtt.h>
#include <JEnd.h>

#endif

#if defined(_EDITOR)
	namespace Texture
	{
		bool createIbl = false;
		nlohmann::json iblJson;
	}
#endif

	TextureJson::TextureJson(nlohmann::json& json) : JTemplate(json)
	{
#include <Attributes/JInit.h>
#include <TextureAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <TextureAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void TextureJson::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <TextureAtt.h>
#include <JEnd.h>
	}
#endif

	TEMPDEF_FULL(Texture);
	TEMPDEF_REFTRACKER(Texture);
	std::vector<std::tuple<unsigned int, std::function<void()>>> textureUploadResourcesFreeCallback;

	void RunTextureUploadFreeResources()
	{
		for (auto it = textureUploadResourcesFreeCallback.begin(); it != textureUploadResourcesFreeCallback.end();)
		{
			auto& [steps, callback] = *it;
			steps--;
			if (steps == 0U)
			{
				callback();
				it = textureUploadResourcesFreeCallback.erase(it);
				continue;
			}
			it++;
		}
	}

	void PushTextureUploadFreeResourceCallback(unsigned int steps, std::function<void()> callback)
	{
		textureUploadResourcesFreeCallback.push_back(std::make_tuple(steps, callback));
	}

	DXGI_FORMAT GetTextureFormat(std::filesystem::path path)
	{
		using namespace Utils;

		DirectX::TexMetadata info{};
		GetImageAttributes(path, info);
		return info.format;
	}

	void Create2DDDSFile(TextureJson& json)
	{
		using namespace Utils;

		std::filesystem::path ddsPath = json.name();
		ddsPath.replace_extension(".dds");

		//in case there is no images(a broken ref) we load from the name
		std::filesystem::path image = (json.images().size() > 0ULL) ? json.images().at(0) : json.name();
		if (json.images().size() == 0ULL)
		{
			json.images_push_back(image.string());
		}

		std::string extension = image.extension().string();
		nostd::ToLower(extension);

		DirectX::TexMetadata info{};
		GetImageAttributes(image, info);

		ImageConverter conv;
		conv.src = image;
		conv.dst = ddsPath;
		conv.format = info.format;
		conv.width = (!IsPowerOfTwo(static_cast<unsigned int>(info.width))) ? PrevPowerOfTwo(static_cast<unsigned int>(info.width)) : static_cast<unsigned int>(info.width);
		conv.height = (!IsPowerOfTwo(static_cast<unsigned int>(info.height))) ? PrevPowerOfTwo(static_cast<unsigned int>(info.height)) : static_cast<unsigned int>(info.height);
		conv.mipLevels = GetMipMaps(static_cast<unsigned int>(conv.width), static_cast<unsigned int>(conv.height));
		ConvertToDDS(conv);

		json.numFrames(conv.numFrames);
		json.format(conv.format);
		json.width(conv.width);
		json.height(conv.height);
		json.mipLevels(conv.mipLevels);
	}

	void CreateArrayDDSFile(TextureJson& json)
	{
		using namespace Utils;

		std::filesystem::path ddsPath = json.name();
		ddsPath.replace_extension(".dds");

		//in case there is no images(a broken ref) we load from the name
		std::filesystem::path image = (json.images().size() > 0ULL) ? json.images().at(0) : json.name();
		if (json.images().size() == 0ULL)
		{
			json.images_push_back(image.string());
		}

		//convert the gif to dds
		AssembleArrayDDSFromGif(ddsPath, image);

		//get the dds attributes
		DirectX::TexMetadata info{};
		GetImageAttributes(ddsPath, info);

		//prepare the dds conversion to calculate sizes and mipmaps
		ImageConverter conv;
		conv.src = ddsPath;
		conv.dst = ddsPath;
		conv.format = info.format;
		conv.width = (!IsPowerOfTwo(static_cast<unsigned int>(info.width))) ? PrevPowerOfTwo(static_cast<unsigned int>(info.width)) : static_cast<unsigned int>(info.width);
		conv.height = (!IsPowerOfTwo(static_cast<unsigned int>(info.height))) ? PrevPowerOfTwo(static_cast<unsigned int>(info.height)) : static_cast<unsigned int>(info.height);
		conv.mipLevels = GetMipMaps(static_cast<unsigned int>(conv.width), static_cast<unsigned int>(conv.height));

		//only apply dds to dds conversion if the sizes are not matching
		if (conv.width != static_cast<unsigned int>(info.width) || conv.height != static_cast<unsigned int>(info.height))
			ConvertToDDS(conv);

		json.numFrames(conv.numFrames);
		json.format(conv.format);
		json.width(conv.width);
		json.height(conv.height);
		json.mipLevels(conv.mipLevels);
	}

	void CreateCubeDDSFile(TextureJson& json)
	{
		using namespace std;
		using namespace Utils;

		std::filesystem::path ddsPath = json.name();
		ddsPath.replace_extension(".dds");

		unsigned int minWidth;
		unsigned int minHeight;
		std::vector<std::string> facesPath = json.images();

		for (unsigned int i = 0; i < facesPath.size(); i++)
		{
			DirectX::TexMetadata info{};
			GetImageAttributes(facesPath[i], info);
			if (i == 0)
			{
				minWidth = static_cast<unsigned int>(info.width);
				minHeight = static_cast<unsigned int>(info.height);
			}
			else
			{
				minWidth = min(minWidth, static_cast<unsigned int>(info.width));
				minHeight = min(minHeight, static_cast<unsigned int>(info.height));
			}
		}

		AssembleCubeDDS(ddsPath, facesPath, minWidth, minHeight);

		//get the dds attributes
		DirectX::TexMetadata info{};
		GetImageAttributes(ddsPath, info);

		json.numFrames(static_cast<unsigned int>(info.depth));
		json.format(info.format);
		json.width(static_cast<unsigned int>(info.width));
		json.height(static_cast<unsigned int>(info.height));
		json.mipLevels(static_cast<unsigned int>(info.mipLevels));
	}

	void CreateCubeDDSFileFromSkyBox(TextureJson& json)
	{
		using namespace Utils;

		std::filesystem::path ddsPath = json.name();
		ddsPath.replace_extension(".dds");

		//in case there is no images(a broken ref) we load from the name
		std::filesystem::path image = (json.images().size() > 0ULL) ? json.images().at(0) : json.name();
		if (json.images().size() == 0ULL)
		{
			json.images_push_back(image.string());
		}

		AssembleCubeDDSFromSkybox(ddsPath, image);

		//get the dds attributes
		DirectX::TexMetadata info{};
		GetImageAttributes(ddsPath, info);

		if (!IsPowerOfTwo(static_cast<unsigned int>(info.width)) || !IsPowerOfTwo(static_cast<unsigned int>(info.height)))
		{
			//prepare the dds conversion to calculate sizes and mipmaps
			ImageConverter conv;
			conv.src = ddsPath;
			conv.dst = ddsPath;
			conv.format = info.format;
			conv.width = (!IsPowerOfTwo(static_cast<unsigned int>(info.width))) ? PrevPowerOfTwo(static_cast<unsigned int>(info.width)) : static_cast<unsigned int>(info.width);
			conv.height = (!IsPowerOfTwo(static_cast<unsigned int>(info.height))) ? PrevPowerOfTwo(static_cast<unsigned int>(info.height)) : static_cast<unsigned int>(info.height);
			conv.mipLevels = GetMipMaps(static_cast<unsigned int>(conv.width), static_cast<unsigned int>(conv.height));
			ConvertToDDS(conv);
			GetImageAttributes(ddsPath, info);
		}

		json.numFrames(static_cast<unsigned int>(info.depth));
		json.format(info.format);
		json.width(static_cast<unsigned int>(info.width));
		json.height(static_cast<unsigned int>(info.height));
		json.mipLevels(static_cast<unsigned int>(info.mipLevels));
	}

	TextureJson CreateBaseTextureJson(std::string name, unsigned int numFrames, DXGI_FORMAT format)
	{
		nlohmann::json j = {
			{ "uuid", getUUID() },
			{ "name", name },
			{ "images", { name } },
			{ "numFrames", numFrames },
			{ "format", DXGI_FORMATToString.at(format) },
			{ "type", TextureTypeToString.at(TextureType_2D) },
			{ "width", 128 },
			{ "height", 128 },
			{ "mipLevels", 1 }
		};
		return TextureJson(j);
	};

	JUUID CreateTextureTemplate(std::string name, DXGI_FORMAT format)
	{
		//used for creating
		TextureJson texj = CreateBaseTextureJson(name, 0, format);
		nlohmann::json j = texj.json();
		CreateTexture(j);

		//used for referencing
		std::unique_ptr<TextureJson>& text = GetTextureTemplate(texj.uuid());
		CreateDDSFile(text);
		return text->uuid();
	}

	void CreateDDSFile(std::unique_ptr<TextureJson>& tex)
	{
		std::filesystem::path ddsPath = tex->name();
		ddsPath.replace_extension(".dds");

		if (!std::filesystem::exists(ddsPath))
		{
			switch (tex->type())
			{
			case TextureType_2D:
			{
				Create2DDDSFile(*tex);
			}
			break;
			case TextureType_Array:
			{
				CreateArrayDDSFile(*tex);
			}
			break;
			case TextureType_Cube:
			{
				CreateCubeDDSFile(*tex);
			}
			break;
			case TextureType_Skybox:
			{
				CreateCubeDDSFileFromSkyBox(*tex);
			}
			break;
			}
		}
#if defined(_EDITOR)
		Editor::MarkTemplatesPanelAssetsAsDirty();
#endif
	}

#if defined(_EDITOR)
	void CreateTextureFromJsonDefinition(nlohmann::json& json)
	{
		TextureJson texJson(json);
		switch (texJson.type())
		{
		case TextureType_2D:
		{
			Create2DDDSFile(texJson);
		}
		break;
		case TextureType_Array:
		{
			CreateArrayDDSFile(texJson);
		}
		break;
		case TextureType_Cube:
		{
			CreateCubeDDSFile(texJson);
		}
		break;
		case TextureType_Skybox:
		{
			CreateCubeDDSFileFromSkyBox(texJson);
		}
		break;
		}

		nlohmann::json createJson = texJson.json();
		Texture::createIbl = false;
		Texture::iblJson = createJson;

		auto atts = { "createIrradiance", "createPrefilteredEnv", "createBRDFLut" };
		bool createIbl = false;
		for (auto att : atts)
		{
			if (createJson.contains(att))
			{
				Texture::createIbl |= bool(createJson.at(att));
				createJson.erase(att);
			}
		}

		CreateTexture(createJson);
		Editor::MarkTemplatesPanelAssetsAsDirty();
	}

	void TextureJson::EditorPreview(size_t flags)
	{
		if (flags & (1 << Update_images))
		{
			CreatePreviewTexture();
		}
	}

	void TextureJson::DestroyEditorPreview()
	{
		if (preview.previewLoaded != nullptr)
		{
			for (unsigned int i = 0; i < preview.textures.size(); i++)
			{
				DeleteTextureInstance(preview.textures.at(i)());
			}
			preview.textures.clear();
			preview.previewLoaded = nullptr;
		}
	}

	void TextureJson::CreatePreviewTexture()
	{
		using namespace Texture;
		DestroyEditorPreview();
		preview.previewLoaded = std::make_unique<std::atomic_bool>(false);
		preview.frame = 0U;
		preview.playing = false;
		preview.looping = false;
		preview.time = 0.0f;
		preview.timeFactor = 1.0f;
		if (!preview.processorInitialized)
		{
			preview.loadingProcessor.Init(renderer->d3dDevice, 0x10AD3D, 1);
			preview.processorInitialized = true;
		}

		preview.loadingProcessor.ResetCommandList();
		for (unsigned int i = 0U; i < numFrames(); i++)
		{
			JUUID previewUUID = uuid() + "-preview-" + std::to_string(i);
			CreateTextureInstance(previewUUID, [&]
				{
					return std::make_unique<TextureInstance>(preview.loadingProcessor.GetCommandList(), uuid(), i);
				}
			);
			preview.textures.push_back(previewUUID);
		}
		preview.loadingProcessor.CloseCommandList();
		renderer->ExecuteCommands(preview.loadingProcessor.GetCommandList(false), [&]
			{
				preview.previewLoaded->store(true);
			}
		);
	}

	void TextureJsonsStep()
	{
		std::set<TextureJsonUUID> texs;
		std::transform(Texturetemplates.begin(), Texturetemplates.end(), std::inserter(texs, texs.begin()), [](auto& temps)
			{
				return temps.first;
			}
		);

		std::set<TextureJsonUUID> rebuildImages;
		std::copy_if(texs.begin(), texs.end(), std::inserter(rebuildImages, rebuildImages.begin()), [](auto tex)
			{
				return tex->dirty(TextureJson::Update_images);
			}
		);
		std::set<TextureJsonUUID> changedAttributes;
		std::copy_if(texs.begin(), texs.end(), std::inserter(changedAttributes, changedAttributes.begin()), [](auto tex)
			{
				return tex->dirty(TextureJson::Update_format) || tex->dirty(TextureJson::Update_width) ||
					tex->dirty(TextureJson::Update_height) || tex->dirty(TextureJson::Update_mipLevels) ||
					tex->dirty(TextureJson::Update_numFrames);
			}
		);
	}

	void PreviewTexturesStep(DX::StepTimer& timer)
	{
		float elapsedSeconds = static_cast<FLOAT>(timer.GetElapsedSeconds());

		std::vector<TextureJsonUUID> previewsToPlay;

		for (auto& [uuid, textureTemplate] : Texturetemplates)
		{
			TextureJsonUUID tex = uuid;
			if (tex->preview.previewLoaded == nullptr || !tex->preview.previewLoaded->load()) continue;

			previewsToPlay.push_back(tex);
		}

		auto previewStep = [elapsedSeconds](auto& texture)
			{
				float animationLength = texture->numFrames() * (1.0f / 60.0f);
				float currentAnimationTime = texture->preview.time;
				unsigned int currentFrame = static_cast<unsigned int>(texture->numFrames() * (currentAnimationTime / animationLength));

				if (animationLength > 0.0f)
				{
					currentAnimationTime += (texture->preview.playing) ? texture->preview.timeFactor * elapsedSeconds : 0.0f;
					if (texture->preview.timeFactor > 0.0f)
					{
						if (currentAnimationTime >= animationLength)
							currentAnimationTime = (texture->preview.looping) ? fmodf(currentAnimationTime, animationLength) : animationLength;
					}
					else if (texture->preview.timeFactor < 0.0f)
					{
						if (currentAnimationTime < 0.0f)
							currentAnimationTime = (texture->preview.looping) ? (animationLength - fmodf(currentAnimationTime, animationLength)) : 0.0f;
					}
					texture->preview.time = currentAnimationTime;
					unsigned int newFrame = static_cast<unsigned int>(texture->numFrames() * (currentAnimationTime / animationLength));
					if (currentFrame != newFrame)
					{
						texture->preview.frame = std::clamp(newFrame, 0U, texture->numFrames() - 1);
					}
				}
			};

		std::for_each(previewsToPlay.begin(), previewsToPlay.end(), previewStep);
	}

	TextureInstance::TextureInstance(CComPtr<ID3D12GraphicsCommandList2>& commandList, JUUID uuid) : TextureInstance(commandList, uuid, 0U) {}

	TextureInstance::TextureInstance(CComPtr<ID3D12GraphicsCommandList2>& commandList, JUUID uuid, unsigned int startFrame)
	{
		using namespace Scene;
		using namespace Templates;
		materialTexture = uuid;
		std::unique_ptr<TextureJson>& tex = GetTextureTemplate(uuid);
		std::filesystem::path path = tex->name();
#if defined(_DEVELOPMENT)
		if (path.extension() != ".dds")
		{
			path.replace_extension(".dds");
			if (!std::filesystem::exists(path))
			{
				CreateDDSFile(tex);
			}
		}
		else if (tex->images().size() == 0ULL || tex->images().at(0) == "")
		{
			nlohmann::json update = { {"images", nlohmann::json::array({tex->name()}) } };
			tex->JUpdate(update);
		}
#endif
		std::string pathS = path.string();
		CreateTextureResource(commandList, pathS, tex->format(), tex->type(), tex->numFrames(), tex->mipLevels(), startFrame);
		PushTextureUploadFreeResourceCallback(2U, [&]
			{
				OutputDebugStringA(std::string("off-loading:" + tex->name() + "\n").c_str());
				upload = nullptr;
			}
		);
	}
#endif

	TextureInstance::TextureInstance(SceneUnitId id, JUUID uuid) : TextureInstance(id, uuid, 0U) {}

	TextureInstance::TextureInstance(SceneUnitId id, JUUID uuid, unsigned int startFrame)
	{
		using namespace Scene;
		using namespace Templates;
		materialTexture = uuid;
		std::unique_ptr<TextureJson>& tex = GetTextureTemplate(uuid);
		std::filesystem::path path = tex->name();
#if defined(_DEVELOPMENT)
		if (path.extension() != ".dds")
		{
			path.replace_extension(".dds");
			if (!std::filesystem::exists(path))
			{
				CreateDDSFile(tex);
			}
		}
		else if (tex->images().size() == 0ULL || tex->images().at(0) == "")
		{
			nlohmann::json update = { {"images", nlohmann::json::array({tex->name()}) } };
			tex->JUpdate(update);
		}
#endif
		std::string pathS = path.string();
		auto& scene = GetSceneUnit(id);
		auto& commandList = scene->GetLoadingCommandList();
		CreateTextureResource(commandList, pathS, tex->format(), tex->type(), tex->numFrames(), tex->mipLevels(), startFrame);
		PushTextureUploadFreeResourceCallback(2U, [&]
			{
				OutputDebugStringA(std::string("off-loading:" + tex->name() + "\n").c_str());
				upload = nullptr;
			}
		);
	}

	void TextureInstance::CreateTextureResource(CComPtr<ID3D12GraphicsCommandList2>& commandList, std::string& path, DXGI_FORMAT format, TextureType type, unsigned int numFrames, unsigned int nMipMaps, unsigned int firstArraySlice)
	{
		using namespace Scene;
		using namespace DeviceUtils;

		auto& d3dDevice = renderer->d3dDevice;
		//Load the dds file to a buffer using LoadDDSTextureFromFile
		std::unique_ptr<uint8_t[]> ddsData;
		std::vector<D3D12_SUBRESOURCE_DATA> subresources;

		DX::ThrowIfFailed(LoadDDSTextureFromFileEx(
			d3dDevice,
			nostd::StringToWString(path).c_str(),
			0,
			D3D12_RESOURCE_FLAG_NONE,
			NonLinearDxgiFormats.contains(format) ? DDS_LOADER_FORCE_SRGB : DDS_LOADER_IGNORE_SRGB,
			&texture,
			ddsData,
			subresources));

		//get the ammount of memory required for the upload
		bufferSize = GetRequiredIntermediateSize(texture, 0, static_cast<unsigned int>(subresources.size()));

		//create the upload texture
		CD3DX12_HEAP_PROPERTIES heapTypeUpload(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC uploadBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(&heapTypeUpload, D3D12_HEAP_FLAG_NONE, &uploadBufferResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload)));

		CCNAME_D3D12_OBJECT_N(texture, std::string(path + ":texture"));
		LogCComPtrAddress(std::string(path + ":texture"), texture);
		CCNAME_D3D12_OBJECT_N(upload, std::string(path + ":upload"));
		LogCComPtrAddress(std::string(path + ":upload"), upload);

		//use to command list to copy the texture data from cpu to gpu space
		UpdateSubresources(commandList, texture, upload, 0, 0, static_cast<unsigned int>(subresources.size()), subresources.data());

		//put a barrier on the texture from a copy destination to a pixel shader resource
		auto transition = CD3DX12_RESOURCE_BARRIER::Transition(texture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		commandList->ResourceBarrier(1, &transition);

		//these are common attributes for now
		viewDesc.Format = format;
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		switch (type)
		{
		case TextureType_2D:
		{
			//simple static textures
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipLevels = nMipMaps;
			viewDesc.Texture2D.MostDetailedMip = 0;
			viewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			viewDesc.Texture2D.PlaneSlice = 0;
		}
		break;
		case TextureType_Array:
		{
			//array textures(animated gifs)
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			viewDesc.Texture2DArray.MipLevels = nMipMaps;
			viewDesc.Texture2DArray.MostDetailedMip = 0;
			viewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
			viewDesc.Texture2DArray.ArraySize = numFrames - firstArraySlice;
			viewDesc.Texture2DArray.PlaneSlice = 0;
			//viewDesc.Texture2DArray.ArraySize = -1;
			viewDesc.Texture2DArray.FirstArraySlice = firstArraySlice;
		}
		break;
		case TextureType_Cube:
		case TextureType_Skybox:
		{
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			viewDesc.TextureCube = {
				.MostDetailedMip = 0,
				.MipLevels = nMipMaps,
				.ResourceMinLODClamp = 0.0f
			};
		}
		break;
		}

		//allocate descriptors handles for the SRV and kick the resource creation
		AllocCSUDescriptor(cpuHandle, gpuHandle);
		renderer->d3dDevice->CreateShaderResourceView(texture, &viewDesc, cpuHandle);
	}

	void TextureInstance::ReleaseResources()
	{
		using namespace DeviceUtils;
		FreeCSUDescriptor(cpuHandle, gpuHandle);
		texture = nullptr;
		upload = nullptr;
	}
}
