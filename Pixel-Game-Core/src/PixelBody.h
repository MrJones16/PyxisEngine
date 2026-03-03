#pragma once
#include <Element.h>
#include <Pyxis/Game/PhysicsBody2D.h>
#include <VectorHash.h>

using UUID = uint32_t; // remove me

namespace Pyxis {

struct PixelBodyElement {
  public:
    PixelBodyElement() {}
    PixelBodyElement(Element e) : element(e) {}
    Element element = Element();
    glm::ivec2 worldPos = {0, 0};

    // Write to_json and from_json functions
    friend void to_json(json &j, const PixelBodyElement &pbe) {
        j = json{{"Element", pbe.element}, {"WorldPos", pbe.worldPos}};
    }
    friend void from_json(const json &j, PixelBodyElement &pbe) {
        j.at("Element").get_to(pbe.element);
        j.at("WorldPos").get_to(pbe.worldPos);
    }
};

// previously was derived from physics node, but I want to separate that.
class PixelBody : PhysicsBody2D {
  public:
    PixelBody(b2WorldId worldId, PhysicsBody2DType type);

    void AddElement(const Element &e, const glm::ivec2 &worldPosition);
    void RecalculatePhysicsBody();

  protected:
    // helper functions for space transforming and such
    glm::mat4 GetWorldToLocalTransform();
    glm::mat4 GetLocalToWorldTransform();

  protected:
    std::unordered_map<glm::ivec2, PixelBodyElement, VectorHash> m_Elements;

    friend class World;
};

} // namespace Pyxis
