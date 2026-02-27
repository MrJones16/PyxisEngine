#pragma once
#include <Pyxis/Game/PhysicsWorld2D.h>

namespace Pyxis {
class Physics2D {
  public:
    static PhysicsWorld2D s_World;
    static PhysicsWorld2D &GetWorld() { return s_World; }
};
} // namespace Pyxis
