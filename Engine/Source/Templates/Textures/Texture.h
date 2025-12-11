#pragma once
#include <string>
#include <tuple>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>
#include <imgui.h>
#include <JTemplate.h>
#include <TemplateDecl.h>
#include <Json.h>
#include <JTypes.h>

enum TextureType;

namespace Templates
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <TextureAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <TextureAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include <TextureAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <TextureAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <TextureAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <TextureAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <TextureAtt.h>
#include <JEnd.h>

	void TextureJsonsStep();

#endif

	namespace Texture
	{
		inline static const std::string templateName = "textures.json";
		inline static const TemplateType templateType = T_Textures;
	};

	struct TextureInstance;
	struct TextureJson : public JTemplate
	{
		TEMPLATE_DECL(Texture);

#include <Attributes/JFlags.h>
#include <TextureAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <TextureAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
		virtual void EditorPreview(size_t flags);
		virtual void DestroyEditorPreview();

		//load&reload
		TextureInstanceUUID preview;
		int previewFrame = 0U;
		bool reloadPreview = false;
		void CreatePreviewTexture();

		//controller for array(animated) textures
		bool previewIsPlaying = false;
		bool previewIsLooping = false;
		float previewTime = 0.0f;
		float previewTimeFactor = 1.0f;
#endif
	};

	TEMPDECL_FULL(Texture);

	DXGI_FORMAT GetTextureFormat(std::filesystem::path path);
	void Create2DDDSFile(TextureJson& json);
	void CreateArrayDDSFile(TextureJson& json);
	void CreateCubeDDSFile(TextureJson& json);
	void CreateCubeDDSFileFromSkyBox(TextureJson& json);
#if defined(_EDITOR)
	void CreateTextureFromJsonDefinition(nlohmann::json& json);
#endif
	JUUID CreateTextureTemplate(std::string name, DXGI_FORMAT format);
	void CreateDDSFile(std::unique_ptr<TextureJson>& tex);
#if defined(_EDITOR)
	void PreviewTexturesStep(float delta);
	void ReloadPreviewTextures();
#endif

	struct TextureInstance
	{
		TextureInstance(std::string uuid);
		TextureInstance(std::string uuid, unsigned int startFrame);
		~TextureInstance() {}
		JUUID materialTexture;

		//D3D12
		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc;
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		CComPtr<ID3D12Resource> texture;
		CComPtr<ID3D12Resource> upload;
		size_t bufferSize;
		void CreateTextureResource(std::string& path, DXGI_FORMAT format, TextureType type, unsigned int numFrames, unsigned int nMipMaps, unsigned int startFrame = 0U);
		void ReleaseResources();
	};
	TEMPDECL_REFTRACKER(Texture);
};

inline auto ToTextureJson(std::vector<JObject*>& json)
{
	std::vector<TextureJsonUUID> textures;
	std::transform(json.begin(), json.end(), std::back_inserter(textures), [](auto j)
		{
			return std::string(j->at("uuid"));
		}
	);
	return textures;
}
