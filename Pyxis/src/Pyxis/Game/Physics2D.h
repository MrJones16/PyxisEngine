#pragma once
#include <Pyxis/Game/PhysicsWorld2D.h>
#include <box2d/box2d.h>

namespace Pyxis {
class Physics2D {
  public:
    static PhysicsWorld2D s_World;
    static PhysicsWorld2D &GetWorld() {
        if (!b2World_IsValid(s_World.m_B2WorldId)) {
            s_World = PhysicsWorld2D({0, -9.8f}, 6);
        }
        return s_World;
    }
    static void DestroyWorld() { b2DestroyWorld(s_World.m_B2WorldId); };
    static float GetWorldStep() { return s_World.m_Step; }
};
} // namespace Pyxis
