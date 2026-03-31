#include "World.h"
#include "Element.h"
#include "Pyxis/Game/PhysicsBody2D.h"
#include "Pyxis/Renderer/Renderer2D.h"
// #include "ChunkWorker.h"
#include <Pyxis/Game/Physics2D.h>
#include <glm/gtc/matrix_transform.hpp>
#include <poly2tri.h>
#include <random>
#include <tinyxml2.h>

namespace Pyxis {
namespace Utils {
/// <summary>
/// Line solver using Bresenham's line algorithm
/// </summary>
/// <returns></returns>
std::vector<glm::ivec2> getLinePath(glm::ivec2 startPosition,
                                    glm::ivec2 endPosition) {
    std::vector<glm::ivec2> positions;
    glm::ivec2 current = startPosition;
    positions.push_back(current);

    int dx = endPosition.x - startPosition.x;
    int dy = endPosition.y - startPosition.y;
    int steps = std::max(std::abs(dx), std::abs(dy));

    if (steps == 0)
        return positions;

    float xInc = static_cast<float>(dx) / steps;
    float yInc = static_cast<float>(dy) / steps;

    for (int i = 0; i < steps; i++) {
        current.x = startPosition.x + static_cast<int>(round(xInc * (i + 1)));
        current.y = startPosition.y + static_cast<int>(round(yInc * (i + 1)));
        positions.push_back(current);
    }

    return positions;
}

} // namespace Utils

World::World(std::string assetPath, int seed) {
    Physics2D::GetWorld(); // make sure that the physics world is made
    // load the element data
    // make sure the file exists
    if (!std::filesystem::exists(assetPath + "/data/CellData.json")) {
        PX_ASSERT(false, "CellData.json file is missing!");
        PX_ERROR("CellData.json file is missing!");
        m_Error = true;
        Application::Get().Sleep(10000);
        Application::Get().Close();
        return;
    }

    ElementData::LoadElementData(assetPath + "/data/CellData.json");

    // set up world noise data
    Initialize(seed);

    //////////// PRIOR XML SETUP ////////////
    /*if (!std::filesystem::exists(assetPath + "/data/CellData.xml"))
    {
            PX_ASSERT(false, "Failed to load element data, shutting down.");
            PX_ERROR("Failed to load element data, shutting down.");
            m_Error = true;
            Application::Get().Sleep(10000);
            Application::Get().Close();
            return;
    }*/
    // LoadXMLElementData(assetPath);
    ////Output the element data to a file
    // std::ofstream file(assetPath + "/data/CellData.json");
    // json j;
    // for (auto& elementData : ElementData::s_ElementData)
    //{
    //	j += elementData;
    // }
    // for (auto& reaction : ElementData::s_Reactions)
    //{
    //	j += reaction;
    // }
    // file << j;
    // file.close();?
    //////////// PRIOR XML SETUP ////////////
}

void World::Initialize(int worldSeed) {
    m_HeightNoise = FastNoiseLite(m_WorldSeed);
    m_CaveNoise = FastNoiseLite(m_WorldSeed);
}

/// <summary>
/// Takes the message with the world data, and loads the world with it
/// Expects this order to pull items out
///
/// world seed				| int
///
/// How many chunks			| uint32
///		chunk pos			| ivec2
///		chunk dirtyrects	| pair<glm::ivec2,glm::ivec2>
///		chunk data			| Element...
/// //is probably different now!
/// how many pixel bodies	| uint32
///		rigid body data		|
///			id				| uint64_t
///			size			| glm::ivec2
///			array			| Element*
///			position
///			rotation
///			type
///			angular velocity
///			linear velocity
/// </summary>
/// <param name="msg"></param>
void World::DownloadWorldInit(Network::Message &msg) {
    msg >> m_Running;
    msg >> m_WorldSeed;
    Initialize(m_WorldSeed);
    msg >> m_UpdateBit;
    msg >> m_SimulationTick;
}

void World::DownloadWorld(Network::Message &msg) {
    if ((GameMessage)msg.header.id == GameMessage::Server_GameDataRigidBody) {
        // lets load the pixel body! just reverse the upload order.
        std::vector<uint8_t> msgpack;
        msg >> msgpack;
        json j = json::from_msgpack(msgpack);
        Ref<PixelBody2D> PixelBodyNode =
            dynamic_pointer_cast<PixelBody2D>(Node::DeserializeNode(j));
        if (PixelBodyNode)
            m_PixelBodies[PixelBodyNode->GetUUID()] = PixelBodyNode;
    }
    if ((GameMessage)msg.header.id == GameMessage::Server_GameDataChunk) {
        glm::ivec2 chunkPos;
        msg >> chunkPos;

        PX_ASSERT(m_Chunks.find(chunkPos) == m_Chunks.end(),
                  "Tried to load a chunk that already existed");
        Chunk *chunk = new Chunk(chunkPos);
        msg >> chunk->m_MeshChanged;
        msg >> chunk->m_DirtyRect;
        m_Chunks[chunkPos] = chunk;
        for (int ii = (CHUNKSIZE * CHUNKSIZE) - 1; ii >= 0; ii--) {
            msg >> chunk->m_Elements[ii];
        }

        chunk->UpdateWholeTexture();

        PX_TRACE("Loaded Chunk ({0},{1})", chunkPos.x, chunkPos.y);
    }
}

void World::GetGameDataInit(Network::Message &msg) {
    PX_TRACE("Gathering World Data");
    msg.header.id = static_cast<uint32_t>(GameMessage::Server_GameDataInit);
    msg << static_cast<uint32_t>(m_Chunks.size());
    PX_TRACE("# Chunks: {0}", m_Chunks.size());
    msg << static_cast<uint32_t>(m_PixelBodies.size());
    PX_TRACE("# RigidBodies: {0}", Physics2D::GetWorld().GetBodyCount());
    msg << m_SimulationTick;
    msg << m_UpdateBit;
    msg << m_WorldSeed;
    msg << m_Running;
}

void World::GetGameData(std::vector<Network::Message> &messages) {
    for (auto kvp : m_PixelBodies) {
        messages.emplace_back();
        messages.back().header.id =
            static_cast<uint32_t>(GameMessage::Server_GameDataRigidBody);
        messages.back() << kvp.second->SerializeBinary();
    }

    // now we create a separate message for each chunk
    for (auto &pair : m_Chunks) {
        messages.emplace_back();
        messages.back().header.id =
            static_cast<uint32_t>(GameMessage::Server_GameDataChunk);
        for (int i = 0; i < CHUNKSIZE * CHUNKSIZE; i++) {
            messages.back() << pair.second->m_Elements[i];
        }
        messages.back() << pair.second->m_DirtyRect;
        messages.back() << pair.second->m_MeshChanged;
        messages.back() << pair.first;
    }
}

World::~World() {
    // PX_TRACE("Deleting World");

    // Physics2D::DestroyWorld();
    for (auto &pair : m_Chunks) {
        delete (pair.second);
    }
}

Chunk *World::AddChunk(const glm::ivec2 &chunkPos) {
    // make sure chunk doesn't already exist
    if (m_Chunks.find(chunkPos) == m_Chunks.end()) {
        Chunk *chunk = new Chunk(chunkPos);
        m_Chunks[chunkPos] = chunk;
        GenerateChunk(chunk);
        chunk->UpdateWholeTexture();
        return chunk;
    }

    return m_Chunks[chunkPos];
}

Chunk *World::GetChunk(const glm::ivec2 &chunkPos) {
    auto it = m_Chunks.find(chunkPos);
    if (it != m_Chunks.end())
        return it->second;
    return nullptr;

    /*AddChunk(chunkPos);
    return m_Chunks[chunkPos];*/
}

void World::GenerateChunk(Chunk *chunk) {
    glm::vec2 chunkPixelPos = chunk->m_ChunkPos * CHUNKSIZE;
    for (int x = 0; x < CHUNKSIZE; x++) {
        for (int y = 0; y < CHUNKSIZE; y++) {
            glm::vec2 pixelPos = chunkPixelPos + glm::vec2(x, y);
            if (pixelPos.y >= 0) {
                float amplitude = 20.0f;
                float heightNoise =
                    ((m_HeightNoise.GetNoise(pixelPos.x, 0.0f) + 1) / 2) *
                    amplitude;
                float surfaceTop = heightNoise + 80;
                float grassWidth = 20;
                if (pixelPos.y > surfaceTop + grassWidth) {
                    // air
                } else if (pixelPos.y > surfaceTop) {
                    chunk->SetElement(x, y,
                                      ElementData::GetElement("grass", x, y));
                } else if (pixelPos.y > heightNoise) {
                    // dirt
                    chunk->SetElement(x, y,
                                      ElementData::GetElement("dirt", x, y));
                } else {
                    // under the noise value, so stone, blended into the caves
                    float caveNoise =
                        (m_CaveNoise.GetNoise(pixelPos.x, pixelPos.y) + 1) /
                        2.0f;
                    if (caveNoise >= 0.25f) {
                        chunk->SetElement(
                            x, y, ElementData::GetElement("stone", x, y));
                    }
                }

            } else {
                // under y==0, so
                float caveNoise =
                    (m_CaveNoise.GetNoise(pixelPos.x, pixelPos.y) + 1) / 2.0f;
                if (caveNoise >= 0.25f) {
                    chunk->SetElement(x, y,
                                      ElementData::GetElement("stone", x, y));
                }
            }
        }
    }
}

Element &World::GetElement(const glm::ivec2 &pixelPos) {
    auto chunkPos = PixelToChunk(pixelPos);
    auto index = PixelToIndex(pixelPos);
    return GetChunk(chunkPos)->GetElement(index);
}

bool World::TryGetElement(const glm::ivec2 &pixelPos, Element &element) {
    if (m_Chunks.contains(PixelToChunk(pixelPos))) {
        element = GetChunk(PixelToChunk(pixelPos))
                      ->GetElement(PixelToIndex(pixelPos));
        return true;
    }
    return false;
}

Element &World::ForceGetElement(const glm::ivec2 &pixelPos) {
    auto chunkPos = PixelToChunk(pixelPos);
    auto index = PixelToIndex(pixelPos);
    if (!m_Chunks.contains(chunkPos)) {
        AddChunk(chunkPos);
    }
    return GetChunk(chunkPos)->GetElement(PixelToIndex(pixelPos));
}

void World::SetElement(const glm::ivec2 &pixelPos, const Element &element) {
    Chunk *chunk = GetChunk(PixelToChunk(pixelPos));
    auto index = PixelToIndex(pixelPos);
    if (element.m_ID == ElementData::s_ElementNameToID["debug_heat"]) {
        chunk->GetElement(index).m_Temperature++;
    } else if (element.m_ID == ElementData::s_ElementNameToID["debug_cool"]) {
        chunk->GetElement(index).m_Temperature--;
    } else {
        chunk->SetElement(index.x, index.y, element);
    }
    UpdateChunkDirtyRect(index.x, index.y, chunk);
}

void World::SetElementWithoutDirtyRectUpdate(const glm::ivec2 &pixelPos,
                                             const Element &element) {
    Chunk *chunk = GetChunk(PixelToChunk(pixelPos));
    auto index = PixelToIndex(pixelPos);
    if (element.m_ID == ElementData::s_ElementNameToID["debug_heat"]) {
        chunk->GetElement(index).m_Temperature++;
    } else if (element.m_ID == ElementData::s_ElementNameToID["debug_cool"]) {
        chunk->GetElement(index).m_Temperature--;
    } else {
        chunk->SetElement(index.x, index.y, element);
    }
}

void World::PaintBrushElement(glm::ivec2 pixelPos, uint32_t elementID,
                              BrushType brush, uint8_t brushSize) {
    std::unordered_set<Chunk *> chunksToUpdate;

    glm::ivec2 newPos = pixelPos;
    for (int x = -brushSize; x <= brushSize; x++) {
        for (int y = -brushSize; y <= brushSize; y++) {
            newPos = pixelPos + glm::ivec2(x, y);
            Chunk *chunk;
            glm::ivec2 index;
            switch (brush) {
            case BrushType::circle:
                // limit brush to circle
                if (std::sqrt((float)(x * x) + (float)(y * y)) >= brushSize)
                    continue;
                break;
            case BrushType::square:
                break;
            case BrushType::end:
                break;
            }

            // get chunk / index
            glm::ivec2 chunkPos = PixelToChunk(newPos);
            chunk = GetChunk(chunkPos);
            if (chunk == nullptr) {
                chunk = AddChunk(chunkPos);
            }
            chunksToUpdate.insert(chunk);
            index = PixelToIndex(newPos);

            // get element / color
            Element element = Element();
            ElementProperties &elementData =
                ElementData::GetElementProperties(elementID);
            element.m_ID = elementID;
            element.m_Updated = !m_UpdateBit;
            elementData.UpdateElementProperties(element, index.x, index.y);

            // set the element
            if (element.m_ID == ElementData::s_ElementNameToID["debug_heat"]) {
                chunk->GetElement(index).m_Temperature++;
            } else if (element.m_ID ==
                       ElementData::s_ElementNameToID["debug_cool"]) {
                chunk->GetElement(index).m_Temperature--;
            } else {
                chunk->SetElement(index.x, index.y, element);
            }
            chunk->UpdateDirtyRect(index.x, index.y);
        }
    }
    for (auto chunk : chunksToUpdate) {
        chunk->UpdateTexture();
    }
}

void World::PullPixelBodies() {
    // loop over bodies, and get list of current bodies. Also free them if they
    // want to be freed.
    std::vector<Ref<PixelBody2D>> bodies;
    std::vector<UUID> idsToFree;
    for (auto kvp : m_PixelBodies) {
        if (kvp.second->m_FreeMe) {
            idsToFree.push_back(kvp.first);
            kvp.second->ActuallyQueueFree();
        } else {
            // only pushing alive bodies onto the list to pull.
            bodies.push_back(kvp.second);
        }
    }
    // list of ids to remove from world list, can't do during iteration over it!
    for (auto &id : idsToFree) {
        m_PixelBodies.erase(id);
    }

    // we now have a list of bodies that are still alive. Lets iterate over the
    // list and pull them out.
    for (Ref<PixelBody2D> body : bodies) {
        // skip if we are sleeping!
        if (!body->GetAwake()) {
            PX_TRACE("Skipping because it's asleep!");
            continue; // leave sleeping bodies in!
        }

        // keep list of elements to take out after iteration
        std ::vector<glm::ivec2> elementsToRemove;

        // loop over all elements, and attempt to take them out of the world
        for (auto &mappedElement : body->m_Elements) {

            // the world position of the element is already known, so just
            // try to grab it
            Element &worldElement = GetElement(mappedElement.second.worldPos);
            ElementProperties &worldElementData =
                ElementData::GetElementProperties(worldElement.m_ID);

            if (mappedElement.second.element.m_ID !=
                worldElement
                    .m_ID) // || !worldElement.m_Rigid TODO re-implement rigid?
            {
                // element has changed over the last update!

                if (worldElementData.cell_type == ElementType::solid ||
                    (worldElementData.cell_type == ElementType::movableSolid &&
                     worldElement.m_Rigid)) {
                    // replaced element is able to continue being part of
                    // the solid! this could be the player replacing the
                    // blocks, or a solid block reacts with something and
                    // stays solid, like getting stained or something idk
                    // either way, in this situation we just pull the new
                    // element
                    mappedElement.second.element = worldElement;
                    mappedElement.second.element.m_Rigid = true;
                    SetElementWithoutDirtyRectUpdate(
                        mappedElement.second.worldPos, Element());
                } else {
                    // the element that has taken over the spot is not able
                    // to be a solid, so we need to re-construct the rigid
                    // body without that element! so we leave it in the sim,
                    // and erase the previous from the body
                    elementsToRemove.push_back(mappedElement.first);
                }
            } else {
                // element should be the same, so nothing has changed, pull
                // the element out
                // PX_TRACE("pulling out pbe ({0},{1})",
                //         mappedElement.second.worldPos.x,
                //         mappedElement.second.worldPos.y);
                mappedElement.second.element = worldElement;
                // replace with default element
                SetElementWithoutDirtyRectUpdate(mappedElement.second.worldPos,
                                                 Element());
            }
        }

        // now that we pulled all the elements out, try to re-construct the
        // body if needed:
        if (elementsToRemove.size() > 0) {
            // we need to reconstruct!
            // remove the outdated elements
            for (auto &localPos : elementsToRemove) {
                body->m_Elements.erase(localPos);
            }

            // gather up local positions of elements
            std::unordered_set<glm::ivec2, VectorHash> source;
            for (auto kvp : body->m_Elements) {
                source.insert(kvp.first);
            }

            // get a continuous section of the local positions
            auto firstPull = Utils::GridQueuePull(source);
            // firstpull should be the main body, as it may still be fully
            // intact! let's remove whats in source if there's any
            // straglers, and make it into it's own body.

            std::unordered_set<glm::ivec2, VectorHash> pixels;
            for (glm::ivec2 pos : source) {
                // source is now what is remaining after pulling out a
                // continuous set.

                // put the remainder back into the world
                SetElementWithoutDirtyRectUpdate(body->m_Elements[pos].worldPos,
                                                 body->m_Elements[pos].element);

                // lets remove that remainder from this
                // pixelbody, and make new pixel bodies from it after.
                // we also need to track the world positions for the
                // creation of the remainder.
                pixels.insert(body->m_Elements[pos].worldPos);
                body->m_Elements.erase(pos);
            }

            // create the new pixel bodies from the remainder.
            if (pixels.size() > 0) {
                PhysicsBody2DDef def;
                def.type = body->GetType();
                def.linearVelocity = body->GetLinearVelocity();
                def.angularVelocity = body->GetAngularVelocity();
                auto newBody =
                    CreatePixelBody(def, pixels, true, body->m_Name + "-Split");
            }

            if (body->m_Elements.size() == 0) {
                body->QueueFree();
                body->RemoveShapes();

            } else {
                // recalculate the bitarrays and collider/mesh
                body->GenerateMesh();
            }
        }
        // End off with saying this body is no longer in the world. We may even
        // be dead!
        body->m_InWorld = false;
    }
}

void World::PushPixelBodies() {
    // put the elements back into the simulation.
    // no need to copy list, as we won't change it here.
    for (auto kvp : m_PixelBodies) {
        Ref<PixelBody2D> body = kvp.second;
        // PX_TRACE("Pushing PixelBody2D[{}] back into world", kvp.first);
        // PX_TRACE("Position: ({0},{1})", body->GetPosition().x,
        //          body->GetPosition().y);
        if (body->m_InWorld) {
            // PX_TRACE("Skipping because its still in world!");
            // this would include bodies made during last pull as in world is
            // default
            continue; // body is already in world
        }

        // chunkloading
        for (int x = -1; x < 2; x++) {
            for (int y = -1; y < 2; y++) {
                AddChunk(PixelToChunk(WorldToPixel(body->GetPosition())) +
                         glm::ivec2(x, y));
            }
        }

        // update world positions to put back into new locations
        body->UpdateElementWorldPositions();

        body->m_InWorld = true;
        if (body->m_Moved) {
            for (auto &mappedElement : body->m_Elements) {

                // check if there is an element in the way in the world
                Element &e = GetElement(mappedElement.second.worldPos);
                // if (e.m_ID != 0)
                //     CreateParticle(
                //         mappedElement.second.worldPos,
                //         body->GetLocalPixelVelocity(mappedElement.first),
                //         e);

                // PX_TRACE("putting pbe ({0},{1}) back in",
                //          mappedElement.second.worldPos.x,
                //          mappedElement.second.worldPos.y);
                SetElement(mappedElement.second.worldPos,
                           mappedElement.second.element);
            }
        } else {
            // we are still, so put back without updating dirty rect
            // PX_TRACE("we didn't move, so put back in without rect update");
            for (auto &mappedElement : body->m_Elements) {

                // check if there is an element in the way in the world
                Element &e = GetElement(mappedElement.second.worldPos);

                // if (e.m_ID != 0)
                //    CreateParticle(
                //         mappedElement.second.worldPos,
                //         body->GetLocalPixelVelocity(mappedElement.first), e);
                //  PX_TRACE("putting pbe ({0},{1}) back in, but still",
                //           mappedElement.second.worldPos.x,
                //           mappedElement.second.worldPos.y);
                SetElementWithoutDirtyRectUpdate(mappedElement.second.worldPos,
                                                 mappedElement.second.element);
            }
        }
    }
}

void World::UpdateWorld() {
    /*m_Threads.clear();
    for each (auto& pair in m_Chunks)
    {
            m_Threads.push_back(std::thread(&World::UpdateChunk, pair.second));
    }
    for each (std::thread& thread in m_Threads)
    {
            thread.join();
    }*/

    UpdateParticles();

    for (auto &[pos, chunk] : m_Chunks) {
        UpdateChunk(chunk);
    }

    if (!m_ServerMode) {
        for (auto &pair : m_Chunks) {
            pair.second->UpdateTexture();
        }
    }

    // TODO STILL
    //  pull pixelbodies out
    PullPixelBodies();
    Physics2D::GetWorld().Step();
    PushPixelBodies();
    // put pixelbodies back in
    TestMeshGeneration();

    m_UpdateBit = !m_UpdateBit;
    m_SimulationTick++;
}

void World::UpdateTextures() {
    for (auto &pair : m_Chunks) {
        pair.second->UpdateTexture();
    }
}

/// <summary>
/// returns the percent value of the value between the points, from 0-1
/// </summary>

float interpolateBetweenValues(float lower, float higher, float val) {
    // 460, 6000, 1000
    return std::min((std::max(lower, val) - lower), higher - lower) /
           (higher - lower);
}

/// <summary>
///
/// updating a chunk overview:
/// copy the dirt rect state, and shrink it by 1
/// loop over the elements, and update them. if the elements they are trying to
/// swap with is out of bounds, then find the other chunk and get the element
/// like that. because an element can belong to another chunk, there is a lot
/// more conditional logic. const int BUCKETS = chunk->BUCKETS; const int
/// BUCKETSIZE = chunk->BUCKETSIZE;
/// </summary>
void World::UpdateChunk(Chunk *chunk) {
    // make a copy of the dirty rect before shrinking it
    DirtyRect dirtyRect = chunk->m_DirtyRect;
    // instead of resetting completely, just shrink by 1. This allows elements
    // that cross borders to update, since otherwise the dirty rect would forget
    // about it instead of shrinking and still hitting it.
    chunk->m_DirtyRect.min = chunk->m_DirtyRect.min + 1;
    chunk->m_DirtyRect.max = chunk->m_DirtyRect.max - 1;

    // TODO: FIX WAKEUP
    //  make sure to wake up any pixel bodies in area
    // b2AABB queryRegion;
    // glm::vec2 lower =
    //     glm::vec2(dirtyRect.min + (chunk->m_ChunkPos * CHUNKSIZE)) / PPU;
    // glm::vec2 upper =
    //     glm::vec2(dirtyRect.max + (chunk->m_ChunkPos * CHUNKSIZE)) / PPU;
    // queryRegion.lowerBound = b2Vec2(lower.x, lower.y);
    // queryRegion.upperBound = b2Vec2(upper.x, upper.y);
    // WakeUpQueryCallback callback;
    // Physics2D::GetWorld()->QueryAABB(&callback, queryRegion);

    // get the min and max
    // loop from min to max in both "axies"?
    bool minToMax = m_UpdateBit;

    // PX_TRACE("Update Bit: {0}", m_UpdateBit);

    // first lets seed random, so the simulation is deterministic!
    SeedRandom(chunk->m_ChunkPos.x, chunk->m_ChunkPos.y);

    ///////////////////////////////////////////////////////////////
    /// Main Update Loop, back and forth bottom to top, left,right,left....
    ///////////////////////////////////////////////////////////////
    int startY;
    int compareY;
    if (minToMax) {
        // start at the smallest, and go the largest
        startY = std::max(dirtyRect.min.y, 0);
        compareY = std::min(dirtyRect.max.y, CHUNKSIZE - 1) + 1;
    } else {
        // start at the largest, and go the smallest
        startY = std::min(dirtyRect.max.y, CHUNKSIZE - 1);
        compareY = std::max(dirtyRect.min.y, 0) - 1;
    }

    if (dirtyRect.min.x <= dirtyRect.max.x &&
        dirtyRect.min.y <= dirtyRect.max.y)
        for (int y = startY; y != compareY;
             m_UpdateBit ? y++ : y--) // going y then x so we do criss crossing
        {

            minToMax = !minToMax;
            int startX;
            int compareX;
            if (minToMax) {
                // start at the smallest, and go the largest
                startX = std::max(dirtyRect.min.x, 0);
                compareX = std::min(dirtyRect.max.x, CHUNKSIZE - 1) + 1;
            } else {
                // start at the largest, and go the smallest
                startX = std::min(dirtyRect.max.x, CHUNKSIZE - 1);
                compareX = std::max(dirtyRect.min.x, 0) - 1;
            }
            // PX_TRACE("x is: {0}", x);
            for (int x = startX; x != compareX; minToMax ? x++ : x--) {
                // we now have an x and y of the element in the array, so update
                // it

                Element &currElement = chunk->GetElement(x, y);
                ElementProperties &currElementData =
                    ElementData::GetElementProperties(currElement.m_ID);

                // skip if already updated
                if (currElement.m_Updated == m_UpdateBit)
                    continue;
                // flip the update bit so we know we updated this element
                currElement.m_Updated = m_UpdateBit;

                if (currElement.m_ID == 0)
                    continue;

                int xOther = x;
                int yOther = y;

                int r = 0;

                // iterators for reaction lookup
                std::unordered_map<uint32_t, ReactionResult>::iterator it;
                std::unordered_map<uint32_t, ReactionResult>::iterator end;

                Element *elementTop;
                Element *elementBottom;
                Element *elementRight;
                Element *elementLeft;

                Chunk *leftChunk = chunk;
                Chunk *rightChunk = chunk;
                Chunk *topChunk = chunk;
                Chunk *bottomChunk = chunk;

                // get cardinal elements
                {
                    if (IsInBounds(x, y + 1)) {
                        elementTop = &(chunk->GetElement(x, y + 1));
                    } else {
                        // see if the chunk exists, if it does then get that
                        // element, otherwise nullptr
                        glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE +
                                                glm::ivec2(x, y + 1);
                        topChunk = GetChunk(PixelToChunk(pixelSpace));
                        if (topChunk != nullptr) {
                            int indexOther =
                                ((((x) + CHUNKSIZE) % CHUNKSIZE) +
                                 (((y + 1) + CHUNKSIZE) % CHUNKSIZE) *
                                     CHUNKSIZE);
                            elementTop = &(topChunk->GetElement(indexOther));
                        } else {
                            elementTop = nullptr;
                        }
                    }

                    if (IsInBounds(x, y - 1)) {
                        elementBottom = &(chunk->GetElement(x, y - 1));
                    } else {
                        // see if the chunk exists, if it does then get that
                        // element, otherwise nullptr
                        glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE +
                                                glm::ivec2(x, y - 1);
                        bottomChunk = GetChunk(PixelToChunk(pixelSpace));
                        if (bottomChunk != nullptr) {
                            int indexOther =
                                ((((x) + CHUNKSIZE) % CHUNKSIZE) +
                                 (((y - 1) + CHUNKSIZE) % CHUNKSIZE) *
                                     CHUNKSIZE);
                            elementBottom =
                                &(bottomChunk->GetElement(indexOther));
                        } else {
                            elementBottom = nullptr;
                        }
                    }

                    if (IsInBounds(x + 1, y)) {
                        elementRight = &(chunk->GetElement(x + 1, y));
                    } else {
                        // see if the chunk exists, if it does then get that
                        // element, otherwise nullptr
                        glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE +
                                                glm::ivec2(x + 1, y);
                        rightChunk = GetChunk(PixelToChunk(pixelSpace));
                        if (rightChunk != nullptr) {
                            int indexOther =
                                ((((x + 1) + CHUNKSIZE) % CHUNKSIZE) +
                                 (((y) + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE);
                            elementRight =
                                &(rightChunk->GetElement(indexOther));
                        } else {
                            elementRight = nullptr;
                        }
                    }

                    if (IsInBounds(x - 1, y)) {
                        elementLeft = &(chunk->GetElement(x - 1, y));
                    } else {
                        // see if the chunk exists, if it does then get that
                        // element, otherwise nullptr
                        glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE +
                                                glm::ivec2(x - 1, y);
                        leftChunk = GetChunk(PixelToChunk(pixelSpace));
                        if (leftChunk != nullptr) {
                            int indexOther =
                                ((((x - 1) + CHUNKSIZE) % CHUNKSIZE) +
                                 (((y) + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE);
                            elementLeft = &(leftChunk->GetElement(indexOther));
                        } else {
                            elementLeft = nullptr;
                        }
                    }
                }

                ElementProperties *elementLeftData =
                    (elementLeft != nullptr)
                        ? &ElementData::GetElementProperties(elementLeft->m_ID)
                        : nullptr;
                ElementProperties *elementRightData =
                    (elementRight != nullptr)
                        ? &ElementData::GetElementProperties(elementRight->m_ID)
                        : nullptr;
                ElementProperties *elementTopData =
                    (elementTop != nullptr)
                        ? &ElementData::GetElementProperties(elementTop->m_ID)
                        : nullptr;
                ElementProperties *elementBottomData =
                    (elementBottom != nullptr)
                        ? &ElementData::GetElementProperties(
                              elementBottom->m_ID)
                        : nullptr;
                // check for reactions, left,up,right,down
                {
                    if (elementLeft != nullptr) {
                        it =
                            ElementData::s_ReactionTable[currElement.m_ID].find(
                                elementLeft->m_ID);
                        end = ElementData::s_ReactionTable[currElement.m_ID]
                                  .end();
                        if (it != end && GetRandom() < it->second.probability) {
                            currElement.m_ID = it->second.cell0ID;
                            ElementProperties &ed0 =
                                ElementData::GetElementProperties(
                                    it->second.cell0ID);
                            ed0.UpdateElementProperties(currElement, x, y);
                            if (ed0.cell_type == ElementType::solid ||
                                ed0.cell_type == ElementType::movableSolid) {
                                chunk->m_MeshChanged = true;
                            }

                            elementLeft->m_ID = it->second.cell1ID;
                            ElementProperties &ed1 =
                                ElementData::GetElementProperties(
                                    it->second.cell1ID);
                            if (ed1.cell_type == ElementType::solid ||
                                ed1.cell_type == ElementType::movableSolid) {
                                leftChunk->m_MeshChanged = true;
                            }
                            ed1.UpdateElementProperties(elementLeft, x - 1, y);
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        }
                    }

                    if (elementTop != nullptr) {
                        it =
                            ElementData::s_ReactionTable[currElement.m_ID].find(
                                elementTop->m_ID);
                        end = ElementData::s_ReactionTable[currElement.m_ID]
                                  .end();
                        if (it != end && GetRandom() < it->second.probability) {
                            currElement.m_ID = it->second.cell0ID;
                            ElementProperties &ed0 =
                                ElementData::GetElementProperties(
                                    it->second.cell0ID);
                            ed0.UpdateElementProperties(currElement, x, y);
                            if (ed0.cell_type == ElementType::solid ||
                                ed0.cell_type == ElementType::movableSolid) {
                                chunk->m_MeshChanged = true;
                            }

                            elementTop->m_ID = it->second.cell1ID;
                            ElementProperties &ed1 =
                                ElementData::GetElementProperties(
                                    it->second.cell1ID);
                            if (ed1.cell_type == ElementType::solid ||
                                ed1.cell_type == ElementType::movableSolid) {
                                topChunk->m_MeshChanged = true;
                            }
                            ed1.UpdateElementProperties(elementTop, x - 1, y);
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        }
                    }

                    if (elementRight != nullptr) {
                        it =
                            ElementData::s_ReactionTable[currElement.m_ID].find(
                                elementRight->m_ID);
                        end = ElementData::s_ReactionTable[currElement.m_ID]
                                  .end();
                        if (it != end && GetRandom() < it->second.probability) {
                            currElement.m_ID = it->second.cell0ID;
                            ElementProperties &ed0 =
                                ElementData::GetElementProperties(
                                    it->second.cell0ID);
                            ed0.UpdateElementProperties(currElement, x, y);
                            if (ed0.cell_type == ElementType::solid ||
                                ed0.cell_type == ElementType::movableSolid) {
                                chunk->m_MeshChanged = true;
                            }

                            elementRight->m_ID = it->second.cell1ID;
                            ElementProperties &ed1 =
                                ElementData::GetElementProperties(
                                    it->second.cell1ID);
                            if (ed1.cell_type == ElementType::solid ||
                                ed1.cell_type == ElementType::movableSolid) {
                                rightChunk->m_MeshChanged = true;
                            }
                            ed1.UpdateElementProperties(elementRight, x - 1, y);
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        }
                    }

                    if (elementBottom != nullptr) {
                        it =
                            ElementData::s_ReactionTable[currElement.m_ID].find(
                                elementBottom->m_ID);
                        end = ElementData::s_ReactionTable[currElement.m_ID]
                                  .end();
                        if (it != end && GetRandom() < it->second.probability) {
                            currElement.m_ID = it->second.cell0ID;
                            ElementProperties &ed0 =
                                ElementData::GetElementProperties(
                                    it->second.cell0ID);
                            ed0.UpdateElementProperties(currElement, x, y);
                            if (ed0.cell_type == ElementType::solid ||
                                ed0.cell_type == ElementType::movableSolid) {
                                chunk->m_MeshChanged = true;
                            }

                            elementBottom->m_ID = it->second.cell1ID;
                            ElementProperties &ed1 =
                                ElementData::GetElementProperties(
                                    it->second.cell1ID);
                            if (ed1.cell_type == ElementType::solid ||
                                ed1.cell_type == ElementType::movableSolid) {
                                bottomChunk->m_MeshChanged = true;
                            }
                            ed1.UpdateElementProperties(elementBottom, x - 1,
                                                        y);
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        }
                    }
                }

                // handle universal element properties
                if (currElement.m_Temperature >=
                    currElementData.melting_point) {
                    // melt
                    if (currElementData.melted != "") {
                        int newID =
                            ElementData::s_ElementNameToID[currElementData
                                                               .melted];
                        int temp = currElement.m_Temperature;
                        ElementProperties &newData =
                            ElementData::GetElementProperties(newID);
                        newData.UpdateElementProperties(currElement, x, y);
                        if (newData.cell_type == ElementType::solid ||
                            newData.cell_type == ElementType::movableSolid) {
                            chunk->m_MeshChanged = true;
                        }
                        currElement.m_Temperature = temp;
                        currElement.m_ID = newID;
                        chunk->UpdateDirtyRect(x, y);
                        continue;
                    }
                }

                if (currElement.m_Temperature <=
                    currElementData.freezing_point) {
                    // freeze
                    if (currElementData.frozen != "") {
                        int newID =
                            ElementData::s_ElementNameToID[currElementData
                                                               .frozen];
                        int temp = currElement.m_Temperature;

                        ElementProperties &newData =
                            ElementData::GetElementProperties(newID);
                        newData.UpdateElementProperties(currElement, x, y);
                        if (newData.cell_type == ElementType::solid ||
                            newData.cell_type == ElementType::movableSolid) {
                            chunk->m_MeshChanged = true;
                        }

                        currElement.m_Temperature = temp;
                        currElement.m_ID = newID;
                        chunk->UpdateDirtyRect(x, y);
                        continue;
                    }
                }

                if (currElementData.conductivity > 0) {
                    float tempBefore = currElement.m_Temperature;
                    int minConductivity = 0;
                    float diff = 0;

                    if (elementLeft != nullptr && elementLeft->m_ID != 0) {
                        minConductivity =
                            std::min(currElementData.conductivity,
                                     elementLeftData->conductivity);
                        diff = ((currElement.m_Temperature -
                                 elementLeft->m_Temperature) *
                                ((float)minConductivity / 100.0f)) /
                               2;
                        if (diff != 0) {
                            currElement.m_Temperature -=
                                diff / currElementData.density;
                            elementLeft->m_Temperature +=
                                diff / elementLeftData->density;
                        }
                    }

                    if (elementTop != nullptr && elementTop->m_ID != 0) {
                        minConductivity =
                            std::min(currElementData.conductivity,
                                     elementTopData->conductivity);
                        diff = ((float)(currElement.m_Temperature -
                                        elementTop->m_Temperature) *
                                ((float)minConductivity / 100.0f)) /
                               2;
                        if (diff != 0) {
                            currElement.m_Temperature -=
                                diff / currElementData.density;
                            elementTop->m_Temperature +=
                                diff / elementTopData->density;
                        }
                    }

                    if (elementRight != nullptr && elementRight->m_ID != 0) {
                        minConductivity =
                            std::min(currElementData.conductivity,
                                     elementRightData->conductivity);
                        diff = ((float)(currElement.m_Temperature -
                                        elementRight->m_Temperature) *
                                ((float)minConductivity / 100.0f)) /
                               2;
                        if (diff != 0) {
                            currElement.m_Temperature -=
                                diff / currElementData.density;
                            elementRight->m_Temperature +=
                                diff / elementRightData->density;
                        }
                    }

                    if (elementBottom != nullptr && elementBottom->m_ID != 0) {
                        minConductivity =
                            std::min(currElementData.conductivity,
                                     elementBottomData->conductivity);
                        diff = ((float)(currElement.m_Temperature -
                                        elementBottom->m_Temperature) *
                                ((float)minConductivity / 100.0f)) /
                               2;
                        if (diff != 0) {
                            currElement.m_Temperature -=
                                diff / currElementData.density;
                            elementBottom->m_Temperature +=
                                diff / elementBottomData->density;
                        }
                    }
                    // if the temperature changed, update the dirty rect
                    if (std::abs(currElement.m_Temperature - tempBefore) >
                        0.01f)
                        chunk->UpdateDirtyRect(x, y);
                }

                if (currElementData.flammable) {
                    if (currElement.m_Temperature <
                            currElementData.ignition_temperature &&
                        !currElementData.spread_ignition)
                        currElement.m_Ignited = false;
                    if (currElement.m_Temperature >=
                        currElementData.ignition_temperature)
                        currElement.m_Ignited = true;
                    if (currElement.m_Ignited) {
                        // try to spread ignition to surrounding elements
                        // if (elementTopData.flammable)
                        if (currElementData.spread_ignition &&
                            GetRandom() <
                                currElementData.spread_ignition_chance) {
                            if (elementLeft != nullptr &&
                                elementLeftData->flammable)
                                elementLeft->m_Ignited = true;
                            if (elementTop != nullptr &&
                                elementTopData->flammable)
                                elementTop->m_Ignited = true;
                            if (elementRight != nullptr &&
                                elementRightData->flammable)
                                elementRight->m_Ignited = true;
                            if (elementBottom != nullptr &&
                                elementBottomData->flammable)
                                elementBottom->m_Ignited = true;
                        }

                        // check for open air to burn
                        int fireID = ElementData::s_ElementNameToID["fire"];
                        ElementProperties &fireElementData =
                            ElementData::GetElementProperties(fireID);
                        if (currElement.m_ID != fireID) //&& GetRandom() < 5
                        {

                            int healthDiff = currElement.m_Health;
                            if (elementLeft != nullptr &&
                                (elementLeftData->cell_type ==
                                     ElementType::gas ||
                                 elementLeftData->cell_type ==
                                     ElementType::fire)) {
                                fireElementData.UpdateElementProperties(
                                    elementLeft, x - 1, y);
                                elementLeft->m_ID = fireID;
                                elementLeft->m_Temperature =
                                    currElementData.fire_temperature;
                                elementLeft->m_BaseColor =
                                    currElementData.fire_color;
                                elementLeft->m_Color =
                                    currElementData.fire_color;
                                currElement.m_Health--;
                                if (currElement.m_Temperature <
                                    currElementData.fire_temperature -
                                        currElementData
                                            .fire_temperature_increase)
                                    currElement.m_Temperature +=
                                        currElementData
                                            .fire_temperature_increase;
                            }

                            if (elementTop != nullptr &&
                                (elementTopData->cell_type ==
                                     ElementType::gas ||
                                 elementTopData->cell_type ==
                                     ElementType::fire)) {
                                fireElementData.UpdateElementProperties(
                                    elementTop, x, y + 1);
                                elementTop->m_ID = fireID;
                                elementTop->m_Temperature =
                                    currElementData.fire_temperature;
                                elementTop->m_BaseColor =
                                    currElementData.fire_color;
                                elementTop->m_Color =
                                    currElementData.fire_color;
                                currElement.m_Health--;
                                if (currElement.m_Temperature <
                                    currElementData.fire_temperature -
                                        currElementData
                                            .fire_temperature_increase)
                                    currElement.m_Temperature +=
                                        currElementData
                                            .fire_temperature_increase;
                            }

                            if (elementRight != nullptr &&
                                (elementRightData->cell_type ==
                                     ElementType::gas ||
                                 elementRightData->cell_type ==
                                     ElementType::fire)) {
                                fireElementData.UpdateElementProperties(
                                    elementRight, x + 1, y);
                                elementRight->m_ID = fireID;
                                elementRight->m_Temperature =
                                    currElementData.fire_temperature;
                                elementRight->m_BaseColor =
                                    currElementData.fire_color;
                                elementRight->m_Color =
                                    currElementData.fire_color;
                                currElement.m_Health--;
                                if (currElement.m_Temperature <
                                    currElementData.fire_temperature -
                                        currElementData
                                            .fire_temperature_increase)
                                    currElement.m_Temperature +=
                                        currElementData
                                            .fire_temperature_increase;
                            }

                            if (elementBottom != nullptr &&
                                (elementBottomData->cell_type ==
                                     ElementType::gas ||
                                 elementBottomData->cell_type ==
                                     ElementType::fire)) {
                                fireElementData.UpdateElementProperties(
                                    elementBottom, x, y - 1);
                                elementBottom->m_ID = fireID;
                                elementBottom->m_Temperature =
                                    currElementData.fire_temperature;
                                elementBottom->m_BaseColor =
                                    currElementData.fire_color;
                                elementBottom->m_Color =
                                    currElementData.fire_color;
                                currElement.m_Health--;
                                if (currElement.m_Temperature <
                                    currElementData.fire_temperature -
                                        currElementData
                                            .fire_temperature_increase)
                                    currElement.m_Temperature +=
                                        currElementData
                                            .fire_temperature_increase;
                            }
                            if (currElement.m_Health != healthDiff) {
                                currElement.m_Ignited = true;
                            } else
                                currElement.m_Ignited = false;
                        }
                    }

                    if (currElement.m_Ignited) {
                        if (currElement.m_Health <= 0) {
                            // burnt
                            int temp = currElement.m_Temperature;
                            uint32_t burntID =
                                ElementData::s_ElementNameToID[currElementData
                                                                   .burnt];
                            ElementProperties &burntData =
                                ElementData::GetElementProperties(burntID);
                            if (burntData.cell_type == ElementType::solid ||
                                burntData.cell_type ==
                                    ElementType::movableSolid) {
                                chunk->m_MeshChanged = true;
                            }
                            burntData.UpdateElementProperties(currElement, x,
                                                              y);
                            currElement.m_ID = burntID;
                            currElement.m_Temperature = temp;
                            continue;
                        }
                    }
                }

                // update the texture of the element based on temp / glow / ect
                uint32_t EditedBaseColor = currElement.m_BaseColor;
                if (currElement.m_Ignited) {
                    // update color to reflect being on fire
                    if (currElementData.ignited_color != 0)
                        EditedBaseColor = RandomizeABGRColor(
                            currElementData.ignited_color, 5);
                }

                if (currElementData.glow) {
                    int r = (EditedBaseColor >> 0) & 255;
                    int g = (EditedBaseColor >> 8) & 255;
                    int b = (EditedBaseColor >> 16) & 255;
                    int a = EditedBaseColor & 0xff000000;
                    r = std::max(
                        r, (int)(Pyxis::interpolateBetweenValues(
                                     460, 900, currElement.m_Temperature) *
                                 255.0f));
                    g = std::max(
                        g, (int)(Pyxis::interpolateBetweenValues(
                                     460, 1500, currElement.m_Temperature) *
                                 255.0f));
                    b = std::max(
                        b, (int)(Pyxis::interpolateBetweenValues(
                                     1000, 6000, currElement.m_Temperature) *
                                 255.0f));
                    EditedBaseColor =
                        a | ((b & 255) << 16) | ((g & 255) << 8) | (r & 255);
                    // make it appear hotter depending on temp
                    // temp range from 460 to 6000
                }

                currElement.m_Color = EditedBaseColor;

                // switch the behavior based on element type
                switch (currElementData.cell_type) {
                case ElementType::solid:
                    break;
                case ElementType::movableSolid:
                    // skip if we belong to a rigid body? although this isn't a
                    // thing yet.
                    if (currElement.m_Rigid)
                        continue;

                    // check below, and move
                    if (elementBottom != nullptr &&
                        elementBottomData->cell_type != ElementType::solid &&
                        elementBottomData->cell_type !=
                            ElementType::movableSolid &&
                        elementBottomData->density <= currElementData.density) {
                        currElement.m_Sliding = true;
                        Element temp = currElement;
                        chunk->SetElement(x, y, *elementBottom);

                        bottomChunk->SetElement(
                            x, (y + (CHUNKSIZE - 1)) % CHUNKSIZE, temp);
                        //*elementBottom = temp;
                        UpdateChunkDirtyRect(x, y, chunk);
                        if (elementLeft != nullptr)
                            elementLeft->m_Sliding = true;
                        if (elementRight != nullptr)
                            elementRight->m_Sliding = true;
                        continue;
                    }

                    if (currElement.m_Sliding) {
                        // chance to stop sliding
                        int rand = GetRandom();
                        if (rand <= currElementData.friction) {
                            currElement.m_Sliding = false;
                            currElement.m_Horizontal = 0;
                            continue;
                        }
                        if (currElement.m_Horizontal == 0) {
                            currElement.m_Horizontal =
                                (GetRandom() % 2 == 0) ? -1 : 1;
                        }
                    } else {
                        continue;
                    }

                    // try moving to the side
                    if (currElement.m_Horizontal > 0) {
                        if (elementRight != nullptr &&
                            elementRightData->cell_type != ElementType::solid &&
                            elementRightData->cell_type !=
                                ElementType::movableSolid &&
                            elementRightData->density <=
                                currElementData.density) {
                            currElement.m_Sliding = true;
                            Element temp = currElement;
                            chunk->SetElement(x, y, *elementRight);
                            // chunk->m_Elements[x + y * CHUNKSIZE] =
                            // *elementRight;
                            rightChunk->SetElement((x + 1) % CHUNKSIZE, y,
                                                   temp);
                            //*elementRight = temp;
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        }
                    } else {
                        if (elementLeft != nullptr &&
                            elementLeftData->cell_type != ElementType::solid &&
                            elementLeftData->cell_type !=
                                ElementType::movableSolid &&
                            elementLeftData->density <=
                                currElementData.density) {
                            currElement.m_Sliding = true;
                            Element temp = currElement;
                            chunk->SetElement(x, y, *elementLeft);
                            // chunk->m_Elements[x + y * CHUNKSIZE] =
                            // *elementLeft;
                            leftChunk->SetElement(
                                (x + (CHUNKSIZE - 1)) % CHUNKSIZE, y, temp);
                            //*elementLeft = temp;
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        }
                    }

                    break;
                case ElementType::liquid:
                    // check below, and move
                    if (elementBottom != nullptr &&
                        elementBottomData->cell_type != ElementType::solid &&
                        elementBottomData->cell_type !=
                            ElementType::movableSolid &&
                        elementBottomData->density < currElementData.density) {
                        Element temp = currElement;
                        chunk->m_Elements[x + y * CHUNKSIZE] = *elementBottom;
                        *elementBottom = temp;
                        UpdateChunkDirtyRect(x, y, chunk);
                        continue;
                    }

                    r = GetRandom() & 1 ? 1 : -1;
                    // int r = (x ^ 98252 + (m_UpdateBit * y) ^ 6234561) ? 1 :
                    // -1;

                    // try left/right then bottom left/right

                    if (r == 1) {
                        // check right, and move
                        if (elementRight != nullptr &&
                            elementRightData->cell_type != ElementType::solid &&
                            elementRightData->cell_type !=
                                ElementType::movableSolid &&
                            elementRightData->density <
                                currElementData.density) {
                            Element temp = currElement;
                            chunk->m_Elements[x + y * CHUNKSIZE] =
                                *elementRight;
                            *elementRight = temp;
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        }
                        // check left, and move
                        if (elementLeft != nullptr &&
                            elementLeftData->cell_type != ElementType::solid &&
                            elementLeftData->cell_type !=
                                ElementType::movableSolid &&
                            elementLeftData->density <
                                currElementData.density) {
                            Element temp = currElement;
                            chunk->m_Elements[x + y * CHUNKSIZE] = *elementLeft;
                            *elementLeft = temp;
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        }
                    } else {
                        // check left, and move
                        if (elementLeft != nullptr &&
                            elementLeftData->cell_type != ElementType::solid &&
                            elementLeftData->cell_type !=
                                ElementType::movableSolid &&
                            elementLeftData->density <
                                currElementData.density) {
                            Element temp = currElement;
                            chunk->m_Elements[x + y * CHUNKSIZE] = *elementLeft;
                            *elementLeft = temp;
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        }
                        // check right, and move
                        if (elementRight != nullptr &&
                            elementRightData->cell_type != ElementType::solid &&
                            elementRightData->cell_type !=
                                ElementType::movableSolid &&
                            elementRightData->density <
                                currElementData.density) {
                            Element temp = currElement;
                            chunk->m_Elements[x + y * CHUNKSIZE] =
                                *elementRight;
                            *elementRight = temp;
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        }
                    }

                    // bottom left/right? lets try without for now.

                    break;
                case ElementType::gas:

                    // note: using < doesn't make sense, but because air doesn't
                    // update, it itself doesn't move like a gas. therefore,
                    // density for gasses is inverted
                    r = (GetRandom() % 3) - 1; //-1 0 1
                    if (r == 0) {
                        // check above, and move
                        if (elementTop != nullptr &&
                            (elementTopData->cell_type == ElementType::gas ||
                             elementTopData->cell_type ==
                                 ElementType::liquid) &&
                            elementTopData->density < currElementData.density) {
                            currElement.m_Sliding = true;
                            Element temp = currElement;
                            chunk->m_Elements[x + y * CHUNKSIZE] = *elementTop;
                            *elementTop = temp;
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        }
                    }

                    // try left/right
                    if (r > 0) {
                        // check right, and move
                        if (elementRight != nullptr &&
                            (elementRightData->cell_type == ElementType::gas ||
                             elementRightData->cell_type ==
                                 ElementType::liquid) &&
                            elementRightData->density <
                                currElementData.density) {
                            currElement.m_Sliding = true;
                            Element temp = currElement;
                            chunk->m_Elements[x + y * CHUNKSIZE] =
                                *elementRight;
                            *elementRight = temp;
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        }
                        // check left and move
                        if (elementLeft != nullptr &&
                            (elementLeftData->cell_type == ElementType::gas ||
                             elementLeftData->cell_type ==
                                 ElementType::liquid) &&
                            elementLeftData->density <
                                currElementData.density) {
                            currElement.m_Sliding = true;
                            Element temp = currElement;
                            chunk->m_Elements[x + y * CHUNKSIZE] = *elementLeft;
                            *elementLeft = temp;
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        }

                    } else {
                        // check left and move
                        if (elementLeft != nullptr &&
                            (elementLeftData->cell_type == ElementType::gas ||
                             elementLeftData->cell_type ==
                                 ElementType::liquid) &&
                            elementLeftData->density <
                                currElementData.density) {
                            currElement.m_Sliding = true;
                            Element temp = currElement;
                            chunk->m_Elements[x + y * CHUNKSIZE] = *elementLeft;
                            *elementLeft = temp;
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        }
                        // check right, and move
                        if (elementRight != nullptr &&
                            (elementRightData->cell_type == ElementType::gas ||
                             elementRightData->cell_type ==
                                 ElementType::liquid) &&
                            elementRightData->density <
                                currElementData.density) {
                            currElement.m_Sliding = true;
                            Element temp = currElement;
                            chunk->m_Elements[x + y * CHUNKSIZE] =
                                *elementRight;
                            *elementRight = temp;
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        }
                    }

                    break;
                case ElementType::fire:
                    // temp drop quickly
                    // currElement.m_Temperature *= 0.95f;
                    currElement.m_Health--;
                    // die out if cold
                    if (currElement.m_Temperature <
                        currElementData.ignition_temperature) {
                        currElement.m_ID = 0; // air
                        ElementData::GetElementProperties(0)
                            .UpdateElementProperties(currElement, x, y);
                        continue;
                    }
                    // fire gets special color treatment, basically going from
                    // starting color, and losing its b, g, r color values at
                    // slower rates, respectively based on health get rgb
                    // values, and diminish them onto the color

                    uint32_t colorRed =
                        (currElement.m_BaseColor & 0x000000FF) >> 0;
                    uint32_t colorGreen =
                        (currElement.m_BaseColor & 0x0000FF00) >> 8;
                    uint32_t colorBlue =
                        (currElement.m_BaseColor & 0x00FF0000) >> 16;
                    uint32_t colorAlpha = currElement.m_BaseColor & 0xFF000000;

                    float percentAlive = (float)currElement.m_Health /
                                         (float)currElementData.health;
                    colorRed *= Pyxis::interpolateBetweenValues(
                        -10, 20, currElement.m_Health);
                    colorGreen *= Pyxis::interpolateBetweenValues(
                        0, 20, currElement.m_Health);
                    colorBlue *= Pyxis::interpolateBetweenValues(
                        5, 20, currElement.m_Health);
                    colorAlpha *= Pyxis::interpolateBetweenValues(
                        0, 20, currElement.m_Health);

                    currElement.m_Color = colorAlpha | (colorBlue << 16) |
                                          (colorGreen << 8) | colorRed;

                    r = GetRandom();
                    if (r > 20 && r < 80) //~60% to go up
                    {
                        // check above, and move
                        if (elementTop != nullptr &&
                            elementTopData->cell_type == ElementType::gas &&
                            (elementTopData->density >
                                 currElementData.density ||
                             elementTop->m_ID == 0)) {
                            currElement.m_Sliding = true;
                            Element temp = currElement;
                            chunk->m_Elements[x + y * CHUNKSIZE] = *elementTop;
                            *elementTop = temp;
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        } else if (elementTop != nullptr &&
                                   elementTopData->cell_type ==
                                       ElementType::fire) {
                            // moving to fire, so combine temp and leave air
                            elementTop->m_Temperature =
                                std::max(elementTop->m_Temperature,
                                         currElement.m_Temperature);
                            elementTop->m_Health += currElement.m_Health;
                            currElement.m_ID = 0;
                            ElementData::GetElementProperties(0)
                                .UpdateElementProperties(currElement, x, y);
                        }
                    } else if (r > 50) // left / right
                    {
                        if (elementRight != nullptr &&
                            elementRightData->cell_type == ElementType::gas) {
                            currElement.m_Sliding = true;
                            Element temp = currElement;
                            chunk->m_Elements[x + y * CHUNKSIZE] =
                                *elementRight;
                            *elementRight = temp;
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        } else if (elementRight != nullptr &&
                                   elementRightData->cell_type ==
                                       ElementType::fire) {
                            // moving to fire, so combine temp and leave air
                            elementRight->m_Temperature =
                                std::max(elementRight->m_Temperature,
                                         currElement.m_Temperature);
                            currElement.m_ID = 0;
                            ElementData::GetElementProperties(0)
                                .UpdateElementProperties(currElement, x, y);
                        }
                        if (elementLeft != nullptr &&
                            elementLeftData->cell_type == ElementType::gas) {
                            currElement.m_Sliding = true;
                            Element temp = currElement;
                            chunk->m_Elements[x + y * CHUNKSIZE] = *elementLeft;
                            *elementLeft = temp;
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        } else if (elementLeft != nullptr &&
                                   elementLeftData->cell_type ==
                                       ElementType::fire) {
                            // moving to fire, so combine temp and leave air
                            elementLeft->m_Temperature =
                                std::max(elementLeft->m_Temperature,
                                         currElement.m_Temperature);
                            currElement.m_ID = 0;
                            ElementData::GetElementProperties(0)
                                .UpdateElementProperties(currElement, x, y);
                        }
                    } else {
                        if (elementLeft != nullptr &&
                            elementLeftData->cell_type == ElementType::gas) {
                            currElement.m_Sliding = true;
                            Element temp = currElement;
                            chunk->m_Elements[x + y * CHUNKSIZE] = *elementLeft;
                            *elementLeft = temp;
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        } else if (elementLeft != nullptr &&
                                   elementLeftData->cell_type ==
                                       ElementType::fire) {
                            // moving to fire, so combine temp and leave air
                            elementLeft->m_Temperature =
                                std::max(elementLeft->m_Temperature,
                                         currElement.m_Temperature);
                            currElement.m_ID = 0;
                            ElementData::GetElementProperties(0)
                                .UpdateElementProperties(currElement, x, y);
                        }
                        if (elementRight != nullptr &&
                            elementRightData->cell_type == ElementType::gas) {
                            currElement.m_Sliding = true;
                            Element temp = currElement;
                            chunk->m_Elements[x + y * CHUNKSIZE] =
                                *elementRight;
                            *elementRight = temp;
                            UpdateChunkDirtyRect(x, y, chunk);
                            continue;
                        } else if (elementRight != nullptr &&
                                   elementRightData->cell_type ==
                                       ElementType::fire) {
                            // moving to fire, so combine temp and leave air
                            elementRight->m_Temperature =
                                std::max(elementRight->m_Temperature,
                                         currElement.m_Temperature);
                            currElement.m_ID = 0;
                            ElementData::GetElementProperties(0)
                                .UpdateElementProperties(currElement, x, y);
                        }
                    }
                    UpdateChunkDirtyRect(x, y, chunk);
                    break;
                }
            }
        }
}

/// <summary>
/// updated the chunks dirty rect, and will spread the
/// dirty rect to neighboring chunks if it is touching the edge
///
/// The x and y are in index coordinates, not world.
/// </summary>
void World::UpdateChunkDirtyRect(int x, int y, Chunk *chunk) {
    chunk->UpdateDirtyRect(x, y);

    // there are only 8 cases, so i will use a switch statement with bits?
    int result = 0;
    if (y == CHUNKSIZE - 1)
        result |= 8; // top
    if (x == CHUNKSIZE - 1)
        result |= 4; // right
    if (y == 0)
        result |= 2; // bottom
    if (x == 0)
        result |= 1; // left
    if (result == 0)
        return;

    // since we are on an chunk edge, update the other chunk

    // working on updating chunks
    Chunk *ChunkToUpdate;

    switch (result) {
    case 8: // top
        ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(0, 1));
        if (ChunkToUpdate != nullptr) {
            ChunkToUpdate->UpdateDirtyRect(x, -1);
        }
        break;
    case 12: // top right
        // top
        ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(0, 1));
        if (ChunkToUpdate != nullptr) {
            ChunkToUpdate->UpdateDirtyRect(x, -1);
        }
        // top right
        ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(1, 1));
        if (ChunkToUpdate != nullptr) {
            ChunkToUpdate->UpdateDirtyRect(-1, -1);
        }
        // right
        ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(1, 0));
        if (ChunkToUpdate != nullptr) {
            ChunkToUpdate->UpdateDirtyRect(-1, y);
        }
        break;
    case 4: // right
        // right
        ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(1, 0));
        if (ChunkToUpdate != nullptr) {
            ChunkToUpdate->UpdateDirtyRect(-1, y);
        }
        break;
    case 6: // right bottom
        // right
        ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(1, 0));
        if (ChunkToUpdate != nullptr) {
            ChunkToUpdate->UpdateDirtyRect(-1, y);
        }
        // bottom right
        ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(1, -1));
        if (ChunkToUpdate != nullptr) {
            ChunkToUpdate->UpdateDirtyRect(-1, CHUNKSIZE - 1);
        }
        // bottom
        ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(0, -1));
        if (ChunkToUpdate != nullptr) {
            ChunkToUpdate->UpdateDirtyRect(x, CHUNKSIZE - 1);
        }
        break;
    case 2: // bottom
        // bottom
        ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(0, -1));
        if (ChunkToUpdate != nullptr) {
            ChunkToUpdate->UpdateDirtyRect(x, CHUNKSIZE - 1);
        }
        break;
    case 3: // bottom left

        // left
        ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(-1, 0));
        if (ChunkToUpdate != nullptr) {
            ChunkToUpdate->UpdateDirtyRect(CHUNKSIZE - 1, y);
        }
        // bottom left
        ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(-1, -1));
        if (ChunkToUpdate != nullptr) {
            ChunkToUpdate->UpdateDirtyRect(CHUNKSIZE - 1, CHUNKSIZE - 1);
        }
        // bottom
        ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(0, -1));
        if (ChunkToUpdate != nullptr) {
            ChunkToUpdate->UpdateDirtyRect(x, CHUNKSIZE - 1);
        }
        break;
    case 1: // left
        // left
        ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(-1, 0));
        if (ChunkToUpdate != nullptr) {
            ChunkToUpdate->UpdateDirtyRect(CHUNKSIZE - 1, y);
        }
        break;
    case 9: // top left
        // left
        ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(-1, 0));
        if (ChunkToUpdate != nullptr) {
            ChunkToUpdate->UpdateDirtyRect(CHUNKSIZE - 1, y);
        }
        // top left
        ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(-1, 1));
        if (ChunkToUpdate != nullptr) {
            ChunkToUpdate->UpdateDirtyRect(CHUNKSIZE - 1, -1);
        }
        // top
        ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(0, 1));
        if (ChunkToUpdate != nullptr) {
            ChunkToUpdate->UpdateDirtyRect(x, -1);
        }
        break;
    }
}

