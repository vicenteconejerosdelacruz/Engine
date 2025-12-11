#include "pch.h"
#include <Templates.h>
#include <TemplateDef.h>
#include "Material.h"
#include "Variables.h"
#include <ShaderCompiler.h>

namespace Templates
{
#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <MaterialAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <MaterialAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <MaterialAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <MaterialAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <MaterialAtt.h>
#include <JEnd.h>

#endif

	MaterialJson::MaterialJson(nlohmann::json& json) : JTemplate(json)
	{
#include <Attributes/JInit.h>
#include <MaterialAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <MaterialAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void MaterialJson::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <MaterialAtt.h>
#include <JEnd.h>
	}
#endif

	TEMPDEF_FULL(Material);
	TEMPDEF_REFTRACKER(Material);

	void MaterialJsonStep()
	{
		std::set<MaterialJsonUUID> mats;
		std::transform(Materialtemplates.begin(), Materialtemplates.end(), std::inserter(mats, mats.begin()), [](auto& temps)
			{
				return temps.first;
			}
		);

		std::set<MaterialJsonUUID> rebuildMaterials;
		std::copy_if(mats.begin(), mats.end(), std::inserter(rebuildMaterials, rebuildMaterials.begin()), [](auto mat)
			{
				return mat->dirty(MaterialJson::Update_shader_vs) ||
					mat->dirty(MaterialJson::Update_shader_ps) ||
					mat->dirty(MaterialJson::Update_samplers) ||
					mat->dirty(MaterialJson::Update_mappedValues) ||
					mat->dirty(MaterialJson::Update_textures) ||
					mat->dirty(MaterialJson::Update_rasterizerState) ||
					mat->dirty(MaterialJson::Update_blendState);
			}
		);

		if (rebuildMaterials.size() > 0ULL)
		{
			JObject::RunChangesCallback(rebuildMaterials, [](auto mat)
				{
					mat->clean(MaterialJson::Update_shader_vs);
					mat->clean(MaterialJson::Update_shader_ps);
					mat->clean(MaterialJson::Update_samplers);
					mat->clean(MaterialJson::Update_mappedValues);
					mat->clean(MaterialJson::Update_textures);
					mat->clean(MaterialJson::Update_rasterizerState);
					mat->clean(MaterialJson::Update_blendState);
				}
			);
		}
	}

	MaterialInstance::MaterialInstance(
		JUUID instance_uuid,
		JUUID uuid,
		VertexClass vClass,
		bool isShadowed,
		bool hasIBL,
		TextureShaderUsageMap overrideTextures,
		JUUID bindingUUID,
		JObjectChangeCallback materialChangeCallback,
		JObjectChangePostCallback materialChangePostCallback
	)
	{
		instanceUUID = instance_uuid;
		materialUUID = uuid;

		std::unique_ptr<MaterialJson>& material = GetMaterialTemplate(uuid);
		if (bindingUUID != "" && (materialChangeCallback != nullptr || materialChangePostCallback != nullptr)) {
			material->BindChangeCallback(bindingUUID, materialChangeCallback, materialChangePostCallback);
		}

		auto matTextures = material->textures();
		std::transform(matTextures.begin(), matTextures.end(), std::inserter(textures, textures.end()), [](auto& pair)
			{
				CreateTextureInstance(pair.second);
				return TextureUsageInstancePair(pair.first, pair.second);
			}
		);
		std::transform(overrideTextures.begin(), overrideTextures.end(), std::inserter(textures, textures.end()), [](auto& pair)
			{
				CreateTextureInstance(pair.second);
				return TextureUsageInstancePair(pair.first, pair.second);
			}
		);
		if (textures.size() > 0ULL) samplers = material->samplers();

		vertexShaderUUID = material->shader_vs();
		pixelShaderUUID = material->shader_ps();

		vertexClass = vClass;
		shadowed = isShadowed;
		ibl = hasIBL;
		CreateMaterialShaderDefines();
		CreateShaderInstances();
		LoadVariablesMapping();
	}

