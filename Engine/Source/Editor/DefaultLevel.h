#pragma once
#include <nlohmann/json.hpp>

#if defined(_EDITOR)

namespace Editor::DefaultLevel {

	nlohmann::json GetDefaultLevelRenderables();
	nlohmann::json GetDefaultLevelCameras();
	nlohmann::json GetDefaultLevelLights();
	nlohmann::json GetDefaultLevelSounds();
	nlohmann::json GetDefaultLevelPhysicsScenes();
}

#endif