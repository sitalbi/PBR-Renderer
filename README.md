# PBR Renderer

A real-time physically based renderer written in C++ and OpenGL.

This project is primarily a learning and experimentation project. The goal is to explore rendering techniques and renderer/software architecture design rather than provide a production-ready engine. It is not fully optimized yet and may contain bugs.


## Architecture

The project is split into two main components:


- **renderer/**  
  Contains the rendering backend (OpenGL abstraction, materials, render passes, etc.).  
  Designed to be reusable and integrated into other projects.

- **app/**  
  A lightweight application used to test and visualize the renderer.  
  Handles scene setup, camera, UI (ImGui), and model loading.



## Screenshots

<p align="center">
 <img src="https://raw.githubusercontent.com/sitalbi/PBR-Renderer/main/res/screenshots/screen1.png" width="650">
  <img src="https://raw.githubusercontent.com/sitalbi/PBR-Renderer/main/res/screenshots/screen2.png" width="650">
  <img src="https://raw.githubusercontent.com/sitalbi/PBR-Renderer/main/res/screenshots/screen3.png" width="650">
  <img src="https://raw.githubusercontent.com/sitalbi/PBR-Renderer/main/res/screenshots/screen4.png" width="650">
  <img src="https://raw.githubusercontent.com/sitalbi/PBR-Renderer/main/res/screenshots/screen5.png" width="650">
</p>



## Features
- **renderer**
	- Cook-Torrance BRDF (PBR)
	- Material system with texture support
	- Forward rendering pipeline
	- HDR environment maps / skyboxes with Image-Based Lighting (Diffuse + Specular IBL)
	- Post-processing: SSAO (Screen Space Ambient Occlusion), HDR bloom
	- Directional and point lights
	- Shadow mapping (directional + point lights)
- **app**
	- Model loading via Assimp
	- Free camera + orbit camera


## Build Instructions

### Requirements

- CMake ≥ 3.20  
- C++20 compatible compiler  
  - GCC 11+  
  - Clang 13+  
  - MSVC 2019+  


### Dependencies

Dependencies included as git submodules:

- [GLFW](https://www.glfw.org/)
- [GLAD](https://glad.dav1d.de/)
- [glm](https://github.com/g-truc/glm)
- [Assimp](https://github.com/assimp/assimp)
- [imgui](https://github.com/ocornut/imgui)
- [stb](https://github.com/nothings/stb)
- [magic_enum](https://github.com/Neargye/magic_enum)


### Build 
```bash
cmake -B out -S .
cmake --build out --config Release
```

> *Use `--config Debug` for debug builds*

The executable (renderer_app) will be generated in the output directory.

## TODO

- Deferred rendering pipeline

- Improve shadows (using CSM?)

- Improve render pass system

- GPU performance optimizations

## Credits

Assets:

- PBR Kabuto [Samurai Helmet](https://sketchfab.com/3d-models/pbr-kabuto-samurai-helmet-13b3ab49b6bd4247be36b54ba1a56d8a) by [LeahLindner](https://sketchfab.com/illation) licensed under CC-BY-NC-ND-4.0 (http://creativecommons.org/licenses/by-nc-nd/4.0/)
