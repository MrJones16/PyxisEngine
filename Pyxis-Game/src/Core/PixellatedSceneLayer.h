#pragma once

#include "Pyxis/Nodes/PixelCameraNode.h"
#include <Pyxis/Core/Layer.h>
#include <Pyxis/Renderer/FrameBuffer.h>

#include <Pyxis/Nodes/CameraNode.h>
#include <Pyxis/Nodes/Node.h>
#include <Pyxis/Nodes/UI/UI.h>

#include <Pyxis/Events/EventSignals.h>

namespace Pyxis {
class PixellatedSceneLayer : public Layer {
  public:
    PixellatedSceneLayer(bool debug = false);
    virtual ~PixellatedSceneLayer();

    // Layer functions
    virtual void OnAttach() override;
    virtual void OnDetach() override;

    virtual void OnUpdate(Timestep ts) override;
    virtual void OnImGuiRender() override;
    virtual void OnEvent(Event &e) override;

    glm::ivec2 GetMousePositionImGui();

    bool OnWindowResizeEvent(WindowResizeEvent &event);
    bool OnKeyPressedEvent(KeyPressedEvent &event);
    bool OnMouseButtonPressedEvent(MouseButtonPressedEvent &event);
    bool OnMouseButtonReleasedEvent(MouseButtonReleasedEvent &event);
    bool OnMouseScrolledEvent(MouseScrolledEvent &event);

    // std::pair<float, float> GetMousePositionScene();

  public:
    // scene

    // List of now null nodes to be removed
    std::queue<uint32_t> m_NullNodeQueue;
    PixelCameraNode *m_PixelCamera;

  private:
    // debug heirarchy / inspector
    bool m_Debug = false;
    virtual void DrawNodeTree(Ref<Node> Node);
    Ref<Node> m_SelectedNode;

    // fixed update
    double m_FixedUpdateRate = 60.0f;
    std::chrono::time_point<std::chrono::high_resolution_clock>
        m_FixedUpdateTime = std::chrono::high_resolution_clock::now();

    // Other

    uint32_t m_HoveredNodeID = 0;
};
} // namespace Pyxis
