#pragma once

#include <Pyxis/Game/PhysicsBody2D.h>
#include <box2d/id.h>
#include <box2d/types.h>
#include <map>
#include <unordered_set>

namespace Pyxis {
/// <summary>
/// A static wrapper for a B2World system, which would allow for any rigid body
/// object or b2body implementation to create itself
///
/// Nodes must manage the PhysicsWorld2D themselves, it is not handled by the
/// engine or the scene layer. So you must use like a physics manager to update
/// the physics. which also allows for more flexibility for physics updating &
/// use
///
/// </summary>
class PhysicsWorld2D {
  private:
    b2WorldDef m_B2WorldDef = b2DefaultWorldDef();
    b2WorldId m_B2WorldId = b2_nullWorldId;
    std::unordered_map<uint32_t, WeakRef<PhysicsBody2D>> m_Bodies;

  public:
    int m_SubSteps = 4;
    float m_Step = 1.0f / 60.0f;

    PhysicsWorld2D() = default;
    PhysicsWorld2D(const glm::vec2 &gravity, int subSteps);
    ~PhysicsWorld2D();

    b2WorldId GetWorld();
    bool IsValid();

    void ResetWorld();
    void Step();

    int GetBodyCount();

    Ref<PhysicsBody2D> CreateBody(PhysicsBody2DType type,
                                  const glm::vec2 &position,
                                  float angleInRadians);
    Ref<PhysicsBody2D> CreateBody(json &j);

    friend class Physics2D;
};

} // namespace Pyxis
