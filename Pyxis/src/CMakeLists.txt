# Define the Core library
add_library(PyxisEngine)



#set(VCPKG_CRT_LINKAGE static)
#set(VCPKG_LIBRARY_LINKAGE static)
find_package(box2d CONFIG REQUIRED)
#find_package(GameNetworkingSockets CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(poly2tri CONFIG REQUIRED)
find_package(Snappy CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(stb REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)

#target_compile_definitions(PyxisEngine PUBLIC STEAMNETWORKINGSOCKETS_STATIC_LINK)
#linking and including Steamworks SDK
target_include_directories(PyxisEngine PUBLIC Steam)
target_link_directories(PyxisEngine PUBLIC Steam)
target_link_libraries(PyxisEngine PUBLIC steam_api)
if(WIN32)
    target_link_directories(PyxisEngine PUBLIC Steam/win64)
    target_link_libraries(PyxisEngine PUBLIC steam_api64)
endif()


# GameNetworkingSockets static library requires both static and GameNetworkingSockets::GameNetworkingSockets targets
target_link_libraries(PyxisEngine 
    PUBLIC 
    box2d::box2d
    #GameNetworkingSockets::static
    #GameNetworkingSockets::GameNetworkingSockets
    glad::glad
    glfw
    glm::glm
    imgui::imgui
    poly2tri::poly2tri
    Snappy::snappy
    spdlog::spdlog
    tinyxml2::tinyxml2
    Freetype::Freetype
)


# Set the include directory to current directory.
target_include_directories(PyxisEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


# Add all the files from the @Platform and @Pyxis directories
target_sources(PyxisEngine PRIVATE
    # @Platform files
    ## OpenGL
    Platform/OpenGL/OpenGLContext.cpp
    Platform/OpenGL/OpenGLContext.h
    Platform/OpenGL/OpenGLBuffer.cpp
    Platform/OpenGL/OpenGLBuffer.h
    Platform/OpenGL/OpenGLShader.cpp
    Platform/OpenGL/OpenGLShader.h
    Platform/OpenGL/OpenGLRendererAPI.cpp
    Platform/OpenGL/OpenGLRendererAPI.h
    Platform/OpenGL/OpenGLTexture.cpp
    Platform/OpenGL/OpenGLTexture.h
    Platform/OpenGL/OpenGLFrameBuffer.cpp
    Platform/OpenGL/OpenGLFrameBuffer.h
    Platform/OpenGL/OpenGLVertexArray.cpp
    Platform/OpenGL/OpenGLVertexArray.h

    ## Windows
    Platform/Windows/WindowsInput.cpp
    Platform/Windows/WindowsWindow.cpp
    Platform/Windows/WindowsWindow.h

    # @Pyxis files
    ## Core
    Pyxis/Core/Application.cpp
    Pyxis/Core/Application.h
    Pyxis/Core/Core.h
    Pyxis/Core/EntryPoint.h
    Pyxis/Core/Input.h
    Pyxis/Core/InputCodes.h
    Pyxis/Core/Layer.cpp
    Pyxis/Core/Layer.h
    Pyxis/Core/LayerStack.cpp
    Pyxis/Core/LayerStack.h
    Pyxis/Core/Log.cpp
    Pyxis/Core/Log.h
    Pyxis/Core/MathLog.h
    Pyxis/Core/OrthographicCameraController.h
    "Pyxis/Core/Panel.h"
    Pyxis/Core/PerspectiveCameraController.h
    Pyxis/Core/ProfilingPanel.h
    Pyxis/Core/Timestep.h
    Pyxis/Core/Window.h

    #Events
    Pyxis/Events/ApplicationEvent.h
    Pyxis/Events/Event.h
    Pyxis/Events/KeyEvent.h
    Pyxis/Events/MouseEvent.h

    #fastnoiselite
    Pyxis/FastNoiseLite/FastNoiseLite.h

    #Game
    "Pyxis/Game/Node.cpp"
    "Pyxis/Game/Node.h"
    Pyxis/Game/Scene.cpp
    Pyxis/Game/Scene.h

    #ImGui
    Pyxis/ImGui/ImGuiBuild.cpp
    Pyxis/ImGui/ImGuiLayer.cpp
    Pyxis/ImGui/ImGuiLayer.h

    ## Network
    
    
    Pyxis/Network/NetworkClient.cpp
    Pyxis/Network/NetworkClient.h
    Pyxis/Network/NetworkMessage.h
    Pyxis/Network/NetworkServer.cpp
    Pyxis/Network/NetworkServer.h
    Pyxis/Network/NetworkThreadSafeQueue.cpp
    Pyxis/Network/NetworkThreadSafeQueue.h

    ## Renderer
    Pyxis/Renderer/Buffer.cpp
    Pyxis/Renderer/Buffer.h
    Pyxis/Renderer/Camera.cpp
    Pyxis/Renderer/Camera.h
    Pyxis/Renderer/FrameBuffer.cpp
    Pyxis/Renderer/FrameBuffer.h
    Pyxis/Renderer/GraphicsContext.h
    Pyxis/Renderer/Material.h
    Pyxis/Renderer/RenderCommand.cpp
    Pyxis/Renderer/RenderCommand.h
    Pyxis/Renderer/Renderer.cpp
    Pyxis/Renderer/Renderer.h
    Pyxis/Renderer/Renderer2D.cpp
    Pyxis/Renderer/Renderer2D.h
    Pyxis/Renderer/RendererAPI.cpp
    Pyxis/Renderer/RendererAPI.h
    Pyxis/Renderer/Shader.cpp
    Pyxis/Renderer/Shader.h
    Pyxis/Renderer/SubTexture2D.cpp
    Pyxis/Renderer/SubTexture2D.h
    Pyxis/Renderer/Texture.cpp
    Pyxis/Renderer/Texture.h
    Pyxis/Renderer/VertexArray.cpp
    Pyxis/Renderer/VertexArray.h

      "Pyxis/Renderer/UI.h"  "Pyxis/Renderer/UINode.h" "Pyxis/Renderer/UIRect.h" "Pyxis/Renderer/UIText.h" "Pyxis/Renderer/Font.h" "Pyxis/Renderer/Font.cpp" "Pyxis/Renderer/UIButton.h" "Pyxis/Renderer/UIContainer.h")
