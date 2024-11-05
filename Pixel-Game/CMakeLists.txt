#source files, all of them!
add_executable(Pixel-Game 
    src/GameLayer.cpp
    src/PixelGameApp.cpp
    src/MenuLayer.cpp

)


#include directories
include_directories(
    src
    src/Panels
)

#Other linked projects
target_link_libraries(Pixel-Game PRIVATE PyxisEngine Pixel-Game-Core)

add_custom_command(TARGET Pixel-Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/../Pixel-Game-Core/assets
                ${CMAKE_CURRENT_BINARY_DIR}/assets)