void World::CreateParticle(const glm::vec2 &position, const glm::vec2 &velocity,
                           const Element &element) {
    m_ElementParticles.push_back(ElementParticle(position, velocity, element));
}

void World::UpdateParticles() {
    // NOTES FOR LATER IMPLEMENTATION
    //
    // It would be great to have thrown particles use the positive velocity, but
    // currently that shoves the particles into the ground. Perhaps, if the
    // particle hits a non-rigidbody solid, it could reflect?
    //

    // Use size_t for indices to avoid signed/unsigned mismatch
    for (size_t particleIndex = 0; particleIndex < m_ElementParticles.size();) {
        auto &particle = m_ElementParticles[particleIndex];

        //// Update the particle, whatever that might entail.
        particle.Update();

        //// Move the particle along its velocity, checking for collisions

        // begin with seeing if the particle is starting in a collision. this
        // usually would only happen if it just landed with another particle, or
        // if it was spawned inside a solid moving object.
        Element &element = ForceGetElement(particle.m_Position);
        ElementProperties &ed = ElementData::GetElementProperties(element.m_ID);
        // if we are in something rigid, or we collide with it, we need to slow
        // our velocity until we "die"
        bool startedInCollision = false;
        if (element.m_Rigid ||
            ((static_cast<int>(ed.cell_type) &
              static_cast<int>(particle.m_CollisionFlags)) >= 1)) {
            particle.m_Velocity *= 0.8f;
            startedInCollision = true;
        }

        if (particle.m_Velocity.x * particle.m_Velocity.x +
                particle.m_Velocity.y * particle.m_Velocity.y <
            ElementParticle::DEADSPEED) {
            // If the particle is moving slowly, it has "died"
            // keep climbing up the same material until it's not the same
            // material.
            Element *e = &ForceGetElement(particle.m_Position);
            while (e->m_ID == particle.m_Element.m_ID || e->m_Rigid) {
                particle.m_Position.y += 1;
                e = &ForceGetElement(particle.m_Position);
            }
            // e is now not of the same element, and not a rigid body. So see if
            // it collides, and if not, set the particle to that position.
            ElementProperties &deadED =
                ElementData::GetElementProperties(e->m_ID);
            if ((static_cast<ElementTypeType>(deadED.cell_type) &
                 particle.m_CollisionFlags) >= 1) {
                // Collision detected

                // I don't like the idea of particles being lost, but it is what
                // it is for now
                std::swap(m_ElementParticles[particleIndex],
                          m_ElementParticles.back());
                m_ElementParticles.pop_back();
                // Don’t increment particleIndex; the swapped-in element (if
                // any) is now at particleIndex and hasn’t been processed yet
                continue;
            } else {
                // no collision detected, so set the element then erase
                SetElement(particle.m_Position, particle.m_Element);
                std::swap(m_ElementParticles[particleIndex],
                          m_ElementParticles.back());
                m_ElementParticles.pop_back();
                // Don’t increment particleIndex; the swapped-in element (if
                // any) is now at particleIndex and hasn’t been processed yet
                continue;
            }
        }

        // start by getting the path it will take
        glm::vec2 newPos = particle.m_Position + particle.m_Velocity;
        std::vector<glm::ivec2> path =
            Utils::getLinePath(particle.m_Position, newPos);
        particle.m_Position = newPos;

        bool shouldRemove = false;
        int index = 1;
        // loop over the path and see when we collide
        for (auto it = path.begin() + 1; it != path.end(); it++) {
            Element &element = ForceGetElement(*it);
            ElementProperties &ed =
                ElementData::GetElementProperties(element.m_ID);
            // Check collision
            if (element.m_Rigid ||
                ((static_cast<int>(ed.cell_type) &
                  static_cast<int>(particle.m_CollisionFlags)) >= 1)) {
                // Collision detected

                // if we started in collision, then ignore
                if (!startedInCollision) {
                    // set element in world to previous position on path
                    SetElement(path[std::max(index - 1, 0)],
                               particle.m_Element);
                    shouldRemove = true; // Mark for removal
                    break;
                }

            } else {
                if (startedInCollision) {
                    // we were colliding but now we aren't, so lets just get
                    // back into the simulation
                    SetElement(*it, particle.m_Element);
                    shouldRemove = true; // Mark for removal
                    break;
                }
                startedInCollision = false;
            }
            index++;
        }

        if (!startedInCollision)
            particle.m_Velocity.y -= 0.05f;

        if (shouldRemove) {
            // Swap with the last element and pop
            std::swap(m_ElementParticles[particleIndex],
                      m_ElementParticles.back());
            m_ElementParticles.pop_back();
            // Don’t increment particleIndex; the swapped-in element (if any)
            // is now at particleIndex and hasn’t been processed yet
        } else {
            // No collision, move to the next particle
            ++particleIndex;
        }
    }
}

