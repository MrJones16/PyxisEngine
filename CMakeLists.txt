cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_TOOLCHAIN_FILE "C:/Users/Peyton/Documents/GitHub/vcpkg/scripts/buildsystems/vcpkg.cmake")

project("PyxisWorkspace")

    add_subdirectory("Pyxis")
    #add_subdirectory("Pyxis-Game")
    add_subdirectory("Pixel-Game")
    add_subdirectory("Pixel-Game-Core")
    add_subdirectory("Pixel-Game-server")

    if (PX_DEBUG)
        target_compile_definitions(PyxisEngine PUBLIC PX_DEBUG)
    endif()

    
    #Specify that this is for windows
    target_compile_definitions(PyxisEngine PRIVATE PX_PLATFORM_WINDOWS)
    #target_compile_definitions(Pyxis-Game PRIVATE PX_PLATFORM_WINDOWS)
    target_compile_definitions(Pixel-Game PRIVATE PX_PLATFORM_WINDOWS)
    target_compile_definitions(Pixel-Game-Core PRIVATE PX_PLATFORM_WINDOWS)
    target_compile_definitions(Pixel-Game-Server PRIVATE PX_PLATFORM_WINDOWS)
    #for Dynamic linking
    #add_compile_definitions(PX_DYNAMIC_LINK PX_BUILD_DLL)
    target_compile_definitions(PyxisEngine PRIVATE GLFW_INCLUDE_NONE _CRT_SECURE_NO_WARNINGS _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS)