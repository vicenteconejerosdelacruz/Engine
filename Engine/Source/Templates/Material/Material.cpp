#include "pch.h"
#include "Material.h"
#include <Renderer.h>
#include "Variables.h"
#include <ShaderCompiler.h>

extern std::unique_ptr<JRenderer> renderer;

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

#include <Attributes/JV8Att.h>
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

	void MaterialJson::SetPipelineStateCallback(size_t hash, std::function<void()> callback)
	{
		if (pipelineChangeCallbacks.contains(hash)) return;
		pipelineChangeCallbacks.insert_or_assign(hash, callback);
	}

	TEMPDEF_FULL(Material);
	TEMPDEF_REFTRACKER(Material);

	std::unordered_map<MaterialJsonID, std::set<MaterialInstanceID>> materialsTemplatesInstances;

	void MaterialJsonStep()
	{
		std::set<MaterialJsonID> mats;
		std::transform(Materialtemplates.begin(), Materialtemplates.end(), std::inserter(mats, mats.begin()), [](auto& temps)
			{
				return temps.first;
			}
		);

		std::set<MaterialJsonID> rebuildTextures;
		std::copy_if(mats.begin(), mats.end(), std::inserter(rebuildTextures, rebuildTextures.begin()), [](auto mat)
			{
				return mat->dirty(MaterialJson::Update_textures)
					&& materialsTemplatesInstances.contains(mat)
					&& materialsTemplatesInstances.at(mat).size() > 0ULL;
			}
		);

		std::unordered_map<TextureJsonID, std::set<std::tuple<TextureShaderUsage, MaterialInstanceID>>> changes;
		std::for_each(rebuildTextures.begin(), rebuildTextures.end(), [&](auto mat)
			{
				for (auto& texUsage : materialTexturesShaderUsage)
				{
					std::string texUsageS = TextureShaderUsageToString.at(texUsage);
					JUUID prevTexUUID = "";
					if (mat->UpdatePrevValues["textures"].contains(texUsageS))
					{
						prevTexUUID = mat->UpdatePrevValues["textures"].at(texUsageS);
					}
					JUUID newTexUUID = "";
					if (mat->at("textures").contains(texUsageS))
					{
						newTexUUID = mat->at("textures").at(texUsageS);
					}
					if (prevTexUUID == newTexUUID) continue;

					for (auto instance : materialsTemplatesInstances.at(mat))
					{
						changes[newTexUUID].insert(std::make_tuple(texUsage, instance));
					}
				}
				mat->clean(MaterialJson::Update_textures);
			}
		);

		if (changes.size() > 0ULL)
		{
			UpdateMaterialTextures(changes);
		}

		std::set<MaterialJsonID> rebuildPipelineState;
		std::copy_if(mats.begin(), mats.end(), std::inserter(rebuildPipelineState, rebuildPipelineState.begin()), [](auto mat)
			{
				return mat->dirty(MaterialJson::Update_shader_vs) ||
					mat->dirty(MaterialJson::Update_shader_ps) ||
					mat->dirty(MaterialJson::Update_samplers) ||
					mat->dirty(MaterialJson::Update_rasterizerState) ||
					mat->dirty(MaterialJson::Update_blendState) ||
					mat->dirty(MaterialJson::Update_overrideDepthStencil) ||
					mat->dirty(MaterialJson::Update_depthStencil)
					;
			}
		);

		std::for_each(rebuildPipelineState.begin(), rebuildPipelineState.end(), [&](auto mat)
			{
				for (auto& [hash, callback] : mat->pipelineChangeCallbacks)
				{
					callback();
				}
				mat->clean(MaterialJson::Update_shader_vs);
				mat->clean(MaterialJson::Update_shader_ps);
				mat->clean(MaterialJson::Update_samplers);
				mat->clean(MaterialJson::Update_rasterizerState);
				mat->clean(MaterialJson::Update_blendState);
				mat->clean(MaterialJson::Update_overrideDepthStencil);
				mat->clean(MaterialJson::Update_depthStencil);
			}
		);
	}

	static bool updateTexturesProcessorInitialized = false;
	static std::unique_ptr<CommandsProcessor> updateTexturesProcessor;
	void UpdateMaterialTextures(std::unordered_map<TextureJsonID, std::set<std::tuple<TextureShaderUsage, MaterialInstanceID>>> changes)
	{
		using namespace Templates::Texture;

		if (!updateTexturesProcessorInitialized)
		{
			updateTexturesProcessor = std::make_unique<CommandsProcessor>(renderer->d3dDevice, 1, 0x10AD3D);
			updateTexturesProcessorInitialized = true;
		}

		std::thread updateTexThread([](std::unordered_map<TextureJsonID, std::set<std::tuple<TextureShaderUsage, MaterialInstanceID>>> changes)
			{
				updateTexturesProcessor->ResetCommandList();

				std::unordered_map<TextureJsonID, std::set<std::tuple<TextureShaderUsage, MaterialInstanceID>>> postChanges;
				std::transform(changes.begin(), changes.end(), std::inserter(postChanges, postChanges.begin()), [](auto& pair)
					{
						std::pair<TextureJsonID, std::set<std::tuple<TextureShaderUsage, MaterialInstanceID>>> replacement;
						replacement.second = pair.second;
						auto texUUID = pair.first;
						if (!texUUID.empty())
						{
							replacement.first = pair.first;
							return replacement;
						}

						auto usage = std::get<0>(*pair.second.begin());
						switch (usage)
						{
						case TextureShaderUsage_Base:
						{
							replacement.first = GetTextureUUIDByName(defaultBaseTexture);
						}
						break;
						case TextureShaderUsage_NormalMap:
						{
							replacement.first = GetTextureUUIDByName(defaultNormalMap);
						}
						break;
						}
						return replacement;
					}
				);

				for (auto& [texUUID, usageMat] : postChanges)
				{
					if (texUUID.empty()) continue;

					CreateTextureInstance(texUUID(), [&]
						{
							return std::make_unique<TextureInstance>(updateTexturesProcessor->GetCommandList(), texUUID());
						}
					);
				}
				updateTexturesProcessor->CloseCommandList();
				updateTexturesProcessor->RunPostExecution([=]
					{
						for (auto& [texUUID, usageMapSet] : postChanges)
						{
							for (auto [usage, instance] : usageMapSet)
							{
								instance->UpdateTexture(usage, texUUID);
							}
						}
					}
				);
				updateTexturesProcessor->ExecuteCommandList();
			}, changes
		);
		updateTexThread.detach();
	}

	MaterialInstance::MaterialInstance(
		SceneUnitId id,
		JUUID Instance_uuid,
		JUUID Template_uuid,
		VertexClass vClass,
		bool isShadowed,
		bool hasIBL,
		TextureShaderUsageMap overrideTextures,
		JUUID bindingUUID
	)
	{
		instanceUUID = Instance_uuid;
		materialUUID = Template_uuid;

		materialsTemplatesInstances[materialUUID].insert(instanceUUID);

		MaterialJsonID material = materialUUID;

		auto matTextures = material->textures();
		std::transform(matTextures.begin(), matTextures.end(), std::inserter(textures, textures.end()), [&](auto& pair)
			{
				CreateTextureInstance(pair.second, [&]
					{
						return std::make_unique<TextureInstance>(id, pair.second);
					}
				);
				return TextureUsageInstancePair(pair.first, pair.second);
			}
		);
		std::transform(overrideTextures.begin(), overrideTextures.end(), std::inserter(textures, textures.end()), [&](auto& pair)
			{
				CreateTextureInstance(pair.second, [&]
					{
						return std::make_unique<TextureInstance>(id, pair.second);
					}
				);
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
		Source compVS = { .shaderType = VERTEX_SHADER, .shaderTarget = shaderTarget.at(VERTEX_SHADER),.shaderTemplate = vertexShaderUUID, .defines = defines };
		Source compPS = { .shaderType = PIXEL_SHADER, .shaderTarget = shaderTarget.at(PIXEL_SHADER), .shaderTemplate = pixelShaderUUID, .defines = defines };
		vertexShaderInstanceID = vertexShaderUUID() + std::to_string(std::hash<Source>()(compVS));
		pixelShaderInstanceID = pixelShaderUUID() + std::to_string(std::hash<Source>()(compPS));

		CreateShaderInstance(vertexShaderInstanceID(), [this, compVS]
			{
				return std::make_unique<ShaderInstance>(vertexShaderInstanceID(), compVS.shaderTemplate(), compVS, instanceUUID());
			}
		);
		CreateShaderInstance(pixelShaderInstanceID(), [this, compPS]
			{
				return std::make_unique<ShaderInstance>(pixelShaderInstanceID(), compPS.shaderTemplate(), compPS, instanceUUID());
			}
		);
	}

	void MaterialInstance::Destroy()
	{
		using namespace ShaderCompiler;

		DeleteShaderInstance(vertexShaderInstanceID());
		DeleteShaderInstance(pixelShaderInstanceID());
		for (auto& [type, tex] : textures)
		{
			DeleteTextureInstance(tex);
		}
		materialsTemplatesInstances[materialUUID].erase(instanceUUID);
	}

	void MaterialInstance::LoadVariablesMapping()
	{
		auto& vertexShader = vertexShaderInstanceID;
		auto& pixelShader = pixelShaderInstanceID;

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
		ShaderJsonID shader_vs = material->shader_vs();
		ShaderJsonID shader_ps = material->shader_ps();

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
	bool MaterialInstance::ShaderInstanceHasRegister(std::function<int(ShaderInstanceID)> getRegister)
	{
		return (getRegister(vertexShaderInstanceID) != -1) || (getRegister(pixelShaderInstanceID) != -1);
	}

	void MaterialInstance::SetUAVRootDescriptorTable(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& slot)
	{
		auto& pixelShader = pixelShaderInstanceID;
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
		auto& pixelShader = pixelShaderInstanceID;
		for (auto& [textureType, texParam] : pixelShader->srvTexParameters)
		{
			if (texParam.numSRV == 0xFFFFFFFF || iblUsageTexture.contains(textureType)) continue;
			auto& texInstance = GetTextureInstance(textures.at(textureType));
			commandList->SetGraphicsRootDescriptorTable(slot, texInstance->gpuHandle);
			slot++;
		}
	}

	void MaterialInstance::UpdateTexture(TextureShaderUsage usage, TextureJsonID texture)
	{
		using namespace Templates::Texture;

		if (!textures.at(usage).empty())
			DeleteTextureInstance(textures.at(usage));
		textures.at(usage) = texture();
		CreateMaterialShaderDefines();
		CreateShaderInstances();
	}

	void DestroyMaterialInstance(JUUID materialInstance)
	{
		DeleteMaterialInstance(materialInstance);
	}
}