void World::RenderParticles() {
    for (auto &particle : m_ElementParticles) {
        particle.Render();
    }
}

/// <summary>
/// wipes the world, and makes the first chunk empty
/// </summary>
void World::Clear() {
    for (auto &pair : m_Chunks) {
        delete pair.second;
    }
    m_Chunks.clear();

    Physics2D::ClearWorld();
    Physics2D::GetWorld();

    // AddChunk(glm::ivec2(0, 0));
    // m_Chunks[{0, 0}]->Clear();

    ////create a border around first chunk
    // Element ceramic = Element();
    // ceramic.m_ID = m_ElementIDs["ceramic"];
    // ElementProperties& elementData = m_ElementData[ceramic.m_ID];
    // elementData.UpdateElementProperties(ceramic, 0, 0);
    // for (int i = 0; i < CHUNKSIZE; i++)
    //{
    //	SetElement({ i, 0 }, ceramic);//bottom
    //	SetElement({ i, CHUNKSIZE - 1 }, ceramic);//top
    //	SetElement({ 0, i }, ceramic);//left
    //	SetElement({ CHUNKSIZE - 1, i }, ceramic);//right
    // }
}

/// <summary>
/// Renders the world using Pyxis::renderer2d draw quad
/// </summary>
void World::RenderWorld() {
    // PX_TRACE("Rendering world");
    for (auto &pair : m_Chunks) {
        Renderer2D::DrawQuad(
            glm::vec2((pair.second->m_ChunkPos.x * CHUNKSIZEF) + HALFCHUNKSIZEF,
                      (pair.second->m_ChunkPos.y * CHUNKSIZEF) +
                          HALFCHUNKSIZEF),
            {CHUNKSIZEF, CHUNKSIZEF}, pair.second->m_Texture);
        pair.second->RenderChunk();

        // Renderer2D::DrawQuad(glm::vec3(pair.second->m_ChunkPos.x + 0.5f,
        // pair.second->m_ChunkPos.y + 0.5f, 1.0f), {0.1f, 0.1f},
        // glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
    }

    RenderParticles();

    // float pixelSize = (1.0f / CHUNKSIZE);
    if (m_DebugDrawColliders) {
        for (auto &kvp : m_PixelBodies) {
            kvp.second->DebugDraw(10, 16);
        }
        for (auto &kvp : m_Chunks) {
            kvp.second->m_PhysicsBody->DebugDraw(10, PPU);
        }
        // drawing contour vector
        /*for each (auto pixelBody in m_PixelBodies)
        {
                for (int i = 0; i < pixelBody->m_ContourVector.size() - 1; i++)
                {
                        glm::ivec2 pixelPos = {
        pixelBody->m_B2Body->GetPosition().x * PPU ,
        pixelBody->m_B2Body->GetPosition().y * PPU }; pixelPos -=
        pixelBody->m_Origin; pixelPos.y += 1; glm::vec2 worldPos = { pixelPos.x
        / CHUNKSIZEF, pixelPos.y / CHUNKSIZEF }; glm::vec2 start =
        (glm::vec2(pixelBody->m_ContourVector[i].x,
        pixelBody->m_ContourVector[i].y) / 512.0f) + worldPos; glm::vec2 end =
        (glm::vec2(pixelBody->m_ContourVector[i + 1].x,
        pixelBody->m_ContourVector[i + 1].y) / 512.0f) + worldPos;
                        Renderer2D::DrawLine(start, end, { 0,1,0,1 });
                }
        }*/

        // auto count = m_Box2DWorld->GetBodyCount();
        // for (auto body = m_Box2DWorld->GetBodyList(); body != nullptr; body =
        // body->GetNext())
        //{
        //	auto& T = body->GetTransform();
        //	for (auto fixture = body->GetFixtureList(); fixture != nullptr;
        // fixture = fixture->GetNext())
        //	{
        //		auto shape = (b2PolygonShape*)(fixture->GetShape());
        //		for (int i = 0; i < shape->m_count - 1; i++)
        //		{
        //			auto v = shape->m_vertices[i];
        //			float x1 = (T.q.c * v.x - T.q.s * v.y) + T.p.x;
        //			float y1 = (T.q.s * v.x + T.q.c * v.y) + T.p.y;

        //			auto e = shape->m_vertices[i + 1];
        //			float x2 = (T.q.c * e.x - T.q.s * e.y) + T.p.x;
        //			float y2 = (T.q.s * e.x + T.q.c * e.y) + T.p.y;
        //			glm::vec2 start = glm::vec3(x1, y1, 10) / (PPU);
        //			glm::vec2 end = glm::vec3(x2, y2, 10) / (PPU);

        //			Renderer2D::DrawLine(start, end);
        //		}
        //		//draw the last line to connect the shape
        //		auto v = shape->m_vertices[shape->m_count - 1];
        //		float x1 = (T.q.c * v.x - T.q.s * v.y) + T.p.x;
        //		float y1 = (T.q.s * v.x + T.q.c * v.y) + T.p.y;

        //		auto e = shape->m_vertices[0];
        //		float x2 = (T.q.c * e.x - T.q.s * e.y) + T.p.x;
        //		float y2 = (T.q.s * e.x + T.q.c * e.y) + T.p.y;
        //		glm::vec2 start = glm::vec3(x1, y1, 10) / (PPU);
        //		glm::vec2 end = glm::vec3(x2, y2, 10) / (PPU);

        //		Renderer2D::DrawLine(start, end);
        //	}
        //}
    }
}

