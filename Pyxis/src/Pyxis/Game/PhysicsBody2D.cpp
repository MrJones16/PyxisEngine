#include <Pyxis/Game/PhysicsBody2D.h>
#include <box2d/box2d.h>
#include <box2d/math_functions.h>
#include <box2d/types.h>

namespace Pyxis {
uint32_t PhysicsBody2D::s_IDCounter = 0;

// angle is in radians
PhysicsBody2D::PhysicsBody2D(b2WorldId worldId, b2BodyType type,
                             const glm::vec2 &position, float angle) {
    m_B2BodyDefinition = b2DefaultBodyDef();
    m_B2BodyDefinition.type = type;
    m_B2BodyDefinition.position = b2Vec2(position.x, position.y);
    m_B2BodyDefinition.rotation = b2Rot(angle);
    m_B2BodyDefinition.userData = this;
    m_B2BodyId = b2CreateBody(worldId, &m_B2BodyDefinition);
    m_ID = s_IDCounter++;
}

PhysicsBody2D::PhysicsBody2D(b2WorldId worldId, json &j) {
    m_B2BodyDefinition = b2DefaultBodyDef();
    Deserialize(j);
    m_B2BodyDefinition.userData = this;
    m_B2BodyId = b2CreateBody(worldId, &m_B2BodyDefinition);
    m_ID = s_IDCounter++;
}
PhysicsBody2D::~PhysicsBody2D() {
    if (b2Body_IsValid(m_B2BodyId))
        b2DestroyBody(m_B2BodyId);
}

void PhysicsBody2D::CopyToWorld(b2WorldId worldId) {
    // physics2d will use this to move the properties of this body onto the
    // newly made world, so that the simulation is deterministic on the move
    UpdateBodyDefinition();
    m_B2BodyId = b2CreateBody(worldId, &m_B2BodyDefinition);
}

// main wrapper funcs for body interaction
void PhysicsBody2D::SetType(PhysicsBody2DType type) {}
PhysicsBody2DType PhysicsBody2D::GetType() const {
    switch (b2Body_GetType(m_B2BodyId)) {
    case b2BodyType::b2_dynamicBody:
        return PhysicsBody2DType::Dynamic;
        break;
    case b2BodyType::b2_kinematicBody:
        return PhysicsBody2DType::Kinematic;
        break;
    case b2BodyType::b2_staticBody:
        return PhysicsBody2DType::Static;
        break;
    default:
        return PhysicsBody2DType::None;
        break;
    }
}

void PhysicsBody2D::SetPosition(const glm::vec2 &position) {
    b2Body_SetTransform(m_B2BodyId, {position.x, position.y},
                        b2Body_GetRotation(m_B2BodyId));
}
glm::vec2 PhysicsBody2D::GetPosition() const {
    b2Vec2 position = b2Body_GetPosition(m_B2BodyId);
    return {position.x, position.y};
}

void PhysicsBody2D::SetLinearVelocity(const glm::vec2 &velocity) {
    b2Body_SetLinearVelocity(m_B2BodyId, {velocity.x, velocity.y});
}
glm::vec2 PhysicsBody2D::GetLinearVelocity() const {
    b2Vec2 vel = b2Body_GetLinearVelocity(m_B2BodyId);
    return {vel.x, vel.y};
}

void PhysicsBody2D::SetLinearDamping(float damping) {
    b2Body_SetLinearDamping(m_B2BodyId, damping);
}
float PhysicsBody2D::GetLinearDamping() const {
    return b2Body_GetLinearDamping(m_B2BodyId);
}

void PhysicsBody2D::SetRotation(float angleInRadians) {

    b2Body_SetTransform(m_B2BodyId, b2Body_GetPosition(m_B2BodyId),
                        b2Rot(angleInRadians));
}
float PhysicsBody2D::GetRotation() const {
    return b2Rot_GetAngle(b2Body_GetRotation(m_B2BodyId));
}
void PhysicsBody2D::SetAngularVelocity(float angularVelocity) {
    b2Body_SetAngularVelocity(m_B2BodyId, angularVelocity);
}
float PhysicsBody2D::GetAngularVelocity() const {
    return b2Body_GetAngularVelocity(m_B2BodyId);
}
void PhysicsBody2D::SetAngularDamping(float angularDamping) {
    b2Body_SetAngularDamping(m_B2BodyId, angularDamping);
}
float PhysicsBody2D::GetAngularDamping() const {
    return b2Body_GetAngularDamping(m_B2BodyId);
}

// forces
void PhysicsBody2D::ApplyForce(const glm::vec2 &force, const glm::vec2 &point,
                               bool wake) {
    b2Body_ApplyForce(m_B2BodyId, {force.x, force.y}, {point.x, point.y}, wake);
}
void PhysicsBody2D::ApplyTorque(float torque, bool wake) {
    b2Body_ApplyTorque(m_B2BodyId, torque, wake);
}

void PhysicsBody2D::UpdateBodyDefinition() {
    // wipe all body info first
    m_B2BodyDefinition = b2DefaultBodyDef();
    // re-grab all info from actual body
    m_B2BodyDefinition.type = b2Body_GetType(m_B2BodyId);
    m_B2BodyDefinition.position = b2Body_GetPosition(m_B2BodyId);
    m_B2BodyDefinition.linearVelocity = b2Body_GetLinearVelocity(m_B2BodyId);
    m_B2BodyDefinition.linearDamping = b2Body_GetLinearDamping(m_B2BodyId);
    m_B2BodyDefinition.rotation = b2Body_GetRotation(m_B2BodyId);
    m_B2BodyDefinition.angularVelocity = b2Body_GetAngularVelocity(m_B2BodyId);
    m_B2BodyDefinition.angularDamping = b2Body_GetAngularDamping(m_B2BodyId);
}

void PhysicsBody2D::Serialize(json &j) {
    // refresh body def before serializing.
    UpdateBodyDefinition();

    j["Type"] = GetType();
    j["Position"] = GetPosition();
    j["LinearVelocity"] = GetLinearVelocity();
    j["LinearDamping"] = GetLinearDamping();
    j["Rotation"] = GetRotation();
    j["AngularVelocity"] = GetAngularVelocity();
    j["AngularDamping"] = GetAngularDamping();
}

// Updates the body definition with present json values
void PhysicsBody2D::Deserialize(json &j) {
    PhysicsBody2DType type = Kinematic;
    if (j.contains("Type"))
        j.at("type").get_to(type);
    switch (type) {

    case Dynamic:
        m_B2BodyDefinition.type = b2BodyType::b2_dynamicBody;
        break;
    case Kinematic:
        m_B2BodyDefinition.type = b2BodyType::b2_kinematicBody;
        break;
    case Static:
        m_B2BodyDefinition.type = b2BodyType::b2_staticBody;
        break;
    default:
        m_B2BodyDefinition.type = b2BodyType::b2_dynamicBody;
        break;
    }

    glm::vec2 Position = {0, 0};
    if (j.contains("Position"))
        j.at("Position").get_to(Position);
    m_B2BodyDefinition.position = {Position.x, Position.y};

    glm::vec2 LinearVelocity = {0, 0};
    if (j.contains("LinearVelocity"))
        j.at("LinearVelocity").get_to(LinearVelocity);
    m_B2BodyDefinition.linearVelocity = {LinearVelocity.x, LinearVelocity.y};

    m_B2BodyDefinition.linearDamping = b2DefaultBodyDef().linearDamping;
    if (j.contains("LinearDamping"))
        j.at("LinearDamping").get_to(m_B2BodyDefinition.linearDamping);

    float Rotation = 0;
    if (j.contains("Rotation"))
        j.at("Rotation").get_to(Rotation);
    m_B2BodyDefinition.rotation = b2Rot(Rotation);

    m_B2BodyDefinition.angularVelocity = 0;
    if (j.contains("AngularVelocity"))
        j.at("AngularVelocity").get_to(m_B2BodyDefinition.angularVelocity);

    m_B2BodyDefinition.angularDamping = b2DefaultBodyDef().angularDamping;
    if (j.contains("AngularDamping"))
        j.at("AngularDamping").get_to(m_B2BodyDefinition.angularDamping);
}

} // namespace Pyxis
