#include "pch.h"
#include "BootLevel.h"

namespace Game::BootLevel
{
	nlohmann::json renderables =
	{
		{ "renderables",
			{
				{
					{ "cameras" , { "06de4a6c-0393-42b1-91ab-1d2389cb2cbc" }},
					{ "castShadows", false },
					{ "meshMaterials", {
							{
								{ "material", "cc16d62e-f299-4e53-a5f7-0cb290cec466" },
								{ "mesh", "7dec1229-075f-4599-95e1-9ccfad0d48b1" }
							}
						}
					},
					{ "name", "logo" },
					{ "position", { 0.0, 2.0, 0.0 } } ,
					{ "renderNext", { "24ff1774-d2ea-484a-aaf6-0dd742c42aeb" } },
					{ "shadowed", false },
					{ "topology", "TRIANGLELIST" },
					{ "uuid", "845404cd-01ae-48b1-b840-03dd3d5a8351" },
					{ "checkBoundingBox", false }
				},
				{
					{ "cameras",{ "06de4a6c-0393-42b1-91ab-1d2389cb2cbc" } } ,
					{ "castShadows", false },
					{ "meshMaterials", {
							{
								{ "material", "28c4d879-6d21-408f-acbb-120f9fdc05b0" },
								{ "mesh", "7dec1229-075f-4599-95e1-9ccfad0d48b1" }
							}
						}
					} ,
					{ "name", "loadingBar" },
					{ "position", { 0.0, 0.0, 0.0 } },
					{ "shadowed", false },
					{ "topology", "TRIANGLELIST" },
					{ "uuid", "24ff1774-d2ea-484a-aaf6-0dd742c42aeb" },
					{ "checkBoundingBox", false }
				}
			}
		}
	};

	nlohmann::json cameras =
	{
		{ "cameras",
			{
				{
					{ "fitWindow", true },
					{ "name", "boot.cam.0"},
					{ "perspective",
						{
							{"farZ", 100.0 },
							{"fovAngleY", 70.0 },
							{"nearZ", 0.001 }
						}
					},
					{ "position", { 0.0, 0.0, 0.0 } },
					{ "projectionType", "Perspective" },
					{ "rotation", { 0.0, 0.0, 0.0 } },
					{ "speed", 0.05000000074505806 },
					{ "uuid", "06de4a6c-0393-42b1-91ab-1d2389cb2cbc"},
					{
						"renderPasses", {
							"d5a1867a-a480-48a0-b1aa-606cee1e087d",
							"53693830-779c-4ed0-a985-402d6a72485b"
						}
					},
					{ "mouseController", true },
					{ "useSwapChain", false }
				}
			}
		}
	};

	nlohmann::json lights = {
		{ "lights", { } }
	};

	nlohmann::json sounds = {
		{ "sounds", { } }
	};

	nlohmann::json& GetBootLevelRenderables()
	{
		return renderables;
	}

	nlohmann::json& GetBootLevelCameras()
	{
		return cameras;
	}

	nlohmann::json& GetBootLevelLights()
	{
		return lights;
	}

	nlohmann::json& GetBootLevelSounds()
	{
		return sounds;
	}
}