	void MaterialInstance::CreateMaterialShaderDefines()
	{
		defines.clear();

		//OutputDebugStringA((instanceName + ": buildDefines:" + materialUUID + "\n").c_str());

		std::vector<std::string> vertexClassDefines = VertexClassDefines.at(vertexClass);

		auto& mat = materialUUID;
		TextureShaderUsageMap texMap = mat->textures();

		if (texMap.size() == 0UL)
		{
			//remove textures components
			std::copy_if(vertexClassDefines.begin(), vertexClassDefines.end(), std::back_inserter(defines), [](auto& def)
				{
					return !VertexTextureCompoentsString.contains(def);
				}
			);
		}
		else
		{
			std::move(vertexClassDefines.begin(), vertexClassDefines.end(), std::back_inserter(defines));
			for (auto& [texType, texUUID] : texMap)
			{
				defines.push_back(textureShaderUsageToShaderDefine.at(texType));

				std::unique_ptr<TextureJson>& tex = GetTextureTemplate(texUUID);
				if (NonLinearDxgiFormats.contains(tex->format()))
				{
					std::string srgbTexDefine = textureShaderUsageInGammaSpaceToShaderDefine.at(texType);
					defines.push_back(srgbTexDefine);
				}
			}
		}

		if (shadowed)
		{
			defines.push_back(textureShaderUsageToShaderDefine.at(TextureShaderUsage_ShadowMaps));
		}

		if (ibl)
		{
			defines.push_back(textureShaderUsageToShaderDefine.at(TextureShaderUsage_IBLIrradiance));
			defines.push_back(textureShaderUsageToShaderDefine.at(TextureShaderUsage_IBLPreFilteredEnvironment));
			defines.push_back(textureShaderUsageToShaderDefine.at(TextureShaderUsage_IBLBRDFLUT));
		}
	}

	void MaterialInstance::CreateShaderInstances()
	{
		using namespace ShaderCompiler;
		Source compVS = { .shaderType = VERTEX_SHADER, .shaderTarget = shaderTarget.at(VERTEX_SHADER),.shaderUUID = vertexShaderUUID(), .defines = defines };
		Source compPS = { .shaderType = PIXEL_SHADER, .shaderTarget = shaderTarget.at(PIXEL_SHADER), .shaderUUID = pixelShaderUUID(), .defines = defines };
		vertexShaderInstanceUUID = vertexShaderUUID() + std::to_string(std::hash<Source>()(compVS));
		pixelShaderInstanceUUID = pixelShaderUUID() + std::to_string(std::hash<Source>()(compPS));
		auto onVSShaderChange = [this](JUUID vsShader)
			{
				auto& mat = materialUUID;
				mat->flag(MaterialJson::Update_shader_vs);
			};
		auto onPSShaderChange = [this](JUUID psShader)
			{
				auto& mat = materialUUID;
				mat->flag(MaterialJson::Update_shader_ps);
			};
		CreateShaderInstance(vertexShaderInstanceUUID(), [this, compVS, onVSShaderChange]
			{
				return std::make_unique<ShaderInstance>(vertexShaderInstanceUUID(), compVS.shaderUUID, compVS, instanceUUID, onVSShaderChange);
			}
		);
		CreateShaderInstance(pixelShaderInstanceUUID(), [this, compPS, onPSShaderChange]
			{
				return std::make_unique<ShaderInstance>(pixelShaderInstanceUUID(), compPS.shaderUUID, compPS, instanceUUID, onPSShaderChange);
			}
		);
	}

	void MaterialInstance::Destroy()
	{
		using namespace ShaderCompiler;

		if (ShaderTemplateExist(vertexShaderUUID()))
			vertexShaderUUID->UnbindChangeCallback(instanceUUID);

		if (ShaderTemplateExist(pixelShaderUUID()))
			pixelShaderUUID->UnbindChangeCallback(instanceUUID);

		DeleteShaderInstance(vertexShaderInstanceUUID());
		DeleteShaderInstance(pixelShaderInstanceUUID());
		for (auto& [type, tex] : textures)
		{
			DeleteTextureInstance(tex);
		}
	}

