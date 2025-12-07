#include "pch.h"
#include "Model3D.h"
#include <Templates.h>
#include <TemplateDef.h>
#include <Mesh/Mesh.h>
#include <VertexFormats.h>
#include <Animated.h>
#include <d3d12.h>
#include <nlohmann/json.hpp>
#include <Application.h>
#include <NoStd.h>
#include <Textures/Texture.h>
#include <Material/Material.h>
#include <DDSTextures.h>

using namespace Animation;
using namespace DeviceUtils;
using namespace Templates;

#if defined(_EDITOR)
namespace Editor
{
	extern void MarkTemplatesPanelAssetsAsDirty();
};
#endif

namespace Templates
{
#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#endif

	Model3DJson::Model3DJson(nlohmann::json& json) : JTemplate(json)
	{
#include <Attributes/JInit.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <Model3DAtt.h>
#include <JEnd.h>
	}

	TEMPDEF_FULL(Model3D);
	TEMPDEF_REFTRACKER(Model3D);

#if defined(_EDITOR)
	void Model3DJsonStep()
	{
		std::set<Model3DJsonUUID> models;
		std::transform(Model3Dtemplates.begin(), Model3Dtemplates.end(), std::inserter(models, models.begin()), [](auto& mdl)
			{
				return mdl.first;
			}
		);

		std::set<Model3DJsonUUID> seqMdls;
		std::copy_if(models.begin(), models.end(), std::inserter(seqMdls, seqMdls.begin()), [](auto mdl)
			{
				return mdl->dirty(Model3DJson::Update_animationSequences);
			}
		);

		if (seqMdls.size() > 0ULL)
		{
			JObject::RunChangesCallback(seqMdls, [](auto mdl)
				{
					mdl->clean(Model3DJson::Update_animationSequences);
				}
			);
		}
	}
#endif

	JUUID GetModel3DMeshInstanceUUID(JUUID uuid, unsigned int index) {
		return "mesh-" + uuid + "-" + std::to_string(index);
	}

	JUUID GetModel3DMaterialInstanceUUID(JUUID uuid, unsigned int index) {
		return "mat-" + uuid + "-" + std::to_string(index);
	}

	JUUID GetModel3DMaterialInstanceName(JUUID uuid, unsigned int index)
	{
		std::unique_ptr<Model3DJson>& mdl = GetModel3DTemplate(uuid);
		return "mat-" + mdl->name() + "-" + std::to_string(index);
	}

	Model3DInstance::Model3DInstance(JUUID uuid, JUUID objectUUID, JObjectChangeCallback cb, JObjectChangePostCallback postCb)
	{
		model3DUUID = uuid;
		auto& mdl = GetModel3DTemplate(model3DUUID);
		mdl->BindChangeCallback(objectUUID, cb, postCb);
		LoadModel3DInstance();
	}

	Model3DInstance::~Model3DInstance()
	{
		for (auto m : meshes)
		{
			DestroyMeshInstance(m());
		}
		meshes.clear();
	}

