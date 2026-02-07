#pragma once

//extern void GameStep();
//extern void GameRender();
//extern void GamePostRender();
//extern void GameDestroy();
extern void GetAudioListenerVectors(std::function<void(XMFLOAT3, XMVECTOR)>);
extern void WindowResizeReleaseResources();
extern void WindowResize(unsigned int width, unsigned int height);
