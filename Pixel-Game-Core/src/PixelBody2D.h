#pragma once

#include "Element.h"
#include "Pyxis/Game/PhysicsBody2D.h"
#include "VectorHash.h"
#include <Pyxis/Nodes/PhysicsBodyNode2D.h>

/// A pixel body is a rigidbody2D , aka a B2Body.
///
/// The underlying b2body position is different than the world position,
/// because control over the PPU is helpful for deciding how many pixels
/// should be considered a 1x1 cube, because box2D is optimized for 1-50
/// i believe.
///

namespace Pyxis {
/// <summary>
/// this is based on how many pixels represent 1 unit.
/// if an object had a velocity of -10, it would fall 10 pixels at
/// a PPU of 1, whereas it would fall 100 pixels at a PPU of 10;
///
/// basically, what matters is this is the value of an average
/// width / size pixel body. like a 1x1 box is this wide/tall
///
/// I have it at 16 cause a 16x16 box is a box to me!
/// </summary>

struct PixelBodyElement {
    PixelBodyElement() {}
    PixelBodyElement(Element e) : element(e) {}
    PixelBodyElement(Element e, glm::ivec2 worldPosition)
        : element(e), worldPos(worldPosition) {}
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

/*struct PixelBodyData
{
        b2Vec2 linearVelocity;
        float angularVelocity;
        float angle;
        b2Vec2 position;
};*/

/// <summary>
/// A rigid body, but it is tied to elements in the simulation.
///
/// When deserializing, you must set the m_PXWorld afterwards.
/// </summary>
class PixelBody2D : public PhysicsBodyNode2D {
  protected:
    bool m_InWorld = true;
    bool m_Moved =
        false; // track if any elements moved during position updates.

    int m_Width = 0;
    int m_Height = 0;

    // Data needed for bit arrays
    glm::ivec2 m_LocalMinimum = {0, 0};
    std::unordered_map<glm::ivec2, std::vector<uint64_t>, VectorHash>
        m_BitArrays;

    /// <summary>
    /// m_Elements holds the LOCAL positions about the center.
    /// </summary>
    std::unordered_map<glm::ivec2, PixelBodyElement, VectorHash> m_Elements;
    bool m_PixelsChanged = false;

    bool m_DebugDisplay = false;
    float m_DebugDisplayZ = 25.0f;

    // need to override the queue free in case i want to call it from node
    // itself.
    bool m_FreeMe = false;

  public:
    float m_SimulationScale = 1.0f / PPU;
    float m_InverseSimulationScale = PPU;

    /// <summary>
    ///
    /// </summary>
    /// <param name="elements"> elements should contain the element, and their
    /// pixel position in the world</param>
    PixelBody2D(const std::string &name = "PixelBody2D",
                PhysicsBody2DType type = PhysicsBody2DType::Dynamic);
    PixelBody2D(UUID id);

    // Set elements for this pixel body. Assumes pixels are in world already.
    void SetPixelBodyElements(std::vector<PixelBodyElement> &elements);

    // Serialize & Deserialize
    virtual void Serialize(json &j) override;
    virtual void Deserialize(json &j) override;

    // needed as world object holds refs to this
    virtual void QueueFree() override;

    void GenerateMesh();

    // virtual void OnUpdate(Timestep ts) override;
    // virtual void OnFixedUpdate() override;
    // virtual void OnRender() override;

    /// <summary>
    /// Uses the physics body to update where the elements would be in the
    /// world.
    /// </summary>
    void UpdateElementWorldPositions();

    ////////////////////////////////////////////////
    /// Overrides for transforms, since
    /// Pixel bodies display in world,
    /// so the PPU and chunksize are not aligned!
    ////////////////////////////////////////////////

    virtual glm::mat4 GetWorldTransform() override;

    virtual void Translate(const glm::vec2 &translation) override;
    virtual void SetPosition(const glm::vec2 &position) override;
    virtual glm::vec2 GetPosition() override;

  protected:
    // helper functions for space transforming and such
    glm::mat4 GetWorldToLocalTransform();
    glm::mat2x2 GetLocalToWorldTransform();

    friend class World;
    inline void ActuallyQueueFree() { PhysicsBodyNode2D::QueueFree(); }

  private:
    glm::vec2 GetLocalPixelVelocity(glm::ivec2 localPosition);
};

REGISTER_SERIALIZABLE_NODE(PixelBody2D);

} // namespace Pyxis
