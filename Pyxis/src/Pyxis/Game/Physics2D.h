#pragma once
#include <Pyxis/Core/Log.h>
#include <Pyxis/Game/PhysicsWorld2D.h>
#include <box2d/box2d.h>

namespace Pyxis {
class Physics2D {
  public:
    static PhysicsWorld2D s_World;
    inline static PhysicsWorld2D &GetWorld() {
        if (!s_World.IsValid()) {
            PX_TRACE("Called GetWorld, but it was invalid. Creating new!");
            s_World = PhysicsWorld2D({0, -9.8f}, 6);
            return s_World;
        }
        return s_World;
    }
    inline static void ClearWorld() {
        // simply create a new world. Old one is destroyed.
        s_World = PhysicsWorld2D({0, -9.8f}, 6);
    };

    //  reset keeps the world, but resets the determinism by re-building it
    inline static void ResetWorldDeterminism() {
        GetWorld().ResetWorldDeterminism();
    }
    inline static float GetWorldStep() { return s_World.m_Step; }
};
} // namespace Pyxis
