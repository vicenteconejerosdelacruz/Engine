#include "pch.h"
#include "Shader.h"
#include <ShaderCompiler.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <CompilerQueue.h>

namespace Templates {

#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#endif

	namespace Shader
	{
		ShaderIncludesDependencies dependencies;
		std::set<Source> GetSourcesFromShaderUUID(JUUID shaderTemplate)
		{
			ShaderIncludesDependencies matchingUUIDs;
			std::copy_if(dependencies.begin(), dependencies.end(), std::inserter(matchingUUIDs, matchingUUIDs.begin()),
				[shaderTemplate](const auto& dep) { return dep.first.shaderTemplate() == shaderTemplate; }
			);
			std::set<Source> sources;
			std::transform(matchingUUIDs.begin(), matchingUUIDs.end(), std::inserter(sources, sources.begin()), [](const auto& pair) { return pair.first; });
			return sources;
		}
		std::multimap<std::string, std::string> fileNameToShaderTemplate;
	};

	ShaderJson::ShaderJson(nlohmann::json& json) : JTemplate(json)
	{
#include <Attributes/JInit.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <ShaderAtt.h>
#include <JEnd.h>

		Shader::fileNameToShaderTemplate.insert({ path(),uuid() });
	}

#if defined(_EDITOR)
	void ShaderJson::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <ShaderAtt.h>
#include <JEnd.h>
	}
#endif

	TEMPDEF_FULL(Shader);
	TEMPDEF_REFTRACKER(Shader);

#if defined(_DEVELOPMENT)
	void ShaderJsonStep()
	{
		std::set<ShaderJsonID> shaders;
		std::transform(Shadertemplates.begin(), Shadertemplates.end(), std::inserter(shaders, shaders.begin()), [](auto& temps)
			{
				return temps.first;
			}
		);

		std::set<ShaderJsonID> rebuildShaders;
		std::copy_if(shaders.begin(), shaders.end(), std::inserter(rebuildShaders, rebuildShaders.begin()), [](auto shader)
			{
				return shader->dirty(ShaderJson::Update_path);
			}
		);

		if (rebuildShaders.size() > 0ULL)
		{
			//JObject::RunChangesCallback(rebuildShaders, [](auto shader)
			//	{
			//		shader->clean(ShaderJson::Update_path);
			//	}
			//);
		}
	}

	bool CheckChangesCompilation(JUUID uuid)
	{
		using namespace Shader;
		using namespace ShaderCompiler;
		using namespace Templates::Shader;

		std::set<Source> sources = GetSourcesFromShaderUUID(uuid);
		for (auto src : sources)
		{
			ShaderInstance dummy("", src.shaderTemplate(), src);
			ShaderIncludesDependencies deps;
			if (!Compile(dummy, src, deps))
				return false;
		}
		return true;
	}

	void PropagateChangeToShader(std::string shaderFile)
	{
		using namespace Shader;
		auto range = fileNameToShaderTemplate.equal_range(shaderFile);
		for (auto& it = range.first; it != range.second; it++)
		{
			if (!CheckChangesCompilation(it->second)) continue;
			auto& shader = GetShaderTemplate(it->second);
			shader->flag(ShaderJson::Update_path);
		}
	}

	void PropagateChangeToShaderFromDependency(std::string dependency)
	{
		using namespace Shader;
		std::set<std::string> shadersUUIDs;
		for (auto& [src, deps] : dependencies)
		{
			//if the file is not in the dependency skip
			if (!deps.contains(dependency) || !CheckChangesCompilation(src.shaderTemplate())) continue;
			src.shaderTemplate->flag(ShaderJson::Update_path);
		}
	}

	static CompilerQueue changesQueue;
	void MonitorShaderChanges(std::string folder)
	{
		std::thread monitor([folder]()
			{
				HANDLE file = CreateFileA(folder.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);

				if (file == INVALID_HANDLE_VALUE || file == NULL) {
					OutputDebugStringA("ERROR: Invalid HANDLE value.\n");
					ExitProcess(GetLastError());
				}

				OVERLAPPED overlapped;
				overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);

				uint8_t change_buf[1024];
				BOOL success = ReadDirectoryChangesW(
					file, change_buf, 1024, TRUE,
					FILE_NOTIFY_CHANGE_LAST_WRITE,
					NULL, &overlapped, NULL);

				while (true) {
					DWORD result = WaitForSingleObject(overlapped.hEvent, 0);

					if (result == WAIT_OBJECT_0) {
						DWORD bytes_transferred;
						GetOverlappedResult(file, &overlapped, &bytes_transferred, FALSE);

						FILE_NOTIFY_INFORMATION* event = (FILE_NOTIFY_INFORMATION*)change_buf;

						for (;;) {
							DWORD name_len = event->FileNameLength / sizeof(wchar_t);

							switch (event->Action) {
							case FILE_ACTION_MODIFIED: {
								std::string shaderFile = nostd::WStringToString(std::wstring(event->FileName, event->FileNameLength / sizeof(event->FileName[0])));
								changesQueue.push(shaderFile);
							} break;
							default: {
								printf("Unknown action!\n");
							} break;
							}

							// Are there more events to handle?
							if (event->NextEntryOffset) {
								*((uint8_t**)&event) += event->NextEntryOffset;
							}
							else {
								break;
							}
						}
					}
					// Queue the next event
					BOOL success = ReadDirectoryChangesW(
						file, change_buf, 1024, TRUE,
						FILE_NOTIFY_CHANGE_LAST_WRITE,
						NULL, &overlapped, NULL);
				}
			}
		);

		std::thread changesPropagator([]()
			{
				using namespace std::chrono_literals;
				while (TRUE)
				{
					while (changesQueue.size() > 0)
					{

						std::filesystem::path filePath(changesQueue.front());
						std::string fileExtension = filePath.extension().string();

						std::string output = "Modified: " + filePath.string() + "\n";
						OutputDebugStringA(output.c_str());

						if (fileExtension == ".hlsl") {
							PropagateChangeToShader(filePath.string());
						}
						else if (fileExtension == ".h") {
							PropagateChangeToShaderFromDependency(filePath.string());
						}

						changesQueue.pop();
					}
					std::this_thread::sleep_for(200ms);
				}
			}
		);
		monitor.detach();
		changesPropagator.detach();
	}
