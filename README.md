# Culpeo Game Engine

![](https://media.githubusercontent.com/media/vicenteconejerosdelacruz/Engine/refs/heads/main/TestGame/Target/Assets/ui/logo.png)

## Table of Content

- [Installation](#install)
- [Dependencies](#deps)
  - [nlohmann::json](#deps-nlohmann-json)
  - [Assimp](#deps-assimp)
  - [DirectXTK12](#deps-directxtk12)
  - [DirectXTex](#deps-directxtex)
  - [DXIL](#deps-dxil)
  - [PhysX](#deps-physx)
  - [v8](#deps-v8)
  - [UltraLight](#deps-ultra-light)
  - [ImGui](#deps-imgui)
  - [ImGuizmo](#deps-imguizmo)
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

## Dependencies

lets make an explanation of the dependencies listed in the installation step and what is their purpose

### nlohmann::json [GitHub - nlohmann/json: JSON for Modern C++ · GitHub](https://github.com/nlohmann/json)

nlohmann::json is a pure header library made with the specific purpose to load/parse & build json representations. this allows Culpeo to acomplish load&save json files for templates and levels, but also every [JObject](#defs-JObect) derived (Templates, SceneObjects, Controllers & PhysicObjects) can use nlohmann::json representation to interact with

### Assimp [Open Asset Import Library · GitHub](https://github.com/assimp)

Assimp's purpose is to allow 3D Models to be loaded into the engine. it imports the meshes as a readable format which laters get's transformed into Vertices and Indices representations. It also loads descriptions for the materials(textures and properties) which are used to build basic Materials(which can be modified) for the 3D model

### DirectXTK12 [GitHub - microsoft/DirectXTK12: The DirectX Tool Kit (aka DirectXTK12) is a collection of helper classes for writing DirectX 12 code in C++ · GitHub](https://github.com/microsoft/DirectXTK12)

DirectXTK12 is used for processing the Mouse, Gamepad and Keyboard input and also is used to load images in dds format 

### DirectXTex [GitHub - microsoft/DirectXTex: DirectXTex texture processing library · GitHub](https://github.com/microsoft/DirectXTex)

DirectXTex is mainly a Library/CLI utility used to gather information from images like dimensions(texdiag), convert images formats(texconv). but I'have modified DirectXTex to allow to use it as an integrated library so there is no need to open an cmd or powershell terminal in order to interact with it. any jpeg, gif or png is converted to dds using this library

### Physx [GitHub - NVIDIA-Omniverse/PhysX: NVIDIA PhysX SDK · GitHub](https://github.com/NVIDIA-Omniverse/PhysX)

Physx is the physics library developed by NVidia, we currently support making static, dynamic, character controllers and triggers

### V8 [GitHub - v8/v8: The official mirror of the V8 Git repository · GitHub](https://github.com/v8/v8)

V8 is a JavaScript Engine developed by google, used for running scripts. it's installation is done not through github, but through nuget as v8pp [GitHub - pmed/v8pp: Bind C++ functions and classes into V8 JavaScript engine · GitHub](https://github.com/pmed/v8pp) offers a ready to use alternative. nonetheless v8pp is not used in Culpeo, but attributes and functions are binded using Culpeo own implementations needed in ordere to interact with nlohmann::json 

### Ultralight https://ultralig.ht/

Ultralight allow us to render UI elements using HTML webpages, it's built using JavaScriptCore(another JavaScript Engine) and it's possible to have rich and complex UI elements in the game built by a web designer supporting simple HTML webpage or larger pages embedding frameworks like react

### ImGui [GitHub - ocornut/imgui: Dear ImGui: Bloat-free Graphical User interface for C++ with minimal dependencies · GitHub](https://github.com/ocornut/imgui)

ImGui is an inmediate mode UI library, great for building customizable tools like the Modals, Panels. In culpeo's case the whole Editor is built using ImGui 

### ImGuizmo [GitHub - CedricGuillemet/ImGuizmo: Immediate mode 3D gizmo for scene editing and other controls based on Dear Imgui · GitHub](https://github.com/cedricguillemet/imguizmo)

ImGuizmo is a small library that allows to easily modify objects transformation(position, rotation & scale) of objects using the mouse

## Architecture

### JSON

Most of the architecture on how data is handled are done through JSON objects, either using nlohmann::json interfaces or class methods created to interact with nlohmann::json 

for example a simple boolean attribute in nlohmann::json is represented like this

```cpp
nlohmann::json attributes = { {"visible", true} };
attributes["visible"] = false;
attributes.at("visible") = true;
```

in the Engine case you can expose attributes to be used like this

```cpp
bool visible();
void visible(bool value);
```

to acomplish this you must use c/c++ macros in the attribute header file(will be explained later). for example a simple version of an attribute header file would be

```cpp
JCLASS(Renderable, GetRenderables)
JTYPE(SceneObjectType, SO_Renderables))
JEXPOSE(bool, visible, true, jedv_t_boolean, 0, false)
JTRACKUUID(Renderable, Renderables, 0, true)
```

the JEXPOSE macro will make the "visible"" attribute to be accesible using functions and the nlohmann::json representation

### Macros magic

you will find several macros aimed to some specific use cases like

#### JSON interoperability

the Scene Objects and Templates will define an attribute file "*Att.h" on which macros like

```cpp
#define JCLASS(CLASS,GETJOBJECTS)
#define JTYPE(TYPE,VALUE)
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_CALLBACK(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_TRANSFORM_CALLBACK(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_MAP_OBJECT(TYPE, ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR_OBJECT(TYPE, ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_FLAG(ATT, INITIAL, GETFLAGSVALUESFUNCTION,UPDATEMASK,REQUIREDTOCREATE)
#define JPREVIEW(NAME,JEDVALUETYPE)
#define JTRACKUUID(CLASS,NAME,LIMIT,COND)
```

are used 

- uuid based object pointers

- 
