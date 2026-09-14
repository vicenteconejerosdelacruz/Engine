#pragma once

namespace UI
{
	void InitUI(std::string resource_path_prefix);
	void DestroyUI();
	void UIStep();
	void ResizeReleaseUI();
	void ResizeUI(uint32_t width, uint32_t height);
};