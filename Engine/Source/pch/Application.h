#pragma once

static const std::string defaultLevelsFolder = "Levels/";
static const std::string defaultTemplatesFolder = "Templates/";
static const std::string defaultShadersFolder = "Shaders/";
static const std::string defaultShadersBinariesFolder = "Shaders/bin/";
static const std::string defaultAssetsFolder = "Assets/";
static const std::string default3DModelsFolder = "Assets/models/";
static const std::string defaultPhysxCookingFolder = "Assets/cooking/";
static const std::string defaultPhysxCookingSDFFolder = "Assets/cooking/sdf/";
static const std::string defaultSoundsFolder = "Assets/sounds/";
static const std::string defaultUIFolder = "Assets/html/";
static const std::vector<std::string> defaultTexturesFilters = {
	"All Image files. (*.jpg,*jpeg,*.png)","JPEG files. (*.jpg,*jpeg)", "PNG files. (*.png)"
};
static const std::vector<std::string> defaultTexturesExtensions = {
	"*.jpg;*jpeg;*.png","*.jpg;*jpeg", "*.png"
};
static const std::vector<std::string> defaultAnimatedTexturesFilters = {
	"Gif files. (*.gif)"
};
static const std::vector<std::string> defaultAnimatedTexturesExtensions = {
	"*.gif"
};
static const std::vector<std::string> cubeTextureAxesNames = { "X+" , "X-" , "Y+" , "Y-" , "Z+" , "Z-" , };

extern RECT hWndRect;
#define HWNDWIDTH	(static_cast<unsigned int>(hWndRect.right - hWndRect.left))
#define HWNDHEIGHT (static_cast<unsigned int>(hWndRect.bottom - hWndRect.top))
#define HWNDWIDTHF	(static_cast<float>(abs(hWndRect.right - hWndRect.left)))
#define HWNDHEIGHTF (static_cast<float>(abs(hWndRect.bottom - hWndRect.top)))
