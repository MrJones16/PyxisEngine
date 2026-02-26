#pragma once

#include <Pyxis/Game/PhysicsBody2D.h>
#include <Pyxis/Nodes/B2BodyNode.h>
#include <Pyxis/Nodes/Node.h>
#include <box2d/id.h>
#include <map>
#include <unordered_set>

namespace Pyxis {
/// <summary>
/// A static wrapper for a B2World system, which would allow for any rigid body
/// object or b2body implementation to create itself
///
/// Nodes must manage the Physics2D themselves, it is not handled by the engine
/// or the scene layer. So you must use like a physics manager to update the
/// physics. which also allows for more flexibility for physics updating & use
///
/// </summary>
class Physics2D {
  private:
    /// <summary>
    ///
    /// </summary>
    inline static b2WorldId m_World = b2_nullWorldId;
    inline static std::unordered_map<uint32_t, PhysicsBody2D> m_Bodies;

  protected:
    // inline static std::map<b2Body*, B2BodyNode*> s_BodyToNode;
    // inline static std::unordered_set<B2BodyNode*> s_RigidBodies;
    // inline static std::queue<B2BodyNode*> s_RigidBodiesToAdd;
    // inline static std::queue<B2BodyNode*> s_RigidBodiesToRemove;

  public:
    inline static int32_t m_VelocityIterations = 6;
    inline static int32_t m_PositionIterations = 2;
    inline static float m_Step = 1.0f / 60.0f;

    inline static b2WorldId GetWorld() {
        if (b2World_IsValid(m_World)) {
            b2WorldDef def = b2DefaultWorldDef();
            def.gravity = {0, -9.8};
            m_World = b2CreateWorld(&def);
        }

        return m_World;
    }

    static inline b2BodyId CreateBody(b2BodyDef &bodyDef) {
        b2BodyId id = b2CreateBody(m_World, &bodyDef);

        return id;
    }

    inline static void ResetWorld() {
        PX_TRACE("new b2world made");
        b2WorldDef def = b2DefaultWorldDef();
        def.gravity = {0, -9.8};
        m_World = b2CreateWorld(&def);

        std::queue<b2BodyId> IterateQueue;
    }

    inline static void DeleteWorld() {}

    inline static void Step() {}

    friend class B2BodyNode;
};

} // namespace Pyxis