	void Model3DInstance::LoadModel3DInstance()
	{
		auto& mdl = GetModel3DTemplate(model3DUUID);

		std::string filename = default3DModelsFolder + mdl->path();

		std::filesystem::path path(filename);

		Assimp::Importer importer;
		const aiScene* aiModel = importer.ReadFile(path.string(),
			aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_CalcTangentSpace |
			aiProcess_Triangulate | aiProcess_GenBoundingBoxes | aiProcess_ConvertToLeftHanded
		);

		if (!aiModel)
		{
			OutputDebugStringA(importer.GetErrorString());
			assert(!aiModel);
		}

		//fill the length of the animations so they can be looped
		if (aiModel->mNumAnimations > 0U)
		{
			animations = CreateAnimatedFromAssimp(aiModel);
		}

		CreateModel3DMaterialsTemplates(aiModel);

		vertexClass = !animations ? POS_NORMAL_TANGENT_TEXCOORD0 : POS_NORMAL_TANGENT_TEXCOORD0_SKINNING;
		size_t vertexSize = !animations ? sizeof(Vertex<POS_NORMAL_TANGENT_TEXCOORD0>) : sizeof(Vertex<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>);

		//go through all the meshes in the model
		for (unsigned int meshIndex = 0; meshIndex < aiModel->mNumMeshes; meshIndex++)
		{
			auto aMesh = aiModel->mMeshes[meshIndex];

			std::vector<byte> vertexData(vertexSize * aMesh->mNumVertices);
			VerticesLoader.at(vertexClass)(aMesh, vertexData);

			std::vector<unsigned int> indicesData;
			LoadIndices(aMesh, indicesData);

			if (animations) {
				LoadBonesInVertices(aMesh, animations->bonesOffsets, reinterpret_cast<Vertex<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>*>(vertexData.data()));
			}

			unsigned int indicesCount = aMesh->mNumFaces * aMesh->mFaces[0].mNumIndices;
			std::unique_ptr<MeshInstance>& mesh = GetMeshInstance(GetModel3DMeshInstanceUUID(model3DUUID, meshIndex), vertexClass, vertexData.data(), static_cast<unsigned int>(vertexSize), aMesh->mNumVertices, indicesData.data(), indicesCount);

			CreateBoundingBox(mesh->boundingBox, aMesh);

			meshes.push_back(mesh->uuid);

			auto mats = mdl->materials();
			JUUID fallbackMaterial = GetMaterialUUIDByName(Model3D::fallbackMaterialName);
			std::transform(mats.begin(), mats.end(), std::back_inserter(materialUUIDs), [fallbackMaterial](JUUID matUUID)
				{
					return (!matUUID.empty()) ? matUUID : fallbackMaterial;
				}
			);
		}

		importer.FreeScene();
	}

	void Model3DInstance::CreateModel3DMaterialsTemplates(const aiScene* aiModel)
	{
		using namespace Templates;

		std::unique_ptr<Model3DJson>& mdl = GetModel3DTemplate(model3DUUID);

		if (mdl->materials().size() == aiModel->mNumMeshes)
			return;

		std::string filename = default3DModelsFolder + mdl->path();
		std::filesystem::path path(filename);

#if defined(_EDITOR)
		bool dirtyTemplatesPanel = false;
#endif

		//go through all the meshes in the model
		for (unsigned int meshIndex = 0; meshIndex < aiModel->mNumMeshes; meshIndex++)
		{
			auto aMesh = aiModel->mMeshes[meshIndex];
			aiMaterial* aiMat = aiModel->mMaterials[aMesh->mMaterialIndex];

			nlohmann::json texturesMaterialJson = GetAssimpTexturesMaterialJson(path.relative_path(), aiModel, aiMat);

			std::string materialUUID = GetModel3DMaterialInstanceUUID(model3DUUID, meshIndex);

			if (!MaterialTemplateExist(materialUUID))
			{
#if defined(_DEVELOPMENT)
				MaterialJson materialJson = CreateModel3DMaterialJson(
					materialUUID,
					GetModel3DMaterialInstanceName(model3DUUID, meshIndex),
					mdl->shader_vs(),
					mdl->shader_ps(),
					aiModel->mMaterials[aMesh->mMaterialIndex]
				);
				materialJson.merge_patch(texturesMaterialJson);
				nlohmann::json j = materialJson.json();
				CreateMaterial(j);
#if defined(_EDITOR)
				dirtyTemplatesPanel = true;
#endif
#endif
			}
			mdl->materials_push_back(materialUUID);
		}
#if defined(_EDITOR) //economic
		if (dirtyTemplatesPanel)
		{
			Editor::MarkTemplatesPanelAssetsAsDirty();
		}
#endif
	}

	void Model3DInstance::CreateBoundingBox(BoundingBox& boundingBox, aiMesh* aMesh)
	{
		XMFLOAT3 center = {
			0.5f * (aMesh->mAABB.mMin[0] + aMesh->mAABB.mMax[0]),
			0.5f * (aMesh->mAABB.mMin[1] + aMesh->mAABB.mMax[1]),
			0.5f * (aMesh->mAABB.mMin[2] + aMesh->mAABB.mMax[2]),
		};

		XMFLOAT3 extents = {
			0.5f * fabs(aMesh->mAABB.mMin[0] - aMesh->mAABB.mMax[0]),
			0.5f * fabs(aMesh->mAABB.mMin[1] - aMesh->mAABB.mMax[1]),
			0.5f * fabs(aMesh->mAABB.mMin[2] - aMesh->mAABB.mMax[2]),
		};

		boundingBox = BoundingBox(center, extents);
	}

