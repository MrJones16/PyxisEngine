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