add_library(Pixel-Game-Core)

target_sources(Pixel-Game-Core PRIVATE
	src/Chunk.cpp
	src/Chunk.h
	src/Element.h
	
	src/VectorHash.h
	src/World.cpp
	src/World.h

 "src/PixelNetworking.h" "../Pyxis-Game/src/GameNode.cpp" "../Pyxis-Game/src/GameNode.h" "../Pyxis-Game/src/SingleplayerGameNode.h" "../Pyxis/src/Pyxis/Nodes/RigidBody2D.h"    "src/PixelBody2D.h" "src/PixelBody2D.cpp")

target_include_directories(Pixel-Game-Core PUBLIC
	src
)

target_link_libraries(Pixel-Game-Core PRIVATE PyxisEngine)