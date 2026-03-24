#pragma once
#include "PixelBody2D.h"
#include "Pyxis/Game/PhysicsBody2D.h"
#include <Pyxis/Events/EventSignals.h>

#include <Pyxis/Nodes/CameraNode.h>

namespace Pyxis {
class Player : public PixelBody2D {
  public:
    Reciever<void(KeyPressedEvent &)> m_KeyPressedReciever;

    Player(const std::string &name, PhysicsBody2DType type)
        : PixelBody2D(name, type),
          m_KeyPressedReciever(this, &Player::OnKeyPressedEvent) {
        EventSignal::s_KeyPressedEventSignal.AddReciever(m_KeyPressedReciever);

        // TODO: add fixed rotation
    }

    Player(UUID id)
        : PixelBody2D(id),
          m_KeyPressedReciever(this, &Player::OnKeyPressedEvent) {
        EventSignal::s_KeyPressedEventSignal.AddReciever(m_KeyPressedReciever);
        // TODO: add fixed rotation
    }

    ~Player() {}

    void Serialize(json &j) override {
        PixelBody2D::Serialize(j);
        j["Type"] = "Player";
    }

    void Deserialize(json &j) override { PixelBody2D::Deserialize(j); }

    void OnKeyPressedEvent(KeyPressedEvent &e) {
        PX_TRACE("Key Pressed: {0}", e.GetKeyCode());
        if (e.GetKeyCode() == PX_KEY_W) {
            AddLinearVelocity({0, 0.3f});
        }

        // Save to file this player, so we can debug test easier by loading it
        if (e.GetKeyCode() == PX_KEY_J) {
            // open a file and write to it the json
            std::ofstream file("assets/Player.json");
            json j;
            Serialize(j);
            file << j.dump(4);
            file.close();
        }
    }

    void OnUpdate(Timestep ts) override {
        PixelBody2D::OnUpdate(ts);

        // Camera follow
        CameraNode *camera = dynamic_cast<CameraNode *>(CameraNode::Main());
        camera->SetPosition({GetPosition().x, GetPosition().y, 0});
    }

    /*void OnFixedUpdate() override
    {
            PixelBody2D::OnFixedUpdate();
    }*/

    /*void OnInspectorRender() override
    {
            PixelBody2D::OnInspectorRender();
    }*/

    /*void OnRender() override
    {
            PixelBody2D::OnRender();
    }*/
};
REGISTER_SERIALIZABLE_NODE(Player);
} // namespace Pyxis
