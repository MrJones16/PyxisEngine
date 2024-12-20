add_library(Pixel-Game-Core)

target_sources(Pixel-Game-Core PRIVATE
	src/Chunk.cpp
	src/Chunk.h
	src/Element.h
	
	src/PixelRigidBody.cpp
	src/PixelRigidBody.h
	src/VectorHash.h
	src/World.cpp
	src/World.h

	src/GameLayer.cpp
	src/GameLayer.h
	src/SingleplayerGameLayer.cpp
	src/SingleplayerGameLayer.h
	src/MultiplayerGameLayer.cpp
	src/MultiplayerGameLayer.h
	src/HostingGameLayer.cpp
	src/HostingGameLayer.h


 "src/PixelNetworking.h")

target_include_directories(Pixel-Game-Core PUBLIC
	src
)

target_link_libraries(Pixel-Game-Core PRIVATE PyxisEngine)