#include "PixelBody2D.h"
#include "Pyxis/Game/PhysicsBody2D.h"
#include "Pyxis/Nodes/PhysicsBodyNode2D.h"
#include <Pyxis/Game/Physics2D.h>
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
    // TODO: not sure what needs to happen here right now.
}

void PixelBody2D::QueueFree() { m_FreeMe = true; }

void PixelBody2D::GenerateMesh() {}

void PixelBody2D::EnterWorld() {
    if (m_Elements.size() == 0) {
        QueueFree();
        return;
    }

    // chunkloading
    // get current chunk we are in, then get the chunks around us
    glm::vec2 pixelPos = GetPosition();
    glm::ivec2 chunkPos =
        m_PXWorld->PixelToChunk(glm::ivec2(pixelPos.x, pixelPos.y));
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            glm::ivec2 checkPos = chunkPos + glm::ivec2(x, y);
            if (!m_PXWorld->m_Chunks.contains(checkPos)) {
                m_PXWorld->AddChunk(checkPos);
            }
        }
    }

    for (auto &mappedElement : m_Elements) {
        Element &e = m_PXWorld->GetElement(mappedElement.second.worldPos);
        ElementProperties &ed = ElementData::GetElementProperties(e.m_ID);

        // if it is a solid, or it is rigid, we need to become hidden.
        if (ed.cell_type == ElementType::solid || e.m_Rigid) {
            mappedElement.second.hidden = true;
            continue;
        } else {
            if (e.m_ID != 0) {
                // its not air, so we need to throw it as a particle
                // we will use the opposite of the velocity of the body

                glm::vec2 velocity = GetLocalPixelVelocity(mappedElement.first);
                float lengthSquared =
                    velocity.x * velocity.x + velocity.y * velocity.y;
                if (lengthSquared > 0.0f) {
                    m_PXWorld->CreateParticle(mappedElement.second.worldPos,
                                              -velocity, e);
                } else {
                    // we are too slow to make a particle, so we hide instead
                    mappedElement.second.hidden = true;
                    continue;
                }

                // TODO: make pixelbody lose velocity based on the particles
                // thrown
            }
            glm::vec2 lv = GetLinearVelocity();
            float lvLengthSqr = (lv.x * lv.x) + (lv.y * lv.y);
            if (std::abs(GetAngularVelocity()) > 0.01f || lvLengthSqr > 0.01f) {
                // we are moving, so update dirty rect
                // PX_TRACE("we are moving!: angular: {0}, linear: {1}",
                // pair.second->m_B2Body->GetAngularVelocity(),
                // pair.second->m_B2Body->GetLinearVelocity().LengthSquared());
                m_PXWorld->SetElement(mappedElement.second.worldPos,
                                      mappedElement.second.element);
            } else {
                // we are still, so stop updating region!
                m_PXWorld->SetElementWithoutDirtyRectUpdate(
                    mappedElement.second.worldPos,
                    mappedElement.second.element);
            }

            // we are no longer hidden!
            mappedElement.second.hidden = false;
        }
    }
}

void PixelBody2D::ExitWorld() {}

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
    // center of the pixel body in the world
    glm::ivec2 centerPixelWorld = glm::floor(GetPosition());
    glm::mat4 rotMatrix = GetLocalToWorldTransform();

    for (auto &mappedElement : m_Elements) {
        // find the element in the world by using the transform of the body
        // and the stored local position
        glm::ivec2 skewedPos = mappedElement.first * rotationMatrix;

        mappedElement.second.worldPos =
            glm::ivec2(skewedPos.x, skewedPos.y) + centerPixelWorld;
    }
}

void PixelBody2D::GeneratePixelBody(bool SkipCalculations) {

    // step 1 make sure we aren't completely erased!
    if (m_Elements.size() == 0) {
        return;
    }

    // if we weren't created from a split, we need to check if we have been
    // split or are not contiguous to begin with
    if (!SkipCalculations) {
        glm::vec2 linearVelocity = GetLinearVelocity();
        float angularVelocity = GetAngularVelocity();

        // get the full set of elements for the seed pull set
        std::unordered_set<glm::ivec2, HashVector> source;
        for (auto &pair : m_Elements) {
            source.insert(pair.first);
        }
        // we need to split the elements that are continuous into seperate
        // bodies, so we use a flood algorithm to pull contiguous vertices out
        std::vector<std::unordered_set<glm::ivec2, HashVector>> areas =
            GetContiguousAreas(source);

        // we need to make the split bodies first, because m_elements contains
        // the actual element data
        while (areas.size() > 1) {
            // we have a split body to make!

            // get a list of pixel body elements to turn into the new
            // pixelbody2d
            std::vector<PixelBodyElement> elements;
            for (auto &pos : areas.back()) {
                elements.push_back(m_Elements[pos]);
            }
            // const std::string& name, b2BodyType type, World* world,
            // std::vector<PixelBodyElement>& elements, bool CreatedFromSplit =
            // false); auto newPixelBody2D = PixelBody2D(m_Name, GetType(),
            // m_PXWorld, elements, true);
            auto newPixelBody2D = Instantiate<PixelBody2D>(
                m_Name, GetType(), m_PXWorld, elements, true);
            newPixelBody2D->SetLinearVelocity(linearVelocity);
            newPixelBody2D->SetAngularVelocity(angularVelocity);
            newPixelBody2D->EnterWorld();

            if (m_Parent != nullptr) {
                m_Parent->AddChild(newPixelBody2D);
            }

            areas.pop_back();
        }
        // we only have our area left (or maybe we never even split)

        // find the new width, height & center of the set of elements
        int minX = INT_MAX, maxX = INT_MIN, minY = INT_MAX, maxY = INT_MIN;
        for (auto &localPos : areas[0]) {
            PixelBodyElement &e = m_Elements[localPos];
            minX = std::min(e.worldPos.x, minX);
            maxX = std::max(e.worldPos.x, maxX);
            minY = std::min(e.worldPos.y, minY);
            maxY = std::max(e.worldPos.y, maxY);
        }
        m_Width = std::abs(maxX - minX) + 1;
        m_Height = std::abs(maxY - minY) + 1;

        glm::ivec2 CenterPixelPos =
            glm::vec2((m_Width / 2.0f) + minX, (m_Height / 2.0f) + minY);
        SetPosition(CenterPixelPos);

        // use the minimum to get local positions from bottom left
        std::unordered_map<glm::ivec2, PixelBodyElement, HashVector>
            newElements;
        for (auto &oldPos : areas[0]) {
            auto &pbe = m_Elements[oldPos];
            glm::ivec2 localPos = pbe.worldPos - CenterPixelPos;
            newElements[localPos] = pbe;
        }

        m_Elements = std::move(newElements);
    }

    // now that we have the width, height, and contiguous elements of the pixel
    // body, we can generate the fixtures and such
    // TODO: Implement the greedy binary meshing here!
    //
    //
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
