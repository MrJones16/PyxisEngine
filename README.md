# Pyxis Engine

![Pyxis Engine Demo](PyxisGifWebp.webp)

Pyxis is a multiplayer falling sand simulation and game engine written in C++. The engine currently powers a falling sand simulation inspired by [Noita](https://store.steampowered.com/app/881100/Noita/), with the goal of evolving into either a sandbox survival game or a Little Big Planet-like creation platform.

## Project Structure

* **Pyxis Engine (Main)** - Core game engine that powers the falling sand simulation
* **Pyxis Game** - Multiplayer falling sand simulation game
* **Pyxis Editor** - Visual editor for the Pyxis Engine
* **Sandbox** - Testing environment for experimental features

## Development Status

> Pyxis is in active development, and features are continuously being added and improved, and broken.

## Key Features

**Multiplayer**
  * P2P gameplay using Steamworks API
  * Connect to friends via Steam Overlay
  * LAN and direct IP connection support
  
**Vast World & Integrated Physics**
  * Infinite world generation (theoretical)
  * Dynamic and static rigid bodies using Box2D integration
  * Rigidbodies integrated with the falling sand simulation with runtime deformation.
  
**Customizable & Dynamic**
  * Easy to make Elements written in JSON that are deserialized during runtime
  * Custom reactions also using deserialized JSON
  * Simple to create new Nodes in the game engine using inheritance
  
**Modular Architecture**
  * Event-driven design with custom event system
  * Hierarchy Node-based component framework
  * Abstracted rendering pipeline


## Technology Stack

### Core Libraries
* [OpenGL](https://www.opengl.org) - Graphics API
* [GLFW](https://github.com/glfw/glfw) - Window and input handling
* [GLAD](https://glad.dav1d.de) - OpenGL loader
* [GLM](https://github.com/g-truc/glm) - Mathematics library
* [ImGui](https://github.com/ocornut/imgui) - UI framework with experimental docking

### Physics & Simulation
* [Box2D](https://box2d.org) - 2D physics engine
* [Poly2Tri](https://github.com/jhasse/poly2tri) - Delaunay triangulation
* [FastNoiseLite](https://github.com/Auburn/FastNoiseLite) - Noise generation

### Networking & Data
* [Steamworks API](https://partner.steamgames.com/doc/sdk) - P2P networking
* [GameNetworkingSockets](https://github.com/ValveSoftware/GameNetworkingSockets) - Game networking
* [nlohmann-json](https://github.com/nlohmann/json) - JSON handling
* [Snappy](https://github.com/google/snappy) - Compression

### Utilities
* [spdlog](https://github.com/gabime/spdlog) - Logging
* [stb](https://github.com/nothings/stb) - Single-file utilities
* [FreeType](https://freetype.org/) - Font rendering
* [VCPKG](https://vcpkg.io/en/) - Dependency management

## Architecture Overview

The Pyxis Engine is built with a modular architecture that separates core functionality into distinct components:

### Core Systems
* Event-driven architecture with a custom event system
* Layer-based application structure for organizing functionality
* Resource management for assets and memory optimization
* Signal/slot mechanism for inter-component communication

### Rendering
* Abstracted renderer with OpenGL implementation
* 2D and 3D rendering capabilities
* Camera systems with orthographic and perspective options
* Material and shader management
* Framebuffer support for advanced rendering techniques

### UI System
* Node-based UI framework with various components:
  * Buttons, Text, Containers, Input fields
  * Hierarchical layout system
  * Event handling for UI interactions

### Physics
* Integration with Box2D for 2D physics
* Custom rigid body implementation
* Collision detection and response
* Chain-based deformable bodies

### Networking
* Steam integration for P2P connectivity
* Thread-safe message queuing
* Client-server architecture
* Optimized data serialization for network transfer

### Game Components
* Element-based particle system for falling sand simulation
* Chunk-based world management for large environments
* Scene management through hierarchical nodes
* Custom physics bodies for the falling sand simulation

This architecture provides a flexible foundation for different types of games, with a current focus on falling sand simulations with multiplayer capabilities.

## Knowledge Gained

Through developing this engine, I've gained extensive knowledge in:

### Project Management
* Pre-processor definitions and precompiled headers
* Core-App linking patterns
* Build system configuration (CMake, Premake5)

### Programming Techniques
* Linear algebra for graphics programming
* Header-implementation separation
* Type casting (static vs. dynamic)
* Smart pointers (shared, weak)
* Event-driven programming
* Thread-safe concurrent data structures
* Resource management
* Node-based system design

### Graphics Programming
* Rendering pipeline implementation
* GLSL shader development
* Lighting models (Phong)
* Mesh generation and manipulation
* Framebuffer techniques
* Font rendering

### Game Development
* Particle systems
* Multiplayer networking
* Chunk-based world management
* Physics simulation
* Custom UI framework development

## Installation & Building

### Prerequisites
- CMake 3.15+
- C++17 compatible compiler
- [VCPKG](https://vcpkg.io) for dependency management

### Building

1. Clone the repository
   ```
   git clone https://github.com/YourUsername/PyxisEngine.git
   cd PyxisEngine
   ```

2. Configure and build with CMake
   ```
   # Windows
   .\Windows-CMake-VS17.bat
   
   # Manual configuration
   cmake -B build
   cmake --build build
   ```

3. Run the sandbox or game
   ```
   .\build\bin\Pyxis-Game.exe
   ```



## Acknowledgements

This engine was initially inspired by [TheCherno](https://www.youtube.com/@TheCherno)'s Hazel engine series on YouTube. While the base engine architecture shares similarities, Pyxis has evolved with its own unique features and design choices.

A lot of my very early education was thanks to [LearnOpenGL](https://learnopengl.com), and it has continued to be an amazing resource.

A good portion of trying to mimic how Noita was even possible was thanks to the developers sharing their wisdom through a [gdc talk](https://www.youtube.com/watch?v=prXuyMCgbTc)

## License

Pyxis Engine is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

```
MIT License

Copyright (c) 2024 Peyton

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files...
```