	nlohmann::json Model3DInstance::GetAssimpTexturesMaterialJson(std::filesystem::path relativePath, const aiScene* aiModel, aiMaterial* material)
	{
		using namespace Templates::Model3D;

		nlohmann::json mat(nlohmann::json({}));

		//process diffuse texture
		aiString diffuseName;
		material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), diffuseName);
		const aiTexture* embeddedDiffuse = aiModel->GetEmbeddedTexture(diffuseName.C_Str());
		if (!embeddedDiffuse || std::string(diffuseName.C_Str()).empty())
		{
			PushAssimpTextureToJson(mat, TextureShaderUsage_Base, relativePath, diffuseName, defaultBaseTexture);
		}
		else
		{
			PushEmbeddedAsimpTextureToJson(mat, embeddedDiffuse, TextureShaderUsage_Base, relativePath, diffuseName);
		}

		aiString normalMapName;
		material->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), normalMapName);
		const aiTexture* embeddedNormalMap = aiModel->GetEmbeddedTexture(normalMapName.C_Str());
		if (!embeddedNormalMap || std::string(normalMapName.C_Str()).empty())
		{
			PushAssimpTextureToJson(mat, TextureShaderUsage_NormalMap, relativePath, normalMapName, defaultNormalMap, DXGI_FORMAT_R8G8B8A8_UNORM);
		}
		else
		{
			PushEmbeddedAsimpTextureToJson(mat, embeddedNormalMap, TextureShaderUsage_NormalMap, relativePath, normalMapName, DXGI_FORMAT_R8G8B8A8_UNORM);
		}

		aiString metallicRoughnessName;
		material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &metallicRoughnessName);
		const aiTexture* embeddedMetallicRoughness = aiModel->GetEmbeddedTexture(metallicRoughnessName.C_Str());
		if (!embeddedMetallicRoughness || std::string(metallicRoughnessName.C_Str()).empty())
		{
			PushAssimpTextureToJson(mat, TextureShaderUsage_MetallicRoughness, relativePath, metallicRoughnessName, "", DXGI_FORMAT_R8G8B8A8_UNORM);
		}
		else
		{
			PushEmbeddedAsimpTextureToJson(mat, embeddedMetallicRoughness, TextureShaderUsage_MetallicRoughness, relativePath, metallicRoughnessName, DXGI_FORMAT_R8G8B8A8_UNORM);
		}

		return mat;
	}

	void Model3DInstance::PushAssimpTextureToJson(nlohmann::json& m, TextureShaderUsage textureType, std::filesystem::path relativePath, aiString& aiTextureName, std::string fallbackTexture, DXGI_FORMAT fallbackFormat)
	{
		std::filesystem::path textureJsonPath = fallbackTexture;
		DXGI_FORMAT textureJsonFormat = fallbackFormat;

		if (aiTextureName.length > 0)
		{
			textureJsonPath = nostd::normalize_path(
				relativePath.parent_path().append(aiTextureName.C_Str()).string()
			);
		}

		if (textureJsonPath != "")
		{
			JUUID texUUID = GetTextureUUIDByName(textureJsonPath.string());
			if (texUUID.empty())
			{
				if (std::filesystem::exists(textureJsonPath))
				{
					std::filesystem::path textureJsonPathDDS(textureJsonPath);
					textureJsonPathDDS.replace_extension(".dds");
					if (std::filesystem::exists(textureJsonPathDDS))
					{
						textureJsonFormat = GetTextureFormat(textureJsonPathDDS);
					}
					else
					{
						textureJsonFormat = GetTextureFormat(textureJsonPath);
					}
				}
				texUUID = CreateTextureTemplate(textureJsonPath.string(), textureJsonFormat);
			}
			//m.textures_insert(textureType, texUUID);
			if (!m.contains("textures"))
			{
				m["textures"] = nlohmann::json::object({});
			}
			m["textures"][TextureShaderUsageToString.at(textureType)] = texUUID;
		}
	}

	void Model3DInstance::PushEmbeddedAsimpTextureToJson(nlohmann::json& m, const aiTexture* embeddedTexture, TextureShaderUsage textureType, std::filesystem::path relativePath, aiString& aiTextureName, DXGI_FORMAT fallbackFormat)
	{
		DXGI_FORMAT textureJsonFormat = fallbackFormat;

		std::string fileName = aiTextureName.C_Str();
		fileName = TextureShaderUsageToString.at(textureType) + "_" + std::regex_replace(fileName, std::regex("\\*"), "");
		std::filesystem::path textureJsonPath = nostd::normalize_path(
			relativePath.parent_path().append("textures/").append(fileName).replace_extension(embeddedTexture->achFormatHint).string()
		);

		if (!std::filesystem::exists(textureJsonPath))
		{
			//first create the directory if needed
			std::filesystem::path directory = textureJsonPath.parent_path();
			std::filesystem::create_directory(directory);

			//then write the image file
			std::ofstream file;
			file.open(textureJsonPath, std::ios::app | std::ios::binary);
			file.write(reinterpret_cast<const char*>(&embeddedTexture->pcData[0].b), embeddedTexture->mWidth);
			file.close();
		}

		std::string texUUID = GetTextureUUIDByName(textureJsonPath.string());
		if (texUUID.empty())
		{
			if (std::filesystem::exists(textureJsonPath))
			{
				std::filesystem::path textureJsonPathDDS(textureJsonPath);
				textureJsonPathDDS.replace_extension(".dds");
				if (std::filesystem::exists(textureJsonPathDDS))
				{
					textureJsonFormat = GetTextureFormat(textureJsonPathDDS);
				}
				else
				{
					textureJsonFormat = GetTextureFormat(textureJsonPath);
				}
			}
			texUUID = CreateTextureTemplate(textureJsonPath.string(), textureJsonFormat);
		}
		//m.textures_insert(textureType, texUUID);
		if (!m.contains("textures"))
		{
			m["textures"] = nlohmann::json::object({});
		}
		m["textures"][TextureShaderUsageToString.at(textureType)] = texUUID;
	}

	MaterialJson Model3DInstance::CreateModel3DMaterialJson(std::string materialUUID, std::string materialName, std::string vertexShader, std::string pixelShader, aiMaterial* material)
	{
		nlohmann::json j;
		MaterialJson matJson(j);

		matJson.uuid(materialUUID);
		matJson.name(materialName);
		matJson.shader_vs(vertexShader);
		matJson.shader_ps(pixelShader);
		matJson.mappedValues({});

		bool twoSided;
		material->Get(AI_MATKEY_TWOSIDED, twoSided);
		CD3DX12_RASTERIZER_DESC rasterizerState(D3D12_DEFAULT);
		rasterizerState.CullMode = twoSided ? D3D12_CULL_MODE_NONE : D3D12_CULL_MODE_BACK;
		matJson.rasterizerState(rasterizerState);

		ai_real shininess;
		material->Get(AI_MATKEY_SHININESS, shininess);
		matJson.mappedValues_push_back({ "specularExponent",{ MaterialVariablesTypes::MAT_VAR_FLOAT, shininess } });

		ai_real metallic;
		material->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
		matJson.mappedValues_push_back({ "metallicFactor",{ MaterialVariablesTypes::MAT_VAR_FLOAT, metallic } });

		ai_real roughness;
		material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
		matJson.mappedValues_push_back({ "roughnessFactor",{ MaterialVariablesTypes::MAT_VAR_FLOAT, roughness } });

		aiString alphaMode;
		material->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
		if (strcmp(alphaMode.C_Str(), "OPAQUE") == 0)
		{
			matJson.mappedValues_push_back({ "alphaCut",{ MaterialVariablesTypes::MAT_VAR_FLOAT, 1.0f } });
		}
		else
		{
			ai_real alphaCut;
			material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCut);
			matJson.mappedValues_push_back({ "alphaCut",{ MaterialVariablesTypes::MAT_VAR_FLOAT, alphaCut } });
		}

		matJson.create_samplers({ MaterialSamplerDesc() });

		return matJson;
	}
}