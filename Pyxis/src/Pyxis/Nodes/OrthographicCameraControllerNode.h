#pragma once

#include "Pyxis/Nodes/PixelCameraNode.h"
#include <Pyxis/Core/Input.h>
#include <Pyxis/Events/EventSignals.h>

namespace Pyxis {

// Node to act as a WASD camera controller.
class OrthographicCameraControllerNode : public PixelCameraNode {
  public:
    float m_CameraSpeed = 16;
    float m_Sensitivity = 0.5f;

    Reciever<void(MouseScrolledEvent &)> m_MouseScrolledReciever;

  public:
    OrthographicCameraControllerNode(
        const std::string &name = "OrthographicCameraControllerNode")
        : PixelCameraNode(name),
          m_MouseScrolledReciever(
              this, &OrthographicCameraControllerNode::OnMouseScrolledEvent) {
        EventSignal::s_MouseScrolledEventSignal.AddReciever(
            m_MouseScrolledReciever);
    };

    OrthographicCameraControllerNode(UUID id)
        : PixelCameraNode(id),
          m_MouseScrolledReciever(
              this, &OrthographicCameraControllerNode::OnMouseScrolledEvent) {
        EventSignal::s_MouseScrolledEventSignal.AddReciever(
            m_MouseScrolledReciever);
    };

    virtual ~OrthographicCameraControllerNode() = default;

    // Serialization
    virtual void Serialize(json &j) override {
        PixelCameraNode::Serialize(j);
        j["Type"] =
            "OrthographicCameraControllerNode"; // Override type identifier
        j["m_CameraSpeed"] = m_CameraSpeed;
        j["m_Sensitivity"] = m_Sensitivity;
    }

    virtual void Deserialize(json &j) override {
        PixelCameraNode::Deserialize(j);
        if (j.contains("m_CameraSpeed"))
            j.at("m_CameraSpeed").get_to(m_CameraSpeed);
        if (j.contains("m_Sensitivity"))
            j.at("m_Sensitivity").get_to(m_Sensitivity);
    }

    void OnMouseScrolledEvent(MouseScrolledEvent &e) {
        // if (!Input::IsKeyPressed(PX_KEY_LEFT_CONTROL)) {
        //     if (e.GetYOffset() < 0) {
        //         Zoom(2); // scroll down, increase size to zoom out
        //     } else {
        //         Zoom(0.5f);
        //     }
        // }
    }

    void Zoom(float multiplier) {
        SetWidth(GetWidth() * multiplier);
        SetHeight(GetHeight() * multiplier);
    }

    inline virtual void OnUpdate(Timestep ts) override {
        glm::mat4 RotationMatrix = GetRotationMatrix();
        // movement
        glm::vec4 direction = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

        if (Pyxis::Input::IsKeyPressed(PX_KEY_W)) {
            direction += RotationMatrix * glm::vec4(0, 1, 0, 1);
        }
        if (Pyxis::Input::IsKeyPressed(PX_KEY_A)) {
            direction -= RotationMatrix * glm::vec4(1, 0, 0, 1);
        }
        if (Pyxis::Input::IsKeyPressed(PX_KEY_S)) {
            direction -= RotationMatrix * glm::vec4(0, 1, 0, 1);
        }
        if (Pyxis::Input::IsKeyPressed(PX_KEY_D)) {
            direction += RotationMatrix * glm::vec4(1, 0, 0, 1);
        }

        Translate((glm::vec3)direction * m_CameraSpeed * ts.GetSeconds());
    }
};

REGISTER_SERIALIZABLE_NODE(OrthographicCameraControllerNode);
} // namespace Pyxis
