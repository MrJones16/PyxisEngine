#pragma once

#include "Pyxis/Nodes/Node.h"
#include <box2d/box2d.h>

namespace Pyxis {

struct B2BodyStorage {
  public:
    b2Vec2 position;
    b2Vec2 linearVelocity;
    float linearDamping;
    b2Rot rotation;
    float angularVelocity;
    float angularDamping;

    B2BodyStorage(b2BodyId body) {
        position = b2Body_GetPosition(body);
        linearVelocity = b2Body_GetLinearVelocity(body);
        linearDamping = b2Body_GetLinearDamping(body);
        rotation = b2Body_GetRotation(body);
        angularVelocity = b2Body_GetAngularVelocity(body);
        angularDamping = b2Body_GetAngularDamping(body);
    }

    void TransferData(b2BodyId body) {
        b2Body_SetTransform(body, position, rotation);
        b2Body_SetLinearVelocity(body, linearVelocity);
        b2Body_SetLinearDamping(body, linearDamping);
        b2Body_SetAngularVelocity(body, angularVelocity);
        b2Body_SetAngularDamping(body, angularDamping);
    }
};

// A wrapper for box2d bodies. The main reason for doing this, is so that:
// 1. could be slightly easier to replace box2d in future if somehow needed
// 2. So that I can implement a way to refresh / recreate the box2d worlds, as
// that is needed!
struct PhysicsBody2D {
  public:
    PhysicsBody2D(b2WorldId worldId);
    ~PhysicsBody2D();

    void TransferToWorld(b2WorldId worldId);

    // Serialization
    void Serialize(json &j);

    void Deserialize(json &j);

  protected:
    void UpdateBodyDefinition();

    static uint32_t s_IDCounter;
    uint32_t m_ID;
    b2BodyDef m_B2BodyDefinition;
    b2BodyId m_B2BodyId;
};
} // namespace Pyxis