void World::ResetPhysicsDeterminism() {
    PX_TRACE("Box2D sim reset at sim tick {0}", m_SimulationTick);
    Physics2D::ResetWorldDeterminism();
}

void World::DestroyPixelBody(UUID id) {
    m_PixelBodies[id]->ActuallyQueueFree();
    m_PixelBodies.erase(id);
}

void World::DestroyPixelBody(Ref<PixelBody2D> body) {
    body->ActuallyQueueFree();
    m_PixelBodies.erase(body->GetUUID());
}

/// <summary>
/// Seeds Rand() based on a few factors of the world.
/// It is deterministic by update tick and position, so
/// it is thread safe.
/// </summary>
void World::SeedRandom(int xPos, int yPos) {
    unsigned int seed = ((xPos * 58102) << m_SimulationTick % 5) +
                        (((yPos * 986124) * m_SimulationTick) >> 2);

    // PX_TRACE("Seeded rand with: {0}", seed);
    m_RandomEngine.seed(seed);
    // std::srand(seed);
}

int World::GetRandom() {
    int result = m_Rand(m_RandomEngine);
    // PX_TRACE("Got random number: {0}", result);
    return result;
}

const bool World::IsInBounds(int x, int y) {
    // having y first might actually be faster, simply because things tend
    // to fall
    if (y < 0 || y >= CHUNKSIZE)
        return false;
    if (x < 0 || x >= CHUNKSIZE)
        return false;
    return true;
}

