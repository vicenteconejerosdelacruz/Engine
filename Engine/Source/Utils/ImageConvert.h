#pragma once

#include <filesystem>
#include <dxgiformat.h>
#include <Texconv.h>
#include <ShaderMaterials.h>

namespace Utils
{
	struct ImageConverter
	{
		std::filesystem::path src;
		std::filesystem::path dst;
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		unsigned int width = 0U;
		unsigned int height = 0U;
		unsigned int mipLevels = 0U;
		unsigned int numFrames = 0U;
		TextureType type = TextureType_2D;
	};

	void GetImageAttributes(std::filesystem::path src, DirectX::TexMetadata& info);
	void ConvertToDDS(ImageConverter& conversion);
	void AssembleArrayDDSFromGif(std::filesystem::path image, std::filesystem::path gif);
	void AssembleCubeDDS(std::filesystem::path image, std::vector<std::string> facesPath, unsigned int width, unsigned int height);
	void AssembleCubeDDSFromSkybox(std::filesystem::path image, std::filesystem::path skybox);
}
