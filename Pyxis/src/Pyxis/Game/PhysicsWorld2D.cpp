#include "Pyxis/Game/PhysicsBody2D.h"
#include <Pyxis/Game/PhysicsWorld2D.h>
#include <box2d/box2d.h>
#include <box2d/types.h>

namespace Pyxis {

PhysicsWorld2D::PhysicsWorld2D(const glm::vec2 &gravity, int subSteps) {
    m_B2WorldDef.gravity = {gravity.x, gravity.y};
    m_B2WorldDef.enableSleep = false;
    m_SubSteps = subSteps;
    m_B2WorldId = b2CreateWorld(&m_B2WorldDef);
}
PhysicsWorld2D::~PhysicsWorld2D() {
    if (b2World_IsValid(m_B2WorldId)) {
        b2DestroyWorld(m_B2WorldId);
    }
}

b2WorldId PhysicsWorld2D::GetWorld() {
    if (!b2World_IsValid(m_B2WorldId)) {
        m_B2WorldId = b2CreateWorld(&m_B2WorldDef);
    }

    return m_B2WorldId;
}

bool PhysicsWorld2D::IsValid() { return b2World_IsValid(m_B2WorldId); }
void PhysicsWorld2D::ResetWorldDeterminism() {
    // make a new world
    b2WorldId newId = b2CreateWorld(&m_B2WorldDef);
    // loop over the bodies, and copy them into the new world
    std::vector<uint32_t> expiredIds;
    for (auto kvp : m_Bodies) {
        if (!kvp.second.expired()) {
            // still active, so lets transfer it to the new world.
            kvp.second.lock()->CopyToWorld(newId);

        } else {
            expiredIds.push_back(kvp.first);
        }
    }
    // we transferred all bodies to the new world, so lets remove expired ones
    // (post iteration)
    for (uint32_t id : expiredIds)
        m_Bodies.erase(id);
    // now lets delete the prior world
    b2DestroyWorld(m_B2WorldId);
    m_B2WorldId = newId;
}
void PhysicsWorld2D::Step() { b2World_Step(m_B2WorldId, m_Step, m_SubSteps); }

int PhysicsWorld2D::GetBodyCount() {
    std::vector<uint32_t> expiredIds;
    for (auto kvp : m_Bodies) {
        if (kvp.second.expired()) {
            expiredIds.push_back(kvp.first);
        }
    }
    for (uint32_t id : expiredIds)
        m_Bodies.erase(id);
    return m_Bodies.size();
}

Ref<PhysicsBody2D> PhysicsWorld2D::CreateBody(PhysicsBody2DType type,
                                              const glm::vec2 &position,
                                              float angleInRadians) {
    Ref<PhysicsBody2D> ref =
        CreateRef<PhysicsBody2D>(GetWorld(), type, position, angleInRadians);
    m_Bodies[ref->m_ID] = ref;
    return ref;
}

Ref<PhysicsBody2D> PhysicsWorld2D::CreateBody(json &j) {
    Ref<PhysicsBody2D> ref = CreateRef<PhysicsBody2D>(GetWorld(), j);
    m_Bodies[ref->m_ID] = ref;
    return ref;
}

} // namespace Pyxis
