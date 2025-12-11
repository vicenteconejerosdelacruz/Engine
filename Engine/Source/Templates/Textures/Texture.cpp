#include "pch.h"
#include "Texture.h"
#include <DXTypes.h>
#include <NoStd.h>
#include <DDSTextureLoader.h>
#include <Templates.h>
#include <TemplateDef.h>
#include <Renderer.h>
#include <DirectXHelper.h>
#include <ImageConvert.h>
#include <IBL/DiffuseIrradianceMap.h>
#include <IBL/PrefilteredEnvironmentMap.h>
#include <IBL/BRDFLUT.h>
#include <ShaderMaterials.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <algorithm>
#include <NoMath.h>

extern std::unique_ptr<Renderer> renderer;

namespace Editor {
	void MarkTemplatesPanelAssetsAsDirty();
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
	TextureInstanceUUID texturePreview;

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

#if defined(_EDITOR)
	void TextureJson::EditorPreview(size_t flags)
	{
		if (flags & (1 << Update_images))
		{
			previewFrame = 0;
			previewIsPlaying = false;
			previewIsLooping = false;
			previewTime = 0.0f;
			previewTimeFactor = 1.0f;
			CreatePreviewTexture();
		}
	}

	void TextureJson::DestroyEditorPreview()
	{
		if (!preview.empty())
		{
			DeleteTextureInstance(preview());
			preview.clear();
		}
	}