#endif

	ShaderInstance::ShaderInstance(
		JUUID instance_uuid,
		JUUID uuid, Source params,
		JUUID bindingUUID
	)
	{
		using namespace ShaderCompiler;
		using namespace Templates::Shader;

		instanceUUID = instance_uuid;
		shaderUUID = uuid;

		Compile(*this, params, dependencies);
	}

	void ShaderInstance::CreateVSSemantics(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc)
	{
		if (shaderSource.shaderType != VERTEX_SHADER) return;
		for (unsigned int paramIdx = 0; paramIdx < desc.InputParameters; paramIdx++)
		{
			D3D12_SIGNATURE_PARAMETER_DESC signatureDesc{};
			reflection->GetInputParameterDesc(paramIdx, &signatureDesc);
			vsSemantics.push_back(signatureDesc.SemanticName);
		}
	}

	void ShaderInstance::CreateResourcesBinding(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc)
	{
		using namespace DeviceUtils;

		const std::unordered_map<std::string, int& > registersMap =
		{
			{ CameraConstantBufferName, CBV.camera },
			{ LightConstantBufferName, CBV.light },
			{ AnimationConstantBufferName, CBV.animation },
			{ ShadowMapConstantBufferName, CBV.lightsShadowMap },
			{ ShadowMapLightsShaderResourceViewName, SRV.lightsShadowMap },
			{ TextureShaderUsageToString.at(TextureShaderUsage_IBLIrradiance), SRV.iblIrradiance },
			{ TextureShaderUsageToString.at(TextureShaderUsage_IBLPreFilteredEnvironment), SRV.iblPrefiteredEnv },
			{ TextureShaderUsageToString.at(TextureShaderUsage_IBLBRDFLUT), SRV.iblBRDFLUT },
		};

		auto bindSITCBuffer = [this](D3D12_SHADER_INPUT_BIND_DESC& desc, std::string resourceName)
			{
				constantsBuffersParameters.insert_or_assign(resourceName, ShaderConstantsBufferParameter({ .registerId = desc.BindPoint, .numConstantsBuffers = desc.BindCount }));
			};
		auto bindSITUAVRWStructured = [this](D3D12_SHADER_INPUT_BIND_DESC& desc, std::string resourceName)
			{
				uavParameters.insert_or_assign(resourceName, ShaderUAVParameter({ .registerId = desc.BindPoint, .numUAV = desc.BindCount }));
			};
		auto bindSITCSStructured = [this](D3D12_SHADER_INPUT_BIND_DESC& desc, std::string resourceName)
			{
				if (shaderSource.shaderType == JShaderType::COMPUTE_SHADER)
					srvCSParameters.insert_or_assign(resourceName, ShaderSRVParameter({ .registerId = desc.BindPoint, .numSRV = desc.BindCount }));
			};
		auto bindSITPSStructured = [this](D3D12_SHADER_INPUT_BIND_DESC& desc, std::string resourceName)
			{
				if (shaderSource.shaderType == JShaderType::PIXEL_SHADER)
					srvTexParameters.insert_or_assign(StringToTextureShaderUsage.at(resourceName), ShaderSRVParameter({ .registerId = desc.BindPoint, .numSRV = desc.BindCount > 0 ? desc.BindCount : desc.NumSamples })); //if N > 0 -> N else -1
			};
		auto bindSITByteAddress = [this](D3D12_SHADER_INPUT_BIND_DESC& desc, std::string resourceName)
			{
				uavParameters.insert_or_assign(resourceName, ShaderUAVParameter({ .registerId = desc.BindPoint, .numUAV = desc.BindCount }));
			};
		auto bindSITUAVRWByteAddress = [this](D3D12_SHADER_INPUT_BIND_DESC& desc, std::string resourceName)
			{
				uavParameters.insert_or_assign(resourceName, ShaderUAVParameter({ .registerId = desc.BindPoint, .numUAV = desc.BindCount }));
			};
		auto bindSITUAVRWTyped = [this](D3D12_SHADER_INPUT_BIND_DESC& desc, std::string resourceName)
			{
				uavParameters.insert_or_assign(resourceName, ShaderUAVParameter({ .registerId = desc.BindPoint, .numUAV = desc.BindCount }));
			};
		auto bindSITSampler = [this](D3D12_SHADER_INPUT_BIND_DESC& desc, std::string resourceName)
			{
				samplersParameters.insert_or_assign(resourceName, ShaderSamplerParameter({ .registerId = desc.BindPoint, .numSamplers = desc.BindCount }));
			};
		auto bindSITTexture = [this](D3D12_SHADER_INPUT_BIND_DESC& desc, std::string resourceName)
			{
				srvTexParameters.insert_or_assign(StringToTextureShaderUsage.at(resourceName), ShaderSRVParameter({ .registerId = desc.BindPoint, .numSRV = desc.BindCount > 0 ? desc.BindCount : desc.NumSamples })); //if N > 0 -> N else -1
			};

		const std::map<D3D_SHADER_INPUT_TYPE, std::vector<std::function<void(D3D12_SHADER_INPUT_BIND_DESC&, std::string)>>> registerBinding =
		{
			{ D3D_SIT_CBUFFER, { bindSITCBuffer } },
			{ D3D_SIT_UAV_RWSTRUCTURED, { bindSITUAVRWStructured } },
			{ D3D_SIT_STRUCTURED, { bindSITCSStructured, bindSITPSStructured }},
			{ D3D_SIT_BYTEADDRESS, { bindSITByteAddress }},
			{ D3D_SIT_UAV_RWBYTEADDRESS, {bindSITUAVRWByteAddress}},
			{ D3D_SIT_UAV_RWTYPED, { bindSITUAVRWTyped }},
			{ D3D_SIT_SAMPLER, { bindSITSampler }},
			{ D3D_SIT_TEXTURE, { bindSITTexture }},
		};

		for (unsigned int paramIdx = 0; paramIdx < desc.BoundResources; paramIdx++)
		{
			D3D12_SHADER_INPUT_BIND_DESC bindDesc{};
			reflection->GetResourceBindingDesc(paramIdx, &bindDesc);

			std::string resourceName(bindDesc.Name);

			if (registersMap.contains(resourceName)) registersMap.at(resourceName) = bindDesc.BindPoint;
			auto& binders = registerBinding.at(bindDesc.Type);
			for (auto& binder : binders)
			{
				binder(bindDesc, resourceName);
			}
		}
	}

	void ShaderInstance::CreateConstantsBuffersVariables(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc)
	{
		using namespace Templates;
		using namespace DeviceUtils;

		const std::set<std::string> paramsToSkip =
		{
			CameraConstantBufferName,
			LightConstantBufferName,
			AnimationConstantBufferName,
			ShadowMapConstantBufferName,
		};

		for (unsigned int paramIdx = 0; paramIdx < desc.ConstantBuffers; paramIdx++)
		{
			auto cbReflection = reflection->GetConstantBufferByIndex(paramIdx);
			D3D12_SHADER_BUFFER_DESC paramDesc{};
			cbReflection->GetDesc(&paramDesc);

			std::string cbufferName(paramDesc.Name);
			if (HLSLNonConstantsBuffers.contains(cbufferName) || paramsToSkip.contains(cbufferName)) continue;

			bool skipVar = false;
			for (unsigned int varIdx = 0; varIdx < paramDesc.Variables; varIdx++)
			{
				ID3D12ShaderReflectionVariable* varReflection = cbReflection->GetVariableByIndex(varIdx);
				D3D12_SHADER_VARIABLE_DESC varDesc;
				varReflection->GetDesc(&varDesc);

				if (!(varDesc.uFlags & D3D_SVF_USED)) continue;

				ID3D12ShaderReflectionType* varType = varReflection->GetType();
				D3D12_SHADER_TYPE_DESC varTypeDesc;
				varType->GetDesc(&varTypeDesc);

				if (!HLSLVariableClassAllowedTypes.contains(varTypeDesc.Class))
				{
					skipVar = true;
					break;
				}

				std::string varName(varDesc.Name);
				constantsBuffersVariables.insert_or_assign(varName, ShaderConstantsBufferVariable({ .bufferIndex = paramIdx, .size = varDesc.Size, .offset = varDesc.StartOffset }));

				std::string varClass = varTypeDesc.Name;
				bool useNamePattern = false;
				for (auto it = HLSLVariablePatternToMaterialVariableTypes.begin(); it != HLSLVariablePatternToMaterialVariableTypes.end(); it++)
				{
					std::string tieClass = std::get<0>(it->first);
					if (varClass != tieClass) continue;

					std::string tiePattern = std::get<1>(it->first);

					std::string lowerVarName = varName;
					nostd::ToLower(lowerVarName);

					if (lowerVarName.find(tiePattern) == std::string::npos) continue;

					useNamePattern = true;
					break;
				}

				if (useNamePattern) continue;
			}

			if (!skipVar)
			{
				cbufferSize.push_back(paramDesc.Size);
			}
		}
	}

	void ShaderInstance::CreateByteCode(const ComPtr<IDxcResult>& result)
	{
		ComPtr<IDxcBlob> code;
		result->GetResult(&code);

		byteCode.resize(code->GetBufferSize());
		memcpy_s(byteCode.data(), code->GetBufferSize(), code->GetBufferPointer(), code->GetBufferSize());
	}
}