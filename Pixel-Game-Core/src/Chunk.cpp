#include "Chunk.h"
#include "Pyxis/Renderer/Renderer2D.h"
/*
//create a static body for the chunk
                float RBToWorld = (CHUNKSIZEF / PPU);
                m_OwnedChainBody2D = new ChunkChainBody("ChunkChainBody",
b2_staticBody); m_OwnedChainBody2D->SetPosition({ m_ChunkPos.x, m_ChunkPos.y });
*/

namespace Pyxis {
Chunk::Chunk(glm::ivec2 chunkPos) {
    m_ChunkPos = chunkPos;
    for (int i = 0; i < CHUNKSIZE * CHUNKSIZE; i++) {
        m_Elements[i] = Element();
    }

    // reset dirty rect
    ResetDirtyRect();

    m_Texture = Texture2D::Create(CHUNKSIZE, CHUNKSIZE);
    // set texture to fill color
    std::fill(m_PixelBuffer, m_PixelBuffer + (CHUNKSIZE * CHUNKSIZE),
              0xFF000000);
    m_Texture->SetData(m_PixelBuffer, sizeof(m_PixelBuffer));
}

void Chunk::Clear() {
    for (int i = 0; i < CHUNKSIZE * CHUNKSIZE; i++) {
        m_Elements[i] = Element();
    }

    ResetDirtyRect();

    UpdateTexture();
}

Element &Chunk::GetElement(int x, int y) {
    return m_Elements[x + y * CHUNKSIZE];
}

Element &Chunk::GetElement(const glm::ivec2 &index) {
    return m_Elements[index.x + index.y * CHUNKSIZE];
}
Element &Chunk::GetElement(int index) { return m_Elements[index]; }

void Chunk::SetElement(int x, int y, const Element &element) {
    // test if we are placing a rigid element
    if (element.m_Rigid) {
        // we don't worry about updating the collider if we are setting a rigid
        // element
        m_Elements[x + y * CHUNKSIZE] = element;
        return;
    }
    // test if we are replacing a rigid element
    if (m_Elements[x + y * CHUNKSIZE].m_Rigid) {
        m_Elements[x + y * CHUNKSIZE] = element;

        // we are replacing a rigid element, so we should set the new element to
        // be rigid if it is a solid!
        ElementType newType = ElementData::GetElementProperties(
                                  m_Elements[x + y * CHUNKSIZE].m_ID)
                                  .cell_type;
        if (newType == ElementType::solid)
            m_Elements[x + y * CHUNKSIZE].m_Rigid = true;

        return;
    }

    ElementType currType =
        ElementData::GetElementProperties(m_Elements[x + y * CHUNKSIZE].m_ID)
            .cell_type;
    if (currType == ElementType::solid ||
        currType == ElementType::movableSolid) {
        // we are currently solid, so see if we are changing to something that
        // is not
        currType = ElementData::GetElementProperties(element.m_ID).cell_type;
        if (!(currType == ElementType::solid ||
              currType == ElementType::movableSolid)) {
            // we are no longer solid, so we need to update the collider
            m_MeshChanged = true;
            // set bit array at that spot to 0
            uint64_t AndMask =
                ~(1 << y); // inverse of 1 bitshifted to the position of y.
            m_BitArray[x] &= AndMask;
        }

        m_Elements[x + y * CHUNKSIZE] = element;

    } else {
        // we are not solid, so see if we will become one.
        currType = ElementData::GetElementProperties(element.m_ID).cell_type;
        if (currType == ElementType::solid ||
            currType == ElementType::movableSolid) {
            // we are becoming solid, so we need to update the collider
            m_MeshChanged = true;
            // set bit array at that spot to 1
            m_BitArray[x] |= (1 << y);
        }

        m_Elements[x + y * CHUNKSIZE] = element;
    }
}

/// <summary>
/// updates the dirty rect for the chunk
void Chunk::UpdateDirtyRect(int x, int y) {
    // update minimums
    if (x < m_DirtyRect.min.x + m_DirtyRectBorderWidth)
        m_DirtyRect.min.x = x - m_DirtyRectBorderWidth;
    if (y < m_DirtyRect.min.y + m_DirtyRectBorderWidth)
        m_DirtyRect.min.y = y - m_DirtyRectBorderWidth;
    // update maximums
    if (x > m_DirtyRect.max.x - m_DirtyRectBorderWidth)
        m_DirtyRect.max.x = x + m_DirtyRectBorderWidth;
    if (y > m_DirtyRect.max.y - m_DirtyRectBorderWidth)
        m_DirtyRect.max.y = y + m_DirtyRectBorderWidth;
}

void Chunk::ResetDirtyRect() {
    m_DirtyRect.min.x = CHUNKSIZE - 1;
    m_DirtyRect.min.y = CHUNKSIZE - 1;
    m_DirtyRect.max.x = 0;
    m_DirtyRect.max.y = 0;
}

void Chunk::UpdateTexture() {
    bool dataChanged = false;

    // if min.x <= max.x
    if (m_DirtyRect.min.x <= m_DirtyRect.max.x) {
        dataChanged = true;
        for (int x = std::max(m_DirtyRect.min.x, 0);
             x <= std::min(m_DirtyRect.max.x, CHUNKSIZE - 1); x++) {
            // loop from min x to max x
            for (int y = std::max(m_DirtyRect.min.y, 0);
                 y <= std::min(m_DirtyRect.max.y, CHUNKSIZE - 1); y++) {
                m_PixelBuffer[x + y * CHUNKSIZE] =
                    m_Elements[x + y * CHUNKSIZE].m_Color;
            }
        }
    }

    if (dataChanged) {
        // PX_TRACE("Uploading texture");
        m_Texture->SetData(m_PixelBuffer, sizeof(m_PixelBuffer));
    }
}

void Chunk::UpdateWholeTexture() {
    // set data first, then update the pixels. this allows you to draw over the
    // texture without interupting the actual elements in the map.

    for (int x = 0; x < CHUNKSIZE; x++) {
        for (int y = 0; y < CHUNKSIZE; y++) {
            m_PixelBuffer[x + y * CHUNKSIZE] =
                m_Elements[x + y * CHUNKSIZE].m_Color;
        }
    }
    m_Texture->SetData(m_PixelBuffer, sizeof(m_PixelBuffer));
}

void Chunk::RenderChunk() {
    if (s_DebugChunks) {
        // draw center position
        glm::vec4 chunkStatusColor = {1, 1, 1, 0.2f};
        if (m_MeshGenerated)
            chunkStatusColor = {0, 0, 1, 0.2f};
        if (m_MeshGenerated && m_MeshChanged)
            chunkStatusColor = {0, 1, 1, 0.2f};
        // Renderer2D::DrawQuad({ (m_ChunkPos.x + 0.5f) * CHUNKSIZEF,
        // (m_ChunkPos.y + 0.5f) * CHUNKSIZEF, 20 }, glm::vec2(HALFCHUNKSIZEF),
        // chunkStatusColor);

        // Draw the collider
        glm::vec2 min = {std::max(m_DirtyRect.min.x, 0),
                         std::max(m_DirtyRect.min.y, 0)};
        glm::vec2 max = {std::min(m_DirtyRect.max.x, CHUNKSIZE - 1),
                         std::min(m_DirtyRect.max.y, CHUNKSIZE - 1)};
        if (!(min.x > max.x || min.y > max.y)) {
            float width = (max.x - min.x) + 1;
            float height = (max.y - min.y) + 1;

            float posX = (min.x + (width / 2.0f));
            float posY = (min.y + (height / 2.0f));

            posX += m_ChunkPos.x * CHUNKSIZEF;
            posY += m_ChunkPos.y * CHUNKSIZEF;

            Renderer2D::DrawQuad({posX, posY, 30}, {width, height},
                                 {1, 0, 0, 0.2f});
        }
    }
}

void Chunk::GenerateMesh() { m_MeshGenerated = true; }

void Chunk::AddPreviousMesh() { m_MeshGenerated = true; }

void Chunk::QueuePull(glm::ivec2 startPos,
                      std::unordered_set<glm::ivec2, HashVector> &result,
                      std::unordered_set<glm::ivec2, HashVector> &source) {
    if (source.size() == 0)
        return;

    // make sure the start pos is valid
    if (!source.contains(startPos)) {
        startPos = *source.begin();
    }

    // we make a queue of positions to check
    std::queue<glm::ivec2> queue;
    std::unordered_set<glm::ivec2, HashVector> visited;

    queue.push(startPos);
    visited.insert(startPos);
    result.insert(startPos);
    source.erase(
        startPos); // since we know it is valid, and it won't be reached by loop

    const glm::ivec2 neighbors[] = {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1},
    };

