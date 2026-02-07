#pragma once
#include <nlohmann/json.hpp>

namespace Game::BootLevel {

	nlohmann::json& GetBootLevelRenderables();
	nlohmann::json& GetBootLevelCameras();
	nlohmann::json& GetBootLevelLights();
	nlohmann::json& GetBootLevelSounds();
}
