#include "pch.h"
#include "ImageConvert.h"
#include <DirectXTex.h>
#include <texdiag.h>
#include <texassemble.h>
#include <regex>
#include <DXTypes.h>
#include <NoStd.h>

namespace Utils
{
	void GetImageAttributes(std::filesystem::path src, DirectX::TexMetadata& info)
	{
		std::vector<std::wstring> args = { L"", L"info", src.wstring() };
		std::vector<wchar_t*> texDiagArgs;
		std::transform(args.begin(), args.end(), std::back_inserter(texDiagArgs), [](auto& arg) { return arg.data(); });
		texDiag(static_cast<int>(texDiagArgs.size()), texDiagArgs.data(), &info);
	}

	void ConvertToDDS(ImageConverter& conversion)
	{
		std::vector<std::wstring> args = { L"" };

		args.push_back(conversion.src.wstring());
		if (conversion.format != DXGI_FORMAT_UNKNOWN)
		{
			args.push_back(L"-f");
			args.push_back(nostd::StringToWString(DXGI_FORMATToString.at(conversion.format)));
		}
		args.push_back(L"-y");

		if (conversion.width != 0U)
		{
			args.push_back(L"-w");
			args.push_back(std::to_wstring(conversion.width));
		}
		if (conversion.height != 0U)
		{
			args.push_back(L"-h");
			args.push_back(std::to_wstring(conversion.height));
		}
		if (conversion.mipLevels != 0U)
		{
			args.push_back(L"-m");
			args.push_back(std::to_wstring(conversion.mipLevels));
		}
		args.push_back(L"-l");
		args.push_back(L"-o");
		args.push_back(conversion.dst.parent_path().relative_path().wstring());

		std::vector<wchar_t*> texConvArgs;
		std::transform(args.begin(), args.end(), std::back_inserter(texConvArgs), [](auto& arg) { return arg.data(); });
		texConv(static_cast<int>(texConvArgs.size()), texConvArgs.data());
		DirectX::TexMetadata info;
		GetImageAttributes(conversion.dst, info);
		conversion.width = static_cast<unsigned int>(info.width);
		conversion.height = static_cast<unsigned int>(info.height);
		conversion.format = info.format;
		conversion.mipLevels = static_cast<unsigned int>(info.mipLevels);
		conversion.numFrames = static_cast<unsigned int>(info.arraySize);
		conversion.type = info.IsCubemap() ? (TextureType_Cube) : (info.arraySize > 1ULL) ? TextureType_Array : TextureType_2D;
	}

	void AssembleArrayDDSFromGif(std::filesystem::path image, std::filesystem::path gif)
	{
		std::vector<std::wstring> args = { L"", L"gif", L"-o", image.wstring(), L"-l", L"-y", gif.wstring() };
		std::vector<wchar_t*> texAsmArgs;
		std::transform(args.begin(), args.end(), std::back_inserter(texAsmArgs), [](auto& arg) { return arg.data(); });
		texAssemble(static_cast<int>(texAsmArgs.size()), texAsmArgs.data());

		std::filesystem::path ddsUpperCase = image;
		ddsUpperCase.replace_extension(".DDS");

		std::filesystem::rename(ddsUpperCase, image);
	}

	void AssembleCubeDDS(std::filesystem::path image, std::vector<std::string> facesPath, unsigned int width, unsigned int height)
	{
		std::vector<std::wstring> args = {
			L"", L"cube", L"-o", image.wstring(), L"-l", L"-y",
			L"-w", std::to_wstring(width),
			L"-h", std::to_wstring(height),
			nostd::StringToWString(facesPath[0]),
			nostd::StringToWString(facesPath[1]),
			nostd::StringToWString(facesPath[2]),
			nostd::StringToWString(facesPath[3]),
			nostd::StringToWString(facesPath[4]),
			nostd::StringToWString(facesPath[5])
		};
		std::vector<wchar_t*> texAsmArgs;
		std::transform(args.begin(), args.end(), std::back_inserter(texAsmArgs), [](auto& arg) { return arg.data(); });
		texAssemble(static_cast<int>(texAsmArgs.size()), texAsmArgs.data());

		std::filesystem::path ddsUpperCase = image;
		ddsUpperCase.replace_extension(".DDS");

		std::filesystem::rename(ddsUpperCase, image);
	}

	void AssembleCubeDDSFromSkybox(std::filesystem::path image, std::filesystem::path skybox)
	{
		std::vector<std::wstring> args = { L"", L"cube-from-hc", L"-o", image.wstring(), L"-l", L"-y", skybox.wstring() };
		std::vector<wchar_t*> texAsmArgs;
		std::transform(args.begin(), args.end(), std::back_inserter(texAsmArgs), [](auto& arg) { return arg.data(); });
		texAssemble(static_cast<int>(texAsmArgs.size()), texAsmArgs.data());

		std::filesystem::path ddsUpperCase = image;
		ddsUpperCase.replace_extension(".DDS");

		std::filesystem::rename(ddsUpperCase, image);
	}
}