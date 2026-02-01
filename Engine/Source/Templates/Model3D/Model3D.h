#pragma once

//#include <assimp/Importer.hpp>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>
//#include <assimp/GltfMaterial.h>
//#include <VertexFormats.h>
//#include <Mesh/Mesh.h>
//#include <Material/Material.h>
//#include <Animated.h>
//#include <DirectXCollision.h>
#include <Templates.h>
#include <JTemplate.h>
//#include <JTypes.h>
//#include <TemplateDecl.h>
#include <Sequence/AnimationSequences.h>

//namespace Animation { struct Animated; };
//namespace Templates { struct TextureJson; struct MaterialJson; };

namespace Templates
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <Model3DAtt.h>
#include <JEnd.h>

	void Model3DJsonStep();

#endif

	namespace Model3D
	{
		inline static const std::string templateName = "model3ds.json";
		inline static const std::string fallbackMaterialName = "BaseLighting";
		inline static const TemplateType templateType = T_Models3D;
	}

	struct Model3DJson : public JTemplate
	{
		TEMPLATE_DECL(Model3D);

#include <Attributes/JFlags.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif
	};

	TEMPDECL_FULL(Model3D);

	JUUID GetModel3DMeshInstanceUUID(std::string uuid, unsigned int index);
	JUUID GetModel3DMaterialInstanceUUID(std::string uuid, unsigned int index);
	JUUID GetModel3DMaterialTemplateName(std::string uuid, unsigned int index);

	struct Model3DInstance
	{
		JUUID model3DUUID;

		Model3DInstance(JUUID uuid) { assert(!!!"do not use"); }
		explicit Model3DInstance(SceneUnitId id, JUUID uuid, JUUID objectUUID);
		~Model3DInstance();
		void LoadModel3DInstance(SceneUnitId id);
		void CreateModel3DMaterialsTemplates(const aiScene* aiModel);
		void CreateBoundingBox(BoundingBox& boundingBox, aiMesh* aMesh);
		nlohmann::json GetAssimpTexturesMaterialJson(std::filesystem::path relativePath, const aiScene* aiModel, aiMaterial* material);
#if defined(_DEVELOPMENT)
		void PushAssimpTextureToJson(nlohmann::json& j, TextureShaderUsage textureType, std::filesystem::path relativePath, aiString& aiTextureName, std::string fallbackTexture = "", DXGI_FORMAT fallbackFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		void PushEmbeddedAsimpTextureToJson(nlohmann::json& m, const aiTexture* embeddedTexture, TextureShaderUsage textureType, std::filesystem::path relativePath, aiString& aiTextureName, DXGI_FORMAT fallbackFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		MaterialJson CreateModel3DMaterialJson(JUUID materialUUID, JNAME materialName, JUUID vertexShader, JUUID pixelShader, aiMaterial* material);
#endif
		VertexClass vertexClass;
		std::vector<MeshInstanceUUID> meshes;
		std::vector<JUUID> materialUUIDs;
		//animation
		std::unique_ptr<Animation::Animated> animations;
	};

	TEMPDECL_REFTRACKER(Model3D);
}
