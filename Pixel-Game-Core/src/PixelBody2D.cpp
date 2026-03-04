#include "PixelBody2D.h"
#include "Pyxis/Game/PhysicsBody2D.h"
#include "Pyxis/Nodes/PhysicsBodyNode2D.h"
#include "glm/gtc/matrix_transform.hpp"
#include <Pyxis/Game/Physics2D.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Pyxis {

PixelBody2D::PixelBody2D(const std::string &name, PhysicsBody2DType type)
    : PhysicsBodyNode2D(name, type) {}

PixelBody2D::PixelBody2D(UUID id) : PhysicsBodyNode2D(id) {}

void PixelBody2D::SetPixelBodyElements(
    std::vector<PixelBodyElement> &elements) {

    // we need to convert the vector of elements to local positions to store
    // them

    // find the width & height of the set of elements
    int minX = INT_MAX, maxX = INT_MIN, minY = INT_MAX, maxY = INT_MIN;
    for (auto &pbe : elements) {
        minX = std::min(pbe.worldPos.x, minX);
        maxX = std::max(pbe.worldPos.x, maxX);
        minY = std::min(pbe.worldPos.y, minY);
        maxY = std::max(pbe.worldPos.y, maxY);
    }
    m_Width = std::abs(maxX - minX) + 1;
    m_Height = std::abs(maxY - minY) + 1;
    glm::ivec2 CenterPixelPos =
        glm::vec2((m_Width / 2.0f) + minX, (m_Height / 2.0f) + minY);

    // Since we are creating the body from scratch, we should reset these
    // values.
    SetPosition(CenterPixelPos);
    SetRotation(0);
    SetLinearVelocity({0, 0});
    SetAngularVelocity(0);

    // use the minimum to get local positions [example, from -10,-10 to 10,10]
    for (auto &pbe : elements) {
        glm::ivec2 localPos = pbe.worldPos - CenterPixelPos;
        m_Elements[localPos] = pbe;
    }
    m_LocalMinimum = glm::ivec2(minX, minY) - CenterPixelPos;
    m_InWorld = true;
    GenerateMesh();
}

void PixelBody2D::Serialize(json &j) {
    PhysicsBodyNode2D::Serialize(j);
    j["Type"] = "PixelBody2D";

    j["m_InWorld"] = m_InWorld;
    j["m_Width"] = m_Width;
    j["m_Height"] = m_Height;

    for (auto &[key, value] : m_Elements) {
        j["m_Elements"] += {{"Key", key}, {"Value", value}};
    }
}

void PixelBody2D::Deserialize(json &j) {
    PhysicsBodyNode2D::Deserialize(j);

    // Extract new member variables
    if (j.contains("m_InWorld"))
        j.at("m_InWorld").get_to(m_InWorld);
    if (j.contains("m_Width"))
        j.at("m_Width").get_to(m_Width);
    if (j.contains("m_Height"))
        j.at("m_Height").get_to(m_Height);
    if (j.contains("m_Elements")) {
        for (auto &element : j.at("m_Elements")) {
            glm::ivec2 key;
            PixelBodyElement value;
            element.at("Key").get_to(key);
            element.at("Value").get_to(value);
            m_Elements[key] = value;
        }
    }
    // the body is made, and we have the elements. The elements should already
    // be in the world too. I don't think theres anything else to do here.
}

void PixelBody2D::QueueFree() { m_FreeMe = true; }

