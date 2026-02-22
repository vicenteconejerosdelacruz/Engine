#ifndef _TEMPLATES_TEXTURE_H
#define _TEMPLATES_TEXTURE_H

#include <string>
#include <tuple>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>
#include <imgui.h>
#include <Templates.h>
#include <JTemplate.h>
#if defined(_EDITOR)
#include <DeviceUtils/CommandsProcessor/CommandsProcessor.h>
using namespace DeviceUtils;
#endif

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
		inline static const std::string defaultBaseTexture = "Assets/textures/gridmap.dds";
		inline static const std::string defaultNormalMap = "Assets/textures/bumpmapflat.dds";
		inline static const TemplateType templateType = T_Textures;
	};

	struct TextureJson;
	struct TextureInstance;
#if defined(_EDITOR)
	struct TexturePreview;
#endif

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
		void CreatePreviewTexture();

		std::unique_ptr<TexturePreview> preview;
#endif
	};

	TEMPDECL_FULL(Texture);

	void RunTextureUploadFreeResources();
	void PushTextureUploadFreeResourceCallback(unsigned int steps, std::function<void()> callback);
	DXGI_FORMAT GetTextureFormat(std::filesystem::path path);
	void Create2DDDSFile(TextureJson& json);
	void CreateArrayDDSFile(TextureJson& json);
	void CreateCubeDDSFile(TextureJson& json);
	void CreateCubeDDSFileFromSkyBox(TextureJson& json);
	JUUID CreateTextureTemplate(std::string name, DXGI_FORMAT format);
	void CreateDDSFile(std::unique_ptr<TextureJson>& tex);
#if defined(_EDITOR)
	void CreateTextureFromJsonDefinition(nlohmann::json& json);
	void PreviewTexturesStep(DX::StepTimer& timer);
#endif

	struct TextureInstance
	{
#if defined(_EDITOR)
		TextureInstance(CComPtr<ID3D12GraphicsCommandList2>& commandList, JUUID uuid);
		TextureInstance(CComPtr<ID3D12GraphicsCommandList2>& commandList, JUUID uuid, unsigned int startFrame);
#endif
		TextureInstance(JUUID uuid) { assert(!!!"do not use"); }
		TextureInstance(SceneUnitId id, JUUID uuid);
		TextureInstance(SceneUnitId id, JUUID uuid, unsigned int startFrame);
		~TextureInstance() {}
		void CreateTextureResource(CComPtr<ID3D12GraphicsCommandList2>& commandList, std::string& path, DXGI_FORMAT format, TextureType type, unsigned int numFrames, unsigned int nMipMaps, unsigned int startFrame = 0U);
		void ReleaseResources();

		JUUID materialTexture;

		//D3D12
		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc;
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		CComPtr<ID3D12Resource> texture;
		CComPtr<ID3D12Resource> upload;
		size_t bufferSize;
	};

	TEMPDECL_REFTRACKER(Texture);
	DEF_TEMPLATE_ID(TextureJson, GetTextureTemplate);
	DEF_TEMPLATE_ID(TextureInstance, GetTextureInstance);

#if defined(_EDITOR)
	struct TexturePreview
	{
		bool processorInitialized = false;
		CommandsProcessor loadingProcessor;
		std::unique_ptr<std::atomic_bool> previewLoaded;
		std::vector<TextureInstanceID> textures;
		int frame;
		bool playing;
		bool looping;
		float time;
		float timeFactor;
	};
#endif
};

using namespace Templates;
DEF_TEMPLATE_ID_HASH(TextureJson);
DEF_TEMPLATE_ID_HASH(TextureInstance);

inline auto ToTextureJson(std::vector<JObject*>& json)
{
	std::vector<TextureJsonID> textures;
	std::transform(json.begin(), json.end(), std::back_inserter(textures), [](auto j)
		{
			return std::string(j->at("uuid"));
		}
	);
	return textures;
}

#endif