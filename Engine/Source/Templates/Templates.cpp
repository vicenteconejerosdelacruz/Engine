#include "pch.h"
#include <fstream>
#include "Templates.h"

#if defined(_EDITOR)
namespace Editor
{
	extern void PromptTemplateDeletion(std::vector<nlohmann::json> references, std::function<void(std::vector<nlohmann::json>)> OnDelete, std::function<void()> OnCancel);
	extern void CloseDeletionPrompt();
	extern void MarkSceneUnitAsModified(SceneUnitId id);
	extern void MarkTemplatesPanelAssetsAsDirty();
	extern std::set<std::string> GetOpenedScenes(bool skipDefault = true);
	extern std::set<SceneUnitId> GetOpenedSceneUnitIds(bool skipDefault = true);
	extern void RemoveFromTemplateSelection(std::set<JUUID> uuids);
}

namespace Scene
{
	std::unordered_map<JUUID, SceneObjectType>& GetSceneObjectsTypes(SceneUnitId id);
	SceneObject* GetSceneObjectPointer(SceneUnitId id, JUUID uuid);
}
#endif

namespace Templates
{
	nlohmann::json systemShaders = nlohmann::json::array(
		{
			{
				{ "name", "IBLBRDFLUT_cs" },
				{ "path", "CSBRDFLUT.hlsl"},
				{ "systemCreated", true },
				{ "uuid", "bb76e846-4015-48a0-ab94-5286dd843052"},
				{ "type", JShaderTypeToStr.at(JShaderType::COMPUTE_SHADER) }
			},
			{
				{ "name","IBLPrefilteredEnvironmentMap_cs"},
				{ "path" , "CSPreFilteredEnvironmentMap.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "6e278619-da6b-48ec-8434-53c3506e7bfd"},
				{ "type" , JShaderTypeToStr.at(JShaderType::COMPUTE_SHADER) }
			},
			{
				{ "name", "IBLDiffuseIrradianceMap_cs" },
				{ "path", "CSDiffuseIrradianceMap.hlsl" },
				{ "systemCreated", true },
				{ "uuid", "5ebcccb5-477a-49c9-9878-9ff6453266a0" },
				{ "type", JShaderTypeToStr.at(JShaderType::COMPUTE_SHADER) }
			},
			{
				{ "name","LuminanceHistogramAverage_cs"},
				{ "path" , "CSLuminanceHistogramAverage.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "1d436897-e925-415f-9209-1364005792a0"},
				{ "type" , JShaderTypeToStr.at(JShaderType::COMPUTE_SHADER)}
			},
			{
				{ "name","LuminanceHistogram_cs"},
				{ "path" , "CSLuminanceHistogram.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "43b52d31-7040-47e7-80e6-97490550cbae"},
				{ "type" , JShaderTypeToStr.at(JShaderType::COMPUTE_SHADER)}
			},
			{
				{ "name","BoundingBox_cs"},
				{ "path" , "CSBoundingBox.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "c23ab559-be11-45ad-b598-1e48e5280914"},
				{ "type" , JShaderTypeToStr.at(JShaderType::COMPUTE_SHADER)}
			},
			{
				{ "name","BoundingBox_vs" },
				{ "path" , "BoundingBox.hlsl" },
				{ "systemCreated" , true },
				{ "uuid" , "ae7a35a5-f012-4eb6-bbe1-1f52e6203ccb" },
				{ "type" , JShaderTypeToStr.at(JShaderType::VERTEX_SHADER) }
			},
			{
				{ "name","BoundingBox_ps" },
				{ "path" , "BoundingBox.hlsl" },
				{ "systemCreated", true },
				{ "mappedValues",
					{
						{
							{ "value", { 1.0, 0.0, 0.0 } },
							{ "variable" , "baseColor" },
							{ "variableType" , "RGB" }
						}
					}
				},
				{ "uuid" , "1bf837a7-1282-4fae-a1ba-9e74e6a99b37" },
				{ "type" , JShaderTypeToStr.at(JShaderType::PIXEL_SHADER) }
			},
			{
				{ "name","BaseLighting_vs" },
				{ "path" , "BaseLighting.hlsl" },
				{ "systemCreated" , true },
				{ "uuid" , "bc331f48-6a40-4b48-b435-8276051d6993" },
				{ "type" , JShaderTypeToStr.at(JShaderType::VERTEX_SHADER) }
			},
			{
				{ "name","BaseLighting_ps" },
				{ "path" , "BaseLighting.hlsl" },
				{ "systemCreated" , true },
				{ "mappedValues" ,
					{
						{
							{ "value", { 0.11764706671237946, 0.5647059082984924, 1.0} },
							{ "variable" , "baseColor" },
							{ "variableType" , "RGB" }
						},
						{
							{ "value", 400.0 },
							{ "variable" , "specularExponent"},
							{ "variableType" , "FLOAT" }
						}
					}
				},
				{ "uuid" , "719c0122-1e9f-46e3-90aa-8f1e5e81c098"},
				{ "type" , JShaderTypeToStr.at(JShaderType::PIXEL_SHADER)}
			},
			{
				{ "name","Grid_vs"},
				{ "path" , "Grid.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "5af4ba59-a09c-41ef-bc1f-13a51fc68439"},
				{ "type" , JShaderTypeToStr.at(JShaderType::VERTEX_SHADER)}
			},
			{
				{ "name","Grid_ps" },
				{ "path", "Grid.hlsl" },
				{ "systemCreated" , true},
				{ "mappedValues" , {
					{
						{ "value", { 1.0, 0.0, 1.0 } },
						{ "variable" , "baseColor" },
						{ "variableType" , "RGB" }
					},
					{
						{ "value", 1024.0 },
						{ "variable" , "specularExponent"},
						{ "variableType" , "FLOAT" }
					}
				}
				},
				{ "uuid" , "5929c8f6-e9b7-4680-8447-a430b5accdbf"},
				{ "type" , JShaderTypeToStr.at(JShaderType::PIXEL_SHADER) }
			},
			{
				{ "name","ShadowMap_vs"},
				{ "path" , "ShadowMap.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "0069d1e9-45b0-4fd3-a28f-1f7508503a91"},
				{ "type" , JShaderTypeToStr.at(JShaderType::VERTEX_SHADER)}
			},
			{
				{ "name","ShadowMap_ps"},
				{ "path" , "ShadowMap.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "ed41913d-1a28-40ce-9c92-07549714f367"},
				{ "type" , JShaderTypeToStr.at(JShaderType::PIXEL_SHADER)}
			},
			{
				{ "name","DepthMinMax_vs"},
				{ "path" , "DepthMinMax.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "2ad43d9e-8dec-421c-b8f2-bda3520748bd"},
				{ "type" , JShaderTypeToStr.at(JShaderType::VERTEX_SHADER)}
			},
			{
				{ "name","DepthMinMax_ps"},
				{ "path" , "DepthMinMax.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "dd93a59f-a87e-4d9a-a57c-b91066e7520e"},
				{ "type" , JShaderTypeToStr.at(JShaderType::PIXEL_SHADER)}
			},
			{
				{ "name","DepthMinMaxToRGBA_vs" },
				{ "path" , "DepthMinMaxToRGBA.hlsl" },
				{ "systemCreated" , true },
				{ "uuid" , "9815152b-84ad-45e5-8b91-0642cfde0543" },
				{ "type" , JShaderTypeToStr.at(JShaderType::VERTEX_SHADER) }
			},
			{
				{ "name","DepthMinMaxToRGBA_ps" },
				{ "path" , "DepthMinMaxToRGBA.hlsl" },
				{ "systemCreated" , true },
				{ "uuid" , "22c13e3e-5a88-4868-a5cf-bcc65864cf6c" },
				{ "type" , JShaderTypeToStr.at(JShaderType::PIXEL_SHADER) }
			},
			{
				{ "name","DepthMinMaxToRGBASpot_vs" },
				{ "path" , "DepthMinMaxToRGBASpot.hlsl" },
				{ "systemCreated" , true },
				{ "uuid" , "173a942d-83e2-4d51-83cd-59016cb5be4e" },
				{ "type" , JShaderTypeToStr.at(JShaderType::VERTEX_SHADER) }
			},
			{
				{ "name","DepthMinMaxToRGBASpot_ps" },
				{ "path" , "DepthMinMaxToRGBASpot.hlsl" },
				{ "systemCreated" , true },
				{ "uuid" , "438f86fd-9ef3-433f-ad7b-c1e60643cd3e" },
				{ "type" , JShaderTypeToStr.at(JShaderType::PIXEL_SHADER) }
			},
			{
				{ "name","FullScreenQuad_vs" },
				{ "path" , "FullScreenQuad.hlsl" },
				{ "systemCreated" , true },
				{ "uuid" , "8e26fbd4-3a2c-4c04-a628-d2f11d474d60" },
				{ "type" , JShaderTypeToStr.at(JShaderType::VERTEX_SHADER) }
			},
			{
				{ "name","FullScreenQuad_ps"},
				{ "path" , "FullScreenQuad.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "9ab3d65f-be9a-49cc-87f8-bcbf1dafeac7"},
				{ "type" , JShaderTypeToStr.at(JShaderType::PIXEL_SHADER)}
			},
			{
				{ "name","FullScreenUIQuad_vs" },
				{ "path" , "FullScreenUIQuad.hlsl" },
				{ "systemCreated" , true },
				{ "uuid" , "a44d0097-6e84-433a-82da-0969b8bf31ba" },
				{ "type" , JShaderTypeToStr.at(JShaderType::VERTEX_SHADER) }
			},
			{
				{ "name","FullScreenUIQuad_ps"},
				{ "path" , "FullScreenUIQuad.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "658b3241-1c63-4480-8cfe-28bf34b317f6"},
				{ "type" , JShaderTypeToStr.at(JShaderType::PIXEL_SHADER)}
			},
			{
				{ "name","ToneMap_vs"},
				{ "path" , "ToneMap.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "8ee7a4d0-91f1-4264-aa56-9f82b3c38397"},
				{ "type" , JShaderTypeToStr.at(JShaderType::VERTEX_SHADER)}
			},
			{
				{ "name","ToneMap_ps"},
				{ "path" , "ToneMap.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "75e834c4-6898-4156-af67-43abba7fc6b5"},
				{ "type" , JShaderTypeToStr.at(JShaderType::PIXEL_SHADER)}
			},
			{
				{ "name","LoadingBar_vs"},
				{ "path" , "LoadingBar.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "d0192f97-a56a-469d-b6f1-07d403ae331a"},
				{ "type" , JShaderTypeToStr.at(JShaderType::VERTEX_SHADER)}
			},
			{
				{ "name","LoadingBar_ps"},
				{ "path" , "LoadingBar.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "b5ef5d53-2174-4d12-b231-5e07a7f5a7f8"},
				{ "type" , JShaderTypeToStr.at(JShaderType::PIXEL_SHADER)}
			},
			{
				{ "name","Picking_vs"},
				{ "path" , "Picking.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "79568541-34c8-4464-bec1-77debde975e0"},
				{ "type" , JShaderTypeToStr.at(JShaderType::VERTEX_SHADER)}
			},
			{
				{ "name","Picking_ps"},
				{ "path" , "Picking.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "e32c5e9c-26a5-4f2b-8d0c-5899c67f1def"},
				{ "type" , JShaderTypeToStr.at(JShaderType::PIXEL_SHADER)}
			},
			{
				{ "name","Translucent_vs"},
				{ "path" , "Translucent.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "5231b6af-fc5a-4d77-ba71-5dc40cfd0da5"},
				{ "type" , JShaderTypeToStr.at(JShaderType::VERTEX_SHADER)}
			},
			{
				{ "name","Translucent_ps"},
				{ "path" , "Translucent.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "bc666a1e-97b4-4b01-979f-af4857e0d4b7"},
				{ "type" , JShaderTypeToStr.at(JShaderType::PIXEL_SHADER)}
			}
		}
	);

	nlohmann::json systemSounds = nlohmann::json::array({});

	nlohmann::json systemMaterials = nlohmann::json::array(
		{
			{
				{ "name","BoundingBox"},
				{ "shader_vs" , "ae7a35a5-f012-4eb6-bbe1-1f52e6203ccb"},
				{ "shader_ps" , "1bf837a7-1282-4fae-a1ba-9e74e6a99b37"},
				{ "systemCreated" , true},
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "uuid" , "2e4d8bf0-0761-45d9-8313-17cdf9b5f8fc"}
			},
			{
				{ "name","BaseLighting"},
				{ "shader_vs" , "bc331f48-6a40-4b48-b435-8276051d6993"},
				{ "shader_ps" , "719c0122-1e9f-46e3-90aa-8f1e5e81c098"},
				{ "systemCreated" , true},
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "samplers" ,{
					{
						{ "Filter","MIN_MAG_MIP_LINEAR" },
						{ "AddressU" , "ADDRESS_MODE_BORDER" },
						{ "AddressV" , "ADDRESS_MODE_BORDER" },
						{ "AddressW" , "ADDRESS_MODE_BORDER" },
						{ "MipLODBias" , 0 },
						{ "MaxAnisotropy" , 0 },
						{ "ComparisonFunc" , "NEVER" },
						{ "BorderColor" , "OPAQUE_WHITE" },
						{ "MinLOD" , 0.0 },
						{ "MaxLOD" , 3.4028234663852886e+38 },
						{ "ShaderRegister" , 0 },
						{ "RegisterSpace" , 0 },
						{ "ShaderVisibility" , "PIXEL" }
					}
				}},
				{ "uuid","4a5a2cb8-f2ea-4e15-8584-22bb675ae1bc" }
			},
			{
				{ "name","Floor" },
				{ "shader_vs" , "5af4ba59-a09c-41ef-bc1f-13a51fc68439" },
				{ "shader_ps" , "5929c8f6-e9b7-4680-8447-a430b5accdbf" },
				{ "systemCreated" , true },
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "samplers" ,{
					{
						{ "Filter","MIN_MAG_MIP_LINEAR"},
						{ "AddressU" , "ADDRESS_MODE_BORDER"},
						{ "AddressV" , "ADDRESS_MODE_BORDER"},
						{ "AddressW" , "ADDRESS_MODE_BORDER"},
						{ "MipLODBias" , 0},
						{ "MaxAnisotropy" , 0},
						{ "ComparisonFunc" , "NEVER"},
						{ "BorderColor" , "OPAQUE_WHITE"},
						{ "MinLOD" , 0.0},
						{ "MaxLOD" , 3.4028234663852886e+38},
						{ "ShaderRegister" , 0},
						{ "RegisterSpace" , 0},
						{ "ShaderVisibility" , "PIXEL"}
					}
				}
				},
				{ "uuid","ecd1688c-73d6-49d0-870f-ca916a417c49" }
			},
			{
				{ "name","ShadowMap" },
				{ "shader_vs" , "0069d1e9-45b0-4fd3-a28f-1f7508503a91" },
				{ "shader_ps" , "ed41913d-1a28-40ce-9c92-07549714f367" },
				{ "systemCreated" , true },
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "samplers" , {
					{
						{ "Filter","MIN_MAG_MIP_LINEAR"},
						{ "AddressU" , "ADDRESS_MODE_BORDER"},
						{ "AddressV" , "ADDRESS_MODE_BORDER"},
						{ "AddressW" , "ADDRESS_MODE_BORDER"},
						{ "MipLODBias" , 0},
						{ "MaxAnisotropy" , 0},
						{ "ComparisonFunc" , "NEVER"},
						{ "BorderColor" , "OPAQUE_WHITE"},
						{ "MinLOD" , 0.0},
						{ "MaxLOD" , 3.4028234663852886e+38},
						{ "ShaderRegister" , 0},
						{ "RegisterSpace" , 0},
						{ "ShaderVisibility" , "PIXEL"}
					}
				}
				},
				{ "uuid","3be1cf4e-cc15-41ae-97e1-6bb3e110271f" }
			},
			{
				{ "name","DepthMinMax"},
				{ "shader_vs" , "2ad43d9e-8dec-421c-b8f2-bda3520748bd"},
				{ "shader_ps" , "dd93a59f-a87e-4d9a-a57c-b91066e7520e"},
				{ "systemCreated" , true},
				{ "twoSided" , true},
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "samplers" ,{
					{
						{ "Filter","MIN_MAG_MIP_POINT"},
						{ "AddressU" , "ADDRESS_MODE_BORDER"},
						{ "AddressV" , "ADDRESS_MODE_BORDER"},
						{ "AddressW" , "ADDRESS_MODE_BORDER"},
						{ "MipLODBias" , 0},
						{ "MaxAnisotropy" , 0},
						{ "ComparisonFunc" , "NEVER"},
						{ "BorderColor" , "OPAQUE_WHITE"},
						{ "MinLOD" , 0.0},
						{ "MaxLOD" , 3.4028234663852886e+38},
						{ "ShaderRegister" , 0},
						{ "RegisterSpace" , 0},
						{ "ShaderVisibility" , "PIXEL"}
					}
				}
				},
				{"uuid","35da9e7d-1ef8-4165-8e71-36d6cf599c3c" }
			},
			{
				{ "name","DepthMinMaxToRGBA"},
				{ "shader_vs" , "9815152b-84ad-45e5-8b91-0642cfde0543"},
				{ "shader_ps" , "22c13e3e-5a88-4868-a5cf-bcc65864cf6c"},
				{ "systemCreated" , true},
				{ "twoSided" , true},
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "samplers" , {
					{
						{ "Filter","MIN_MAG_MIP_POINT" },
						{ "AddressU" , "ADDRESS_MODE_BORDER" },
						{ "AddressV" , "ADDRESS_MODE_BORDER" },
						{ "AddressW" , "ADDRESS_MODE_BORDER" },
						{ "MipLODBias" , 0 },
						{ "MaxAnisotropy" , 0 },
						{ "ComparisonFunc" , "NEVER" },
						{ "BorderColor" , "OPAQUE_WHITE" },
						{ "MinLOD" , 0.0 },
						{ "MaxLOD" , 3.4028234663852886e+38 },
						{ "ShaderRegister" , 0 },
						{ "RegisterSpace" , 0 },
						{ "ShaderVisibility" , "PIXEL" }
					}
				}
				},
				{ "uuid" , "84f0cabb-9b0c-4508-ac6e-d7a84dee696f" }
			},
			{
				{ "name","DepthMinMaxToRGBASpot" },
				{ "shader_vs" , "173a942d-83e2-4d51-83cd-59016cb5be4e" },
				{ "shader_ps" , "438f86fd-9ef3-433f-ad7b-c1e60643cd3e" },
				{ "systemCreated" , true },
				{ "twoSided" , true },
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "samplers" , {
					{
						{ "Filter","MIN_MAG_MIP_POINT" },
						{ "AddressU" , "ADDRESS_MODE_BORDER" },
						{ "AddressV" , "ADDRESS_MODE_BORDER" },
						{ "AddressW" , "ADDRESS_MODE_BORDER" },
						{ "MipLODBias" , 0 },
						{ "MaxAnisotropy" , 0 },
						{ "ComparisonFunc" , "NEVER" },
						{ "BorderColor" , "OPAQUE_WHITE" },
						{ "MinLOD" , 0.0 },
						{ "MaxLOD" , 3.4028234663852886e+38 },
						{ "ShaderRegister" , 0 },
						{ "RegisterSpace" , 0 },
						{ "ShaderVisibility" , "PIXEL" }
					}
				}
				},
				{ "uuid" , "908332fb-48b2-42ee-b678-e57fb3ad352e" }
			},
			{
				{ "name","FullScreenQuad"},
				{ "shader_vs" , "8e26fbd4-3a2c-4c04-a628-d2f11d474d60"},
				{ "shader_ps" , "9ab3d65f-be9a-49cc-87f8-bcbf1dafeac7"},
				{ "systemCreated" , true},
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "uuid" , "8e98708c-fe2e-4123-b1f0-5b80fabd1888"}
			},
			{
				{ "name","FullScreenUIQuad"},
				{ "shader_vs" , "a44d0097-6e84-433a-82da-0969b8bf31ba"},
				{ "shader_ps" , "658b3241-1c63-4480-8cfe-28bf34b317f6"},
				{ "systemCreated" , true},
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "blendState",
					{
						{"AlphaToCoverageEnable", 0 },
						{"IndependentBlendEnable", 0 },
						{ "RenderTarget" ,
							{
								{
									{ "BlendEnable", true },
									{ "BlendOp", "ADD" },
									{ "BlendOpAlpha", "ADD" },
									{ "DestBlend", "INV_SRC_ALPHA" },
									{ "DestBlendAlpha", "INV_SRC_ALPHA" },
									{ "LogicOp", "NOOP" },
									{ "LogicOpEnable", false },
									{ "RenderTargetWriteMask", 15 },
									{ "SrcBlend", "SRC_ALPHA" },
									{ "SrcBlendAlpha", "ONE" }
								},
								{
									{ "BlendEnable", 0},
									{ "BlendOp", "ADD"},
									{ "BlendOpAlpha", "ADD"},
									{ "DestBlend", "ZERO"},
									{ "DestBlendAlpha", "ZERO"},
									{ "LogicOp", "NOOP"},
									{ "LogicOpEnable", 0},
									{ "RenderTargetWriteMask", 15},
									{ "SrcBlend", "ONE"},
									{ "SrcBlendAlpha", "ONE" }
								},
								{
									{ "BlendEnable", 0},
									{ "BlendOp" , "ADD"},
									{ "BlendOpAlpha" , "ADD"},
									{ "DestBlend" , "ZERO"},
									{ "DestBlendAlpha" , "ZERO"},
									{ "LogicOp" , "NOOP"},
									{ "LogicOpEnable" , 0},
									{ "RenderTargetWriteMask" , 15},
									{ "SrcBlend" , "ONE"},
									{ "SrcBlendAlpha" , "ONE" }
								},
								{
									{ "BlendEnable", 0},
									{ "BlendOp" , "ADD"},
									{ "BlendOpAlpha" , "ADD"},
									{ "DestBlend" , "ZERO"},
									{ "DestBlendAlpha" , "ZERO"},
									{ "LogicOp" , "NOOP"},
									{ "LogicOpEnable" , 0},
									{ "RenderTargetWriteMask" , 15},
									{ "SrcBlend" , "ONE"},
									{ "SrcBlendAlpha" , "ONE"}
								},
								{
									{ "BlendEnable", 0},
									{ "BlendOp" , "ADD"},
									{ "BlendOpAlpha" , "ADD"},
									{ "DestBlend" , "ZERO"},
									{ "DestBlendAlpha" , "ZERO"},
									{ "LogicOp" , "NOOP"},
									{ "LogicOpEnable" , 0},
									{ "RenderTargetWriteMask" , 15},
									{ "SrcBlend" , "ONE"},
									{ "SrcBlendAlpha" , "ONE"}
								},
								{
									{ "BlendEnable", 0},
									{ "BlendOp" , "ADD"},
									{ "BlendOpAlpha" , "ADD"},
									{ "DestBlend" , "ZERO"},
									{ "DestBlendAlpha" , "ZERO"},
									{ "LogicOp" , "NOOP"},
									{ "LogicOpEnable" , 0},
									{ "RenderTargetWriteMask" , 15},
									{ "SrcBlend" , "ONE"},
									{ "SrcBlendAlpha" , "ONE" }
								},
								{
									{ "BlendEnable", 0},
									{ "BlendOp" , "ADD"},
									{ "BlendOpAlpha" , "ADD"},
									{ "DestBlend" , "ZERO"},
									{ "DestBlendAlpha" , "ZERO"},
									{ "LogicOp" , "NOOP"},
									{ "LogicOpEnable" , 0},
									{ "RenderTargetWriteMask" , 15},
									{ "SrcBlend" , "ONE"},
									{ "SrcBlendAlpha" , "ONE" }
								},
								{
									{ "BlendEnable", 0},
									{ "BlendOp" , "ADD"},
									{ "BlendOpAlpha" , "ADD"},
									{ "DestBlend" , "ZERO"},
									{ "DestBlendAlpha" , "ZERO"},
									{ "LogicOp" , "NOOP"},
									{ "LogicOpEnable" , 0},
									{ "RenderTargetWriteMask" , 15},
									{ "SrcBlend" , "ONE"},
									{ "SrcBlendAlpha" , "ONE" }
								}
							}
						}
					}
				},
				{ "uuid" , "94932d78-6316-4a90-8597-2d1a87fdc376"}
			},
			{
				{ "name","ToneMap"},
				{ "shader_vs" , "8ee7a4d0-91f1-4264-aa56-9f82b3c38397"},
				{ "shader_ps" , "75e834c4-6898-4156-af67-43abba7fc6b5"},
				{ "systemCreated" , true},
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "uuid" , "8291ba82-165d-464b-be15-d9fa6d7b9a7c"}
			},
			{
				{ "name","Picking"},
				{ "shader_vs" , "79568541-34c8-4464-bec1-77debde975e0"},
				{ "shader_ps" , "e32c5e9c-26a5-4f2b-8d0c-5899c67f1def"},
				{ "systemCreated" , true},
				{ "uuid" , "1896d918-4e47-49a6-950b-3135ab020a0b"},
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
			},
			{
				{ "name", "Camera" },
				{ "shader_ps", "abeeba0f-8f50-4780-92c2-02226cecb5dd" },
				{ "shader_vs", "7d076bac-db2b-4ee3-8e4e-eadf891022fb" },
				{ "systemCreated" , true},
				{ "mappedValues",
					{
						{
							{ "value", 0.9803921580314636 },
							{ "variable", "alphaCut" },
							{ "variableType", "FLOAT" }
						}
					}
				},
				{ "textures",
					{
						{ "BaseTexture", "2c207f54-9cdc-4c7e-a70a-60b373f2de79" }
					}
				},
				{ "uuid", "65d6c9ad-226a-4073-924a-74d0c61acfc6" },
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				}
			},
			{
				{ "name", "LightBulb" },
				{ "shader_ps", "abeeba0f-8f50-4780-92c2-02226cecb5dd" },
				{ "shader_vs", "7d076bac-db2b-4ee3-8e4e-eadf891022fb" },
				{ "systemCreated" , true},
				{ "mappedValues",
					{
						{
							{ "value", 0.9803921580314636},
							{ "variable", "alphaCut"},
							{ "variableType", "FLOAT" }
						}
					}
				},
				{ "textures",
					{
						{ "BaseTexture", "fed123fa-e248-47cd-9662-20f73285ad0e" }
					}
				},
				{ "uuid", "7b774c44-527d-4315-a80c-aacf0a1383a6" },
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				}
			},
			{
				{ "name", "SoundEffect" },
				{ "shader_ps", "abeeba0f-8f50-4780-92c2-02226cecb5dd" },
				{ "shader_vs", "7d076bac-db2b-4ee3-8e4e-eadf891022fb" },
				{ "systemCreated" , true},
				{ "mappedValues",
					{
						{
							{ "value", 0.9803921580314636 },
							{ "variable", "alphaCut" },
							{ "variableType", "FLOAT" }
						}
					}
				},
				{ "textures",
					{
						{ "BaseTexture", "5e3cba75-a495-44d8-ba5b-2b888f812a2b" }
					}
				},
				{ "uuid", "e14a13cf-089e-401c-904b-75ebd75984e0" },
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				}
			},
			{
				{ "name", "CameraPicking" },
				{ "shader_ps", "2dbc1cfd-8bcb-484a-bfc2-ec8be5150e55" },
				{ "shader_vs", "744b10ef-4f0c-46d5-bd20-e94f7b66b8f9" },
				{ "systemCreated" , true},
				{ "mappedValues",
					{
						{
							{ "value", 0.9803921580314636 },
							{ "variable", "alphaCut" },
							{ "variableType", "FLOAT" }
						}
					}
				},
				{ "textures",
					{
						{ "BaseTexture", "2c207f54-9cdc-4c7e-a70a-60b373f2de79" }
					}
				},
				{ "uuid", "e82b4687-4705-4202-8d96-65096426b00e" },
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				}
			},
			{
				{ "name", "LightBulbPicking" },
				{ "shader_ps", "2dbc1cfd-8bcb-484a-bfc2-ec8be5150e55" },
				{ "shader_vs", "744b10ef-4f0c-46d5-bd20-e94f7b66b8f9" },
				{ "systemCreated" , true},
				{ "mappedValues",
					{
						{
							{ "value", 0.9803921580314636 },
							{ "variable", "alphaCut" },
							{ "variableType", "FLOAT" }
						}
					}
				},
				{ "textures",
					{
						{ "BaseTexture", "fed123fa-e248-47cd-9662-20f73285ad0e" }
					}
				},
				{ "uuid", "3786f66e-550a-449d-8526-2507ebec6750" },
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				}
			},
			{
				{ "name", "SoundEffectPicking" },
				{ "shader_ps", "2dbc1cfd-8bcb-484a-bfc2-ec8be5150e55" },
				{ "shader_vs", "744b10ef-4f0c-46d5-bd20-e94f7b66b8f9" },
				{ "systemCreated" , true},
				{ "mappedValues",
					{
						{
							{ "value", 0.9803921580314636 },
							{ "variable", "alphaCut" },
							{ "variableType", "FLOAT" }
						}
					}
				},
				{ "textures",
					{
						{ "BaseTexture", "5e3cba75-a495-44d8-ba5b-2b888f812a2b" }
					}
				},
				{ "uuid", "44b7750d-534b-4df6-bc43-776054503b4c" },
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				}
			},
			{
				{ "name", "TriggerPicking" },
				{ "shader_ps", "e32c5e9c-26a5-4f2b-8d0c-5899c67f1def" },
				{ "shader_vs", "79568541-34c8-4464-bec1-77debde975e0" },
				{ "systemCreated" , true},
				{ "mappedValues",
					{
						{
							{ "value", 1.0 },
							{ "variable", "alphaCut" },
							{ "variableType", "FLOAT" }
						}
					}
				},
				{ "textures", {} },
				{ "uuid", "5d14b58e-44b4-4d71-9d28-c758e055ecf3" },
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "overrideDepthStencil", true}
			},
			{
				{ "name", "Translucent" },
				{ "shader_ps", "bc666a1e-97b4-4b01-979f-af4857e0d4b7" },
				{ "shader_vs", "5231b6af-fc5a-4d77-ba71-5dc40cfd0da5" },
				{ "systemCreated" , true},
				{ "mappedValues",
					{
						{
							{ "value", 0.5 },
							{ "variable", "alpha" },
							{ "variableType", "FLOAT" }
						},
						{
							{ "value", { 0.2, 1.0, 0.78 } },
							{ "variable", "baseColor" },
							{ "variableType", "FLOAT3" }
						}
					}
				},
				{ "textures", { } },
				{ "uuid", "e241b072-3aea-4c22-afee-b3887732ea89" },
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "BACK" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", false },
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "blendState",
					{
						{"AlphaToCoverageEnable", 0 },
						{"IndependentBlendEnable", 0 },
						{ "RenderTarget" ,
							{
								{
									{ "BlendEnable", true },
									{ "BlendOp", "ADD" },
									{ "BlendOpAlpha", "ADD" },
									{ "DestBlend", "INV_SRC_ALPHA" },
									{ "DestBlendAlpha", "ZERO" },
									{ "LogicOp", "NOOP" },
									{ "LogicOpEnable", false },
									{ "RenderTargetWriteMask", 15 },
									{ "SrcBlend", "SRC_ALPHA" },
									{ "SrcBlendAlpha", "ONE" }
								},
								{
									{ "BlendEnable", 0},
									{ "BlendOp", "ADD"},
									{ "BlendOpAlpha", "ADD"},
									{ "DestBlend", "ZERO"},
									{ "DestBlendAlpha", "ZERO"},
									{ "LogicOp", "NOOP"},
									{ "LogicOpEnable", 0},
									{ "RenderTargetWriteMask", 15},
									{ "SrcBlend", "ONE"},
									{ "SrcBlendAlpha", "ONE" }
								},
								{
									{ "BlendEnable", 0},
									{ "BlendOp" , "ADD"},
									{ "BlendOpAlpha" , "ADD"},
									{ "DestBlend" , "ZERO"},
									{ "DestBlendAlpha" , "ZERO"},
									{ "LogicOp" , "NOOP"},
									{ "LogicOpEnable" , 0},
									{ "RenderTargetWriteMask" , 15},
									{ "SrcBlend" , "ONE"},
									{ "SrcBlendAlpha" , "ONE" }
								},
								{
									{ "BlendEnable", 0},
									{ "BlendOp" , "ADD"},
									{ "BlendOpAlpha" , "ADD"},
									{ "DestBlend" , "ZERO"},
									{ "DestBlendAlpha" , "ZERO"},
									{ "LogicOp" , "NOOP"},
									{ "LogicOpEnable" , 0},
									{ "RenderTargetWriteMask" , 15},
									{ "SrcBlend" , "ONE"},
									{ "SrcBlendAlpha" , "ONE"}
								},
								{
									{ "BlendEnable", 0},
									{ "BlendOp" , "ADD"},
									{ "BlendOpAlpha" , "ADD"},
									{ "DestBlend" , "ZERO"},
									{ "DestBlendAlpha" , "ZERO"},
									{ "LogicOp" , "NOOP"},
									{ "LogicOpEnable" , 0},
									{ "RenderTargetWriteMask" , 15},
									{ "SrcBlend" , "ONE"},
									{ "SrcBlendAlpha" , "ONE"}
								},
								{
									{ "BlendEnable", 0},
									{ "BlendOp" , "ADD"},
									{ "BlendOpAlpha" , "ADD"},
									{ "DestBlend" , "ZERO"},
									{ "DestBlendAlpha" , "ZERO"},
									{ "LogicOp" , "NOOP"},
									{ "LogicOpEnable" , 0},
									{ "RenderTargetWriteMask" , 15},
									{ "SrcBlend" , "ONE"},
									{ "SrcBlendAlpha" , "ONE" }
								},
								{
									{ "BlendEnable", 0},
									{ "BlendOp" , "ADD"},
									{ "BlendOpAlpha" , "ADD"},
									{ "DestBlend" , "ZERO"},
									{ "DestBlendAlpha" , "ZERO"},
									{ "LogicOp" , "NOOP"},
									{ "LogicOpEnable" , 0},
									{ "RenderTargetWriteMask" , 15},
									{ "SrcBlend" , "ONE"},
									{ "SrcBlendAlpha" , "ONE" }
								},
								{
									{ "BlendEnable", 0},
									{ "BlendOp" , "ADD"},
									{ "BlendOpAlpha" , "ADD"},
									{ "DestBlend" , "ZERO"},
									{ "DestBlendAlpha" , "ZERO"},
									{ "LogicOp" , "NOOP"},
									{ "LogicOpEnable" , 0},
									{ "RenderTargetWriteMask" , 15},
									{ "SrcBlend" , "ONE"},
									{ "SrcBlendAlpha" , "ONE" }
								}
							}
						}
					}
				},
				{ "overrideDepthStencil", true },
				{ "depthStencil",
					{
						{ "BackFace",
							{
								{ "StencilDepthFailOp", "KEEP"},
								{ "StencilFailOp", "KEEP"},
								{ "StencilFunc", "ALWAYS"},
								{ "StencilPassOp", "KEEP" }
							}
						},
						{ "DepthEnable", true },
						{ "DepthFunc", "LESS_EQUAL" },
						{ "DepthWriteMask", "ZERO" },
						{ "FrontFace",
							{
								{ "StencilDepthFailOp", "KEEP"},
								{ "StencilFailOp", "KEEP"},
								{ "StencilFunc", "ALWAYS"},
								{ "StencilPassOp", "KEEP" }
							}
						},
						{ "StencilEnable", false},
						{ "StencilReadMask", 255},
						{ "StencilWriteMask", 255 }
					}
				}
			},
			{
				{ "name", "Translucent_wired" },
				{ "shader_ps", "bc666a1e-97b4-4b01-979f-af4857e0d4b7" },
				{ "shader_vs", "5231b6af-fc5a-4d77-ba71-5dc40cfd0da5" },
				{ "systemCreated" , true},
				{ "mappedValues",
					{
						{
							{ "value", 1.0 },
							{ "variable", "alpha" },
							{ "variableType", "FLOAT" }
						},
						{
							{ "value", { 0.2 * 1.3, 1.3, 0.78 * 1.3 } },
							{ "variable", "baseColor" },
							{ "variableType", "FLOAT3" }
						}
					}
				},
				{ "textures", { } },
				{ "uuid", "1d7630c4-86b0-49eb-88f5-40bacb02a652" },
				{ "rasterizerState",
					{
						{ "FillMode", "WIREFRAME" },
						{ "CullMode", "BACK" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", false },
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "overrideDepthStencil", true },
				{ "depthStencil",
					{
						{ "BackFace",
							{
								{ "StencilDepthFailOp", "KEEP"},
								{ "StencilFailOp", "KEEP"},
								{ "StencilFunc", "ALWAYS"},
								{ "StencilPassOp", "KEEP" }
							}
						},
						{ "DepthEnable", true },
						{ "DepthFunc", "LESS" },
						{ "DepthWriteMask", "ALL" },
						{ "FrontFace",
							{
								{ "StencilDepthFailOp", "KEEP"},
								{ "StencilFailOp", "KEEP"},
								{ "StencilFunc", "ALWAYS"},
								{ "StencilPassOp", "KEEP" }
							}
						},
						{ "StencilEnable", false},
						{ "StencilReadMask", 255},
						{ "StencilWriteMask", 255 }
					}
				}
			},
			{
				{ "name", "TranslucentPicking" },
				{ "shader_ps", "e32c5e9c-26a5-4f2b-8d0c-5899c67f1def" },
				{ "shader_vs", "79568541-34c8-4464-bec1-77debde975e0" },
				{ "systemCreated" , true},
				{ "mappedValues",
					{
						{
							{ "value", 1.0 },
							{ "variable", "alphaCut" },
							{ "variableType", "FLOAT" }
						}
					}
				},
				{ "textures", {} },
				{ "uuid", "c6396dfa-16ba-4c07-adfc-daa9c3866eb3" },
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "overrideDepthStencil", true}
			}
		}
	);

	nlohmann::json systemRenderPasses = nlohmann::json::array(
		{
			{
				{ "name", "mainPass" },
				{ "uuid", "b4ee2cf4-0231-4ff1-ada8-ae745cf0709e" },
				{ "type", "RenderToTexturePass" },
				{ "fitWindow", true },
				{ "renderTargetFormats", { "R32G32B32A32_FLOAT" }},
				{ "depthStencilFormat", "D32_FLOAT" },
				{ "materialOverride", "None" },
				{ "renderCallbackOverride", "None" },
				{ "systemCreated" , true}
			},
			{
				{ "name", "toneMappingPass" },
				{ "uuid", "722d8147-6483-4675-94a3-1e8af09ec5e1" },
				{ "type", "RenderToTexturePass" },
				{ "fitWindow", true },
				{ "renderTargetFormats", { "R8G8B8A8_UNORM" }},
				{ "depthStencilFormat", "UNKNOWN" },
				{ "materialOverride", "None" },
				{ "renderCallbackOverride", "ToneMapping" },
				{ "systemCreated" , true}
			},
			{
				{ "name", "resolvePass" },
				{ "uuid", "53693830-779c-4ed0-a985-402d6a72485b" },
				{ "type", "SwapChainPass" },
				{ "fitWindow", true },
				{ "renderTargetFormats", { "R8G8B8A8_UNORM" }},
				{ "depthStencilFormat", "UNKNOWN" },
				{ "materialOverride", "None" },
				{ "renderCallbackOverride", "Resolve" },
				{ "systemCreated" , true}
			},
			{
				{ "name", "resolveUIPass" },
				{ "uuid", "90fc8b65-23ff-4823-880d-50868186100a" },
				{ "type", "SwapChainPass" },
				{ "fitWindow", true },
				{ "renderTargetFormats", { "R8G8B8A8_UNORM" }},
				{ "depthStencilFormat", "UNKNOWN" },
				{ "materialOverride", "None" },
				{ "renderCallbackOverride", "ResolveUI" },
				{ "systemCreated" , true},
				{ "usePrevPassTexture", false }
			},
			{
				{ "name", "simplePass" },
				{ "uuid", "c483d4c9-94ce-48d6-8116-ea838e69119b" },
				{ "type", "SwapChainPass" },
				{ "fitWindow", true },
				{ "renderTargetFormats", { "R8G8B8A8_UNORM" }},
				{ "depthStencilFormat", "D32_FLOAT" },
				{ "materialOverride", "None" },
				{ "renderCallbackOverride", "None" },
				{ "systemCreated" , true}
			},
			{
				{ "name", "ShadowMap" },
				{ "uuid", "241cbc97-c047-4334-9393-ae5d33268220" },
				{ "type", "RenderToTexturePass" },
				{ "fitWindow", false },
				{ "renderTargetFormats", { }},
				{ "depthStencilFormat", "D32_FLOAT" },
				{ "materialOverride", "ShadowMap" },
				{ "renderCallbackOverride", "None" },
				{ "systemCreated" , true}
			},
			{
				{ "name", "PickingPass" },
				{ "uuid", "d607f54c-11cf-461e-9a3e-0a74a84feb2f" },
				{ "type", "RenderToTexturePass" },
				{ "fitWindow", false },
				{ "renderTargetFormats", { "R32_UINT" }},
				{ "depthStencilFormat", "D32_FLOAT" },
				{ "materialOverride", "Picking" },
				{ "renderCallbackOverride", "None" },
				{ "systemCreated" , true}
			},
			{
				{ "name", "ShadowMapMinMaxChainPass" },
				{ "uuid", "fcd248e2-55b3-42f8-ab98-8b65d6fec86e" },
				{ "type", "RenderToTexturePass" },
				{ "fitWindow", false },
				{ "renderTargetFormats", { "R32_FLOAT", "R32_FLOAT" }},
				{ "depthStencilFormat", "UNKNOWN" },
				{ "materialOverride", "None" },
				{ "renderCallbackOverride", "MinMaxChain" },
				{ "systemCreated" , true}
			},
			{
				{ "name", "ShadowMapMinMaxChainResultPass" },
				{ "uuid", "6b1bc75a-956f-4673-b35a-a8bc820f5153" },
				{ "type", "RenderToTexturePass" },
				{ "fitWindow", false },
				{ "renderTargetFormats", { "R8G8B8A8_UNORM" }},
				{ "depthStencilFormat", "UNKNOWN" },
				{ "materialOverride", "None" },
				{ "renderCallbackOverride", "MinMaxChainResult" },
				{ "systemCreated" , true},
			},
			{
				{ "depthStencilFormat", "D32_FLOAT" },
				{ "fitWindow", false },
				{ "materialOverride", "None" },
				{ "name", "ModelPreviewPass" },
				{ "renderCallbackOverride", "None" },
				{ "renderTargetFormats", { "R8G8B8A8_UNORM" }} ,
				{ "type", "RenderToTexturePass" },
				{ "uuid", "7b06958d-2897-4110-9095-c9b541070eaa" },
				{ "systemCreated" , true },
			},
			{
				{ "depthStencilFormat", "UNKNOWN" },
				{ "fitWindow", true },
				{ "materialOverride", "None" },
				{ "name", "simpleUI" },
				{ "renderCallbackOverride", "None" },
				{ "renderTargetFormats", { "R8G8B8A8_UNORM" }} ,
				{ "type", "RenderToTexturePass" },
				{ "uuid", "d5a1867a-a480-48a0-b1aa-606cee1e087d" },
				{ "systemCreated" , true },
			}
		}
	);

	nlohmann::json systemTextures = nlohmann::json::array(
		{
			{
				{ "format", "B8G8R8A8_UNORM_SRGB" },
				{ "height", 256 },
				{ "images" , {
					"Assets/gizmos/light-bulb.png"
				}},
				{ "mipLevels", 8 },
				{ "name", "Assets/gizmos/light-bulb.png" },
				{ "numFrames", 1 },
				{ "type", "2D" },
				{ "uuid", "fed123fa-e248-47cd-9662-20f73285ad0e" },
				{ "width", 256 },
				{ "systemCreated" , true}
			},
			{
				{ "format", "B8G8R8A8_UNORM_SRGB" },
				{ "height", 64 },
				{ "images" , {
					"Assets/gizmos/soundspeaker.png"
				}},
				{ "mipLevels", 6 },
				{ "name", "Assets/gizmos/soundspeaker.png" },
				{ "numFrames", 1 },
				{ "type", "2D" },
				{ "uuid", "5e3cba75-a495-44d8-ba5b-2b888f812a2b" },
				{ "width", 64 },
				{ "systemCreated" , true}
			},
			{
				{ "format", "B8G8R8A8_UNORM_SRGB" },
				{ "height", 64 },
				{ "images" , {
					"Assets/gizmos/camera.png"
				}},
				{ "mipLevels", 6 },
				{ "name", "Assets/gizmos/camera.png" },
				{ "numFrames", 1 },
				{ "type", "2D" },
				{ "uuid", "2c207f54-9cdc-4c7e-a70a-60b373f2de79" },
				{ "width", 64 },
				{ "systemCreated" , true}
			}
		}
	);

	nlohmann::json systemPhysicGeometries = nlohmann::json::array(
		{
			{
				{ "name", "floor" },
				{ "uuid", "330c6bfd-2c71-4c6b-be42-b797d07ab5ba" },
				{ "model", ""},
				{ "mesh", "d41e5c29-49bb-4f2c-aa2b-da781fbac512"},
				{ "systemCreated" , true}
			},
			{
				{ "name", "cube" },
				{ "uuid", "a5d06057-5344-413c-9647-ed2aac874021" },
				{ "model", ""},
				{ "mesh", "f7786ac1-e296-4e9a-a7e6-6f1949de75ef"},
				{ "systemCreated" , true}
			},
			{
				{ "name", "pyramid" },
				{ "uuid", "36c75d6d-970f-43dc-97bd-9bc13a035a3a" },
				{ "model", ""},
				{ "mesh", "d76b3bd8-0f53-4128-974e-2d6d5062bc00"},
				{ "systemCreated" , true}
			},
			{
				{ "name", "sphere" },
				{ "uuid", "a96cbfaf-0827-4b04-ae68-548c9bffaf87" },
				{ "model", ""},
				{ "mesh", "4d1174b2-8225-4c09-9db6-ff09718ae0f5"},
				{ "systemCreated" , true  }
			},
			{
				{ "name", "cone" },
				{ "uuid", "8a41c342-e388-4e1f-8ba9-8b0a41b1c975" },
				{ "model", ""},
				{ "mesh", "ad73990a-c59d-45d2-8ec3-807b1f52f5b9"},
				{ "systemCreated" , true  }
			},
			{
				{ "name", "capsule" },
				{ "uuid", "95eee3bb-5b2f-41d7-b43a-11ab5524cca1" },
				{ "model", ""},
				{ "mesh", "c900056b-9f67-47d1-a252-71e0ef1f9a65"},
				{ "systemCreated" , true  }
			},
		}
	);

	std::unordered_map<TemplateType, std::set<JUUID>> templates;
	std::unordered_map<JUUID, TemplateType> templatesTypes;
	std::set<JUUID>& GetTemplates(TemplateType type)
	{
		if (!templates.contains(type))
			templates[type].clear();
		return templates.at(type);
	}
	std::unordered_map<JUUID, TemplateType>& GetTemplatesTypes()
	{
		return templatesTypes;
	}
	TemplateType GetTemplateType(JUUID uuid)
	{
		return templatesTypes.at(uuid);
	}
	bool TemplateExists(JUUID uuid)
	{
		return templatesTypes.contains(uuid);
	}

	void CreateSystemTemplates()
	{
		LoadTemplates(systemShaders, CreateShader);
		LoadTemplates(systemSounds, CreateSound);
		LoadTemplates(systemMaterials, CreateMaterial);
		LoadTemplates(systemRenderPasses, CreateRenderPass);
		LoadTemplates(systemTextures, CreateTexture);
		LoadTemplates(systemPhysicGeometries, CreatePhysicGeometry);

		CreatePrimitiveMeshTemplate("d41e5c29-49bb-4f2c-aa2b-da781fbac512", "floor");
		CreatePrimitiveMeshTemplate("d8bfdef4-55f9-4f6e-b4a8-20915eb854d6", "utahteapot");
		CreatePrimitiveMeshTemplate("f7786ac1-e296-4e9a-a7e6-6f1949de75ef", "cube");
		CreatePrimitiveMeshTemplate("d76b3bd8-0f53-4128-974e-2d6d5062bc00", "pyramid");
		CreatePrimitiveMeshTemplate("7dec1229-075f-4599-95e1-9ccfad0d48b1", "decal");
		CreatePrimitiveMeshTemplate("30f15e68-db42-46fa-b846-b2647a0ac9b9", "boxlines");
		CreatePrimitiveMeshTemplate("4d1174b2-8225-4c09-9db6-ff09718ae0f5", "sphere");
		CreatePrimitiveMeshTemplate("ad73990a-c59d-45d2-8ec3-807b1f52f5b9", "cone");
		CreatePrimitiveMeshTemplate("c900056b-9f67-47d1-a252-71e0ef1f9a65", "capsule");
	}

	void CreateTemplates()
	{
		LoadTemplates(defaultTemplatesFolder, Shader::templateName, CreateShader);
		LoadTemplates(defaultTemplatesFolder, Material::templateName, CreateMaterial);
		LoadTemplates(defaultTemplatesFolder, Model3D::templateName, CreateModel3D);
		LoadTemplates(defaultTemplatesFolder, Sound::templateName, CreateSound);
		LoadTemplates(defaultTemplatesFolder, Texture::templateName, CreateTexture);
		LoadTemplates(defaultTemplatesFolder, RenderPass::templateName, CreateRenderPass);
		LoadTemplates(defaultTemplatesFolder, PhysicGeometry::templateName, CreatePhysicGeometry);
		LoadTemplates(defaultTemplatesFolder, HtmlUI::templateName, CreateHtmlUI);
		LoadTemplates(defaultTemplatesFolder, Mold::templateName, CreateMold);
	}

#if defined(_EDITOR)
	void SaveTemplates(const std::string folder, const std::string fileName, std::function<void(nlohmann::json&)> writer)
	{
		//first create the directory if needed
		std::filesystem::path directory(folder);
		std::filesystem::create_directory(directory);

		//then create the json level file
		const std::string finalFilename = folder + fileName;
		std::filesystem::path path(finalFilename);
		std::string pathStr = path.generic_string();
		std::ofstream file;
		file.open(pathStr);
		nlohmann::json data = nlohmann::json::array();
		writer(data);
		std::string dataString = data.dump(4);
		file.write(dataString.c_str(), dataString.size());
		file.close();
	}
#endif

	void LoadTemplates(nlohmann::json templates, std::function<void(nlohmann::json&)> loader)
	{
		for (unsigned int i = 0; i < templates.size(); i++)
		{
			loader(templates.at(i));
		}
	}

	void LoadTemplates(const std::string folder, const std::string fileName, std::function<void(nlohmann::json&)> loader)
	{
		//first create the directory if needed
		std::filesystem::path directory(folder);
		const std::string finalFilename = folder + fileName;
		std::filesystem::path path(finalFilename);
		if (!std::filesystem::exists(path)) return;
		std::string pathStr = path.generic_string();
		std::ifstream file(pathStr);
		nlohmann::json data = nlohmann::json::parse(file);
		file.close();

		for (unsigned int i = 0; i < data.size(); i++)
		{
			loader(data.at(i));
		}
	}

	void DestroyTemplatesInstances()
	{
		ClearHtmlUIInstances();
		ClearRenderPassInstances();
		ClearTextureInstances();
		ClearMaterialInstances();
		ClearShaderInstances();
		ClearPhysicGeometryInstances();
	}

	void DestroyTemplates()
	{
		ReleaseRenderPassTemplates();
		ReleaseTextureTemplates();
		ReleaseSoundTemplates();
		ReleaseModel3DTemplates();
		ReleaseMaterialTemplates();
		ReleaseShaderTemplates();
		ReleasePhysicGeometryTemplates();
		ReleaseHtmlUITemplates();
		ReleaseMoldTemplates();
	}

	void TemplatesStep(DX::StepTimer& timer)
	{
		ShaderJsonStep();
#if defined(_EDITOR)
		TextureJsonsStep();
		PreviewTexturesStep(timer);
		Model3DJsonStep();
#endif
		MaterialJsonStep();
		RenderPassJsonStep();
		SoundJsonStep();
	}

	JTemplate* GetJTemplatePointer(JUUID uuid)
	{
		std::unordered_map<TemplateType, std::function<JTemplate* (JUUID)>> getP =
		{
			{ T_Shaders, [](JUUID uuid)
				{
					auto& t = GetShaderTemplate(uuid);
					return static_cast<JTemplate*>(t.get());
				}
			},
			{ T_Materials, [](JUUID uuid)
				{
					auto& t = GetMaterialTemplate(uuid);
					return static_cast<JTemplate*>(t.get());
				}
			},
			{ T_Models3D, [](JUUID uuid)
				{
					auto& t = GetModel3DTemplate(uuid);
					return static_cast<JTemplate*>(t.get());
				}
			},
			{ T_Sounds, [](JUUID uuid)
				{
					auto& t = GetSoundTemplate(uuid);
					return static_cast<JTemplate*>(t.get());
				}
			},
			{ T_Textures, [](JUUID uuid)
				{
					auto& t = GetTextureTemplate(uuid);
					return static_cast<JTemplate*>(t.get());
				}
			},
			{ T_RenderPasses, [](JUUID uuid)
				{
					auto& t = GetRenderPassTemplate(uuid);
					return static_cast<JTemplate*>(t.get());
				}
			},
			{ T_PhysicGeometries, [](JUUID uuid)
				{
					auto& t = GetPhysicGeometryTemplate(uuid);
					return static_cast<JTemplate*>(t.get());
				}
			},
			{ T_HtmlUIs, [](JUUID uuid)
				{
					auto& t = GetHtmlUITemplate(uuid);
					return static_cast<JTemplate*>(t.get());
				}
			},
			{ T_Molds, [](JUUID uuid)
				{
					auto& t = GetMoldTemplate(uuid);
					return static_cast<JTemplate*>(t.get());
				}
			},
		};

		TemplateType type = GetTemplateType(uuid);
		return getP.at(type)(uuid);
	}

#if defined(_EDITOR)
	std::vector<JUUIDName> GetTemplatesTypesList()
	{
		auto str2JUUIDName = [](std::string type, std::string uuid, std::string name)
			{
				JUUIDName uuidname;
				std::string& u = std::get<0>(uuidname);
				std::string& n = std::get<1>(uuidname);
				u = uuid;
				n = type + "/" + name;
				return uuidname;
			};

		std::unordered_map<TemplateType, std::function<JUUIDName(JUUID)>> getJUUIDName =
		{
			{ T_Shaders, [str2JUUIDName](JUUID uuid)
				{
					ShaderJsonID o = uuid;
					return str2JUUIDName(TemplateTypeToString.at(T_Shaders), o->uuid(),o->name());
				}
			},
			{ T_Materials, [str2JUUIDName](JUUID uuid)
				{
					MaterialJsonID o = uuid;
					return str2JUUIDName(TemplateTypeToString.at(T_Materials), o->uuid(),o->name());
				}
			},
			{ T_Models3D, [str2JUUIDName](JUUID uuid)
				{
					Model3DJsonID o = uuid;
					return str2JUUIDName(TemplateTypeToString.at(T_Models3D), o->uuid(),o->name());
				}
			},
			{ T_Sounds, [str2JUUIDName](JUUID uuid)
				{
					SoundJsonID o = uuid;
					return str2JUUIDName(TemplateTypeToString.at(T_Sounds), o->uuid(),o->name());
				}
			},
			{ T_Textures, [str2JUUIDName](JUUID uuid)
				{
					TextureJsonID o = uuid;
					return str2JUUIDName(TemplateTypeToString.at(T_Textures), o->uuid(),o->name());
				}
			},
			{ T_RenderPasses, [str2JUUIDName](JUUID uuid)
				{
					RenderPassJsonID o = uuid;
					return str2JUUIDName(TemplateTypeToString.at(T_RenderPasses), o->uuid(),o->name());
				}
			},
			{ T_PhysicGeometries, [str2JUUIDName](JUUID uuid)
				{
					PhysicGeometryJsonID o = uuid;
					return str2JUUIDName(TemplateTypeToString.at(T_PhysicGeometries), o->uuid(),o->name());
				}
			},
			{ T_HtmlUIs, [str2JUUIDName](JUUID uuid)
				{
					HtmlUIJsonID o = uuid;
					return str2JUUIDName(TemplateTypeToString.at(T_HtmlUIs), o->uuid(),o->name());
				}
			},
			{ T_Molds, [str2JUUIDName](JUUID uuid)
				{
					MoldJsonID o = uuid;
					return str2JUUIDName(TemplateTypeToString.at(T_Molds), o->uuid(),o->name());
				}
			},
		};

		std::vector<JUUIDName> templatesTypeList;
		for (auto& [type, uuids] : templates)
		{
			for (auto& uuid : uuids)
			{
				templatesTypeList.push_back(getJUUIDName.at(type)(uuid));
			}
		}
		return templatesTypeList;
	}

	std::vector<std::pair<std::string, JsonToEditorValueType>> GetTemplateAttributes(TemplateType t)
	{
		const std::map<TemplateType, std::function<std::vector<std::pair<std::string, JsonToEditorValueType>>()>> GetTAtts =
		{
			{ T_Materials, GetMaterialAttributes },
			{ T_Models3D, GetModel3DAttributes },
			{ T_Shaders, GetShaderAttributes },
			{ T_Sounds, GetSoundAttributes },
			{ T_Textures, GetTextureAttributes },
			{ T_RenderPasses, GetRenderPassAttributes },
			{ T_PhysicGeometries, GetPhysicGeometryAttributes },
			{ T_HtmlUIs, GetHtmlUIAttributes },
			{ T_Molds, GetMoldAttributes },
		};
		return GetTAtts.at(t)();
	}

	std::map<std::string, JEdvEditorDrawerFunction> GetTemplateDrawers(TemplateType t)
	{
		const std::map<TemplateType, std::function<std::map<std::string, JEdvEditorDrawerFunction>()>> GetTDrawers =
		{
			{ T_Materials, GetMaterialDrawers },
			{ T_Models3D, GetModel3DDrawers },
			{ T_Shaders, GetShaderDrawers },
			{ T_Sounds, GetSoundDrawers },
			{ T_Textures, GetTextureDrawers },
			{ T_RenderPasses, GetRenderPassDrawers },
			{ T_PhysicGeometries, GetPhysicGeometryDrawers },
			{ T_HtmlUIs, GetHtmlUIDrawers },
			{ T_Molds, GetMoldDrawers },
		};
		return GetTDrawers.at(t)();
	}

	std::map<std::string, JEdvEditorDrawerFunction> GetTemplatePreviewers(TemplateType t)
	{
		const std::map<TemplateType, std::function<std::map<std::string, JEdvEditorDrawerFunction>()>> GetTPreviewers =
		{
			{ T_Materials, GetMaterialPreviewers },
			{ T_Models3D, GetModel3DPreviewers },
			{ T_Shaders, GetShaderPreviewers },
			{ T_Sounds, GetSoundPreviewers },
			{ T_Textures, GetTexturePreviewers },
			{ T_RenderPasses, GetRenderPassPreviewers },
			{ T_PhysicGeometries, GetPhysicGeometryPreviewers },
			{ T_HtmlUIs, GetHtmlUIPreviewers },
			{ T_Molds, GetMoldPreviewers },
		};
		return GetTPreviewers.at(t)();
	}

	nlohmann::json GetTemplateJson(TemplateType t)
	{
		const std::map<TemplateType, std::function<nlohmann::json()>> GetTJson =
		{
			{ T_Materials, CreateMaterialJson },
			{ T_Models3D, CreateModel3DJson },
			{ T_Shaders, CreateShaderJson },
			{ T_Sounds, CreateSoundJson },
			{ T_Textures, CreateTextureJson },
			{ T_RenderPasses, CreateRenderPassJson },
			{ T_PhysicGeometries, CreatePhysicGeometryJson },
			{ T_HtmlUIs, CreateHtmlUIJson },
			{ T_Molds, CreateMoldJson },
		};
		return GetTJson.at(t)();
	}

	nlohmann::json GetTemplateCreationModalProperties(TemplateType t)
	{
		const std::map<TemplateType, std::function<nlohmann::json()>> GetTJson =
		{
			{ T_Materials, [] { return nlohmann::json(
				{
					{ "assetsFolder" , defaultAssetsFolder },
					{ "fileFolder" , defaultAssetsFolder }
				});
			}},
			{ T_Models3D, [] { return nlohmann::json(
				{
					{ "assetsFolder" , default3DModelsFolder },
					{ "fileFolder" , default3DModelsFolder }
				});
			}},
			{ T_Shaders, [] { return nlohmann::json(
				{
					{ "assetsFolder" , defaultShadersFolder},
					{ "fileFolder" , defaultShadersFolder }
				});
			}},
			{ T_Sounds, [] { return nlohmann::json(
				{
					{ "assetsFolder" , defaultSoundsFolder },
					{ "fileFolder" , defaultSoundsFolder }
				});
			}},
			{ T_Textures, [] { return nlohmann::json(
				{
					{ "assetsFolder" , "../Target/"},
					{ "fileFolder" , defaultAssetsFolder }
				});
			}},
			{ T_RenderPasses, [] { return nlohmann::json(
				{
					{ "assetsFolder" , defaultAssetsFolder },
					{ "fileFolder" , defaultAssetsFolder }
				});
			}},
			{ T_PhysicGeometries, [] { return nlohmann::json(
				{
					{ "assetsFolder" , defaultAssetsFolder },
					{ "fileFolder" , defaultAssetsFolder }
				});
			}},
			{ T_HtmlUIs, [] { return nlohmann::json(
				{
					{ "assetsFolder" , defaultUIFolder },
					{ "fileFolder" , defaultUIFolder }
				});
			}},
			{ T_Molds, [] { return nlohmann::json(
				{
					{ "assetsFolder" , defaultMoldsFolder },
					{ "fileFolder" , defaultMoldsFolder }
				});
			}},
		};
		return GetTJson.at(t)();
	}

	std::vector<std::string> GetTemplateRequiredAttributes(TemplateType t)
	{
		const std::map<TemplateType, std::function<std::vector<std::string>()>> GetTRequiredAtts =
		{
			{ T_Materials, GetMaterialRequiredAttributes },
			{ T_Models3D, GetModel3DRequiredAttributes },
			{ T_Shaders, GetShaderRequiredAttributes },
			{ T_Sounds, GetSoundRequiredAttributes },
			{ T_Textures, GetTextureRequiredAttributes },
			{ T_RenderPasses, GetRenderPassRequiredAttributes },
			{ T_PhysicGeometries, GetPhysicGeometryRequiredAttributes },
			{ T_HtmlUIs, GetHtmlUIRequiredAttributes },
			{ T_Molds, GetMoldRequiredAttributes },
		};
		return GetTRequiredAtts.at(t)();
	}

	std::map<std::string, JEdvCreatorDrawerFunction> GetTemplateCreatorDrawers(TemplateType t)
	{
		const std::map<TemplateType, std::function<std::map<std::string, JEdvCreatorDrawerFunction>()>> GetTDrawers =
		{
			{ T_Materials, GetMaterialCreatorDrawers },
			{ T_Models3D, GetModel3DCreatorDrawers },
			{ T_Shaders, GetShaderCreatorDrawers },
			{ T_Sounds, GetSoundCreatorDrawers },
			{ T_Textures, GetTextureCreatorDrawers },
			{ T_RenderPasses, GetRenderPassCreatorDrawers },
			{ T_PhysicGeometries, GetPhysicGeometryCreatorDrawers },
			{ T_HtmlUIs, GetHtmlUICreatorDrawers },
			{ T_Molds, GetMoldCreatorDrawers },
		};
		return GetTDrawers.at(t)();
	}

	std::map<std::string, JEdvCreatorValidatorFunction> GetTemplateValidators(TemplateType t)
	{
		const std::map<TemplateType, std::function<std::map<std::string, JEdvCreatorValidatorFunction>()>> GetTValidator =
		{
			{ T_Materials, GetMaterialCreatorValidator },
			{ T_Models3D, GetModel3DCreatorValidator },
			{ T_Shaders, GetShaderCreatorValidator },
			{ T_Sounds, GetSoundCreatorValidator },
			{ T_Textures, GetTextureCreatorValidator },
			{ T_RenderPasses, GetRenderPassCreatorValidator },
			{ T_PhysicGeometries, GetPhysicGeometryCreatorValidator },
			{ T_HtmlUIs, GetHtmlUICreatorValidator },
			{ T_Molds, GetMoldCreatorValidator },
		};
		return GetTValidator.at(t)();
	}

	void CreateTemplateFromJson(nlohmann::json& json, std::function<void(nlohmann::json& json)> creator)
	{
		if (!json.contains("uuid") || json.at("uuid") == "")
		{
			nlohmann::json patch = { {"uuid",getUUID()} };
			json.merge_patch(patch);
		}
		creator(json);
	}

	void CreateTemplate(TemplateType t, nlohmann::json json)
	{
		const std::map<TemplateType, std::function<void(nlohmann::json json)>> CreateT =
		{
			{ T_Materials, [](nlohmann::json json) { CreateTemplateFromJson(json,Templates::CreateMaterial); } },
			{ T_Models3D,[](nlohmann::json json) { CreateTemplateFromJson(json,Templates::CreateModel3D); } },
			{ T_Shaders,[](nlohmann::json json) { CreateTemplateFromJson(json,Templates::CreateShader); } },
			{ T_Sounds,[](nlohmann::json json) { CreateTemplateFromJson(json,Templates::CreateSound); } },
			{ T_Textures,[](nlohmann::json json) { CreateTemplateFromJson(json,Templates::CreateTextureFromJsonDefinition); } },
			{ T_RenderPasses,[](nlohmann::json json) { CreateTemplateFromJson(json,Templates::CreateRenderPass); } },
			{ T_PhysicGeometries,[](nlohmann::json json) { CreateTemplateFromJson(json,Templates::CreatePhysicGeometry); } },
			{ T_HtmlUIs,[](nlohmann::json json) { CreateTemplateFromJson(json,Templates::CreateHtmlUI); } },
			{ T_Molds,[](nlohmann::json json) { CreateTemplateFromJson(json,Templates::CreateMold); } },
		};
		CreateT.at(t)(json);
		Editor::MarkTemplatesPanelAssetsAsDirty();
	}

	TemplateType GetTemplateTypeFromFile(std::string file)
	{
		const std::map<std::string, TemplateType> GetT4F = {
			{ Material::templateName, T_Materials },
			{ Model3D::templateName, T_Models3D },
			{ Shader::templateName, T_Shaders },
			{ Sound::templateName, T_Sounds },
			{ Texture::templateName, T_Textures },
			{ RenderPass::templateName, T_RenderPasses },
			{ PhysicGeometry::templateName, T_PhysicGeometries },
			{ HtmlUI::templateName, T_HtmlUIs },
			{ Mold::templateName, T_Molds },
		};
		return GetT4F.at(file);
	}

	JNAME GetTemplateName(TemplateType t, JUUID uuid)
	{
		const std::map<TemplateType, std::function<JNAME(JUUID)>> GetTName = {
			{ T_Materials, GetMaterialName },
			{ T_Models3D, GetModel3DName },
			{ T_Shaders, GetShaderName },
			{ T_Sounds, GetSoundName },
			{ T_Textures, GetTextureName },
			{ T_RenderPasses, GetRenderPassName },
			{ T_PhysicGeometries, GetPhysicGeometryName },
			{ T_HtmlUIs, GetHtmlUIName },
			{ T_Molds, GetMoldName },
		};
		return GetTName.at(t)(uuid);
	}

	std::string GetTemplateFile(TemplateType t)
	{
		const std::map<TemplateType, std::string> GetF = {
			{ T_Materials, Material::templateName },
			{ T_Models3D, Model3D::templateName },
			{ T_Shaders, Shader::templateName },
			{ T_Sounds, Sound::templateName },
			{ T_Textures, Texture::templateName },
			{ T_RenderPasses, RenderPass::templateName },
			{ T_PhysicGeometries, PhysicGeometry::templateName },
			{ T_HtmlUIs, HtmlUI::templateName },
			{ T_Molds, Mold::templateName },
		};
		return defaultTemplatesFolder + GetF.at(t);
	}

	void DeleteTemplate(TemplateType t, JUUID uuid)
	{
		templates.at(t).erase(uuid);
		templatesTypes.erase(uuid);

		const std::map<TemplateType, std::function<void(JUUID)>> DeleteT = {
			{ T_Materials, DeleteMaterialTemplate },
			{ T_Models3D, DeleteModel3DTemplate },
			{ T_Shaders, DeleteShaderTemplate },
			{ T_Sounds, DeleteSoundTemplate },
			{ T_Textures, DeleteTextureTemplate },
			{ T_RenderPasses, DeleteRenderPassTemplate },
			{ T_PhysicGeometries, DeletePhysicGeometryTemplate },
			{ T_HtmlUIs, DeleteHtmlUITemplate },
			{ T_Molds, DeleteMoldTemplate },
		};
		DeleteT.at(t)(uuid);
	}

	void DeleteTemplate(JUUID uuid)
	{
		using namespace Editor;
		TemplateType type = GetTemplateType(uuid);
		std::string name = GetTemplateName(type, uuid);
		std::set<std::string> skipTemplateFile = { GetTemplateFile(type) };
		std::set<std::string> levelNames = GetOpenedScenes();
		std::vector<nlohmann::json> references;
		std::set<std::string> skipLevelFiles;
		std::transform(levelNames.begin(), levelNames.end(), std::inserter(skipLevelFiles, skipLevelFiles.begin()), [](auto& name)
			{
				return defaultLevelsFolder + name + ".json";
			}
		);

		auto deleteTemplate = [type, uuid](std::vector<nlohmann::json> references)
			{
				DeleteTemplate(type, uuid);
				std::vector<nlohmann::json> templateReferences;
				std::copy_if(references.begin(), references.end(), std::back_inserter(templateReferences), [](auto& ref)
					{
						std::string type = ref.at("type");
						bool del = ref.at("delete");
						return type == "template" && del;
					}
				);
				std::vector<nlohmann::json> sceneObjectReferences;
				std::copy_if(references.begin(), references.end(), std::back_inserter(sceneObjectReferences), [](auto& ref)
					{
						std::string type = ref.at("type");
						bool del = ref.at("delete");
						return type == "sceneobject" && del;
					}
				);
				std::vector<nlohmann::json> currentLevelReferences;
				std::copy_if(references.begin(), references.end(), std::back_inserter(currentLevelReferences), [](auto& ref)
					{
						std::string type = ref.at("type");
						bool del = ref.at("delete");
						return type == "currentlevel" && del;
					}
				);
				DeleteTemplateReferences(templateReferences);
				DeleteTemplateReferencesInLevels(sceneObjectReferences);
				DeleteTemplateReferencesInOpenedLevels(currentLevelReferences);
				for (int i = 0; i < currentLevelReferences.size(); i++)
				{
					MarkSceneUnitAsModified(static_cast<SceneUnitId>(currentLevelReferences[i].at("unit")));
				}
				MarkTemplatesPanelAssetsAsDirty();
				CloseDeletionPrompt();
				RemoveFromTemplateSelection({ uuid });
			};

		FindTemplatesReferencesInTemplates(uuid, skipTemplateFile, [&references](nlohmann::json nav)
			{
				references.push_back(nav);
			}
		);
		FindTemplatesReferencesInLevels(uuid, skipLevelFiles, [&references](nlohmann::json nav)
			{
				references.push_back(nav);
			}
		);
		FindTemplatesReferencesInOpenedLevels(uuid, [&references](nlohmann::json nav)
			{
				references.push_back(nav);
			}
		);
		if (references.size() > 0U)
		{
			PromptTemplateDeletion(references, deleteTemplate, []()
				{
					CloseDeletionPrompt();
				}
			);
		}
		else
		{
			deleteTemplate(references);
		}
	}

	void DeleteTemplateReferences(std::vector<nlohmann::json> references)
	{
		for (auto& ref : references)
		{
			std::string uuid = ref.at("uuid");
			std::string path = ref.at("path");
			TemplateType type = StringToTemplateType.at(ref.at("template"));
			JObject* j = GetJTemplatePointer(uuid);

			nlohmann::json j_patch = {
				{
					{ "op", "replace" },
					{ "path", path },
					{ "value", "" },
				}
			};

			j->patch_inplace(j_patch);
		}
	}

	void DeleteTemplateReferencesInLevels(std::vector<nlohmann::json> references)
	{
		std::map<std::string, std::set<std::string>> levelPaths;
		for (auto& ref : references)
		{
			std::string filename = ref.at("filename");
			std::string path = ref.at("path");
			levelPaths[filename].insert(path);
		}

		for (auto& [level, paths] : levelPaths)
		{
			std::filesystem::path path = defaultLevelsFolder + level;
			std::ifstream file(path);
			nlohmann::json data = nlohmann::json::parse(file);
			file.close();
			for (auto& p : paths)
			{
				nlohmann::json j_patch = {
					{
						{ "op", "replace" },
						{ "path", p },
						{ "value", "" },
					}
				};
				data.patch_inplace(j_patch);
			}
			std::ofstream ofile(path);
			std::string levelString = data.dump(4);
			ofile.write(levelString.c_str(), levelString.size());
			ofile.close();
		}
	}

	void DeleteTemplateReferencesInOpenedLevels(std::vector<nlohmann::json> references)
	{
		using namespace Scene;

		for (auto& ref : references)
		{
			SceneUnitId id = ref.at("unit");
			std::string uuid = ref.at("uuid");
			SceneObject* so = GetSceneObjectPointer(id, uuid);

			std::string path = ref.at("path");
			std::vector<std::string> parts = nostd::split(path, "/");

			parts.erase(parts.begin());
			nlohmann::json att = so->at(parts.front());
			if (att.is_string())
			{
				nlohmann::json patch = { {parts.front(),""} };
				so->JUpdate(patch);
			}
			else
			{
				parts.erase(parts.begin());
				std::string replacePath;
				std::for_each(parts.begin(), parts.end(), [&replacePath](std::string part)
					{
						replacePath += "/" + part;
					}
				);

				nlohmann::json patch = {
				{
					{ "op", "replace" },
					{ "path", replacePath },
					{ "value", "" },
					}
				};

				att.patch_inplace(patch);
				so->JUpdate(att);
			}
		}
	}

	void FindTemplatesReferencesInTemplates(JUUID uuid, std::set<std::string> skipTemplateFile, std::function<void(nlohmann::json)> addReference)
	{
		for (auto& entry : std::filesystem::directory_iterator(defaultTemplatesFolder))
		{
			auto path = entry.path();
			if (path.extension() != ".json" || skipTemplateFile.contains(path.string())) continue;

			std::ifstream file(path);
			nlohmann::json data = nlohmann::json::parse(file);

			TemplateType templateType = GetTemplateTypeFromFile(path.filename().string());

			unsigned int size = static_cast<unsigned int>(data.size());
			for (unsigned int i = 0U; i < size; i++)
			{
				auto& j = data.at(i);

				nlohmann::json json = {
					{ "delete", false },
					{ "uuid", j.at("uuid") },
					{ "name", j.at("name") },
					{ "type", "template" },
					{ "template", TemplateTypeToString.at(templateType) }
				};
				FindRecursiveJsonReference(j, uuid, "", [&json, addReference](std::string path)
					{
						json["path"] = path;
						addReference(json);
					}
				);
			}
		}
	}

	void FindTemplatesReferencesInLevels(JUUID uuid, std::set<std::string> skipLevelFiles, std::function<void(nlohmann::json)> addReference)
	{
		for (auto& entry : std::filesystem::directory_iterator(defaultLevelsFolder))
		{
			auto path = entry.path();
			if (path.extension() != ".json" || skipLevelFiles.contains(path.string())) continue;

			std::ifstream file(path);
			nlohmann::json data = nlohmann::json::parse(file);

			for (auto& [_, item] : SceneObjectTypeJsonContainer)
			{
				if (!data.contains(item)) continue;

				unsigned int size = static_cast<unsigned int>(data.at(item).size());
				for (unsigned int i = 0U; i < size; i++)
				{
					auto& j = data.at(item).at(i);

					nlohmann::json json = {
						{ "delete", false },
						{ "uuid", j.at("uuid") },
						{ "name", j.at("name") },
						{ "type", "sceneobject" },
						{ "sceneObject", item },
						{ "filename", path.filename().string() }
					};
					FindRecursiveJsonReference(j, uuid, "/" + item + "/" + std::to_string(i), [&json, addReference](std::string path)
						{
							json["path"] = path;
							addReference(json);
						}
					);
				}
			}
		}
	}

	void FindTemplatesReferencesInOpenedLevels(JUUID uuid, std::function<void(nlohmann::json)> addReference)
	{
		using namespace Scene;
		using namespace Editor;

		std::set<SceneUnitId> units = GetOpenedSceneUnitIds(false);

		for (auto id : units)
		{
			std::unordered_map<JUUID, SceneObjectType>& objectTypes = GetSceneObjectsTypes(id);

			for (auto& [soUUID, type] : objectTypes)
			{
				SceneObject* so = GetSceneObjectPointer(id, soUUID);

				nlohmann::json json = {
					{ "delete", true },
					{ "uuid", soUUID },
					{ "name", so->at("name") },
					{ "type", "currentlevel" },
					{ "sceneObject", SceneObjectTypeJsonContainer.at(type) },
					{ "unit", id }
				};

				FindRecursiveJsonReference(so->json(), uuid, "", [&json, addReference](std::string path)
					{
						json["path"] = path;
						addReference(json);
					}
				);
			}
		}
	}

	void FindRecursiveJsonReference(nlohmann::json json, JUUID uuid, std::string path, std::function<void(std::string path)> addReference)
	{
		if (json.is_object())
		{
			for (auto& [key, val] : json.items())
			{
				auto& j = json.at(key);
				FindRecursiveJsonReference(j, uuid, path + "/" + key, addReference);
			}
		}
		else if (json.is_array())
		{
			unsigned int size = static_cast<unsigned int>(json.size());
			for (unsigned int i = 0; i < size; i++)
			{
				auto& j = json.at(i);
				FindRecursiveJsonReference(j, uuid, path + "/" + std::to_string(i), addReference);
			}
		}
		else if (json.is_string())
		{
			if (json.get_ref<std::string&>() == uuid)
			{
				addReference(path);
			}
		}
	}
#endif
}