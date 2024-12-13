Project(Pixel-Game-Server VERSION 0.1)

add_executable(Pixel-Game-Server)

target_sources(Pixel-Game-Server PRIVATE
	#Main Sources
	src/PixelGameServer.cpp
	src/PixelGameServer.h
	src/PixelGameServerApp.cpp

)
include_directories(src ../Pixel-Game/src)

target_link_libraries(Pixel-Game-Server PRIVATE PyxisEngine Pixel-Game-Core)

add_custom_command(TARGET Pixel-Game-Server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/../Pixel-Game-Core/assets
                ${CMAKE_CURRENT_BINARY_DIR}/assets)

				add_custom_command(TARGET Pixel-Game-Server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/../steam_appid.txt
                ${CMAKE_CURRENT_BINARY_DIR})

if (WIN32)
    add_custom_command(TARGET Pixel-Game-Server POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${CMAKE_CURRENT_SOURCE_DIR}/../Pyxis/Src/Steam/win64/steam_api64.dll
                    ${CMAKE_CURRENT_BINARY_DIR})
endif()