glm::ivec2 World::WorldToPixel(const glm::vec2 &worldPos) {
    // glm::ivec2 result = glm::ivec2(worldPos.x * CHUNKSIZE, worldPos.y *
    // CHUNKSIZE); now, world and pixel are equivalent.

    glm::ivec2 result = worldPos;
    if (worldPos.x < 0.0f)
        result.x--;
    if (worldPos.y < 0.0f)
        result.y--;
    return result;
}

// Helper to get a chunk from a world pixel position
glm::ivec2 World::PixelToChunk(const glm::ivec2 &pixelPos) {
    glm::ivec2 result = {0, 0};
    if (pixelPos.x < 0) {
        result.x = (pixelPos.x + 1) / CHUNKSIZE;
        result.x--;
    } else
        result.x = pixelPos.x / CHUNKSIZE;
    if (pixelPos.y < 0) {
        result.y = (pixelPos.y + 1) / CHUNKSIZE;
        result.y--;
    } else
        result.y = pixelPos.y / CHUNKSIZE;
    return result;
}

// Helper to get an index from a world pixel position
glm::ivec2 World::PixelToIndex(const glm::ivec2 &pixelPos) {
    glm::ivec2 result = {0, 0};
    if (pixelPos.x < 0) {
        result.x = CHUNKSIZE - (std::abs(pixelPos.x) % CHUNKSIZE);
        result.x = result.x % CHUNKSIZE;
    } else
        result.x = pixelPos.x % CHUNKSIZE;
    if (pixelPos.y < 0) {
        result.y = CHUNKSIZE - (std::abs(pixelPos.y) % CHUNKSIZE);
        result.y = result.y % CHUNKSIZE;
    } else
        result.y = pixelPos.y % CHUNKSIZE;
    return result;
}

} // namespace Pyxis
