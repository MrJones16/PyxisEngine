#include "World.h"
//#include "ChunkWorker.h"
#include <random>
#include <thread>
#include <mutex>
#include <tinyxml2.h>
#include <box2d/b2_math.h>
#include <poly2tri.h>
#include <glm/gtc/matrix_transform.hpp>
#include <PixelBody2D.h>
#include <Pyxis/Game/Physics2D.h>


namespace Pyxis
{
	namespace Utils
	{
		/// <summary>
		/// Line solver using Bresenham's line algorithm
		/// </summary>
		/// <returns></returns>
		std::vector<glm::ivec2> getLinePath(glm::ivec2 startPosition, glm::ivec2 endPosition) {
			std::vector<glm::ivec2> positions;
			glm::ivec2 current = startPosition;
			positions.push_back(current);

			int dx = endPosition.x - startPosition.x;
			int dy = endPosition.y - startPosition.y;
			int steps = std::max(std::abs(dx), std::abs(dy));

			if (steps == 0) return positions;

			float xInc = static_cast<float>(dx) / steps;
			float yInc = static_cast<float>(dy) / steps;

			for (int i = 0; i < steps; i++) {
				current.x = startPosition.x + static_cast<int>(round(xInc * (i + 1)));
				current.y = startPosition.y + static_cast<int>(round(yInc * (i + 1)));
				positions.push_back(current);
			}

			return positions;
		}
	}

	World::World(std::string assetPath, int seed)
	{
		Physics2D::GetWorld();//make sure that the physics world is made
		//load the element data
		//make sure the file exists
		if (!std::filesystem::exists(assetPath + "/data/CellData.json"))
		{
			PX_ASSERT(false, "CellData.json file is missing!");
			PX_ERROR("CellData.json file is missing!");
			m_Error = true;
			Application::Get().Sleep(10000);
			Application::Get().Close();
			return;
		}

		ElementData::LoadElementData(assetPath + "/data/CellData.json");

		//set up world noise data
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
		//LoadXMLElementData(assetPath);
		////Output the element data to a file
		//std::ofstream file(assetPath + "/data/CellData.json");
		//json j;
		//for (auto& elementData : ElementData::s_ElementData)
		//{
		//	j += elementData;
		//}
		//for (auto& reaction : ElementData::s_Reactions)
		//{
		//	j += reaction;
		//}
		//file << j;
		//file.close();?
		//////////// PRIOR XML SETUP ////////////		
	}