    while (queue.size() > 0) {
        // check neighbors to see if we need to add them to the queue, or
        // have already visited them
        glm::ivec2 position = queue.front();
        queue.pop();

        for (int i = 0; i < 4; i++) {
            glm::ivec2 pos = position + neighbors[i];
            if (!visited.contains(pos) && source.contains(pos)) {
                queue.push(pos);
                result.insert(pos);
                source.erase(pos);
            }
            visited.insert(pos);
        }
    }

    // done!
}
int Chunk::GetMarchingSquareCase(
    const glm::ivec2 &localPosition,
    const std::unordered_set<glm::ivec2, HashVector> &source) {
    int result = 0;
    if (source.contains(localPosition + glm::ivec2(0, 0)))
        result += 8;
    if (source.contains(localPosition + glm::ivec2(-1, 0)))
        result += 4;
    if (source.contains(localPosition + glm::ivec2(0, 1)))
        result += 2;
    if (source.contains(localPosition + glm::ivec2(-1, 1)))
        result += 1;
    return result;
}
std::vector<b2Vec2>
Chunk::SimplifyPoints(const std::vector<b2Vec2> &contourVector, int startIndex,
                      int endIndex, float threshold) {
    float maxDist = 0.0f;
    int maxIndex = 0;
    float dividend = (contourVector[endIndex].x - contourVector[startIndex].x);
    if (dividend != 0) {
        float m = (contourVector[endIndex].y - contourVector[startIndex].y) /
                  dividend;
        float b = contourVector[startIndex].y - m * contourVector[startIndex].x;
        float dividend2 = ((m * m) + 1);
        for (int i = startIndex + 1; i < endIndex; i++) {
            float distToLine =
                std::abs((-m * contourVector[i].x) + contourVector[i].y - b) /
                std::sqrt(dividend2);
            if (distToLine > maxDist) {
                maxDist = distToLine;
                maxIndex = i;
            }
        }
    } else {
        for (int i = startIndex + 1; i < endIndex; i++) {
            float distToLine = contourVector[i].x - contourVector[startIndex].x;
            if (distToLine > maxDist) {
                maxDist = distToLine;
                maxIndex = i;
            }
        }
    }

    if (maxDist > threshold) {
        // do another simplify to the left and right, and combine them as the
        // result
        auto result =
            SimplifyPoints(contourVector, startIndex, maxIndex, threshold);
        auto right =
            SimplifyPoints(contourVector, maxIndex, endIndex, threshold);
        for (int i = 1; i < right.size(); i++) {
            result.push_back(right[i]);
        }
        return result;
    }

    auto endResult = std::vector<b2Vec2>();
    endResult.push_back(contourVector[startIndex]);
    endResult.push_back(contourVector[endIndex]);
    // PX_TRACE("Removed Points");
    return endResult;
}
} // namespace Pyxis