	void TextureJson::CreatePreviewTexture()
	{
		DestroyEditorPreview();
		CreateTextureInstance(uuid(), [this]
			{
				return std::make_unique<TextureInstance>(uuid(), previewFrame);
			}
		);
		preview = uuid();
		reloadPreview = false;
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

		if (Texture::createIbl)
		{
			Texture::createIbl = false;
			using namespace ComputeShader;

			std::string envMapUUID = GetTextureUUIDByName(Texture::iblJson.at("name"));

			auto getIBLFile = [](auto attribute, auto name)
				{
					std::filesystem::path path = Texture::iblJson.at("name");
					std::string stem = path.stem().string() + "_" + name;
					path = path.relative_path().parent_path() / (stem + ".dds");
					path = nostd::normalize_path(path.string());
					return path;
				};

			if (Texture::iblJson.contains("createIrradiance") && bool(Texture::iblJson.at("createIrradiance")))
			{
				std::filesystem::path irradiance = getIBLFile("createIrradiance", "irradiance");
				std::shared_ptr<DiffuseIrradianceMap> diffuseIrradianceMap;
				renderer->Flush();
				renderer->RenderCriticalFrame([&diffuseIrradianceMap, envMapUUID, irradiance]
					{
						diffuseIrradianceMap = std::make_shared<DiffuseIrradianceMap>(envMapUUID, irradiance);
						diffuseIrradianceMap->Compute();
					}
				);
				diffuseIrradianceMap->Solution();
			}
			if (Texture::iblJson.contains("createPrefilteredEnv") && bool(Texture::iblJson.at("createPrefilteredEnv")))
			{
				std::filesystem::path prefiltered_env = getIBLFile("createPrefilteredEnv", "prefiltered_env");
				std::shared_ptr<PreFilteredEnvironmentMap> preFilteredEnvironmentMap;
				renderer->Flush();
				renderer->RenderCriticalFrame([&preFilteredEnvironmentMap, envMapUUID, prefiltered_env]
					{
						preFilteredEnvironmentMap = std::make_shared<PreFilteredEnvironmentMap>(envMapUUID, prefiltered_env);
						preFilteredEnvironmentMap->Compute();
					}
				);
				preFilteredEnvironmentMap->Solution();
			}
			if (Texture::iblJson.contains("createBRDFLut") && bool(Texture::iblJson.at("createBRDFLut")))
			{
				std::filesystem::path brdflut = getIBLFile("createBRDFLut", "brdf_lut");
				std::shared_ptr<BRDFLUT> lut;
				renderer->Flush();
				renderer->RenderCriticalFrame([&lut, brdflut]
					{
						lut = std::make_shared<BRDFLUT>(brdflut);
						lut->Compute();
					}
				);
				lut->Solution();
			}

			Editor::MarkTemplatesPanelAssetsAsDirty();
		}

		bool criticalFrame = rebuildImages.size() > 0ULL || changedAttributes.size() > 0ULL;

		if (criticalFrame)
		{
			renderer->Flush();
			renderer->RenderCriticalFrame([&rebuildImages, &changedAttributes]
				{
					std::for_each(rebuildImages.begin(), rebuildImages.end(), [](auto tex)
						{
							std::filesystem::path p = tex->name();
							if (p.extension() != ".dds")
							{
								CreateDDSFile(*tex);
							}
							tex->CreatePreviewTexture();
							tex->clean(TextureJson::Update_images);
						}
					);
					std::for_each(changedAttributes.begin(), changedAttributes.end(), [](auto tex)
						{
							using namespace Utils;
							std::filesystem::path ddsPath = tex->name();
							ddsPath.replace_extension(".dds");
							ImageConverter convert = {
								.src = tex->name(), .dst = ddsPath, .format = tex->format(),
								.width = tex->width(), .height = tex->height(),
								//.mipLevels = tex->mipLevels(), .numFrames = tex->numFrames(),
							};
							if (tex->dirty(TextureJson::Update_mipLevels)) convert.mipLevels = tex->mipLevels();
							if (tex->dirty(TextureJson::Update_numFrames)) convert.numFrames = tex->numFrames();

							ConvertToDDS(convert);
							tex->format(convert.format);
							tex->width(convert.width);
							tex->height(convert.height);
							tex->mipLevels(convert.mipLevels);
							tex->numFrames(convert.numFrames);
							tex->type(convert.type);

							tex->clean(TextureJson::Update_format);
							tex->clean(TextureJson::Update_width);
							tex->clean(TextureJson::Update_height);
							tex->clean(TextureJson::Update_mipLevels);
							tex->clean(TextureJson::Update_numFrames);
							tex->CreatePreviewTexture();
						}
					);
				}
			);
		}
	}
#endif

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
#endif

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
	void PreviewTexturesStep(float elapsedSeconds)
	{
		std::vector<TextureJsonUUID> previewsToPlay;

		for (auto& [uuid, textureTemplate] : Texturetemplates)
		{
			TextureJsonUUID tex = uuid;
			if (tex->preview.empty() || !tex->previewIsPlaying || tex->numFrames() <= 1) continue;

			previewsToPlay.push_back(tex);
		}

		auto previewStep = [elapsedSeconds](auto& texture)
			{
				float animationLength = texture->numFrames() * (1.0f / 60.0f);
				float currentAnimationTime = texture->previewTime;
				unsigned int currentFrame = static_cast<unsigned int>(texture->numFrames() * (currentAnimationTime / animationLength));

				if (animationLength > 0.0f)
				{
					currentAnimationTime += (texture->previewIsPlaying) ? texture->previewTimeFactor * elapsedSeconds : 0.0f;
					if (texture->previewTimeFactor > 0.0f)
					{
						if (currentAnimationTime >= animationLength)
							currentAnimationTime = (texture->previewIsLooping) ? fmodf(currentAnimationTime, animationLength) : animationLength;
					}
					else if (texture->previewTimeFactor < 0.0f)
					{
						if (currentAnimationTime < 0.0f)
							currentAnimationTime = (texture->previewIsLooping) ? (animationLength - fmodf(currentAnimationTime, animationLength)) : 0.0f;
					}
					texture->previewTime = currentAnimationTime;
					unsigned int newFrame = static_cast<unsigned int>(texture->numFrames() * (currentAnimationTime / animationLength));
					if (currentFrame != newFrame)
					{
						texture->previewFrame = std::clamp(newFrame, 0U, texture->numFrames() - 1);
						texture->reloadPreview = true;
					}
				}
			};

		std::for_each(previewsToPlay.begin(), previewsToPlay.end(), previewStep);
	}

	void ReloadPreviewTextures()
	{
		std::vector<TextureJsonUUID> previewsToReload;

		for (auto& [uuid, textureTemplate] : Texturetemplates)
		{
			TextureJsonUUID tex = uuid;
			if (tex->preview.empty() || !tex->reloadPreview) continue;

			previewsToReload.push_back(tex);
		}

		if (previewsToReload.empty()) return;

		renderer->Flush();
		renderer->RenderCriticalFrame([&previewsToReload]
			{
				for (auto& tex : previewsToReload)
				{
					tex->CreatePreviewTexture();
				}
			}
		);
	}
#endif

	TextureInstance::TextureInstance(std::string uuid) :
		TextureInstance(uuid, 0U) {
	}

	TextureInstance::TextureInstance(std::string uuid, unsigned int startFrame)
	{
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
		CreateTextureResource(pathS, tex->format(), tex->type(), tex->numFrames(), tex->mipLevels(), startFrame);
	}

	void TextureInstance::CreateTextureResource(std::string& path, DXGI_FORMAT format, TextureType type, unsigned int numFrames, unsigned int nMipMaps, unsigned int firstArraySlice)
	{
		using namespace DeviceUtils;

		auto& d3dDevice = renderer->d3dDevice;
		auto& commandList = renderer->commandList;

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