	void World::Initialize(int worldSeed)
	{
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
	void World::DownloadWorldInit(Network::Message& msg)
	{
		msg >> m_Running;
		msg >> m_WorldSeed;
		Initialize(m_WorldSeed);
		msg >> m_UpdateBit;
		msg >> m_SimulationTick;

	}

	void World::DownloadWorld(Network::Message& msg)
	{
		if ((GameMessage)msg.header.id == GameMessage::Server_GameDataRigidBody)
		{
			//lets load the pixel body! just reverse the upload order.
			std::vector<uint8_t> msgpack;
			msg >> msgpack;
			json j = json::from_msgpack(msgpack);
			auto RigidBodyNode = Node::DeserializeNode(j);
			
			if (j["Type"] == "PixelBody2D")
			{
				//register to Node::Nodes
				Node::Nodes[RigidBodyNode->GetUUID()] = RigidBodyNode;
				PixelBody2D* body = (PixelBody2D*)RigidBodyNode.get();
				body->m_PXWorld = this;
				//m_PixelBodyMap[body->m_UUID] = body;
				PX_TRACE("Loaded PixelBody2D: Pos[{},{}] UUID[{}]", body->GetPosition().x, body->GetPosition().y, RigidBodyNode->GetUUID());
				// can safely ignore putting it in the world, since when we got the world
				// data it is already there!
			}
			else if (j["Type"] == "ChunkChainBody")
			{
				//we don't register these to the "scene layer" aka node:nodes

				Ref<ChunkChainBody> ccb = std::dynamic_pointer_cast<ChunkChainBody>(RigidBodyNode);
				GetChunk(ccb->m_ChunkOwnerPos)->AddPreviousStaticCollider(ccb);
				PX_TRACE("Loaded ChunkChainBody: Pos[{},{}] UUID[{}]", ccb->m_ChunkOwnerPos.x, ccb->m_ChunkOwnerPos.y, ccb->GetUUID());
			}
			else//"RigidBody2D")
			{
				//register to Node::Nodes
				Node::Nodes[RigidBodyNode->GetUUID()] = RigidBodyNode;

				PX_TRACE("Loaded Generic RigidBody2D, UUID[{0}]", RigidBodyNode->GetUUID());
			}
		}
		if ((GameMessage)msg.header.id == GameMessage::Server_GameDataChunk)
		{
			glm::ivec2 chunkPos;
			msg >> chunkPos;

			PX_ASSERT(m_Chunks.find(chunkPos) == m_Chunks.end(), "Tried to load a chunk that already existed");
			Chunk* chunk = new Chunk(chunkPos);
			msg >> chunk->m_StaticColliderChanged;
			msg >> chunk->m_DirtyRect;
			m_Chunks[chunkPos] = chunk;
			for (int ii = (CHUNKSIZE * CHUNKSIZE) - 1; ii >= 0; ii--)
			{
				msg >> chunk->m_Elements[ii];
			}

			chunk->UpdateWholeTexture();

			PX_TRACE("Loaded Chunk ({0},{1})", chunkPos.x, chunkPos.y);
		}
	}

	void World::GetGameDataInit(Network::Message& msg)
	{
		PX_TRACE("Gathering World Data");
		msg.header.id = static_cast<uint32_t>(GameMessage::Server_GameDataInit);
		msg << static_cast<uint32_t>(m_Chunks.size());
		PX_TRACE("# Chunks: {0}", m_Chunks.size());
		msg << static_cast<uint32_t>(Physics2D::GetWorld()->GetBodyCount());
		PX_TRACE("# RigidBodies: {0}", Physics2D::GetWorld()->GetBodyCount());
		msg << m_SimulationTick;
		msg << m_UpdateBit;
		msg << m_WorldSeed;
		msg << m_Running;
	}

	void World::GetGameData(std::vector<Network::Message>& messages)
	{
		b2Body* body = Physics2D::GetWorld()->GetBodyList();

		//FILO to swap order of sent pixel bodies, so box2d can stay synced?
		std::deque<b2Body*> bodies;
		while (body != nullptr)
		{
			bodies.push_back(body);			
			body = body->GetNext();
		}
		while (!bodies.empty())
		{
			RigidBody2D* rb = (RigidBody2D*)bodies.back()->GetUserData().pointer;
			if (rb)
			{
				PX_TRACE("Packing RigidBody2D {0}", rb->GetUUID());
				messages.emplace_back();
				messages.back().header.id = static_cast<uint32_t>(GameMessage::Server_GameDataRigidBody);
				messages.back() << rb->SerializeBinary();
			}
			else
			{
				PX_ASSERT(false, "RigidBody2D has no user data!");
			}
			bodies.pop_back();
		}


		//now we create a separate message for each chunk 
		for (auto& pair : m_Chunks)
		{
			messages.emplace_back();
			messages.back().header.id = static_cast<uint32_t>(GameMessage::Server_GameDataChunk);
			for (int i = 0; i < CHUNKSIZE * CHUNKSIZE; i++)
			{
				messages.back() << pair.second->m_Elements[i];
			}
			messages.back() << pair.second->m_DirtyRect;
			messages.back() << pair.second->m_StaticColliderChanged;
			messages.back() << pair.first;
		}

	}

	World::~World()
	{
		PX_TRACE("Deleting World");

		Physics2D::DeleteWorld();
		for (auto& pair : m_Chunks)
		{
			delete(pair.second);
		}
		
	}

	Chunk* World::AddChunk(const glm::ivec2& chunkPos)
	{
		//make sure chunk doesn't already exist
		if (m_Chunks.find(chunkPos) == m_Chunks.end())
		{
			Chunk* chunk = new Chunk(chunkPos);
			m_Chunks[chunkPos] = chunk;
			GenerateChunk(chunk);
			chunk->UpdateWholeTexture();
			return chunk;
		}

		return m_Chunks[chunkPos];				
	}


	Chunk* World::GetChunk(const glm::ivec2& chunkPos)
	{
		auto it = m_Chunks.find(chunkPos);
		if (it != m_Chunks.end())
			return it->second;
		return nullptr;

		/*AddChunk(chunkPos);
		return m_Chunks[chunkPos];*/
	}

	void World::GenerateChunk(Chunk* chunk)
	{
		glm::vec2 chunkPixelPos = chunk->m_ChunkPos * CHUNKSIZE;
		for (int x = 0; x < CHUNKSIZE; x++)
		{
			for (int y = 0; y < CHUNKSIZE; y++)
			{
				glm::vec2 pixelPos = chunkPixelPos + glm::vec2(x,y);
				if (pixelPos.y >=0)
				{
					float amplitude = 20.0f;
					float heightNoise = ((m_HeightNoise.GetNoise(pixelPos.x, 0.0f) + 1) / 2) * amplitude;
					float surfaceTop = heightNoise + 80;
					float grassWidth = 20;
					if (pixelPos.y > surfaceTop + grassWidth)
					{
						//air
					}
					else if (pixelPos.y > surfaceTop)
					{
						chunk->SetElement(x, y, ElementData::GetElement("grass", x, y));
					}
					else if (pixelPos.y > heightNoise)
					{
						//dirt
						chunk->SetElement(x, y, ElementData::GetElement("dirt", x, y));
					}
					else
					{
						//under the noise value, so stone, blended into the caves
						float caveNoise = (m_CaveNoise.GetNoise(pixelPos.x, pixelPos.y) + 1) / 2.0f;
						if (caveNoise >= 0.25f)
						{
							chunk->SetElement(x, y, ElementData::GetElement("stone", x, y));
						}
					}
					
				}
				else
				{
					//under y==0, so 
					float caveNoise = (m_CaveNoise.GetNoise(pixelPos.x, pixelPos.y) + 1) / 2.0f;
					if (caveNoise >= 0.25f)
					{
						chunk->SetElement(x, y, ElementData::GetElement("stone", x, y));
					}
				}
			}
		}
		
	}

	Element& World::GetElement(const glm::ivec2& pixelPos)
	{
		auto chunkPos = PixelToChunk(pixelPos);
		auto index = PixelToIndex(pixelPos);
		return GetChunk(chunkPos)->m_Elements[index.x + index.y * CHUNKSIZE];
	}

	Element& World::ForceGetElement(const glm::ivec2& pixelPos)
	{
		auto chunkPos = PixelToChunk(pixelPos);
		auto index = PixelToIndex(pixelPos);
		if (!m_Chunks.contains(chunkPos))
		{
			AddChunk(chunkPos);
		}
		return GetChunk(chunkPos)->m_Elements[index.x + index.y * CHUNKSIZE];
	}

	void World::SetElement(const glm::ivec2& pixelPos, const Element& element)
	{
		Chunk* chunk = GetChunk(PixelToChunk(pixelPos));
		auto index = PixelToIndex(pixelPos);
		if (element.m_ID == ElementData::s_ElementNameToID["debug_heat"])
		{
			chunk->m_Elements[index.x + index.y * CHUNKSIZE].m_Temperature++;
		}
		else if (element.m_ID == ElementData::s_ElementNameToID["debug_cool"])
		{
			chunk->m_Elements[index.x + index.y * CHUNKSIZE].m_Temperature--;
		}
		else
		{
			chunk->SetElement(index.x, index.y, element);
		}
		UpdateChunkDirtyRect(index.x, index.y, chunk);

	}

	void World::SetElementWithoutDirtyRectUpdate(const glm::ivec2& pixelPos, const Element& element)
	{
		Chunk* chunk = GetChunk(PixelToChunk(pixelPos));
		auto index = PixelToIndex(pixelPos);
		if (element.m_ID == ElementData::s_ElementNameToID["debug_heat"])
		{
			chunk->m_Elements[index.x + index.y * CHUNKSIZE].m_Temperature++;
		}
		else if (element.m_ID == ElementData::s_ElementNameToID["debug_cool"])
		{
			chunk->m_Elements[index.x + index.y * CHUNKSIZE].m_Temperature--;
		}
		else
		{
			chunk->SetElement(index.x, index.y, element);
		}

	}

	void World::PaintBrushElement(glm::ivec2 pixelPos, uint32_t elementID, BrushType brush, uint8_t brushSize)
	{
		std::unordered_set<Chunk*> chunksToUpdate;

		glm::ivec2 newPos = pixelPos;
		for (int x = -brushSize; x <= brushSize; x++)
		{
			for (int y = -brushSize; y <= brushSize; y++)
			{
				newPos = pixelPos + glm::ivec2(x, y);
				Chunk* chunk;
				glm::ivec2 index;
				switch (brush)
				{
				case BrushType::circle:
					//limit brush to circle
					if (std::sqrt((float)(x * x) + (float)(y * y)) >= brushSize) continue;
					break;
				case BrushType::square:
					break;
				}

				//get chunk / index
				glm::ivec2 chunkPos = PixelToChunk(newPos);
				chunk = GetChunk(chunkPos);
				if (chunk == nullptr)
				{
					chunk = AddChunk(chunkPos);
				}
				chunksToUpdate.insert(chunk);
				index = PixelToIndex(newPos);

				//get element / color
				Element element = Element();
				ElementData& elementData = ElementData::GetElementData(elementID);
				element.m_ID = elementID;
				element.m_Updated = !m_UpdateBit;
				elementData.UpdateElementData(element, index.x, index.y);

				//set the element
				if (element.m_ID == ElementData::s_ElementNameToID["debug_heat"])
				{
					chunk->m_Elements[index.x + index.y * CHUNKSIZE].m_Temperature++;
				}
				else if (element.m_ID == ElementData::s_ElementNameToID["debug_cool"])
				{
					chunk->m_Elements[index.x + index.y * CHUNKSIZE].m_Temperature--;
				}
				else
				{
					chunk->SetElement(index.x, index.y, element);
				}
				chunk->UpdateDirtyRect(index.x, index.y);
			}
		}
		for (auto chunk : chunksToUpdate)
		{
			chunk->UpdateTexture();
		}
	}

	void World::UpdateWorld()
	{
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


		Physics2D::Step();
		//physiscs 2d updates the pixel bodies,
		//and the pixel bodies manage their own insertion
		//and deletion from the world, and throwing the
		//particles on overlap

		for (auto& [pos, chunk] : m_Chunks)
		{
			UpdateChunk(chunk);
		}
		
		if (!m_ServerMode)
		{
			for (auto & pair : m_Chunks)
			{
				pair.second->UpdateTexture();
			}
		}

		m_UpdateBit = !m_UpdateBit;
		m_SimulationTick++;
	}

	void World::UpdateTextures()
	{
		for (auto & pair : m_Chunks)
		{
			pair.second->UpdateTexture();
		}
	}

	/// <summary>
	/// returns the percent value of the value between the points, from 0-1
	/// </summary>

	float interpolateBetweenValues(float lower, float higher, float val)
	{
		//460, 6000, 1000
		return std::min((std::max(lower, val) - lower), higher - lower) / (higher - lower);
	}


	/// <summary>
	/// 
	/// updating a chunk overview:
	/// copy the dirt rect state, and shrink it by 1
	/// loop over the elements, and update them. if the elements they are trying to swap with
	/// is out of bounds, then find the other chunk and get the element like that.
	/// because an element can belong to another chunk, there is a lot more conditional logic.
	/// const int BUCKETS = chunk->BUCKETS;
	/// const int BUCKETSIZE = chunk->BUCKETSIZE;
	/// </summary>
	void World::UpdateChunk(Chunk* chunk)
	{
		//make a copy of the dirty rect before shrinking it
		DirtyRect dirtyRect = chunk->m_DirtyRect;
		//instead of resetting completely, just shrink by 1. This allows elements that cross borders to update, since otherwise the dirty rect would
		//forget about it instead of shrinking and still hitting it.
		chunk->m_DirtyRect.min = chunk->m_DirtyRect.min + 1;
		chunk->m_DirtyRect.max = chunk->m_DirtyRect.max - 1;


		//make sure to wake up any pixel bodies in area
		b2AABB queryRegion;
		glm::vec2 lower = glm::vec2(dirtyRect.min + (chunk->m_ChunkPos * CHUNKSIZE)) / PPU;
		glm::vec2 upper = glm::vec2(dirtyRect.max + (chunk->m_ChunkPos * CHUNKSIZE)) / PPU;
		queryRegion.lowerBound = b2Vec2(lower.x, lower.y);
		queryRegion.upperBound = b2Vec2(upper.x, upper.y);
		WakeUpQueryCallback callback;
		Physics2D::GetWorld()->QueryAABB(&callback, queryRegion);

		//get the min and max
		//loop from min to max in both "axies"?
		bool minToMax = m_UpdateBit;

		//PX_TRACE("Update Bit: {0}", m_UpdateBit);

		//first lets seed random, so the simulation is deterministic!
		SeedRandom(chunk->m_ChunkPos.x, chunk->m_ChunkPos.y);

		///////////////////////////////////////////////////////////////
		/// Main Update Loop, back and forth bottom to top, left,right,left....
		///////////////////////////////////////////////////////////////
		int startY;
		int compareY;
		if (minToMax)
		{
			//start at the smallest, and go the largest
			startY = std::max(dirtyRect.min.y, 0);
			compareY = std::min(dirtyRect.max.y, CHUNKSIZE - 1) + 1;
		}
		else
		{
			//start at the largest, and go the smallest
			startY = std::min(dirtyRect.max.y, CHUNKSIZE - 1);
			compareY = std::max(dirtyRect.min.y, 0) - 1;
		}


		if (dirtyRect.min.x <= dirtyRect.max.x && dirtyRect.min.y <= dirtyRect.max.y)
		for (int y = startY; y != compareY; m_UpdateBit ? y++ : y--) //going y then x so we do criss crossing 
		{
			
			minToMax = !minToMax;
			int startX;
			int compareX;
			if (minToMax)
			{
				//start at the smallest, and go the largest
				startX = std::max(dirtyRect.min.x, 0);
				compareX = std::min(dirtyRect.max.x, CHUNKSIZE - 1) + 1;
			}
			else
			{
				//start at the largest, and go the smallest
				startX = std::min(dirtyRect.max.x, CHUNKSIZE - 1);
				compareX = std::max(dirtyRect.min.x, 0) - 1;
			}
			//PX_TRACE("x is: {0}", x);
			for (int x = startX; x != compareX; minToMax ? x++ : x--)
			{
				//we now have an x and y of the element in the array, so update it
				
				Element& currElement = chunk->m_Elements[x + y * CHUNKSIZE];
				ElementData& currElementData = ElementData::GetElementData(currElement.m_ID);

				//skip if already updated
				if (currElement.m_Updated == m_UpdateBit) continue;
				//flip the update bit so we know we updated this element
				currElement.m_Updated = m_UpdateBit;

				if (currElement.m_ID == 0) continue;
				

				int xOther = x;
				int yOther = y;

				int r = 0;

				//iterators for reaction lookup
				std::unordered_map<uint32_t, ReactionResult>::iterator it;
				std::unordered_map<uint32_t, ReactionResult>::iterator end;


				Element* elementTop;
				Element* elementBottom;
				Element* elementRight;
				Element* elementLeft;

				Chunk* leftChunk = chunk;
				Chunk* rightChunk = chunk;
				Chunk* topChunk = chunk;
				Chunk* bottomChunk = chunk;
				
				//get cardinal elements
				{
					if (IsInBounds(x, y + 1))
					{
						elementTop = chunk->m_Elements + (x)+(y + 1) * CHUNKSIZE;
					}
					else
					{
						//see if the chunk exists, if it does then get that element, otherwise nullptr
						glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(x, y + 1);
						topChunk = GetChunk(PixelToChunk(pixelSpace));
						if (topChunk != nullptr)
						{
							int indexOther = ((((x)+CHUNKSIZE) % CHUNKSIZE) + (((y + 1) + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE);
							elementTop = topChunk->m_Elements + indexOther;
						}
						else
						{
							elementTop = nullptr;
						}
					}

					if (IsInBounds(x, y - 1))
					{
						elementBottom = chunk->m_Elements + (x)+(y - 1) * CHUNKSIZE;
					}
					else
					{
						//see if the chunk exists, if it does then get that element, otherwise nullptr
						glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(x, y - 1);
						bottomChunk = GetChunk(PixelToChunk(pixelSpace));
						if (bottomChunk != nullptr)
						{
							int indexOther = ((((x)+CHUNKSIZE) % CHUNKSIZE) + (((y - 1) + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE);
							elementBottom = bottomChunk->m_Elements + indexOther;
						}
						else
						{
							elementBottom = nullptr;
						}
					}

					if (IsInBounds(x + 1, y))
					{
						elementRight = chunk->m_Elements + (x + 1) + (y)*CHUNKSIZE;
					}
					else
					{
						//see if the chunk exists, if it does then get that element, otherwise nullptr
						glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(x + 1, y);
						rightChunk = GetChunk(PixelToChunk(pixelSpace));
						if (rightChunk != nullptr)
						{
							int indexOther = ((((x + 1) + CHUNKSIZE) % CHUNKSIZE) + (((y)+CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE);
							elementRight = rightChunk->m_Elements + indexOther;
						}
						else
						{
							elementRight = nullptr;
						}
					}

					if (IsInBounds(x - 1, y))
					{
						elementLeft = chunk->m_Elements + (x - 1) + (y)*CHUNKSIZE;
					}
					else
					{
						//see if the chunk exists, if it does then get that element, otherwise nullptr
						glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(x - 1, y);
						leftChunk = GetChunk(PixelToChunk(pixelSpace));
						if (leftChunk != nullptr)
						{
							int indexOther = ((((x - 1) + CHUNKSIZE) % CHUNKSIZE) + (((y)+CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE);
							elementLeft = leftChunk->m_Elements + indexOther;
						}
						else
						{
							elementLeft = nullptr;
						}
					}
				}

                ElementData* elementLeftData	= (elementLeft != nullptr)	? &ElementData::GetElementData(elementLeft->m_ID) : nullptr;
                ElementData* elementRightData	= (elementRight != nullptr) ? &ElementData::GetElementData(elementRight->m_ID) : nullptr;
                ElementData* elementTopData		= (elementTop != nullptr)	? &ElementData::GetElementData(elementTop->m_ID) : nullptr;
                ElementData* elementBottomData	= (elementBottom != nullptr)? &ElementData::GetElementData(elementBottom->m_ID) : nullptr;
				//check for reactions, left,up,right,down
				{
					if (elementLeft != nullptr)
					{
						it = ElementData::s_ReactionTable[currElement.m_ID].find(elementLeft->m_ID);
						end = ElementData::s_ReactionTable[currElement.m_ID].end();
						if (it != end && GetRandom() < it->second.probability)
						{
							currElement.m_ID = it->second.cell0ID;
							ElementData& ed0 = ElementData::GetElementData(it->second.cell0ID);
							ed0.UpdateElementData(currElement, x, y);
							if (ed0.cell_type == ElementType::solid || ed0.cell_type == ElementType::movableSolid)
							{
								chunk->m_StaticColliderChanged = true;
							}

							elementLeft->m_ID = it->second.cell1ID;
							ElementData& ed1 = ElementData::GetElementData(it->second.cell1ID);
							if (ed1.cell_type == ElementType::solid || ed1.cell_type == ElementType::movableSolid)
							{
								leftChunk->m_StaticColliderChanged = true;
							}
							ed1.UpdateElementData(elementLeft, x - 1, y);
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
					}
					
					if (elementTop != nullptr)
					{
						it = ElementData::s_ReactionTable[currElement.m_ID].find(elementTop->m_ID);
						end = ElementData::s_ReactionTable[currElement.m_ID].end();
						if (it != end && GetRandom() < it->second.probability)
						{
							currElement.m_ID = it->second.cell0ID;
							ElementData& ed0 = ElementData::GetElementData(it->second.cell0ID);
							ed0.UpdateElementData(currElement, x, y);
							if (ed0.cell_type == ElementType::solid || ed0.cell_type == ElementType::movableSolid)
							{
								chunk->m_StaticColliderChanged = true;
							}


							elementTop->m_ID = it->second.cell1ID;
							ElementData& ed1 = ElementData::GetElementData(it->second.cell1ID);
							if (ed1.cell_type == ElementType::solid || ed1.cell_type == ElementType::movableSolid)
							{
								topChunk->m_StaticColliderChanged = true;
							}
							ed1.UpdateElementData(elementTop, x - 1, y);
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
					}
					
					if (elementRight != nullptr)
					{
						it = ElementData::s_ReactionTable[currElement.m_ID].find(elementRight->m_ID);
						end = ElementData::s_ReactionTable[currElement.m_ID].end();
						if (it != end && GetRandom() < it->second.probability)
						{
							currElement.m_ID = it->second.cell0ID;
							ElementData& ed0 = ElementData::GetElementData(it->second.cell0ID);
							ed0.UpdateElementData(currElement, x, y);
							if (ed0.cell_type == ElementType::solid || ed0.cell_type == ElementType::movableSolid)
							{
								chunk->m_StaticColliderChanged = true;
							}

							elementRight->m_ID = it->second.cell1ID;
							ElementData& ed1 = ElementData::GetElementData(it->second.cell1ID);
							if (ed1.cell_type == ElementType::solid || ed1.cell_type == ElementType::movableSolid)
							{
								rightChunk->m_StaticColliderChanged = true;
							}
							ed1.UpdateElementData(elementRight, x - 1, y);
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
					}
					
					if (elementBottom != nullptr)
					{
						it = ElementData::s_ReactionTable[currElement.m_ID].find(elementBottom->m_ID);
						end = ElementData::s_ReactionTable[currElement.m_ID].end();
						if (it != end && GetRandom() < it->second.probability)
						{
							currElement.m_ID = it->second.cell0ID;
							ElementData& ed0 = ElementData::GetElementData(it->second.cell0ID);
							ed0.UpdateElementData(currElement, x, y);
							if (ed0.cell_type == ElementType::solid || ed0.cell_type == ElementType::movableSolid)
							{
								chunk->m_StaticColliderChanged = true;
							}

							elementBottom->m_ID = it->second.cell1ID;
							ElementData& ed1 = ElementData::GetElementData(it->second.cell1ID);
							if (ed1.cell_type == ElementType::solid || ed1.cell_type == ElementType::movableSolid)
							{
								bottomChunk->m_StaticColliderChanged = true;
							}
							ed1.UpdateElementData(elementBottom, x - 1, y);
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
					}
					
				}

				//handle universal element properties
				if (currElement.m_Temperature >= currElementData.melting_point)
				{
					//melt
					if (currElementData.melted != "")
					{
						int newID = ElementData::s_ElementNameToID[currElementData.melted];
						int temp = currElement.m_Temperature;
						ElementData& newData = ElementData::GetElementData(newID);
						newData.UpdateElementData(currElement, x, y);
						if (newData.cell_type == ElementType::solid || newData.cell_type == ElementType::movableSolid)
						{
							chunk->m_StaticColliderChanged = true;
						}
						currElement.m_Temperature = temp;
						currElement.m_ID = newID;
						chunk->UpdateDirtyRect(x, y);
						continue;
					}
				}

				if (currElement.m_Temperature <= currElementData.freezing_point)
				{
					//freeze
					if (currElementData.frozen != "")
					{
						int newID = ElementData::s_ElementNameToID[currElementData.frozen];
						int temp = currElement.m_Temperature;

						ElementData& newData = ElementData::GetElementData(newID);
						newData.UpdateElementData(currElement, x, y);
						if (newData.cell_type == ElementType::solid || newData.cell_type == ElementType::movableSolid)
						{
							chunk->m_StaticColliderChanged = true;
						}

						currElement.m_Temperature = temp;
						currElement.m_ID = newID;
						chunk->UpdateDirtyRect(x, y);
						continue;
					}
				}

				if (currElementData.conductivity > 0)
				{
					float tempBefore = currElement.m_Temperature;
					int minConductivity = 0;
					float diff = 0;

					if (elementLeft != nullptr && elementLeft->m_ID != 0)
					{
						minConductivity = std::min(currElementData.conductivity, elementLeftData->conductivity);
						diff = ((currElement.m_Temperature - elementLeft->m_Temperature) * ((float)minConductivity / 100.0f)) / 2;
						if (diff != 0)
						{
							currElement.m_Temperature -= diff / currElementData.density;
							elementLeft->m_Temperature += diff / elementLeftData->density;
						}
					}
					
					if (elementTop != nullptr && elementTop->m_ID != 0)
					{
						minConductivity = std::min(currElementData.conductivity, elementTopData->conductivity);
						diff = ((float)(currElement.m_Temperature - elementTop->m_Temperature) * ((float)minConductivity / 100.0f)) / 2;
						if (diff != 0)
						{
							currElement.m_Temperature -= diff / currElementData.density;
							elementTop->m_Temperature += diff / elementTopData->density;
						}						
					}

					if (elementRight != nullptr && elementRight->m_ID != 0)
					{
						minConductivity = std::min(currElementData.conductivity, elementRightData->conductivity);
						diff = ((float)(currElement.m_Temperature - elementRight->m_Temperature) * ((float)minConductivity / 100.0f)) / 2;
						if (diff != 0)
						{
							currElement.m_Temperature -= diff / currElementData.density;
							elementRight->m_Temperature += diff / elementRightData->density;
						}
					}
					
					if (elementBottom != nullptr && elementBottom->m_ID != 0)
					{
						minConductivity = std::min(currElementData.conductivity, elementBottomData->conductivity);
						diff = ((float)(currElement.m_Temperature - elementBottom->m_Temperature) * ((float)minConductivity / 100.0f)) / 2;
						if (diff != 0)
						{
							currElement.m_Temperature -= diff / currElementData.density;
							elementBottom->m_Temperature += diff / elementBottomData->density;
						}
					}
					//if the temperature changed, update the dirty rect
					if (std::abs(currElement.m_Temperature - tempBefore) > 0.01f) chunk->UpdateDirtyRect(x, y);
					
				}

				if (currElementData.flammable)
				{
					if (currElement.m_Temperature < currElementData.ignition_temperature && !currElementData.spread_ignition) currElement.m_Ignited = false;
					if (currElement.m_Temperature >= currElementData.ignition_temperature) currElement.m_Ignited = true;
					if (currElement.m_Ignited)
					{
						//try to spread ignition to surrounding elements
						//if (elementTopData.flammable) 
						if (currElementData.spread_ignition && GetRandom() < currElementData.spread_ignition_chance)
						{
							if (elementLeft != nullptr && elementLeftData->flammable) elementLeft->m_Ignited = true;
							if (elementTop != nullptr && elementTopData->flammable) elementTop->m_Ignited = true;
							if (elementRight != nullptr && elementRightData->flammable) elementRight->m_Ignited = true;
							if (elementBottom != nullptr && elementBottomData->flammable) elementBottom->m_Ignited = true;
						}

						//check for open air to burn
						int fireID = ElementData::s_ElementNameToID["fire"];
						ElementData& fireElementData = ElementData::GetElementData(fireID);
						if (currElement.m_ID != fireID) //&& GetRandom() < 5
						{

							int healthDiff = currElement.m_Health;
							if (elementLeft != nullptr && (elementLeftData->cell_type == ElementType::gas || elementLeftData->cell_type == ElementType::fire))
							{
								fireElementData.UpdateElementData(elementLeft, x-1, y);
								elementLeft->m_ID = fireID;
								elementLeft->m_Temperature = currElementData.fire_temperature;
								elementLeft->m_BaseColor = currElementData.fire_color;
								elementLeft->m_Color = currElementData.fire_color;
								currElement.m_Health--;
								if (currElement.m_Temperature < currElementData.fire_temperature - currElementData.fire_temperature_increase) currElement.m_Temperature += currElementData.fire_temperature_increase;
							}
							
							if (elementTop != nullptr && (elementTopData->cell_type == ElementType::gas || elementTopData->cell_type == ElementType::fire))
							{
								fireElementData.UpdateElementData(elementTop, x, y+1);
								elementTop->m_ID = fireID;
								elementTop->m_Temperature = currElementData.fire_temperature;
								elementTop->m_BaseColor = currElementData.fire_color;
								elementTop->m_Color = currElementData.fire_color;
								currElement.m_Health--;
								if (currElement.m_Temperature < currElementData.fire_temperature - currElementData.fire_temperature_increase) currElement.m_Temperature += currElementData.fire_temperature_increase;
							}
							
							if (elementRight != nullptr && (elementRightData->cell_type == ElementType::gas || elementRightData->cell_type == ElementType::fire))
							{
								fireElementData.UpdateElementData(elementRight, x+1, y);
								elementRight->m_ID = fireID;
								elementRight->m_Temperature = currElementData.fire_temperature;
								elementRight->m_BaseColor = currElementData.fire_color;
								elementRight->m_Color = currElementData.fire_color;
								currElement.m_Health--;
								if (currElement.m_Temperature < currElementData.fire_temperature - currElementData.fire_temperature_increase) currElement.m_Temperature += currElementData.fire_temperature_increase;
							}
							
							if (elementBottom != nullptr && (elementBottomData->cell_type == ElementType::gas || elementBottomData->cell_type == ElementType::fire))
							{
								fireElementData.UpdateElementData(elementBottom, x, y-1);
								elementBottom->m_ID = fireID;
								elementBottom->m_Temperature = currElementData.fire_temperature;
								elementBottom->m_BaseColor = currElementData.fire_color;
								elementBottom->m_Color = currElementData.fire_color;
								currElement.m_Health--;
								if (currElement.m_Temperature < currElementData.fire_temperature - currElementData.fire_temperature_increase) currElement.m_Temperature += currElementData.fire_temperature_increase;
							}
							if (currElement.m_Health != healthDiff)
							{
								currElement.m_Ignited = true;
							}
							else currElement.m_Ignited = false;
						}
					}
					
					
					if (currElement.m_Ignited)
					{
						if (currElement.m_Health <= 0)
						{
							//burnt
							int temp = currElement.m_Temperature;
							uint32_t burntID = ElementData::s_ElementNameToID[currElementData.burnt];
							ElementData& burntData = ElementData::GetElementData(burntID);
							if (burntData.cell_type == ElementType::solid || burntData.cell_type == ElementType::movableSolid)
							{
								chunk->m_StaticColliderChanged = true;
							}
							burntData.UpdateElementData(currElement, x, y);
							currElement.m_ID = burntID;
							currElement.m_Temperature = temp;
							continue;
						}
					}
					
					
				}


				//update the texture of the element based on temp / glow / ect
				uint32_t EditedBaseColor = currElement.m_BaseColor;
				if (currElement.m_Ignited)
				{
					//update color to reflect being on fire
					if (currElementData.ignited_color != 0)
						EditedBaseColor = RandomizeABGRColor(currElementData.ignited_color, 5);
				}

				if (currElementData.glow)
				{
					int r = (EditedBaseColor >> 0) & 255;
					int g = (EditedBaseColor >> 8) & 255;
					int b = (EditedBaseColor >> 16) & 255;
					int a = EditedBaseColor & 0xff000000;
					r = std::max(r, (int)(Pyxis::interpolateBetweenValues(460, 900, currElement.m_Temperature) * 255.0f));
					g = std::max(g, (int)(Pyxis::interpolateBetweenValues(460, 1500, currElement.m_Temperature) * 255.0f));
					b = std::max(b, (int)(Pyxis::interpolateBetweenValues(1000, 6000, currElement.m_Temperature) * 255.0f));
					EditedBaseColor = a | ((b & 255) << 16) | ((g & 255) << 8) | (r & 255);
					//make it appear hotter depending on temp
					//temp range from 460 to 6000
				}

				currElement.m_Color = EditedBaseColor;


				//switch the behavior based on element type
				switch (currElementData.cell_type)
				{
				case ElementType::solid:
					break;
				case ElementType::movableSolid:
					//skip if we belong to a rigid body? although this isn't a thing yet.
					if (currElement.m_Rigid) continue;


					//check below, and move					
					if (elementBottom != nullptr && elementBottomData->cell_type != ElementType::solid && elementBottomData->cell_type != ElementType::movableSolid && elementBottomData->density <= currElementData.density)
					{
						currElement.m_Sliding = true;
						Element temp = currElement;
						chunk->SetElement(x, y, *elementBottom);

						bottomChunk->SetElement(x, (y + (CHUNKSIZE - 1)) % CHUNKSIZE, temp);
						//*elementBottom = temp;
						UpdateChunkDirtyRect(x, y, chunk);
						if (elementLeft != nullptr) elementLeft->m_Sliding = true;
						if (elementRight != nullptr) elementRight->m_Sliding = true;
						continue;
					}
					
					

					if (currElement.m_Sliding)
					{
						//chance to stop sliding
						int rand = GetRandom();
						if (rand <= currElementData.friction)
						{
							currElement.m_Sliding = false;
							currElement.m_Horizontal = 0;
							continue;
						}
						if (currElement.m_Horizontal == 0)
						{
							currElement.m_Horizontal = (GetRandom() % 2 == 0) ? -1 : 1;
						}
					}
					else
					{
						continue;
					}

					//try moving to the side
					if (currElement.m_Horizontal > 0)
					{
						if (elementRight != nullptr && elementRightData->cell_type != ElementType::solid && elementRightData->cell_type != ElementType::movableSolid && elementRightData->density <= currElementData.density)
						{
							currElement.m_Sliding = true;
							Element temp = currElement;
							chunk->SetElement(x, y, *elementRight);
							//chunk->m_Elements[x + y * CHUNKSIZE] = *elementRight;
							rightChunk->SetElement((x + 1) % CHUNKSIZE, y, temp);
							//*elementRight = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
					}
					else
					{
						if (elementLeft != nullptr && elementLeftData->cell_type != ElementType::solid && elementLeftData->cell_type != ElementType::movableSolid && elementLeftData->density <= currElementData.density)
						{
							currElement.m_Sliding = true;
							Element temp = currElement;
							chunk->SetElement(x, y, *elementLeft);
							//chunk->m_Elements[x + y * CHUNKSIZE] = *elementLeft;
							leftChunk->SetElement((x + (CHUNKSIZE - 1)) % CHUNKSIZE, y, temp);
							//*elementLeft = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
					}

					break;
				case ElementType::liquid:
					//check below, and move
					if (elementBottom != nullptr && elementBottomData->cell_type != ElementType::solid && elementBottomData->cell_type != ElementType::movableSolid && elementBottomData->density < currElementData.density)
					{
						Element temp = currElement;
						chunk->m_Elements[x + y * CHUNKSIZE] = *elementBottom;
						*elementBottom = temp;
						UpdateChunkDirtyRect(x, y, chunk);
						continue;
					}
						
					r = GetRandom() & 1 ? 1 : -1;
					//int r = (x ^ 98252 + (m_UpdateBit * y) ^ 6234561) ? 1 : -1;

					//try left/right then bottom left/right

					if (r == 1)
					{
						//check right, and move
						if (elementRight != nullptr && elementRightData->cell_type != ElementType::solid && elementRightData->cell_type != ElementType::movableSolid && elementRightData->density < currElementData.density)
						{
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementRight;
							*elementRight = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
						//check left, and move
						if (elementLeft != nullptr && elementLeftData->cell_type != ElementType::solid && elementLeftData->cell_type != ElementType::movableSolid && elementLeftData->density < currElementData.density)
						{
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementLeft;
							*elementLeft = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
					}
					else
					{
						//check left, and move
						if (elementLeft != nullptr && elementLeftData->cell_type != ElementType::solid && elementLeftData->cell_type != ElementType::movableSolid && elementLeftData->density < currElementData.density)
						{
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementLeft;
							*elementLeft = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
						//check right, and move
						if (elementRight != nullptr && elementRightData->cell_type != ElementType::solid && elementRightData->cell_type != ElementType::movableSolid && elementRightData->density < currElementData.density)
						{
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementRight;
							*elementRight = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
					}

					

					//bottom left/right? lets try without for now.


					break;
				case ElementType::gas:

					// note: using < doesn't make sense, but because air doesn't update,
					// it itself doesn't move like a gas. therefore, density for gasses is inverted
					r = (GetRandom() % 3) - 1; //-1 0 1
					if (r == 0)
					{
						//check above, and move
						if (elementTop != nullptr && (elementTopData->cell_type == ElementType::gas || elementTopData->cell_type == ElementType::liquid) && elementTopData->density < currElementData.density)
						{
							currElement.m_Sliding = true;
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementTop;
							*elementTop = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
						
					}

					//try left/right
					if (r > 0)
					{
						//check right, and move
						if (elementRight != nullptr && (elementRightData->cell_type == ElementType::gas || elementRightData->cell_type == ElementType::liquid) && elementRightData->density < currElementData.density)
						{
							currElement.m_Sliding = true;
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementRight;
							*elementRight = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
						//check left and move
						if (elementLeft != nullptr && (elementLeftData->cell_type == ElementType::gas || elementLeftData->cell_type == ElementType::liquid) && elementLeftData->density < currElementData.density)
						{
							currElement.m_Sliding = true;
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementLeft;
							*elementLeft = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}

					}
					else
					{
						//check left and move
						if (elementLeft != nullptr && (elementLeftData->cell_type == ElementType::gas || elementLeftData->cell_type == ElementType::liquid) && elementLeftData->density < currElementData.density)
						{
							currElement.m_Sliding = true;
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementLeft;
							*elementLeft = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
						//check right, and move
						if (elementRight != nullptr && (elementRightData->cell_type == ElementType::gas || elementRightData->cell_type == ElementType::liquid) && elementRightData->density < currElementData.density)
						{
							currElement.m_Sliding = true;
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementRight;
							*elementRight = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
					}

					break;
				case ElementType::fire:
					//temp drop quickly
					//currElement.m_Temperature *= 0.95f;
					currElement.m_Health--;
					//die out if cold
					if (currElement.m_Temperature < currElementData.ignition_temperature)
					{
						currElement.m_ID = 0;//air
						ElementData::GetElementData(0).UpdateElementData(currElement, x, y);
						continue;
					}
					//fire gets special color treatment, basically going from starting color, and
					//losing its b, g, r color values at slower rates, respectively based on health
					//get rgb values, and diminish them onto the color
					
					uint32_t colorRed   = (currElement.m_BaseColor & 0x000000FF) >> 0;
					uint32_t colorGreen = (currElement.m_BaseColor & 0x0000FF00) >> 8;
					uint32_t colorBlue	= (currElement.m_BaseColor & 0x00FF0000) >> 16;
					uint32_t colorAlpha = currElement.m_BaseColor & 0xFF000000;

					float percentAlive = (float)currElement.m_Health / (float)currElementData.health;
					colorRed *= Pyxis::interpolateBetweenValues(-10, 20, currElement.m_Health);
					colorGreen *= Pyxis::interpolateBetweenValues(0, 20, currElement.m_Health);
					colorBlue *= Pyxis::interpolateBetweenValues(5, 20, currElement.m_Health);
					colorAlpha *= Pyxis::interpolateBetweenValues(0, 20, currElement.m_Health);

					currElement.m_Color = colorAlpha | (colorBlue << 16) | (colorGreen << 8) | colorRed;
					
					r = GetRandom();
					if (r > 20 && r < 80) //~60% to go up
					{
						//check above, and move						
						if (elementTop != nullptr && elementTopData->cell_type == ElementType::gas && (elementTopData->density > currElementData.density || elementTop->m_ID == 0))
						{
							currElement.m_Sliding = true;
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementTop;
							*elementTop = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
						else if (elementTop != nullptr && elementTopData->cell_type == ElementType::fire)
						{
							//moving to fire, so combine temp and leave air
							elementTop->m_Temperature = std::max(elementTop->m_Temperature, currElement.m_Temperature);
							elementTop->m_Health += currElement.m_Health;
							currElement.m_ID = 0;
							ElementData::GetElementData(0).UpdateElementData(currElement, x, y);
						}						
					}
					else if (r > 50) // left / right
					{
						if (elementRight != nullptr && elementRightData->cell_type == ElementType::gas)
						{
							currElement.m_Sliding = true;
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementRight;
							*elementRight = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
						else if (elementRight != nullptr && elementRightData->cell_type == ElementType::fire)
						{
							//moving to fire, so combine temp and leave air
							elementRight->m_Temperature = std::max(elementRight->m_Temperature, currElement.m_Temperature);
							currElement.m_ID = 0;
							ElementData::GetElementData(0).UpdateElementData(currElement, x, y);
						}
						if (elementLeft != nullptr && elementLeftData->cell_type == ElementType::gas)
						{
							currElement.m_Sliding = true;
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementLeft;
							*elementLeft = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
						else if (elementLeft != nullptr && elementLeftData->cell_type == ElementType::fire)
						{
							//moving to fire, so combine temp and leave air
							elementLeft->m_Temperature = std::max(elementLeft->m_Temperature, currElement.m_Temperature);
							currElement.m_ID = 0;
							ElementData::GetElementData(0).UpdateElementData(currElement, x, y);
						}
					}
					else
					{
						if (elementLeft != nullptr && elementLeftData->cell_type == ElementType::gas)
						{
							currElement.m_Sliding = true;
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementLeft;
							*elementLeft = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
						else if (elementLeft != nullptr && elementLeftData->cell_type == ElementType::fire)
						{
							//moving to fire, so combine temp and leave air
							elementLeft->m_Temperature = std::max(elementLeft->m_Temperature, currElement.m_Temperature);
							currElement.m_ID = 0;
							ElementData::GetElementData(0).UpdateElementData(currElement, x, y);
						}
						if (elementRight != nullptr && elementRightData->cell_type == ElementType::gas)
						{
							currElement.m_Sliding = true;
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementRight;
							*elementRight = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
						else if (elementRight != nullptr && elementRightData->cell_type == ElementType::fire)
						{
							//moving to fire, so combine temp and leave air
							elementRight->m_Temperature = std::max(elementRight->m_Temperature, currElement.m_Temperature);
							currElement.m_ID = 0;
							ElementData::GetElementData(0).UpdateElementData(currElement, x, y);
						}
					}
					UpdateChunkDirtyRect(x, y, chunk);
					break;
				}

			}
		}

		//check for pixel bodies that are in the surroundings, and if there is, update the collider if necessary.
		b2AABB queryFullRegion;
		glm::vec2 fullLower = glm::vec2(chunk->m_ChunkPos * CHUNKSIZE) / PPU;
		glm::vec2 fullUpper = glm::vec2((chunk->m_ChunkPos + glm::ivec2(1, 1)) * CHUNKSIZE) / PPU;
		queryFullRegion.lowerBound = b2Vec2(fullLower.x - 1, fullLower.y - 1);
		queryFullRegion.upperBound = b2Vec2(fullUpper.x + 1, fullUpper.y + 1);

		bool found = false;
		FoundDynamicBodyQuery dynamicCallback(found);
		Physics2D::GetWorld()->QueryAABB(&dynamicCallback, queryFullRegion);
		if (found)
		{
			//Found Dynamic Body
			if (chunk->m_StaticColliderChanged == true)
			{
				chunk->m_StaticColliderChanged = false;
				chunk->GenerateStaticCollider();
			}
		}

	}


	/// <summary>
	/// updated the chunks dirty rect, and will spread the
	/// dirty rect to neighboring chunks if it is touching the edge
	/// 
	/// The x and y are in index coordinates, not world.
	/// </summary>
	void World::UpdateChunkDirtyRect(int x, int y, Chunk* chunk)
	{
		chunk->UpdateDirtyRect(x, y);		

		//there are only 8 cases, so i will use a switch statement with bits?
		int result = 0;
		if (y == CHUNKSIZE - 1) result |= 8; //top
		if (x == CHUNKSIZE - 1) result |= 4; //right
		if (y == 0)			  result |= 2; //bottom
		if (x == 0)			  result |= 1; //left
		if (result == 0) return;
		
		//since we are on an chunk edge, update the other chunk

		//working on updating chunks
		Chunk* ChunkToUpdate;

		switch (result)
		{
		case 8:  // top
			ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(0, 1));
			if (ChunkToUpdate != nullptr)
			{
				ChunkToUpdate->UpdateDirtyRect(x, -1);
			}
			break;
		case 12: // top right
			//top
			ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(0, 1));
			if (ChunkToUpdate != nullptr)
			{
				ChunkToUpdate->UpdateDirtyRect(x, -1);
			}
			//top right
			ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(1, 1));
			if (ChunkToUpdate != nullptr)
			{
				ChunkToUpdate->UpdateDirtyRect(-1, -1);
			}
			//right
			ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(1, 0));
			if (ChunkToUpdate != nullptr)
			{
				ChunkToUpdate->UpdateDirtyRect(-1, y);
			}
			break;
		case 4:  // right
			//right
			ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(1, 0));
			if (ChunkToUpdate != nullptr)
			{
				ChunkToUpdate->UpdateDirtyRect(-1, y);
			}
			break;
		case 6:  // right bottom
			//right
			ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(1, 0));
			if (ChunkToUpdate != nullptr)
			{
				ChunkToUpdate->UpdateDirtyRect(-1, y);
			}
			//bottom right
			ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(1, -1));
			if (ChunkToUpdate != nullptr)
			{
				ChunkToUpdate->UpdateDirtyRect(-1, CHUNKSIZE - 1);
			}
			//bottom
			ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(0, -1));
			if (ChunkToUpdate != nullptr)
			{
				ChunkToUpdate->UpdateDirtyRect(x, CHUNKSIZE - 1);
			}
			break;
		case 2:  // bottom
			//bottom
			ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(0, -1));
			if (ChunkToUpdate != nullptr)
			{
				ChunkToUpdate->UpdateDirtyRect(x, CHUNKSIZE - 1);
			}
			break;
		case 3:  // bottom left

			//left
			ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(-1, 0));
			if (ChunkToUpdate != nullptr)
			{
				ChunkToUpdate->UpdateDirtyRect(CHUNKSIZE - 1, y);
			}
			//bottom left
			ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(-1, -1));
			if (ChunkToUpdate != nullptr)
			{
				ChunkToUpdate->UpdateDirtyRect(CHUNKSIZE - 1, CHUNKSIZE - 1);
			}
			//bottom
			ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(0, -1));
			if (ChunkToUpdate != nullptr)
			{
				ChunkToUpdate->UpdateDirtyRect(x, CHUNKSIZE - 1);
			}
			break;
		case 1:  // left
			//left
			ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(-1, 0));
			if (ChunkToUpdate != nullptr)
			{
				ChunkToUpdate->UpdateDirtyRect(CHUNKSIZE - 1, y);
			}
			break;
		case 9:  // top left
			//left
			ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(-1, 0));
			if (ChunkToUpdate != nullptr)
			{
				ChunkToUpdate->UpdateDirtyRect(CHUNKSIZE - 1, y);
			}
			//top left
			ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(-1, 1));
			if (ChunkToUpdate != nullptr)
			{
				ChunkToUpdate->UpdateDirtyRect(CHUNKSIZE - 1, -1);
			}
			//top
			ChunkToUpdate = GetChunk(chunk->m_ChunkPos + glm::ivec2(0, 1));
			if (ChunkToUpdate != nullptr)
			{
				ChunkToUpdate->UpdateDirtyRect(x, -1);
			}
			break;
		}
	}

	void World::CreateParticle(const glm::vec2& position, const glm::vec2& velocity, const Element& element)
	{
		m_ElementParticles.push_back(ElementParticle(position, velocity, element));
	}

	void World::UpdateParticles()
	{

		// NOTES FOR LATER IMPLEMENTATION
		// 
		// It would be great to have thrown particles use the positive velocity, but currently
		// that shoves the particles into the ground. Perhaps, if the particle hits a non-rigidbody solid, it could reflect?
		//
		
		// Use size_t for indices to avoid signed/unsigned mismatch
		for (size_t particleIndex = 0; particleIndex < m_ElementParticles.size(); ) {
			auto& particle = m_ElementParticles[particleIndex];

			//// Update the particle, whatever that might entail.
			particle.Update();



			//// Move the particle along its velocity, checking for collisions			


			// begin with seeing if the particle is starting in a collision. this usually would only happen if it
			// just landed with another particle, or if it was spawned inside a solid moving object.
			Element& element = ForceGetElement(particle.m_Position);
			ElementData& ed = ElementData::GetElementData(element.m_ID);
			// if we are in something rigid, or we collide with it, we need to slow our velocity until we "die"
			bool startedInCollision = false;
			if (element.m_Rigid || ((static_cast<int>(ed.cell_type) & static_cast<int>(particle.m_CollisionFlags)) >= 1))
			{
				particle.m_Velocity *= 0.8f;
				startedInCollision = true;
			}

			if (particle.m_Velocity.x * particle.m_Velocity.x + particle.m_Velocity.y * particle.m_Velocity.y < ElementParticle::DEADSPEED) {
				// If the particle is moving slowly, it has "died"
				// keep climbing up the same material until it's not the same material.
				Element* e = &ForceGetElement(particle.m_Position);
				while (e->m_ID == particle.m_Element.m_ID || e->m_Rigid) {
					particle.m_Position.y += 1;
					e = &ForceGetElement(particle.m_Position);
				}
				//e is now not of the same element, and not a rigid body. So see if it collides, and if not, set the particle to that position.
				ElementData& deadED = ElementData::GetElementData(e->m_ID);
				if ((static_cast<ElementTypeType>(deadED.cell_type) & particle.m_CollisionFlags) >= 1) {
					// Collision detected
					
					//I don't like the idea of particles being lost, but it is what it is for now
					std::swap(m_ElementParticles[particleIndex], m_ElementParticles.back());
					m_ElementParticles.pop_back();
					// Don’t increment particleIndex; the swapped-in element (if any) is now at particleIndex
					// and hasn’t been processed yet
					continue;
				}
				else
				{
					//no collision detected, so set the element then erase
					SetElement(particle.m_Position, particle.m_Element);
					std::swap(m_ElementParticles[particleIndex], m_ElementParticles.back());
					m_ElementParticles.pop_back();
					// Don’t increment particleIndex; the swapped-in element (if any) is now at particleIndex
					// and hasn’t been processed yet
					continue;
				}
				
				
			}

			//start by getting the path it will take
			glm::vec2 newPos = particle.m_Position + particle.m_Velocity;
			std::vector<glm::ivec2> path = Utils::getLinePath(particle.m_Position, newPos);
			particle.m_Position = newPos;


			bool shouldRemove = false;
			int index = 1;
			//loop over the path and see when we collide
			for (auto it = path.begin() + 1; it != path.end(); it++) {
				Element& element = ForceGetElement(*it);
				ElementData& ed = ElementData::GetElementData(element.m_ID);
				// Check collision
				if (element.m_Rigid || ((static_cast<int>(ed.cell_type) & static_cast<int>(particle.m_CollisionFlags)) >= 1)) {
					// Collision detected
					
					//if we started in collision, then ignore
					if (!startedInCollision)
					{
						// set element in world to previous position on path 
						SetElement(path[std::max(index - 1, 0)], particle.m_Element);
						shouldRemove = true; // Mark for removal
						break;
					}
					
				}
				else
				{
					if (startedInCollision)
					{
						//we were colliding but now we aren't, so lets just get back into the simulation
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
				std::swap(m_ElementParticles[particleIndex], m_ElementParticles.back());
				m_ElementParticles.pop_back();
				// Don’t increment particleIndex; the swapped-in element (if any) is now at particleIndex
				// and hasn’t been processed yet
			}
			else {
				// No collision, move to the next particle
				++particleIndex;
			}
		}
	}

	void World::RenderParticles()
	{
		for (auto& particle : m_ElementParticles) {
			particle.Render();
		}
	}

	/// <summary>
	/// wipes the world, and makes the first chunk empty
	/// </summary>
	void World::Clear()
	{
		for (auto& pair : m_Chunks)
		{
			delete pair.second;
		}
		m_Chunks.clear();

		Physics2D::DeleteWorld();
		Physics2D::GetWorld();

		//AddChunk(glm::ivec2(0, 0));
		//m_Chunks[{0, 0}]->Clear();

		////create a border around first chunk
		//Element ceramic = Element();
		//ceramic.m_ID = m_ElementIDs["ceramic"];
		//ElementData& elementData = m_ElementData[ceramic.m_ID];
		//elementData.UpdateElementData(ceramic, 0, 0);
		//for (int i = 0; i < CHUNKSIZE; i++)
		//{
		//	SetElement({ i, 0 }, ceramic);//bottom
		//	SetElement({ i, CHUNKSIZE - 1 }, ceramic);//top
		//	SetElement({ 0, i }, ceramic);//left
		//	SetElement({ CHUNKSIZE - 1, i }, ceramic);//right
		//}
	}

	/// <summary>
	/// Renders the world using Pyxis::renderer2d draw quad
	/// </summary>
	void World::RenderWorld()
	{

		//PX_TRACE("Rendering world");
		for (auto& pair : m_Chunks)
		{
			Renderer2D::DrawQuad(glm::vec2(pair.second->m_ChunkPos.x + 0.5f, pair.second->m_ChunkPos.y + 0.5f), { 1,1 }, pair.second->m_Texture);
			pair.second->RenderChunk();
			//Renderer2D::DrawQuad(glm::vec3(pair.second->m_ChunkPos.x + 0.5f, pair.second->m_ChunkPos.y + 0.5f, 1.0f), {0.1f, 0.1f}, glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
		}

		RenderParticles();

		//float pixelSize = (1.0f / CHUNKSIZE);
		if (m_DebugDrawColliders)
		{
			//drawing contour vector
			/*for each (auto pixelBody in m_PixelBodies)
			{
				for (int i = 0; i < pixelBody->m_ContourVector.size() - 1; i++)
				{
					glm::ivec2 pixelPos = { pixelBody->m_B2Body->GetPosition().x * PPU , pixelBody->m_B2Body->GetPosition().y * PPU };
					pixelPos -= pixelBody->m_Origin;
					pixelPos.y += 1;
					glm::vec2 worldPos = { pixelPos.x / CHUNKSIZEF, pixelPos.y / CHUNKSIZEF };
					glm::vec2 start = (glm::vec2(pixelBody->m_ContourVector[i].x, pixelBody->m_ContourVector[i].y) / 512.0f) + worldPos;
					glm::vec2 end = (glm::vec2(pixelBody->m_ContourVector[i + 1].x, pixelBody->m_ContourVector[i + 1].y) / 512.0f) + worldPos;
					Renderer2D::DrawLine(start, end, { 0,1,0,1 });
				}
			}*/

			//auto count = m_Box2DWorld->GetBodyCount();
			//for (auto body = m_Box2DWorld->GetBodyList(); body != nullptr; body = body->GetNext())
			//{
			//	auto& T = body->GetTransform();
			//	for (auto fixture = body->GetFixtureList(); fixture != nullptr; fixture = fixture->GetNext())
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


	void World::ResetBox2D()
	{
		PX_TRACE("Box2D sim reset at sim tick {0}", m_SimulationTick);
		Physics2D::ResetWorld();
		////struct to hold the data of the b2 bodies
		//std::unordered_map<uint64_t, PixelBodyData> storage;
		//for (auto pair : m_PixelBodyMap)
		//{
		//	//store the data to re-apply later
		//	PixelBodyData data;
		//	data.linearVelocity = pair.second->m_B2Body->GetLinearVelocity();
		//	data.angularVelocity = pair.second->m_B2Body->GetAngularVelocity();
		//	data.angle = pair.second->m_B2Body->GetAngle();
		//	data.position = pair.second->m_B2Body->GetPosition();
		//	//data.elementArray = pair.second->m_ElementArray;
		//	storage[pair.first] = data;
		//	//delete each b2body
		//	m_Box2DWorld->DestroyBody(pair.second->m_B2Body);
		//}
		////in theory this should delete the bodies so the above
		////step is unnecessary but it is nicer like this
		//m_Box2DWorld->~b2World();
		//m_Box2DWorld = new b2World({ 0, -9.8f });
		//for (auto pair : m_PixelBodyMap)
		//{
		//	//recreate the box2d body for each rigid body!
		//	pair.second->CreateB2Body(m_Box2DWorld);

		//	PixelBodyData& data = storage[pair.first];
		//	pair.second->m_B2Body->SetTransform(data.position, data.angle);
		//	pair.second->m_B2Body->SetLinearVelocity(data.linearVelocity);
		//	pair.second->m_B2Body->SetAngularVelocity(data.angularVelocity);
		//}
	}

	/// <summary>
	/// Creates a pixel rigid body, but it is not placed in the world!
	/// you have to call PutPixelBodyInWorld after setting the pixel bodies
	/// position
	/// </summary>
	//PixelRigidBody* World::CreatePixelRigidBody(uint64_t uuid, const glm::ivec2& size, Element* ElementArray, b2BodyType type)
	//{
	//	PixelRigidBody* body = new PixelRigidBody();
	//	body->m_Width = size.x;
	//	body->m_Height = size.y;
	//	body->m_Origin = { body->m_Width / 2, body->m_Height / 2 };
	//	body->m_ElementArray = ElementArray;
	//	
	//	//create the base body of the whole pixel body
	//	b2BodyDef pixelBodyDef;
	//	
	//	//for the scaling of the box world and pixel bodies, every PPU pixels is 1 unit in the world space
	//	pixelBodyDef.position = {0,0};
	//	pixelBodyDef.type = type;
	//	body->m_B2Body = m_Box2DWorld->CreateBody(&pixelBodyDef);


	//	//BUG ERROR FIX TODO WRONG BROKEN
	//	// getcontour points is able to have repeating points, which is a no-no for box2d triangles / triangulation
	//	auto contour = body->GetContourPoints();
	//	if (contour.size() == 0)
	//	{
	//		m_Box2DWorld->DestroyBody(body->m_B2Body);
	//		delete body;
	//		return nullptr;
	//	}

	//	auto contourVector = body->SimplifyPoints(contour, 0, contour.size() - 1, 1.0f);
	//	//auto simplified = body->SimplifyPoints(contour);

	//	//run triangulation algorithm to create the needed triangles/fixtures
	//	std::vector<p2t::Point*> polyLine;
	//	for each (auto point in contourVector)
	//	{
	//		polyLine.push_back(new p2t::Point(point));
	//	}

	//	//create each of the triangles to comprise the body, each being a fixture
	//	p2t::CDT* cdt = new p2t::CDT(polyLine);
	//	cdt->Triangulate();
	//	auto triangles = cdt->GetTriangles();
	//	for each (auto triangle in triangles)
	//	{
	//		b2PolygonShape triangleShape;
	//		b2Vec2 points[3] = {
	//			{(float)((triangle->GetPoint(0)->x - body->m_Origin.x) / PPU), (float)((triangle->GetPoint(0)->y - body->m_Origin.y) / PPU)},
	//			{(float)((triangle->GetPoint(1)->x - body->m_Origin.x) / PPU), (float)((triangle->GetPoint(1)->y - body->m_Origin.y) / PPU)},
	//			{(float)((triangle->GetPoint(2)->x - body->m_Origin.x) / PPU), (float)((triangle->GetPoint(2)->y - body->m_Origin.y) / PPU)}
	//		};
	//		triangleShape.Set(points, 3);
	//		b2FixtureDef fixtureDef;
	//		fixtureDef.density = 1;
	//		fixtureDef.friction = 0.3f;
	//		fixtureDef.shape = &triangleShape;
	//		body->m_B2Body->CreateFixture(&fixtureDef);
	//	}
	//	PX_TRACE("Pixel body created with ID: {0}", uuid);
	//	m_PixelBodyMap.insert({ uuid, body });
	//	return body;
	//}


	/// <summary>
	/// Seeds Rand() based on a few factors of the world.
	/// It is deterministic by update tick and position, so
	/// it is thread safe.
	/// </summary>
	void World::SeedRandom(int xPos, int yPos)
	{
		unsigned int seed = ((xPos * 58102) << m_SimulationTick % 5)
			+ (((yPos * 986124) * m_SimulationTick) >> 2);
		
		//PX_TRACE("Seeded rand with: {0}", seed);
		m_RandomEngine.seed(seed);
		//std::srand(seed);
	}

	int World::GetRandom()
	{
		int result = m_Rand(m_RandomEngine);
		//PX_TRACE("Got random number: {0}", result);
		return result;
	}

	const bool World::IsInBounds(int x, int y)
	{
		//having y first might actually be faster, simply because things tend to fall
		if (y < 0 || y >= CHUNKSIZE) return false;
		if (x < 0 || x >= CHUNKSIZE) return false;
		return true;
	}

	glm::ivec2 World::WorldToPixel(const glm::vec2& worldPos)
	{
		glm::ivec2 result = glm::ivec2(worldPos.x * CHUNKSIZE, worldPos.y * CHUNKSIZE);
		if (worldPos.x < 0) result.x--;
		if (worldPos.y < 0) result.y--;
		return result;
	}

	//Helper to get a chunk from a world pixel position
	glm::ivec2 World::PixelToChunk(const glm::ivec2& pixelPos)
	{
		glm::ivec2 result = {0,0};
		if (pixelPos.x < 0)
		{
			result.x = (pixelPos.x + 1) / CHUNKSIZE;
			result.x--;
		}
		else result.x = pixelPos.x / CHUNKSIZE;
		if (pixelPos.y < 0)
		{
			result.y = (pixelPos.y + 1) / CHUNKSIZE;
			result.y--;
		}
		else result.y = pixelPos.y / CHUNKSIZE;
		return result;
	}

	//Helper to get an index from a world pixel position
	glm::ivec2 World::PixelToIndex(const glm::ivec2& pixelPos)
	{
		glm::ivec2 result = {0,0};
		if (pixelPos.x < 0)
		{
			result.x = CHUNKSIZE - (std::abs(pixelPos.x) % CHUNKSIZE);
			result.x = result.x % CHUNKSIZE;
		}
		else result.x = pixelPos.x % CHUNKSIZE;
		if (pixelPos.y < 0)
		{
			result.y = CHUNKSIZE - (std::abs(pixelPos.y) % CHUNKSIZE);
			result.y = result.y % CHUNKSIZE;
		}
		else result.y = pixelPos.y % CHUNKSIZE;
		return result;
	}

}

