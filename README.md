
# Pyxis Game Engine

A game engine written in C++ for learning purposes. The goal is to make an adorable 2D game engine engine with an aseprite-like theme.!

## About

This engine was made by following [TheCherno](https://www.youtube.com/@TheCherno)'s game engine series on Youtube, where he creates the Hazel game engine. For this reason, the code will be almost, if not exactly, the same as it is there.

The project uses [Premake 5](https://github.com/premake/premake-core) to build solution files, and currently has support for windows, but should be feasable to make Linux or Mac support using it.


## Currently Used Frameworks/Libraries/Software/ect.

* OpenGL

* [GLFW](https://github.com/glfw/glfw) (GLFW is an Open Source, multi-platform library for OpenGL)

* [GLAD](https://glad.dav1d.de) (Vulkan/GL/GLES/EGL/GLX/WGL Loader-Generator)

* [GLM](https://github.com/g-truc/glm) (GLM is an OpenGL based C++ mathematics library for graphics software)

* [ImGui](https://github.com/ocornut/imgui) (Dear ImGui is a bloat-free graphical user interface library for C++)

* [spdlog](https://github.com/gabime/spdlog) (Very fast, header-only/compiled, C++ logging library.)

* [tinyXML2](https://github.com/leethomason/tinyxml2) (TinyXML-2 is a simple, small, efficient, C++ XML parser)

* (Planned) ASSIMP (In repository, not yet implemented) 

## What I've Learned

I've learned a lot about:

**Projects:**

* Pre-Processor Definitions

* Precompiled Headers

* Core - App Linking

* Solution Building Through Premake5 / Cmake / ect

**Programming**
* Using linear algebra for lighting calculations and different space matrices
* A lot of practice with Header Definitions and CPP Implementations
* Picking a style and sticking to it (cleaner and more consistent code)
* Static Cast(Compile Time) vs Dynamic Cast (Runtime, does inheritance check)

**OpenGL/GLAD/GLFW/ImGui**
* Rendering / Graphics pipeline
* Creating and using GLSL shaders
* Phong Lighting
* More practice with meshes, vertices, triangles, ect
* ImGui GUI rendering

## What it looks like  now:
![A peek at the Engine](peek.png)