	void MaterialInstance::LoadVariablesMapping()
	{
		auto& vertexShader = vertexShaderInstanceUUID;
		auto& pixelShader = pixelShaderInstanceUUID;

		//do i really need this?
		unsigned int numConstantsBuffers = static_cast<unsigned int>(
			std::max(vertexShader->cbufferSize.size(), pixelShader->cbufferSize.size())
			);

		variablesBufferSize.clear();
		variablesBuffer.clear();
		for (unsigned int index = 0; index < numConstantsBuffers; index++)
		{
			//get the CBuffer size
			size_t vsConstantBufferSize = vertexShader->cbufferSize.size() > index ? vertexShader->cbufferSize[index] : 0;
			size_t psConstantBufferSize = pixelShader->cbufferSize.size() > index ? pixelShader->cbufferSize[index] : 0;

			unsigned int constantsBufferSize = static_cast<unsigned int>(std::max(vsConstantBufferSize, psConstantBufferSize));
			variablesBufferSize.push_back(constantsBufferSize);

			//allocate memory for the cbuffer on the CPU
			variablesBuffer.push_back(std::vector<byte>(constantsBufferSize));
		}

		ShaderConstantsBufferVariablesMap& vsVars = vertexShader->constantsBuffersVariables;
		ShaderConstantsBufferVariablesMap& psVars = pixelShader->constantsBuffersVariables;

		//initialize the variables mapping
		auto& material = materialUUID;
		ShaderJsonUUID shader_vs = material->shader_vs();
		ShaderJsonUUID shader_ps = material->shader_ps();

		nlohmann::json materialMappedValues = nlohmann::json::array();
		nlohmann::json shaderVSMappedValues = nlohmann::json::array();
		nlohmann::json shaderPSMappedValues = nlohmann::json::array();

		std::map<std::string, nlohmann::json> mappedValues;

		//mapped values comes from the material, vertex and pixel shaders
		if (material->contains("mappedValues")) { materialMappedValues = material->at("mappedValues"); }
		if (shader_vs->contains("mappedValues")) { shaderVSMappedValues = shader_vs->at("mappedValues"); }
		if (shader_ps->contains("mappedValues")) { shaderPSMappedValues = shader_ps->at("mappedValues"); }
		for (nlohmann::json mappedValue : materialMappedValues) { mappedValues.insert_or_assign(mappedValue.at("variable"), mappedValue); }
		for (nlohmann::json mappedValue : shaderVSMappedValues) { mappedValues.insert_or_assign(mappedValue.at("variable"), mappedValue); }
		for (nlohmann::json mappedValue : shaderPSMappedValues) { mappedValues.insert_or_assign(mappedValue.at("variable"), mappedValue); }

		variablesMapping.clear();

		for (auto& [varName, mappedValue] : mappedValues)
		{
			if (vsVars.contains(varName))
			{
				MaterialVariablesTypes variableType = StringToMaterialVariablesTypes.at(mappedValue.at("variableType"));
				ShaderConstantsBufferVariable& mapping = vsVars.at(varName);
				variablesMapping.insert_or_assign(varName, MaterialVariableMapping({ .variableType = variableType, .mapping = mapping }));
				continue;
			}

			if (psVars.contains(varName))
			{
				MaterialVariablesTypes variableType = StringToMaterialVariablesTypes.at(mappedValue.at("variableType"));
				ShaderConstantsBufferVariable& mapping = psVars.at(varName);
				variablesMapping.insert_or_assign(varName, MaterialVariableMapping({ .variableType = variableType, .mapping = mapping }));
				continue;
			}
		}

		auto constantsBufferContains = [this, &vertexShader, &pixelShader](std::string varName)
			{
				ShaderConstantsBufferVariablesMap& vsVars = vertexShader->constantsBuffersVariables;
				ShaderConstantsBufferVariablesMap& psVars = pixelShader->constantsBuffersVariables;

				return vsVars.contains(varName) || psVars.contains(varName);
			};

		auto getConstantsBufferVariable = [this, &vertexShader, &pixelShader](std::string varName)
			{
				ShaderConstantsBufferVariablesMap& vsVars = vertexShader->constantsBuffersVariables;
				ShaderConstantsBufferVariablesMap& psVars = pixelShader->constantsBuffersVariables;
				return (vsVars.contains(varName) ? vsVars.at(varName) : psVars.at(varName));
			};

		std::set<std::string> unmapped;
		for (auto& [varName, mappedValue] : mappedValues)
		{
			if (!constantsBufferContains(varName)) continue;

			auto def = getConstantsBufferVariable(varName);

			unmapped.insert(varName);

			size_t size = def.size;
			byte* dst = variablesBuffer[def.bufferIndex].data();
			dst += def.offset;
			auto initialValue = JsonToMaterialInitialValue(mappedValue);
			WriteMappedInitialValuesToDestination(initialValue, dst, size);
		}

		//write the values to mapped memory
		for (auto [varName, mapping] : variablesMapping)
		{
			if (!mappedValues.contains(varName)) continue;

			nlohmann::json def = mappedValues.at(varName);

			size_t size = mapping.mapping.size;
			byte* dst = variablesBuffer[mapping.mapping.bufferIndex].data();
			dst += mapping.mapping.offset;
			auto initialValue = JsonToMaterialInitialValue(def);
			WriteMappedInitialValuesToDestination(initialValue, dst, size);
			unmapped.erase(varName);
		}

		//map variables defined in the shader that the material has not yet define, so it can be updated in the rendereable cbuffer
		for (auto varName : unmapped)
		{
			if (!mappedValues.contains(varName)) continue;
			if (!constantsBufferContains(varName)) continue;

			//auto& matdef = material.at("mappedValues").at(matVarIndex.at(varName));
			auto& matdef = mappedValues.at(varName);
			auto& variableType = StringToMaterialVariablesTypes.at(std::string(matdef.at("variableType")));
			auto mapping = getConstantsBufferVariable(varName);

			variablesMapping.insert_or_assign(varName, MaterialVariableMapping({ .variableType = variableType, .mapping = mapping }));
		}
	}

