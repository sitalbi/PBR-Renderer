# Physically Based Renderer

This is a simple 3D physically based renderer written in C++ and OpenGL.
It implements the [Cook-Torrance BRDF microfacet model](https://en.wikipedia.org/wiki/Specular_highlight#Cook%E2%80%93Torrance_model) to simulate the reflection of light on a surface.

***Note***: *this project is mainly a learning exercise and is not intended for production use. The goal was for me to have a project where I could try to design a renderer and its base architecture as well as implement various PBR techniques using OpenGL. It is not fully optimized for performance and can contain bugs.*

## Images
![renderer_image](https://simono.fr/renderer.png)

## Features:
- Cook-Torrance BRDF model
- Material system with texture support
- Forward rendering
- Diffuse and Specular IBL (Image Based Lighting)
- HDRI skyboxes for environment lighting
- Model loading with Assimp
- SSAO (Screen Space Ambient Occlusion)
- HDR Bloom
- Camera: free movement camera and orbit camera

## Dependencies:
- [GLFW](https://www.glfw.org/) Window management and input handling
- [GLAD](https://glad.dav1d.de/) OpenGL function loading
- [glm](https://github.com/g-truc/glm) OpenGL mathematics library
- [imgui](https://github.com/ocornut/imgui) GUI

## TODO:
- add hot reload for shaders
- Shadows
- Deferred rendering
- Scene save/load 
- multiple lights and types

## Credits:

- learnopengl.com (https://learnopengl.com/)
- john-chapman-graphics blog (http://john-chapman-graphics.blogspot.com/2013/01/ssao-tutorial.html)
- PBR Kabuto [Samurai Helmet](https://sketchfab.com/3d-models/pbr-kabuto-samurai-helmet-13b3ab49b6bd4247be36b54ba1a56d8a) by [LeahLindner](https://sketchfab.com/illation) licensed under CC-BY-NC-ND-4.0 (http://creativecommons.org/licenses/by-nc-nd/4.0/)
