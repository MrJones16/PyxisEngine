#pragma once

#include "Chunk.h"
#include "PixelBody2D.h"
#include "Pyxis/FastNoiseLite/FastNoiseLite.h"
#include "Pyxis/Game/PhysicsBody2D.h"

// networking game messages / input actions
#include "PixelNetworking.h"

// Particles
#include "ElementParticle.h"
#include <random>

namespace Pyxis {

namespace Utils {
// GridQueuePull will fetch a continuous section of the grid set directly out of
// source, destructive! There could still be more things in source if there were
// non-continuous items. You want to keep pulling till source is empty.
// defined here due to template in header at the bottom of the page.
static std::unordered_set<glm::ivec2, VectorHash>
GridQueuePull(std::unordered_set<glm::ivec2, VectorHash> &source) {
    std::unordered_set<glm::ivec2, VectorHash> result;

    if (source.size() == 0)
        return result;

    // make sure the start pos is valid
    glm::ivec2 startPos = *source.begin();

    // we make a queue of positions to check
    std::queue<glm::ivec2> queue;

    queue.push(startPos);
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
            if (!result.contains(pos) && source.contains(pos)) {
                queue.push(pos);
                result.insert(pos);
                source.erase(pos);
            }
        }
    }
    return result;
}

} // namespace Utils

class World {
  public:
    World(std::string assetPath = "assets", int seed = 1337);
    void Initialize(int worldSeed);

    enum class GameDataMsgType : uint8_t { pixelbody, chunk };
    void DownloadWorldInit(Network::Message &msg);
    void DownloadWorld(Network::Message &msg);
    void GetGameDataInit(Network::Message &msg);
    void GetGameData(std::vector<Network::Message> &messages);
    // void GetWorldData(Network::Message& msg);

    ~World();

    Chunk *AddChunk(const glm::ivec2 &chunkPos);
    Chunk *GetChunk(const glm::ivec2 &chunkPos);
    void GenerateChunk(Chunk *chunk);

    // gets the requested element, undefined behavior if the chunk doesn't
    // exist!
    Element &GetElement(const glm::ivec2 &pixelPos);
    // Tries to get element if chunk exists
    bool TryGetElement(const glm::ivec2 &pixelPos, Element &element);
    // loads the chunk if it doesn't exist, then gets the element
    Element &ForceGetElement(const glm::ivec2 &pixelPos);

    void SetElement(const glm::ivec2 &pixelPos, const Element &element);
    void SetElementWithoutDirtyRectUpdate(const glm::ivec2 &pixelPos,
                                          const Element &element);

    void PaintBrushElement(glm::ivec2 pixelPos, uint32_t elementID,
                           BrushType brush, uint8_t brushSize);

    void UpdateWorld();
    void UpdateTextures();
    void UpdateChunk(Chunk *chunk);
    void UpdateChunkDirtyRect(int x, int y, Chunk *chunk);

    // ElementParticle system
    std::vector<ElementParticle> m_ElementParticles;
    void CreateParticle(const glm::vec2 &position, const glm::vec2 &velocity,
                        const Element &element);
    void UpdateParticles();
    void RenderParticles();

    void Clear();
    void RenderWorld();

    void TestMeshGeneration() {
        for (auto &[key, chunk] : m_Chunks) {
            chunk->GenerateMesh();
        }
    }

  public:
    // Create a pixel body. This will return the (if check continuous, the last)
    // body generated from the set of pixels. This fetches the pixel from the
    // world.
    template <typename T = PixelBody2D>
    Ref<T> CreatePixelBody(PhysicsBody2DType type,
                           std::unordered_set<glm::ivec2, VectorHash> pixels,
                           bool CheckIfContinuous = true,
                           const std::string &name = "PixelBody2D");

    void ResetPhysicsDeterminism();
    void DestroyPixelBody(UUID id);
    void DestroyPixelBody(Ref<PixelBody2D> body);
    // PixelRigidBody* CreatePixelRigidBody(uint64_t uuid, const glm::ivec2&
    // size, Element* ElementArray, b2BodyType type = b2_dynamicBody); void
    // PutPixelBodyInWorld(PixelRigidBody& body);
  protected:
    // store pixel bodies as we need to be able to reference them to update
    // their world positions.
    // world will keep ownership of these.
    std::unordered_map<UUID, Ref<PixelBody2D>> m_PixelBodies;

    // take pixel bodies out of the world
    void PullPixelBodies();
    // put pixel bodies back into the world.
    void PushPixelBodies();

