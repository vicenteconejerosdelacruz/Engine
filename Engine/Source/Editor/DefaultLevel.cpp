#include "pch.h"
#include "DefaultLevel.h"

#if defined(_EDITOR)

namespace Editor::DefaultLevel {

	nlohmann::json renderables = {
		{
			"renderables", {
			{
				{ "castShadows", false },
				{ "shadowed", true },
				{
					"meshMaterial",
					{
						{ "material", "ecd1688c-73d6-49d0-870f-ca916a417c49"},
						{ "mesh",
							{
								{ "primitive", "d41e5c29-49bb-4f2c-aa2b-da781fbac512"}
							}
						}
					}
				},
				{ "name", "floor" },
				{ "position", { 0.0, -1.0, 0.0} },
				{ "scale", { 20.0, 1.0, 20.0} },
				{ "uuid", "31994be6-1fb5-4046-b101-6b83af3c61c4" },
				{ "cameras", { "06de4a6c-0393-42b1-91ab-1d2389cb2cbd" } },
				{ "physicObject",
					{
						{
							{ "behavior", "Static" },
							{ "geometry", "330c6bfd-2c71-4c6b-be42-b797d07ab5ba" }
						}
					}
				}
			}/*,
		{
				{ "castShadows", true },
				{ "shadowed", true },
				{
					"meshMaterial",
					{
						{ "material", "f3f37590-3eac-41c4-8288-a79f279857ce"},
						{ "mesh",
							{
								{ "primitive", "c900056b-9f67-47d1-a252-71e0ef1f9a65"}
							}
						}
					}
				},
				{ "name", "capsule" },
				{ "position", { 0.0, 0.0, 0.0} },
				{ "scale", { 1.0, 1.0, 1.0} },
				{ "uuid", "7f3fdaf2-dfbb-46ec-be09-803f5697f8b7" },
				{ "cameras", { "06de4a6c-0393-42b1-91ab-1d2389cb2cbd" } },
				{ "physicObject",
					{
						{
							{ "behavior", "Static" },
							{ "geometry", "95eee3bb-5b2f-41d7-b43a-11ab5524cca1" }
						}
					}
				}
			}*/
			/*,
			{
				{ "castShadows", true },
				{ "shadowed", true },
				{
					"meshMaterial",
					{
						{ "material", "4a5a2cb8-f2ea-4e15-8584-22bb675ae1bc" },
						{ "mesh",
							{
								{ "primitive", "d8bfdef4-55f9-4f6e-b4a8-20915eb854d6" }
							}
						}
					}
				},
				{ "name", "utahteapot" },
				{ "position", { 0.0, -0.6000000238418579, 2.5} },
				{ "scale", { 1.0, 1.0f, 1.0f } },
				{ "uuid", "4fdb1d72-96c5-4a1a-a81e-f902abba25f6" },
				{ "cameras" , {"06de4a6c-0393-42b1-91ab-1d2389cb2cbd"} },
				{ "controllers",
					{
						{ "spinyaw", {} }
					}
				}
			}*//*,
			{
				{ "castShadows", false },
				{ "shadowed", false },
				{
					"meshMaterial",
					{
						{ "material", "e241b072-3aea-4c22-afee-b3887732ea89"},
						{ "mesh",
							{
								{ "primitive", "f7786ac1-e296-4e9a-a7e6-6f1949de75ef" }
							}
						}
					}
				},
				{ "name", "crate" },
				{ "position", { 0.0, 0.0, 4.0} },
				{ "rotation", { 0.0, 0.0f, 0.0f } },
				{ "scale", { 0.5, 0.5, 0.5} },
				{ "uuid", "729f8eaa-e10f-41fb-968a-0148d0e52971" },
				{ "cameras", { "06de4a6c-0393-42b1-91ab-1d2389cb2cbd" } },
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
						{ "DepthEnable", false },
						{ "DepthFunc", "NONE" },
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
				{ "castShadows", false },
				{ "shadowed", false },
				{
					"meshMaterial",
					{
						{ "material", "1d7630c4-86b0-49eb-88f5-40bacb02a652"},
						{ "mesh",
							{
								{ "primitive", "f7786ac1-e296-4e9a-a7e6-6f1949de75ef" }
							}
						}
					}
				},
				{ "name", "crate_wf" },
				{ "position", { 0.0, 0.0, 4.0} },
				{ "rotation", { 0.0, 0.0f, 0.0f } },
				{ "scale", { 0.5, 0.5, 0.5} },
				{ "uuid", "897d240c-3847-4408-8c69-baf3fb7f184d" },
				{ "cameras", { "06de4a6c-0393-42b1-91ab-1d2389cb2cbd" } },
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
						{ "DepthEnable", false },
						{ "DepthFunc", "NONE" },
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
			}*/
			/*,
			{
				{ "castShadows", true },
				{ "shadowed", true },
				{
					"meshMaterial",
					{
						{ "material", "c50c40b7-9e17-42fb-a6bc-c3d15d72310f"},
						{ "mesh",
							{
								{ "primitive", "d76b3bd8-0f53-4128-974e-2d6d5062bc00"}
							}
						}
					}
				},
				{ "name", "pyramid" },
				{ "position", { 0.0, -1.0, 4.0 } },
				{ "rotation", { 0.0, 0.0f, 0.0f } },
				{ "scale", { 1.0, 1.0, 1.0} },
				{ "uuid", "40f3fd66-0729-4f98-bb6b-5f94d7461f86" },
				{ "cameras", { "06de4a6c-0393-42b1-91ab-1d2389cb2cbd" } },
				{ "physicObject",
					{
						{
							{ "behavior", "Dynamic" },
							{ "geometry", "36c75d6d-970f-43dc-97bd-9bc13a035a3a" }
						}
					}
				}
			}*//*,
			{
				{ "castShadows", true },
				{ "shadowed", true },
				{ "name", "building_row" },
				{ "position", {  -3.693021297454834, -0.034743160009384155, 5.449404716491699 } },
				{ "rotation", { -90.0, -54.384132385253906, 0.0 } },
				{ "scale", { 0.1, 0.1, 0.1} },
				{ "uuid", "387d34eb-b705-40ad-b460-7a75634c0aad" },
				{ "cameras", { "06de4a6c-0393-42b1-91ab-1d2389cb2cbd" } },
				{ "model", "379011d0-8ea6-41bd-9c4e-59bfc11412d2" },
				{ "physicObject",
					{
						{
							{ "behavior", "Static" },
							{ "geometry", "5d85bf6d-4570-46db-a632-40976a685374" }
						}
					}
				}
			}*//*,
			{
				{ "castShadows", true },
				{ "shadowed", true },
				{
					"meshMaterial",
					{
						{ "material", "4a5a2cb8-f2ea-4e15-8584-22bb675ae1bc"},
						{ "mesh",
							{
								{"primitive", "4d1174b2-8225-4c09-9db6-ff09718ae0f5"}
							}
						}
					}
				},
				{ "name", "sphere" },
				{ "position", { 0.0, 5.5706214904785156, 2.5761494636535645 } },
				{ "rotation", { 0, 0.0f, 0.0f } },
				{ "scale", { 1.0, 1.0, 1.0} },
				{ "uuid", "4c582e2a-66df-4c9a-af8d-ee2504a5d18b" },
				{ "cameras", { "06de4a6c-0393-42b1-91ab-1d2389cb2cbd" } }
			}*//*,
			{
				{ "castShadows", true },
				{ "shadowed", true },
				{
					"meshMaterial",
					{
						{ "material", "4a5a2cb8-f2ea-4e15-8584-22bb675ae1bc"},
						{ "mesh",
							{
								{"primitive", "ad73990a-c59d-45d2-8ec3-807b1f52f5b9"}
							}
						}
					}
				},
				{ "name", "cone" },
				{ "position", { 0.0, 2.5706214904785156, 2.5761494636535645 } },
				{ "rotation", { 0, 0.0f, 0.0f } },
				{ "scale", { 1.0, 1.0, 1.0} },
				{ "uuid", "91629983-c901-4126-bf21-e388ab6f6e16" },
				{ "cameras", { "06de4a6c-0393-42b1-91ab-1d2389cb2cbd" } },
				{ "physicObject",
					{
						{
							{ "behavior", "Dynamic" },
							{ "geometry", "8a41c342-e388-4e1f-8ba9-8b0a41b1c975" }
						}
					}
				}
			}*/
		}
		}
	};

