#include <Pyxis/Game/PhysicsBody2D.h>
#include <box2d/box2d.h>
#include <box2d/types.h>

namespace Pyxis {
uint32_t PhysicsBody2D::s_IDCounter = 0;
PhysicsBody2D::PhysicsBody2D(b2WorldId worldId, b2BodyType type) {
    m_B2BodyDefinition = b2DefaultBodyDef();
    m_B2BodyDefinition.type = type;
    m_B2BodyId = b2CreateBody(worldId, &m_B2BodyDefinition);
    m_ID = s_IDCounter++;
}
PhysicsBody2D::~PhysicsBody2D() {
    if (b2Body_IsValid(m_B2BodyId))
        b2DestroyBody(m_B2BodyId);
}

void PhysicsBody2D::TransferToWorld(b2WorldId worldId) {
    // physics2d will use this to move the properties of this body onto the
    // newly made world, so that the simulation is deterministic on the move
    UpdateBodyDefinition();
    m_B2BodyId = b2CreateBody(worldId, &m_B2BodyDefinition);
}

void PhysicsBody2D::UpdateBodyDefinition() {
    m_B2BodyDefinition.position = b2Body_GetPosition(m_B2BodyId);
    m_B2BodyDefinition.linearVelocity = b2Body_GetLinearVelocity(m_B2BodyId);
    m_B2BodyDefinition.linearDamping = b2Body_GetLinearDamping(m_B2BodyId);
    m_B2BodyDefinition.rotation = b2Body_GetRotation(m_B2BodyId);
    m_B2BodyDefinition.angularVelocity = b2Body_GetAngularVelocity(m_B2BodyId);
    m_B2BodyDefinition.angularDamping = b2Body_GetAngularDamping(m_B2BodyId);
}

void PhysicsBody2D::Serialize(json &j) {
    // update the member vars with b2body and then serialize them
    m_Position = GetPosition();
    m_Rotation = GetRotation();

    Node2D::Serialize(j);
    j["Type"] = "B2BodyNode"; // Override type identifier

    // Serialize member variables
    // j["m_HasBody"] = m_HasBody;// this will be determined by if it has the
    // "m_b2Body"
    j["m_CategoryBits"] = m_CategoryBits;
    j["m_MaskBits"] = m_MaskBits;

    // Serialize B2 Body Definition
    // we will re-construct the user data, and
    j["m_B2BodyDef"]["position"] = m_B2BodyDef.position;
    j["m_B2BodyDef"]["type"] = (int)m_B2BodyDef.type;

    // Serialize B2 Body data
    if (m_HasBody) {
        // note: we already have position and angle

        j["m_B2Body"]["linearVelocity"] = b2Body_GetLinearVelocity(m_B2Body);
        j["m_B2Body"]["linearDamping"] = b2Body_GetLinearDamping(m_B2Body);
        j["m_B2Body"]["angularVelocity"] = b2Body_GetAngularVelocity(m_B2Body);
        j["m_B2Body"]["angularDamping"] = b2Body_GetAngularDamping(m_B2Body);
    }
}

void PhysicsBody2D::Deserialize(json &j) {

    if (j.contains("m_CategoryBits"))
        j.at("m_CategoryBits").get_to(m_CategoryBits);
    if (j.contains("m_MaskBits"))
        j.at("m_MaskBits").get_to(m_MaskBits);

    if (j.contains("m_B2BodyDef")) {
        if (j["m_B2BodyDef"].contains("position"))
            j["m_B2BodyDef"].at("position").get_to(m_B2BodyDef.position);
        if (j["m_B2BodyDef"].contains("type"))
            j["m_B2BodyDef"].at("type").get_to(m_B2BodyDef.type);
        m_B2BodyDef.userData = this;
    }

    if (j.contains("m_B2Body")) {
        CreateBody(Physics2D::GetWorld());

        b2Vec2 linearVelocity = {0, 0};
        float linearDamping = 0, angularVelocity = 0, angularDamping = 0;
        if (j["m_B2Body"].contains("linearVelocity"))
            j["m_B2Body"].at("linearVelocity").get_to(linearVelocity);
        if (j["m_B2Body"].contains("linearDamping"))
            j["m_B2Body"].at("linearDamping").get_to(linearDamping);
        if (j["m_B2Body"].contains("angularVelocity"))
            j["m_B2Body"].at("angularVelocity").get_to(angularVelocity);
        if (j["m_B2Body"].contains("angularDamping"))
            j["m_B2Body"].at("angularDamping").get_to(angularDamping);

        b2Vec2 position = {0, 0};
        float angle = 0;
        if (j.contains("m_Position"))
            j.at("m_Position").get_to(position);
        if (j.contains("m_Rotation"))
            j.at("m_Rotation").get_to(angle);

        SetPosition({position.x, position.y});
        SetRotation(angle);

        // b2Body_SetTransform(b2Vec2(m_Position.x, m_Position.y),
        // m_Rotation);
        b2Body_SetLinearVelocity(m_B2Body, linearVelocity);
        b2Body_SetLinearDamping(m_B2Body, linearDamping);
        b2Body_SetAngularVelocity(m_B2Body, angularVelocity);
        b2Body_SetAngularDamping(m_B2Body, angularDamping);
    }
}

} // namespace Pyxis
