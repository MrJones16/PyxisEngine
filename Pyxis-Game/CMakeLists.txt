#source files, all of them!
add_executable(Pyxis-Game 
    "../Pixel-Game-Core/src/GameLayer.cpp"
    "src/PyxisGameApp.cpp"
    "src/MenuNode.cpp"

 "../Pixel-Game-Core/src/SingleplayerGameLayer.h" "../Pixel-Game-Core/src/SingleplayerGameLayer.cpp" "../Pixel-Game-Core/src/HostingGameLayer.h" "../Pixel-Game-Core/src/HostingGameLayer.cpp" "../Pixel-Game-Core/src/MultiplayerGameLayer.h" "../Pixel-Game-Core/src/MultiplayerGameLayer.cpp"
 "src/UIScreenSpace.h")

if(WIN32)
    # for Windows operating system in general
    if (PX_DIST)
        target_link_options(Pyxis-Game PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
    endif()
endif()


#include directories
include_directories(
    src
    src/Panels
)



#Other linked projects
target_link_libraries(Pyxis-Game PRIVATE PyxisEngine Pixel-Game-Core)

add_custom_command(TARGET Pyxis-Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/../Pixel-Game-Core/assets
                ${CMAKE_CURRENT_BINARY_DIR}/assets)

add_custom_command(TARGET Pyxis-Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/../steam_appid.txt
                ${CMAKE_CURRENT_BINARY_DIR})

if (WIN32)
    add_custom_command(TARGET Pyxis-Game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${CMAKE_CURRENT_SOURCE_DIR}/../Pyxis/Src/Steam/win64/steam_api64.dll
                    ${CMAKE_CURRENT_BINARY_DIR})
endif()