	nlohmann::json cameras = {
		{ "cameras",
			{
				{
					{ "fitWindow", true },
					{ "name", "default.cam.0"},
					{ "perspective",
						{
							{"farZ", 100.0 },
							{"fovAngleY", 70.0 },
							{"nearZ", 0.001 }
						}
					},
					{ "position", { 0.0, 0.0, -5.0 } },
					{ "projectionType", "Perspective" },
					{ "rotation", { 0.0, 0.0, 0.0 } },
					{ "speed", 0.05000000074505806 },
					{ "uuid", "06de4a6c-0393-42b1-91ab-1d2389cb2cbd"},
					{
						"renderPasses", { }
					},
					{ "mouseController", true },
					{ "useSwapChain", true}
				}
			}
		}
	};

	nlohmann::json lights = {
		{ "lights",
			{
			{
				{ "cameras" , {"06de4a6c-0393-42b1-91ab-1d2389cb2cbd"} },
				{ "color", { 0.05000000074505806, 0.05000000074505806, 0.05000000074505806 } },
				{ "lightType", "Ambient" },
				{ "name", "light.0.amb" },
				{ "uuid", "fa0f8e67-db28-411d-a042-de3a84f203f2" }
			},
			{
				{ "cameras" , {"06de4a6c-0393-42b1-91ab-1d2389cb2cbd"} },
				{ "color", { 1.0, 1.0, 1.0} },
				{ "farZ" , 1000.0},
				{ "nearZ", 0.01},
				{ "shadowMapHeight", 4096},
				{ "shadowMapWidth", 4096},
				{ "viewHeight", 32.0},
				{ "viewRight", 0.0},
				{ "viewWidth", 32.0},
				{ "viewBottom", 0.0},
				{ "hasShadowMaps", true },
				//{ "rotation", {-130.31087875366211, -0.30000039935112, 0.0} },
				{ "rotation", {63.0, 0.0, 0.0} },
				{ "lightType", "Directional"},
				{ "name", "light.1.dir"},
				{ "uuid", "9ec2714e-0184-45f2-aaee-b9b5d08e5763"},
				{ "zBias", 0.0002 }
			}
		}
	}
	};

	nlohmann::json sounds = {
		{ "sounds",
			{
			{
				{ "uuid", "14dc1115-f076-4293-aa53-708851b99835" },
				{ "name", "music" },
				{ "sound", "14336def-b73c-4d8a-afb3-8f913ef68219" },
				{ "volume", 0.3 },
				{ "autoPlay", true }
			}
			}
		}
	};

	nlohmann::json physicsScenes = {
		{ "physicScenes",
			{
			{
				{ "uuid", "7113a419-952e-4c88-9e2a-faa561ff240e" },
				{ "name", "default-level-physics" },
				{ "gravity", { 0.0, -9.81, 0.0 } }
			}
			}
		}
	};

	nlohmann::json& GetDefaultLevelRenderables()
	{
		return renderables;
	}

	nlohmann::json& GetDefaultLevelCameras()
	{
		return cameras;
	}

	nlohmann::json& GetDefaultLevelLights()
	{
		return lights;
	}

	nlohmann::json& GetDefaultLevelSounds()
	{
		return sounds;
	}

	nlohmann::json& GetDefaultLevelPhysicsScenes()
	{
		return physicsScenes;
	}
}

#endif