	//READ&GET
	bool MaterialInstance::ShaderInstanceHasRegister(std::function<int(ShaderInstanceUUID)> getRegister)
	{
		return (getRegister(vertexShaderInstanceUUID) != -1) || (getRegister(pixelShaderInstanceUUID) != -1);
	}

	void MaterialInstance::SetUAVRootDescriptorTable(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& slot)
	{
		auto& pixelShader = pixelShaderInstanceUUID;
		for (auto& [name, uavParam] : pixelShader->uavParameters)
		{
			if (uav.contains(uavParam.registerId))
			{
				commandList->SetGraphicsRootDescriptorTable(slot, uav.at(uavParam.registerId));
				slot++;
			}
		}
	}

	void MaterialInstance::SetSRVRootDescriptorTable(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& slot)
	{
		auto& pixelShader = pixelShaderInstanceUUID;
		for (auto& [textureType, texParam] : pixelShader->srvTexParameters)
		{
			if (texParam.numSRV == 0xFFFFFFFF || iblUsageTexture.contains(textureType)) continue;
			auto& texInstance = GetTextureInstance(textures.at(textureType));
			commandList->SetGraphicsRootDescriptorTable(slot, texInstance->gpuHandle);
			slot++;
		}
	}

	void DestroyMaterialInstance(JUUID materialInstance)
	{
		DeleteMaterialInstance(materialInstance);
	}
}
