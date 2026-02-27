#include "B2BodyNode.h"
#include <Pyxis/Game/Physics2D.h>
#include <box2d/box2d.h>

namespace Pyxis {
B2BodyNode::B2BodyNode(const std::string &name, b2BodyType type)
    : Node2D(name) {
    m_B2BodyDef.position = {0, 0};
    m_B2BodyDef.type = type;
    m_B2BodyDef.userData = this;
    CreateBody(Physics2D::GetWorld());
}

B2BodyNode::B2BodyNode(UUID id) : Node2D(id) {
    m_B2World = Physics2D::GetWorld();
    m_HasBody = false;
}

B2BodyNode::~B2BodyNode() { DestroyBody(); }

void B2BodyNode::OnPhysicsUpdate() {}

void B2BodyNode::OnInspectorRender() {
    Node2D::OnInspectorRender();
    int bodyType = GetType();
    ImGui::Text("Type: ");
    {
        ImGui::RadioButton("Static", &bodyType, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Kinematic", &bodyType, 1);
        ImGui::SameLine();
        ImGui::RadioButton("Dynamic", &bodyType, 2);
    }
    SetType((b2BodyType)bodyType);
}

// TODO: Completely serialize b2bodies, and not just what i am using!
void B2BodyNode::Serialize(json &j) {
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

void B2BodyNode::Deserialize(json &j) {
    Node2D::Deserialize(j);

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

glm::mat4 B2BodyNode::GetWorldTransform() {
    auto pos = b2Body_GetPosition(m_B2Body);
    glm::mat4 localTransform =
        glm::translate(glm::mat4(), glm::vec3(pos.x, pos.y, m_Layer));
    localTransform = glm::rotate(localTransform, m_Rotation, {0, 0, -1});
    if (Node2D *parent2D = dynamic_cast<Node2D *>(m_Parent)) {
        return parent2D->GetWorldTransform() * localTransform;
    } else {
        return localTransform;
    }
}

void B2BodyNode::Translate(const glm::vec2 &translation) {
    m_Position += translation;
    b2Body_SetTransform(m_B2Body, {m_Position.x, m_Position.y},
                        b2Rot(m_Rotation));
}
void B2BodyNode::SetPosition(const glm::vec2 &position) {
    m_Position = position;
    b2Body_SetTransform(m_B2Body, {m_Position.x, m_Position.y},
                        b2Rot(m_Rotation));
}
glm::vec2 B2BodyNode::GetPosition() {
    b2Vec2 vec = b2Body_GetPosition(m_B2Body);
    return glm::vec2(vec.x, vec.y);
}

void B2BodyNode::Rotate(const float radians) {
    m_Rotation += radians;
    b2Body_SetTransform(m_B2Body, {m_Position.x, m_Position.y},
                        b2Rot(m_Rotation));
}
void B2BodyNode::SetRotation(const float radians) {
    m_Rotation = radians;
    b2Body_SetTransform(m_B2Body, {m_Position.x, m_Position.y},
                        b2Rot(m_Rotation));
}
float B2BodyNode::GetRotation() {
    return b2Rot_GetAngle(b2Body_GetRotation(m_B2Body));
}

void B2BodyNode::TransferWorld(b2WorldId world) {
    if (m_HasBody) {
        // store data from body, then make a new one, then put the new data in.
        B2BodyStorage storage(m_B2Body);
        // trick ourselves to make a new body even though we have one
        m_HasBody = false;
        CreateBody(world);
        storage.TransferData(m_B2Body);
        PX_CORE_TRACE("Transferred body to new world. Position: ({0},{1})",
                      GetPosition().x, GetPosition().y);
    }
}

void B2BodyNode::CreateBody(b2WorldId world) {
    if (!m_HasBody) {
        m_HasBody = true;
        m_B2World = world;

        m_B2Body = Physics2D::CreateBody(m_B2BodyDef);
        // Physics2D::s_RigidBodiesToAdd.push(this);
    }
}

void B2BodyNode::DestroyBody() {
    if (m_HasBody) {
        m_HasBody = false;
        // Physics2D::s_BodyToNode[m_B2Body] = nullptr;
        if (b2Body_IsValid(m_B2Body))
            b2DestroyBody(m_B2Body);
        m_B2Body = b2_nullBodyId;
        m_B2World = b2_nullWorldId;
    }
}

void B2BodyNode::DestroyShapes() {
    int shapeCount = b2Body_GetShapeCount(m_B2Body);
    b2ShapeId shapes[shapeCount];
    b2Body_GetShapes(m_B2Body, shapes, shapeCount);
    for (b2ShapeId id : shapes) {
        b2DestroyShape(id, true);
    }
}

void B2BodyNode::SetType(b2BodyType type) {
    m_B2BodyDef.type = type;
    b2Body_SetType(m_B2Body, type);
}

b2BodyType B2BodyNode::GetType() { return m_B2BodyDef.type; }

void B2BodyNode::ApplyForce(const glm::vec2 &force, const glm::vec2 &point) {
    b2Body_ApplyForce(m_B2Body, {force.x, force.y}, {point.x, point.y}, true);
}

void B2BodyNode::ApplyForceToCenter(const glm::vec2 &force) {
    b2Body_ApplyForceToCenter(m_B2Body, {force.x, force.y}, true);
}

void B2BodyNode::AddLinearVelocity(const glm::vec2 &velocity) {
    b2Body_SetLinearVelocity(m_B2Body, b2Body_GetLinearVelocity(m_B2Body) +
                                           b2Vec2(velocity.x, velocity.y));
}

void B2BodyNode::SetLinearVelocity(const glm::vec2 &velocity) {
    b2Body_SetLinearVelocity(m_B2Body, {velocity.x, velocity.y});
}

glm::vec2 B2BodyNode::GetLinearVelocity() {
    b2Vec2 velocity = b2Body_GetLinearVelocity(m_B2Body);
    return glm::vec2(velocity.x, velocity.y);
}

void B2BodyNode::AddAngularVelocity(float angVel) {
    b2Body_ApplyAngularImpulse(m_B2Body, angVel, true);
}

void B2BodyNode::SetAngularVelocity(float angVel) {
    b2Body_SetAngularVelocity(m_B2Body, angVel);
}

float B2BodyNode::GetAngularVelocity() {
    return b2Body_GetAngularVelocity(m_B2Body);
}

void B2BodyNode::SetAngularDampening(float damping) {
    b2Body_SetAngularDamping(m_B2Body, damping);
}

float B2BodyNode::GetAngularDamping() {
    return b2Body_GetAngularDamping(m_B2Body);
}

} // namespace Pyxis
