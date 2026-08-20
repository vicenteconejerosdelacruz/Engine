#include "pch.h"
#include "DefaultLevel.h"
#include <Yaml2Json.h>

#if defined(_EDITOR)

namespace Editor::DefaultLevel {

	const std::string renderablesYaml = R"(
renderables:
  - cameras:
      - 06de4a6c-0393-42b1-91ab-1d2389cb2cbd
    castShadows: false
    meshMaterial:
      material: ecd1688c-73d6-49d0-870f-ca916a417c49
      mesh:
        primitive: d41e5c29-49bb-4f2c-aa2b-da781fbac512
    name: floor
    physicObject:
      - behavior: Static
        geometry: 330c6bfd-2c71-4c6b-be42-b797d07ab5ba
    position:
      - 0.0
      - -1.0
      - 0.0
    scale:
      - 20.0
      - 1.0
      - 20.0
    shadowed: true
    uuid: 31994be6-1fb5-4046-b101-6b83af3c61c4
)";

	const std::string camerasYaml = R"(
cameras:
  - fitWindow: true
    mouseController: true
    name: default.cam.0
    perspective:
      farZ: 100.0
      fovAngleY: 70.0
      nearZ: 0.001
    position:
      - 0.0
      - 0.0
      - -5.0
    projectionType: Perspective
    renderPasses: []
    rotation:
      - 0.0
      - 0.0
      - 0.0
    speed: 0.05000000074505806
    useSwapChain: true
    uuid: 06de4a6c-0393-42b1-91ab-1d2389cb2cbd
)";

	const std::string lightsYaml = R"(
lights:
  - cameras:
      - 06de4a6c-0393-42b1-91ab-1d2389cb2cbd
    color:
      - 0.05000000074505806
      - 0.05000000074505806
      - 0.05000000074505806
    lightType: Ambient
    name: light.0.amb
    uuid: fa0f8e67-db28-411d-a042-de3a84f203f2

  - cameras:
      - 06de4a6c-0393-42b1-91ab-1d2389cb2cbd
    color:
      - 1.0
      - 1.0
      - 1.0
    farZ: 1000.0
    hasShadowMaps: true
    lightType: Directional
    name: light.1.dir
    nearZ: 0.01
    rotation:
      - 63.0
      - 0.0
      - 0.0
    shadowMapHeight: 4096
    shadowMapWidth: 4096
    uuid: 9ec2714e-0184-45f2-aaee-b9b5d08e5763
    viewBottom: 0.0
    viewHeight: 32.0
    viewRight: 0.0
    viewWidth: 32.0
    zBias: 0.0002
)";

	const std::string soundsYaml = R"(
sounds:
  - autoPlay: true
    name: music
    sound: 14336def-b73c-4d8a-afb3-8f913ef68219
    uuid: 14dc1115-f076-4293-aa53-708851b99835
    volume: 0.3
)";

	const std::string physicsScenesYaml = R"(
physicScenes:
  - gravity:
      - 0.0
      - -9.81
      - 0.0
    name: default-level-physics
    uuid: 7113a419-952e-4c88-9e2a-faa561ff240e
)";

	nlohmann::json GetDefaultLevelRenderables()
	{
		nlohmann::json j = yaml_str_to_json(renderablesYaml);
		return j;
	}

	nlohmann::json GetDefaultLevelCameras()
	{
		nlohmann::json j = yaml_str_to_json(camerasYaml);
		return j;
	}

	nlohmann::json GetDefaultLevelLights()
	{
		nlohmann::json j = yaml_str_to_json(lightsYaml);
		return j;
	}

	nlohmann::json GetDefaultLevelSounds()
	{
		nlohmann::json j = yaml_str_to_json(soundsYaml);
		return j;
	}

	nlohmann::json GetDefaultLevelPhysicsScenes()
	{
		nlohmann::json j = yaml_str_to_json(physicsScenesYaml);
		return j;
	}
}

#endif