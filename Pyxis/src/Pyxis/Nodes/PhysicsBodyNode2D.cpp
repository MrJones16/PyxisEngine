#include "PhysicsBodyNode2D.h"
#include "Pyxis/Game/PhysicsBody2D.h"
#include "Pyxis/Game/PhysicsWorld2D.h"
#include <Pyxis/Game/Physics2D.h>
#include <box2d/box2d.h>
#include <box2d/collision.h>
#include <box2d/math_functions.h>
#include <box2d/types.h>

namespace Pyxis {
PhysicsBodyNode2D::PhysicsBodyNode2D(const std::string &name,
                                     PhysicsBody2DType type)
    : Node2D(name) {
    m_PhysicsBody = Physics2D::GetWorld().CreateBody(type, {0, 0}, 0);
}

PhysicsBodyNode2D::PhysicsBodyNode2D(UUID id) : Node2D(id) {}

PhysicsBodyNode2D::~PhysicsBodyNode2D() {}

// TODO: Completely serialize b2bodies, and not just what i am using!
void PhysicsBodyNode2D::Serialize(json &j) {
    // update the member vars with b2body and then serialize them
    m_Position = GetPosition();
    m_Rotation = GetRotation();

    Node2D::Serialize(j);
    json PhysicsBodyJson;
    m_PhysicsBody->Serialize(PhysicsBodyJson);
    j["m_PhysicsBody"] = PhysicsBodyJson;
}

// create the physics body with this after init with ID.
void PhysicsBodyNode2D::Deserialize(json &j) {
    Node2D::Deserialize(j);

    if (j.contains("m_PhysicsBody")) {
        json PhysicsBodyJson;
        j.at("m_PhysicsBody").get_to(PhysicsBodyJson);
        m_PhysicsBody = Physics2D::GetWorld().CreateBody(PhysicsBodyJson);
    }
}

glm::mat4 PhysicsBodyNode2D::GetWorldTransform() {
    m_Position = m_PhysicsBody->GetPosition();
    m_Rotation = m_PhysicsBody->GetRotation();
    glm::mat4 localTransform = glm::translate(
        glm::mat4(), glm::vec3(m_Position.x, m_Position.y, m_Layer));
    localTransform = glm::rotate(localTransform, m_Rotation, {0, 0, -1});
    if (Node2D *parent2D = dynamic_cast<Node2D *>(m_Parent)) {
        return parent2D->GetWorldTransform() * localTransform;
    } else {
        return localTransform;
    }
}

void PhysicsBodyNode2D::DebugDraw() { m_PhysicsBody->DebugDraw(); }

void PhysicsBodyNode2D::Translate(const glm::vec2 &translation) {
    m_PhysicsBody->SetPosition(m_PhysicsBody->GetPosition() + translation);
}
void PhysicsBodyNode2D::SetPosition(const glm::vec2 &position) {
    m_PhysicsBody->SetPosition(position);
}
glm::vec2 PhysicsBodyNode2D::GetPosition() {
    return m_PhysicsBody->GetPosition();
}

void PhysicsBodyNode2D::Rotate(const float radians) {
    m_PhysicsBody->SetRotation(m_PhysicsBody->GetRotation() + radians);
}
void PhysicsBodyNode2D::SetRotation(const float radians) {
    m_PhysicsBody->SetRotation(radians);
}
float PhysicsBodyNode2D::GetRotation() { return m_PhysicsBody->GetRotation(); }

void PhysicsBodyNode2D::SetType(PhysicsBody2DType type) {
    m_PhysicsBody->SetType(type);
}

PhysicsBody2DType PhysicsBodyNode2D::GetType() {
    return m_PhysicsBody->GetType();
}

void PhysicsBodyNode2D::SetAwake(bool awake) { m_PhysicsBody->SetAwake(awake); }
bool PhysicsBodyNode2D::GetAwake() { return m_PhysicsBody->GetAwake(); }

void PhysicsBodyNode2D::ApplyForce(const glm::vec2 &force,
                                   const glm::vec2 &point) {
    m_PhysicsBody->ApplyForce(force, point);
}

void PhysicsBodyNode2D::AddLinearVelocity(const glm::vec2 &velocity) {
    m_PhysicsBody->SetLinearVelocity(m_PhysicsBody->GetLinearVelocity() +
                                     velocity);
}

void PhysicsBodyNode2D::SetLinearVelocity(const glm::vec2 &velocity) {
    m_PhysicsBody->SetLinearVelocity(m_PhysicsBody->GetLinearVelocity());
}

glm::vec2 PhysicsBodyNode2D::GetLinearVelocity() {
    return m_PhysicsBody->GetLinearVelocity();
}

void PhysicsBodyNode2D::AddAngularVelocity(float angularVelocity) {
    m_PhysicsBody->SetAngularVelocity(m_PhysicsBody->GetAngularVelocity() +
                                      angularVelocity);
}

void PhysicsBodyNode2D::SetAngularVelocity(float angularVelocity) {
    m_PhysicsBody->SetAngularVelocity(angularVelocity);
}

float PhysicsBodyNode2D::GetAngularVelocity() {
    return m_PhysicsBody->GetAngularVelocity();
}

void PhysicsBodyNode2D::SetAngularDampening(float damping) {
    m_PhysicsBody->SetAngularVelocity(damping);
}

float PhysicsBodyNode2D::GetAngularDamping() {
    return m_PhysicsBody->GetAngularDamping();
}

void PhysicsBodyNode2D::RemoveShapes() { m_PhysicsBody->RemoveShapes(); }

void PhysicsBodyNode2D::AddBoxShape(float halfWidth, float halfHeight,
                                    const glm::vec2 &center, float radians) {
    m_PhysicsBody->AddBoxShape(halfWidth, halfHeight, center, radians);
}

} // namespace Pyxis
