#include "pch.h"
#include "UI.h"
#include <Ultralight/Ultralight.h>
#include <AppCore/Platform.h>
#include <string>

using namespace ultralight;

RefPtr<ultralight::Renderer> ultraLightRenderer;

namespace UI
{
	void InitUI(std::string resource_path_prefix)
	{
		Config config;
		config.resource_path_prefix = resource_path_prefix.c_str();
		ultralight::Platform::instance().set_config(config);

		ultralight::Platform::instance().set_font_loader(ultralight::GetPlatformFontLoader());
		ultralight::Platform::instance().set_file_system(ultralight::GetPlatformFileSystem("."));
		ultralight::Platform::instance().set_logger(ultralight::GetDefaultLogger("ultralight.log"));

		ultraLightRenderer = ultralight::Renderer::Create();

		ultralight::ViewConfig viewConfig;
		viewConfig.is_transparent = true;
		viewConfig.initial_device_scale = 1.0f;
	}

	void DestroyUI()
	{
		ultraLightRenderer = nullptr;
		ultralight::Platform::instance().set_font_loader(nullptr);
		ultralight::Platform::instance().set_file_system(nullptr);
		ultralight::Platform::instance().set_logger(nullptr);
	}

	void UIStep()
	{
		ultraLightRenderer->Update();
		ultraLightRenderer->RefreshDisplay(0);
		ultraLightRenderer->Render();
	}
};
