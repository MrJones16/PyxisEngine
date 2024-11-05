# Define the Core library
add_library(PyxisEngine)

target_link_libraries(PyxisEngine
    PUBLIC
    #asio
    glfw
    glad
    imgui
    glm
    spdlog
    box2d
    poly2tri
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
    Pyxis/Core/Panel.h
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
    Pyxis/Game/Entity.cpp
    Pyxis/Game/Entity.h
    Pyxis/Game/Scene.cpp
    Pyxis/Game/Scene.h

    #ImGui
    Pyxis/ImGui/ImGuiBuild.cpp
    Pyxis/ImGui/ImGuiLayer.cpp
    Pyxis/ImGui/ImGuiLayer.h

    ## Network
    Pyxis/Network/Network.cpp
    Pyxis/Network/Network.h
    Pyxis/Network/NetworkClient.cpp
    Pyxis/Network/NetworkClient.h
    Pyxis/Network/NetworkCommon.cpp
    Pyxis/Network/NetworkCommon.h
    Pyxis/Network/NetworkConnection.cpp
    Pyxis/Network/NetworkConnection.h
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
    
)
