#pragma once

#include "resource.h"

//void GameStep();
//void GameDestroy();
//void GameRender();
//void GamePostRender();
void WindowResizeReleaseResources();
void WindowResize(unsigned int width, unsigned int height);
void GetAudioListenerVectors(std::function<void(XMFLOAT3, XMVECTOR)>);

#if defined(_EDITOR)
void DestroyEditorModeBindings();
void CreateEditorIndependentCamera();
void SwitchToEditorCamera();
void SwitchToEditorPlayCamera();
void DestroyEditorCameras();
void ReloadSceneFromPrePlay();

#endif