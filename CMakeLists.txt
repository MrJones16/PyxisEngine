cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#PRIOR, ON WINDOWS
#set(CMAKE_TOOLCHAIN_FILE "C:/Users/Peyton/Documents/GitHub/vcpkg/scripts/buildsystems/vcpkg.cmake")

set(CMAKE_TOOLCHAIN_FILE "~/git/vcpkg/scripts/buildsystems/vcpkg.cmake")

project("PyxisWorkspace")

    add_subdirectory("Pyxis")
    add_subdirectory("Pyxis-Game")
    add_subdirectory("Pixel-Game-Core")

    if (PX_DEBUG)
        target_compile_definitions(PyxisEngine PUBLIC PX_DEBUG)
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