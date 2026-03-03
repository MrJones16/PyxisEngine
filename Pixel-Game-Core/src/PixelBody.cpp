#include <PixelBody.h>

namespace Pyxis {

PixelBody::PixelBody(b2WorldId worldId, PhysicsBody2DType type)
    : PhysicsBody2D(worldId, type) {}

void PixelBody::AddElement(const Element &e, const glm::ivec2 &worldPosition) {}
void PixelBody::RecalculatePhysicsBody() {}

glm::mat4 PixelBody::GetWorldToLocalTransform() {
    return glm::inverse(GetLocalToWorldTransform());
}
glm::mat4 PixelBody::GetLocalToWorldTransform() {
    glm::ivec2 CenterPixelWorldPosition = glm::floor(GetPosition());
    float angle = GetRotation();

    auto rotationMatrix = glm::mat2x2(1);
    // if angle gets above 45 degrees, apply a 90 deg rotation first
    while (angle > 0.78539816339f) {
        angle -= 1.57079632679f;
        rotationMatrix *= glm::mat2x2(0, -1, 1, 0);
    }
    while (angle < -0.78539816339f) {
        angle += 1.57079632679f;
        rotationMatrix *= glm::mat2x2(0, 1, -1, 0);
    }

    float A = -std::tan(angle / 2);
    float B = std::sin(angle);
    auto horizontalSkewMatrix = glm::mat2x2(1, 0, A, 1); // 0 a
    auto verticalSkewMatrix = glm::mat2x2(1, B, 0, 1);   // b 0

    // this could be wrong...
    rotationMatrix = horizontalSkewMatrix * verticalSkewMatrix *
                     horizontalSkewMatrix * rotationMatrix;
    return rotationMatrix;
}

// helper functions for space transforming and such
glm::ivec2 PixelBody::WorldToLocalPos(glm::ivec2 worldPos) {
    GetPosition();
    b2Vec2 b2Position = m_B2Body->GetPosition();
    glm::ivec2 centerPixelWorld = {b2Position.x * B2ToWorld,
                                   b2Position.y * B2ToWorld};
    if (b2Position.x < 0)
        centerPixelWorld.x -= 1;
    if (b2Position.y < 0)
        centerPixelWorld.y -= 1;

    float angle = m_B2Body->GetAngle();
    auto rotationMatrix = glm::mat2x2(1);

    // if angle gets above 45 degrees, apply a 90 deg rotation first
    while (angle > 0.78539816339f) {
        angle -= 1.57079632679f;
        rotationMatrix *= glm::mat2x2(0, -1, 1, 0);
    }
    while (angle < -0.78539816339f) {
        angle += 1.57079632679f;
        rotationMatrix *= glm::mat2x2(0, 1, -1, 0);
    }

    float A = -std::tan(angle / 2);
    float B = std::sin(angle);
    auto horizontalSkewMatrix = glm::mat2x2(1, 0, A, 1); // 0 a
    auto verticalSkewMatrix = glm::mat2x2(1, B, 0, 1);   // b 0
    for (auto &mappedElement : m_Elements) {
        // find the element in the world by using the transform of the body
        // and the stored local position
        glm::ivec2 skewedPos = mappedElement.first * rotationMatrix;

        // horizontal skew:
        int horizontalSkewAmount = (float)skewedPos.y * A;
        skewedPos.x += horizontalSkewAmount;

        // vertical skew
        int skewAmount = (float)skewedPos.x * B;
        skewedPos.y += skewAmount;

        // horizontal skew:
        horizontalSkewAmount = (float)skewedPos.y * A;
        skewedPos.x += horizontalSkewAmount;

        mappedElement.second.worldPos =
            glm::ivec2(skewedPos.x, skewedPos.y) + centerPixelWorld;
    }
}
glm::ivec2 PixelBody::LocalToWorldPos(glm::ivec2 localPos) {}

} // namespace Pyxis