  public:
    // moved to game layer and server respectively
    // void HandleTickClosure(MergedTickClosure& tc);
    // Player* CreatePlayer(uint64_t playerID, glm::ivec2 position);

    // random number generation
    std::mt19937 m_RandomEngine;
    std::uniform_int_distribution<int> m_Rand =
        std::uniform_int_distribution<int>(0, 99);
    std::uniform_int_distribution<uint32_t> dist;
    void SeedRandom(int xPos, int yPos);
    int GetRandom(); // 0 - 99 as seen above

    // Helper functions
    static const bool IsInBounds(int x, int y);
    glm::ivec2 WorldToPixel(const glm::vec2 &worldPos);
    glm::ivec2 PixelToChunk(const glm::ivec2 &pixelPos);
    glm::ivec2 PixelToIndex(const glm::ivec2 &pixelPos);

    // map of chunks, ordered so we update in the same order across machines
    std::map<glm::ivec2, Chunk *> m_Chunks;

    // keeping track of theads to join them
    // std::vector<std::thread> m_Threads;

    // extra data needed
    bool m_Running = true;    // Needs to be synchronized
    bool m_UpdateBit = false; // Needs to be synchronized
    bool m_Error = false;

    // temps
    bool m_DebugDrawColliders = true;

    // server mode ignores textures!
    bool m_ServerMode = false;

    // world settings, for generation and gameplay?
    int m_WorldSeed = 1337;        // Needs to be synchronized
    uint64_t m_SimulationTick = 0; // Needs to be synchronized
    FastNoiseLite m_HeightNoise;
    FastNoiseLite m_CaveNoise;
};

template <typename T>
Ref<T> World::CreatePixelBody(PhysicsBody2DType type,
                              std::unordered_set<glm::ivec2, VectorHash> pixels,
                              bool CheckIfContinuous, const std::string &name) {
    static_assert(std::is_base_of_v<PixelBody2D, T>,
                  "T must inherit from PixelBody2D");

    // restrict pixels to solids
    std::unordered_set<glm::ivec2, VectorHash> pixelsRestricted;
    for (auto &pos : pixels) {
        Element &e = ForceGetElement(pos);
        ElementProperties &eData = ElementData::GetElementProperties(e.m_ID);
        if (eData.cell_type == ElementType::solid)
            pixelsRestricted.insert(pos);
    }

    if (pixelsRestricted.size() == 0) {
        // PX_ASSERT(false, "Tried creating a pixel body with no elements!");
        PX_CORE_ERROR("Tried creating a pixel body with no elements!");
        return nullptr;
    }

    // we have a vector of pixels to make a body from, but we don't know if they
    // are continuous
    if (CheckIfContinuous) {
        int iterations = 0;
        while (pixelsRestricted.size() > 0) {
            std::unordered_set<glm::ivec2, VectorHash> continuousPixels =
                Utils::GridQueuePull(pixelsRestricted);
            std::string newName =
                name + (iterations > 0 ? std::format(" ({})", iterations) : "");
            Ref<T> body = Instantiate<T>(newName, type);
            std::vector<PixelBodyElement> elements;
            for (glm::ivec2 pos : continuousPixels) {
                Element &e =
                    GetElement(pos); // don't need to force since i did above
                // e.m_BaseColor = 0xFFFFFFFF;
                // e.m_Color = 0xFFFFFFFF;
                elements.push_back(PixelBodyElement(e, pos));
            }
            body->SetPixelBodyElements(elements);
            m_PixelBodies[body->GetUUID()] = body;

            PX_TRACE("Instantiated {} with {} pixels", newName,
                     continuousPixels.size());

            if (pixelsRestricted.size() == 0) {
                // we pulled the last of the pixels, so return this body.
                return body;
            }
            iterations++;
        }
    } else {
        Ref<T> body = Instantiate<T>(name, type);
        std::vector<PixelBodyElement> elements;
        for (glm::ivec2 pos : pixelsRestricted) {
            Element &e = GetElement(pos);
            // e.m_BaseColor = 0xFFFFFFFF;
            // e.m_Color = 0xFFFFFFFF;
            elements.push_back(PixelBodyElement(e, pos));
        }
        body->SetPixelBodyElements(elements);
        m_PixelBodies[body->GetUUID()] = body;
        PX_TRACE("Instantiated {} with {} pixels", name,
                 pixelsRestricted.size());

        return body;
    }
    // something went wrong!
    PX_CORE_ERROR("Somehow, we didn't create a pixel body here...");
    return nullptr;
}

} // namespace Pyxis
