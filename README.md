# Culpeo Game Engine

![](https://media.githubusercontent.com/media/vicenteconejerosdelacruz/Engine/refs/heads/main/TestGame/Target/Assets/ui/logo.png)

## Table of Content

- [Installation](#install)
- [Dependencies](#deps)
  - [Assimp](#deps-assimp)
  - [nlohmann::json](#deps-nlohmann-json)
  - [imgui](#deps-imgui)
  - [ImGuizmo](#deps-imguizmo)
  - [v8](#deps-v8)
  - [v8pp](#deps-v8pp)
  - [DirectXTK12](#deps-directxtk12)
  - [DirectXTex](#deps-directxtex)
  - [UltraLight](#deps-ultra-light)
  - [PhysX](#deps-physx)
- [Architecture](#arch)
  - [JSON](#arch-json)
    - [nlohmann::json integration](#arch-json-integration)
    - [Macros magic](#arch-json-macros)
    - [JObject as base object](#arch-json-jobject)
      - [JObject lifecycle](#arch-json-jobject-lifecycle)
  - [Templates](#arch-templates)
    - [Location](#arch-templates-location)
    - [List of available templates](#arch-templates-list)
      - [Model3D](#arch-templates-list-model3d)
        - [Animations](#model3d-animations)
      - [Material](#arch-templates-list-material)
      - [Texture](#arch-templates-list-textures)
      - [RenderPass](#arch-templates-list-renderpass)
      - [Shader](#arch-templates-list-shader)
      - [Sound](#arch-templates-list-sound)
      - [*Mesh](#arch-templates-list-mesh)
    - [Instances](#arch-templates-instances)
  - [Scene Objects](#arch-so)
    - [Location](#arch-so-location)
    - [List of available scene objects](#arch-so-list)
      - [Renderable](#arch-so-list-renderable)
      - [Camera](#arch-so-list-camera)
      - [Light](#arch-so-list-light)
      - [SoundFX](#arch-so-list-soundfx)
  - [Rendering](#arch-rendering)
    - [Shaders](#arch-rendering-shaders)
    - [Pipeline State building](#arch-rendering-pipeline)
    - [Special materials](#arch-rendering-materials)
  - [Audio/Sound](#arch-audio)
    - [2D Sounds](#arch-audio-2d)
    - [3D Sounds](#arch-audio-3d)
  - [Animation Sequencer](#arch-animation-sequencer)

---

## Installation

In order to install you must clone the repository using git. this repository uses git LFS so be sure to have it enabled

in the SDKs folder there is a buildSDK.ps1 powershell script which right now partially builds the entire dependencies. 

The next dependencies are included in the repository

- Assimp

- DirectXTex

- DirectXTK12

- imgui

- imguizmo

Libraries like

- V8

- PhysX

- UltraLight

are required but the installation instructions will be covered in this readme
