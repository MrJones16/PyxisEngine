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
    PhysicsBodyNode2D(const std::string &name, b2BodyType type);
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
    ///   Functions for rigid bodies / B2Bodies
    ///////////////////////////////////////////////

    // creates the underlying b2body. might be better to switch this to take the
    // world as parameter
    virtual void CreatePhysicsBody();

    /// <summary>
    /// Frees the underlying b2body and resets the world.
    /// You need to be able to save the transform
    /// </summary>
    virtual void DestroyBody();

    void DestroyShapes();

    void SetType(b2BodyType type);
    b2BodyType GetType();

    // Forces
    void ApplyForce(const glm::vec2 &force, const glm::vec2 &point);
    void ApplyForceToCenter(const glm::vec2 &force);

    /// Linear Velocity
    void AddLinearVelocity(const glm::vec2 &velocity);
    void SetLinearVelocity(const glm::vec2 &velocity);
    glm::vec2 GetLinearVelocity();

    // Angular Velocity
    void AddAngularVelocity(float angVel);
    void SetAngularVelocity(float angVel);
    float GetAngularVelocity();

    // Angular Dampening
    void SetAngularDampening(float damping);
    float GetAngularDamping();

    /* What needs to be made
    bool CreateB2Body(b2World* world);
    std::vector<PixelRigidBody*> RecreateB2Body(unsigned int randSeed, b2World*
    world);

    void SetPixelPosition(const glm::ivec2& position);
    void SetTransform(const glm::vec2& position, float rotation);
    void SetPosition(const glm::vec2& position);
    void SetRotation(float rotation);
    void SetAngularVelocity(float velocity);
    void SetLinearVelocity(const b2Vec2& velocity);

    glm::ivec2 GetPixelPosition();
    */

    /* overridable
    virtual void ResetLocalTransform();
    virtual void SetLocalTransform(const glm::mat4& transform);
    glm::mat4& GetLocalTransform();
    virtual void Translate(glm::vec3 translation);
    virtual void Rotate(glm::vec3 rotation);
    virtual void SetScale(glm::vec3 scale);
    virtual void Scale(glm::vec3 scale);
    */
};

REGISTER_SERIALIZABLE_NODE(PhysicsBodyNode2D);
} // namespace Pyxis
