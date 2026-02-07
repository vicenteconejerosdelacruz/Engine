#pragma once

#include <memory>
#include <UUID.h>

//DEF_UUID_TYPE(Scene, Renderable, GetRenderableSceneObject)
DEF_SUUUID_TYPE(Scene, Renderable, GetRenderableSUSceneObject)
//DEF_UUID_TYPE(Scene, Camera, GetCameraSceneObject)
DEF_SUUUID_TYPE(Scene, Camera, GetCameraSUSceneObject)
//DEF_UUID_TYPE(Scene, Light, GetLightSceneObject)
DEF_SUUUID_TYPE(Scene, Light, GetLightSUSceneObject)
//DEF_UUID_TYPE(Scene, SoundFX, GetSoundFXSceneObject)
DEF_SUUUID_TYPE(Scene, SoundFX, GetSoundFXSUSceneObject)
