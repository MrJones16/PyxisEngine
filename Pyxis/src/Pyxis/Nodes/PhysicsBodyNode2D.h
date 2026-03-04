#pragma once

#include "Pyxis/Game/PhysicsBody2D.h"
#include <Pyxis/Game/Physics2D.h>
#include <Pyxis/Nodes/Node2D.h>
#include <poly2tri.h>

namespace Pyxis {
// UUID uuid, const glm::ivec2& size, std::unordered_map<glm::ivec2,
// RigidBodyElement, HashVector> elements, b2BodyType type, b2World* world)

/// <summary>
/// A PhysicsBodyNode2D is a node that has an underlying PHysicsBody2D
/// </summary>
class PhysicsBodyNode2D : public Node2D {
  protected:
    Ref<PhysicsBody2D> m_PhysicsBody;

  public:
    PhysicsBodyNode2D(const std::string &name, PhysicsBody2DType type);
    PhysicsBodyNode2D(UUID id);
    ~PhysicsBodyNode2D();

    // Serialization
    void Serialize(json &j) override;

    void Deserialize(json &j) override;

    ////////////////////////////////////
    ///   Overrides for 2D Transform
    ////////////////////////////////////
    virtual glm::mat4 GetWorldTransform() override;

    virtual void Translate(const glm::vec2 &translation) override;
    virtual void SetPosition(const glm::vec2 &position) override;
    virtual glm::vec2 GetPosition() override;

    virtual void Rotate(const float radians) override;
    virtual void SetRotation(const float radians) override;
    virtual float GetRotation() override;

    ///////////////////////////////////////////////
    ///   Functions for rigid bodies
    ///////////////////////////////////////////////

    void SetType(PhysicsBody2DType type);
    PhysicsBody2DType GetType();

    void SetAwake(bool awake);
    bool GetAwake();

    void ApplyForce(const glm::vec2 &force, const glm::vec2 &point);
    void ApplyForceToCenter(const glm::vec2 &force);

    void AddLinearVelocity(const glm::vec2 &velocity);
    void SetLinearVelocity(const glm::vec2 &velocity);
    glm::vec2 GetLinearVelocity();

    void AddAngularVelocity(float angularVelocity);
    void SetAngularVelocity(float angularVelocity);
    float GetAngularVelocity();

    void SetAngularDampening(float damping);
    float GetAngularDamping();
};

REGISTER_SERIALIZABLE_NODE(PhysicsBodyNode2D);

} // namespace Pyxis
