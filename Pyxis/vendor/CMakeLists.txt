# Vendor libraries

# ASIO
add_subdirectory(asio)

# GLFW
add_subdirectory(glfw)

# Glad
add_subdirectory(GLAD)

# ImGui
add_subdirectory(ImGui)

# GLM
add_subdirectory(glm)

# SPDLOG
add_subdirectory(spdlog)

# Box2D
add_subdirectory(box2d)

# Poly2Tri
add_subdirectory(poly2tri)

#Game Networking Sockets
#add_subdirectory(GameNetworkingSockets)
target_link_directories(PyxisEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/GameNetworkingSockets/build/bin/Debug)
target_link_libraries (PyxisEngine PUBLIC GameNetworkingSockets)
#target_link_libraries(PyxisEngine ${CMAKE_CURRENT_SOURCE_DIR}/GameNetworkingSockets/build/bin/Debug/GameNetworkingSockets.dll)

# Include directories for vendor libraries
target_include_directories(PyxisEngine
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/GameNetworkingSockets/include
    ${CMAKE_CURRENT_SOURCE_DIR}/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/poly2tri
    ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include
    ${CMAKE_CURRENT_SOURCE_DIR}/asio/asio/include
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyxml2
    ${CMAKE_CURRENT_SOURCE_DIR}/stb_image
    ${CMAKE_CURRENT_SOURCE_DIR}/box2d/include
    ${CMAKE_CURRENT_SOURCE_DIR}/SteamNetworkingSockets/include
)

target_sources(PyxisEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/stb_image/stb_image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyxml2/tinyxml2.cpp
)