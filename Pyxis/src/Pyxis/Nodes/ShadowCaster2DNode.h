#pragma once

#include <Pyxis/Nodes/B2BodyNode.h>
#include <Pyxis/Renderer/Renderer2D.h>

namespace Pyxis {
class ShadowCaster2DNode : public B2BodyNode {
  public:
    ShadowCaster2DNode(const std::string &name,
                       const glm::vec3 &Color = {1, 1, 1}, float Intensity = 1,
                       float Radius = 1, float FalloffPow = 2);
    ShadowCaster2DNode(UUID id) : B2BodyNode(id) {}
    ~ShadowCaster2DNode();

    void OnRender() override {
        // don't do anything on standard Render loop for nodes. This should be
        // manually rendered by lights on per-light basis.
    }

    void RenderShadowCaster() {}

    // take in the triangulated points, and double the vertices so that the
    // normals on them can be extruded.
    void UpdateShadowCaster() {}

    void Serialize(json &j) override {
        B2BodyNode::Serialize(j);
        j["Type"] = "ShadowCaster2DNode"; // Override type identifier
        // j["m_Color"] = m_Color;
    }
    void Deserialize(json &j) override {
        B2BodyNode::Deserialize(j);
        // if (j.contains("m_Color"))
        //     j.at("m_Color").get_to(m_Color);
    }
};
REGISTER_SERIALIZABLE_NODE(ShadowCaster2DNode);
} // namespace Pyxis
