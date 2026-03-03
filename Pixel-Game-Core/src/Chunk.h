#pragma once

#include "Element.h"
#include "Pyxis/Game/Physics2D.h"
#include "VectorHash.h"
#include <Pyxis.h>
#include <box2d/box2d.h>

namespace Pyxis {
struct DirtyRect {
    glm::ivec2 min = {0, 0};
    glm::ivec2 max = {0, 0};
};

class Chunk {
  public:
    inline static bool s_DebugChunks = false;

    Chunk(glm::ivec2 chunkPos);
    ~Chunk() = default;

    void Clear();

    // Get an element from the chunk via x & y
    Element &GetElement(int x, int y);
    // Get an element from the chunk via vec2
    Element &GetElement(const glm::ivec2 &index);
    // Get an element from the chunk via direct array index
    Element &GetElement(int index);

    void SetElement(int x, int y, const Element &element);

    void UpdateDirtyRect(int x, int y);
    void ResetDirtyRect();

    void UpdateTexture();
    void UpdateWholeTexture();
    void RenderChunk();

    // whether or not this chunk has a static collider
    bool m_MeshGenerated = false;
    bool m_MeshChanged = true;
    void GenerateMesh();
    void AddPreviousMesh();

    void QueuePull(glm::ivec2 startPos,
                   std::unordered_set<glm::ivec2, VectorHash> &result,
                   std::unordered_set<glm::ivec2, VectorHash> &source);

    std::vector<b2Vec2>
    GetContourPoints(const std::unordered_set<glm::ivec2, VectorHash> &source);
    int GetMarchingSquareCase(
        const glm::ivec2 &localPosition,
        const std::unordered_set<glm::ivec2, VectorHash> &source);
    std::vector<b2Vec2> SimplifyPoints(const std::vector<b2Vec2> &contourVector,
                                       int startIndex, int endIndex,
                                       float threshold);

    // core chunk elements
    glm::ivec2 m_ChunkPos;
    Element m_Elements[CHUNKSIZE * CHUNKSIZE];

    // buckets for dirty rects
    int m_DirtyRectBorderWidth = 2;
    DirtyRect m_DirtyRect;
    bool m_PersistDirtyRect = false;

    // textures and rendering
    Ref<Texture2D> m_Texture;
    uint32_t m_PixelBuffer[CHUNKSIZE * CHUNKSIZE];

    // Bitmap array for greedy meshing for collisions & shadows
    uint64_t m_BitArray[64];

    Ref<PhysicsBody2D> m_PhysicsBody;
};
} // namespace Pyxis
