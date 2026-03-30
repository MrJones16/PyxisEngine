#pragma once

#include <Pyxis/Core/Core.h>
#include <box2d/box2d.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace Pyxis {
enum PhysicsBody2DType : uint8_t { Dynamic, Kinematic, Static };

// A wrapper for box2d bodies. The main reason for doing this, is so that:
// 1. could be slightly easier to replace box2d in future if somehow needed
// 2. So that I can implement a way to refresh / recreate the box2d worlds, as
// that is needed!
struct PhysicsBody2D {
  public:
    PhysicsBody2D(b2WorldId worldId, PhysicsBody2DType type,
                  const glm::vec2 &position = {0, 0}, float angle = 0);
    PhysicsBody2D(b2WorldId worldId, json &j);
    ~PhysicsBody2D();

    // draws outlines of shapes with drawline, and draws a quad right at center
    // pos.
    void DebugDraw(float depth = 10, float scale = 1.0f);

    // doesn't remove from the prior world!
    void CopyToWorld(b2WorldId worldId);

    // main wrapper funcs for body interaction
    void SetType(PhysicsBody2DType type);
    PhysicsBody2DType GetType() const;

    void SetAwake(bool awake);
    bool GetAwake();

    void SetPosition(const glm::vec2 &position);
    glm::vec2 GetPosition() const;

    void SetLinearVelocity(const glm::vec2 &velocity);
    glm::vec2 GetLinearVelocity() const;

    void SetLinearDamping(float damping);
    float GetLinearDamping() const;

    void SetRotation(float angleInRadians);
    float GetRotation() const;

    void SetAngularVelocity(float angularVelocity);
    float GetAngularVelocity() const;

    void SetAngularDamping(float angularDamping);
    float GetAngularDamping() const;

    // forces
    void ApplyForce(const glm::vec2 &force, const glm::vec2 &point,
                    bool wake = true);
    void ApplyTorque(float torque, bool wake = true);

    // shapes
    void RemoveShapes();
    void AddBoxShape(float halfwidth, float halfheight, const glm::vec2 &center,
                     float radians);

    // Serialization
    void Serialize(json &j);

    // Updates the body definition with present json values
    void Deserialize(json &j);

  protected:
    void UpdateBodyDefinition();

    static uint32_t s_IDCounter;
    uint32_t m_ID;
    b2BodyDef m_B2BodyDefinition;
    b2BodyId m_B2BodyId;

    friend class PhysicsWorld2D;
};
} // namespace Pyxis