void PixelBody2D::GenerateMesh() {
    // we know the width and height, so we know how many 64x64 areas we would
    // need.
    int bitArrayXCount =
        ((m_Width - 1) / 64) + 1; // 0-63 would produce 0, then add 1.
    int bitArrayYCount = ((m_Height - 1) / 64) + 1;
    for (int y = 0; y < bitArrayYCount; y++) {
        for (int x = 0; x < bitArrayXCount; x++) {
            glm::ivec2 bitArrayCoord = {x, y};
            // looping over all needed bit arrays.
            // we already know the minimum local position, so we offset by
            // there.
            std::vector<uint64_t> bitArray = std::vector<uint64_t>(64);
            for (int x = 0; x < 64; x++) {
                // looping over the x axis, and we will set every bit of array
                uint64_t column = 0;
                for (int y = 0; y < 64; y++) {
                    glm::ivec2 bitArrayPixelCoord = {x, y};
                    if (m_Elements.contains(bitArrayPixelCoord +
                                            m_LocalMinimum)) {
                        column |= ((uint64_t)1 << y); // set bit at y digit
                    }
                }
                bitArray.push_back(column);
            }
            m_BitArrays[bitArrayCoord] = bitArray;
        }
    }

    // m_BitArrays is now populated. We still need to run BGM on it, and set the
    // physics body shapes.
}

glm::mat4 PixelBody2D::GetWorldToLocalTransform() {
    return glm::inverse(GetLocalToWorldTransform());
}
glm::mat4 PixelBody2D::GetLocalToWorldTransform() {
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

void PixelBody2D::UpdateElementWorldPositions() {
    m_Moved = false;
    // center of the pixel body in the world
    glm::ivec2 centerPixelWorld = glm::floor(GetPosition());
    glm::mat2x2 rotationMatrix = GetLocalToWorldTransform();

    for (auto &mappedElement : m_Elements) {
        // find the element in the world by using the transform of the body
        // and the stored local position
        glm::ivec2 skewedPos = mappedElement.first * rotationMatrix;
        glm::ivec2 posPrior = mappedElement.second.worldPos;
        mappedElement.second.worldPos =
            glm::ivec2(skewedPos.x, skewedPos.y) + centerPixelWorld;
        if (posPrior != mappedElement.second.worldPos)
            m_Moved = true;
    }
}

glm::mat4 PixelBody2D::GetWorldTransform() {
    // b2 position is different than world position.
    m_Position = GetPosition();
    glm::mat4 localTransform = glm::translate(
        glm::mat4(), glm::vec3(m_Position.x, m_Position.y, m_Layer));
    localTransform = glm::rotate(localTransform, m_Rotation, {0, 0, -1});
    if (Node2D *parent2D = dynamic_cast<Node2D *>(m_Parent)) {
        return parent2D->GetWorldTransform() * localTransform;
    } else {
        return localTransform;
    }
}

void PixelBody2D::Translate(const glm::vec2 &translation) {
    PhysicsBodyNode2D::Translate(translation * m_SimulationScale);
}
void PixelBody2D::SetPosition(const glm::vec2 &position) {
    PhysicsBodyNode2D::SetPosition(position * m_SimulationScale);
}
glm::vec2 PixelBody2D::GetPosition() {
    return PhysicsBodyNode2D::GetPosition() * m_InverseSimulationScale;
}

glm::vec2 PixelBody2D::GetLocalPixelVelocity(glm::ivec2 localPosition) {

    // get the strength of the tangential velocity
    float tangentialScale = std::sqrtf((localPosition.x * localPosition.x) +
                                       (localPosition.y * localPosition.y)) *
                            GetAngularVelocity();

    // get the angle of the local position in combination with the body angle
    float angle = std::atan2f(localPosition.y, localPosition.x) + GetRotation();
    // rotate the angle by 90 degrees to get the tangential direction
    angle += 1.57079632679f;
    // turn the angle into a vector
    glm::vec2 tangentialDirection =
        glm::vec2(std::cosf(angle), std::sinf(angle));
    // scale the direction by the strength

    // TODO: figure out correct scaling constant, probable based on PPU of the
    // box2d sim
    tangentialDirection *= tangentialScale * 0.05f;

    // combine tangential direction with linear velocity

    return (GetLinearVelocity() + tangentialDirection) *
           Physics2D::GetWorldStep();
}

} // namespace Pyxis
