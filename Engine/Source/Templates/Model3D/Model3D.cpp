#include "pch.h"
#include "Model3D.h"
#include <d3d12.h>
#include <nlohmann/json.hpp>
#include <Application.h>
#include <NoStd.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>
#include <Animated.h>

using namespace Animation;

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

#if defined(_EDITOR)
	void Model3DJson::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <Model3DAtt.h>
#include <JEnd.h>
	}

	void Model3DJson::ListenUpdate(Model3D_UpdateFlags flag, SUUUID suuuid, std::function<void()> callback)
	{
		updateFlagsListeners[flag].insert_or_assign(suuuid, callback);
	}
	void Model3DJson::RemoveUpdateListener(Model3D_UpdateFlags flag, SUUUID suuuid)
	{
		if (updateFlagsListeners[flag].contains(suuuid))
			updateFlagsListeners[flag].erase(suuuid);
	}
#endif

	TEMPDEF_FULL(Model3D);
	TEMPDEF_REFTRACKER(Model3D);

#if defined(_EDITOR)
	void Model3DJsonStep()
	{
		std::set<Model3DJsonID> models;
		std::transform(Model3Dtemplates.begin(), Model3Dtemplates.end(), std::inserter(models, models.begin()), [](auto& mdl)
			{
				return mdl.first;
			}
		);

		std::set<Model3DJsonID> seqMdls;
		std::copy_if(models.begin(), models.end(), std::inserter(seqMdls, seqMdls.begin()), [](auto mdl)
			{
				return mdl->dirty(Model3DJson::Update_animationSequences);
			}
		);

		for (auto seqMdl : seqMdls)
		{
			seqMdl->clean(Model3DJson::Update_animationSequences);

			if (!seqMdl->updateFlagsListeners.contains(Model3DJson::Update_animationSequences)) continue;

			for (auto& [_, cb] : seqMdl->updateFlagsListeners.at(Model3DJson::Update_animationSequences))
			{
				cb();
			}
		}
	}
#endif

	JUUID GetModel3DMeshInstanceID(JUUID meshInstanceUUID, unsigned int index) {
		return "mesh-" + meshInstanceUUID + "-" + std::to_string(index);
	}

	JUUID GetModel3DMaterialInstanceID(JUUID materialInstanceUUID, unsigned int index) {
		return "mat-" + materialInstanceUUID + "-" + std::to_string(index);
	}

	JUUID GetModel3DMaterialTemplateName(Model3DJsonID mdl, unsigned int index)
	{
		return mdl->name() + "/mat-" + std::to_string(index);
	}

	Model3DInstance::Model3DInstance(SceneUnitId id, JUUID uuid, JUUID objectUUID)
	{
		model3D = uuid;
		LoadModel3DInstance(id);
	}

	Model3DInstance::~Model3DInstance()
	{
		for (auto m : meshes)
		{
			DestroyMeshInstance(m());
		}
		meshes.clear();
	}

	void Model3DInstance::LoadModel3DInstance(SceneUnitId id)
	{
		std::string filename = default3DModelsFolder + model3D->path();

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
			std::unique_ptr<MeshInstance>& mesh = GetMeshInstance(id, GetModel3DMeshInstanceID(model3D(), meshIndex), vertexClass, vertexData.data(), static_cast<unsigned int>(vertexSize), aMesh->mNumVertices, indicesData.data(), indicesCount);

			CreateBoundingBox(mesh->boundingBox, aMesh);

			meshes.push_back(mesh->uuid);

			auto mats = model3D->materials();
			JUUID fallbackMaterial = GetMaterialUUIDByName(Model3D::fallbackMaterialName);
			std::transform(mats.begin(), mats.end(), std::back_inserter(materials), [fallbackMaterial](JUUID mat)
				{
					return (!mat.empty()) ? mat : fallbackMaterial;
				}
			);
		}

		importer.FreeScene();
	}

	void Model3DInstance::CreateModel3DMaterialsTemplates(const aiScene* aiModel)
	{
		using namespace Templates;

		if (model3D->materials().size() == aiModel->mNumMeshes)
			return;

		std::string filename = default3DModelsFolder + model3D->path();
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

			std::string materialUUID = GetModel3DMaterialInstanceID(model3D(), meshIndex);

			if (!MaterialTemplateExist(materialUUID))
			{
#if defined(_DEVELOPMENT)
				MaterialJson materialJson = CreateModel3DMaterialJson(
					materialUUID,
					GetModel3DMaterialTemplateName(model3D, meshIndex),
					model3D->shader_vs(),
					model3D->shader_ps(),
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
			model3D->materials_push_back(materialUUID);
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
		//using namespace Templates::Model3D;
		using namespace Templates::Texture;

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
		if (!m.contains("textures"))
		{
			m["textures"] = nlohmann::json::object({});
		}
		m["textures"][TextureShaderUsageToString.at(textureType)] = texUUID;
	}

	MaterialJson Model3DInstance::CreateModel3DMaterialJson(JUUID materialUUID, JNAME materialName, JUUID vertexShader, JUUID pixelShader, aiMaterial* material)
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