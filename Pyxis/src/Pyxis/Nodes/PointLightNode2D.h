#pragma once

#include <Pyxis/Nodes/B2BodyNode.h>
#include <Pyxis/Renderer/Renderer2D.h>

namespace Pyxis {
class PointLight2DNode : public B2BodyNode {
  public:
    PointLight2DNode(const std::string &name,
                     const glm::vec3 &Color = {1, 1, 1}, float Intensity = 1,
                     float Radius = 1, float FalloffPow = 2);
    PointLight2DNode(UUID id) : B2BodyNode(id) {}
    ~PointLight2DNode();

    void OnRender() override {
        Renderer2D::DrawPointLight(m_Position, m_Color, m_Intensity, m_Radius,
                                   m_Falloff);
    }

    void Serialize(json &j) override {
        Node2D::Serialize(j);
        j["Type"] = "PointLight2DNode"; // Override type identifier
        j["m_Color"] = m_Color;
        j["m_Intensity"] = m_Intensity;
        j["m_Radius"] = m_Radius;
        j["m_Falloff"] = m_Falloff;
    }
    void Deserialize(json &j) override {
        Node2D::Deserialize(j);
        if (j.contains("m_Color"))
            j.at("m_Color").get_to(m_Color);
        if (j.contains("m_Intensity"))
            j.at("m_Intensity").get_to(m_Intensity);
        if (j.contains("m_Radius"))
            j.at("m_Radius").get_to(m_Radius);
        if (j.contains("m_Falloff"))
            j.at("m_Falloff").get_to(m_Falloff);
    }

    glm::vec3 m_Color = {1, 1, 1};
    float m_Intensity = 1;
    float m_Radius = 1;
    float m_Falloff = 2;
};
REGISTER_SERIALIZABLE_NODE(PointLight2DNode);
} // namespace Pyxis
