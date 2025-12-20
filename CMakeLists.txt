cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#PRIOR, ON WINDOWS
if(WIN32)
    set(CMAKE_TOOLCHAIN_FILE "C:/Users/Peyton/Documents/GitHub/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

if(UNIX)
    set(CMAKE_TOOLCHAIN_FILE "~/git/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
endif()

project("PyxisWorkspace")

    add_subdirectory("Pyxis")
    add_subdirectory("Pyxis-Game")

    add_subdirectory("Pixel-Game-Core")
    
    # Make compilier more strict
    #add_compile_options(
    #    $<$<CXX_COMPILER_ID:GNU,Clang>:
    #    -Wall -Wextra -Wpedantic -Wreturn-type
    #    >
    #)

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(PyxisEngine PUBLIC PX_DEBUG)
        
        #target_link_options(Pyxis-Game PRIVATE
        #-fsanitize=address,undefined
        #)
    endif()
    if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        target_compile_definitions(PyxisEngine PUBLIC PX_RELEASE)
    endif()
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_definitions(PyxisEngine PUBLIC PX_DIST)
    endif()

    
    #Specify the current platform!
    if(WIN32)
        target_compile_definitions(PyxisEngine PRIVATE PX_PLATFORM_WINDOWS=1)
        target_compile_definitions(Pyxis-Game PRIVATE PX_PLATFORM_WINDOWS=1)
        target_compile_definitions(Pixel-Game-Core PRIVATE PX_PLATFORM_WINDOWS=1)
    elseif(APPLE)
        target_compile_definitions(PyxisEngine PRIVATE PX_PLATFORM_APPLE=1)
        target_compile_definitions(Pyxis-Game PRIVATE PX_PLATFORM_APPLE=1)
        target_compile_definitions(Pixel-Game-Core PRIVATE PX_PLATFORM_APPLE=1)
    else(UNIX)
        target_compile_definitions(PyxisEngine PRIVATE PX_PLATFORM_LINUX=1)
        target_compile_definitions(Pyxis-Game PRIVATE PX_PLATFORM_LINUX=1)
        target_compile_definitions(Pixel-Game-Core PRIVATE PX_PLATFORM_LINUX=1)
    endif()
    


    #for Dynamic linking
    #add_compile_definitions(PX_DYNAMIC_LINK PX_BUILD_DLL)
    target_compile_definitions(PyxisEngine PRIVATE GLFW_INCLUDE_NONE _CRT_SECURE_NO_WARNINGS _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS)
