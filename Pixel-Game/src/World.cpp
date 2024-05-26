#include "World.h"
#include "ChunkWorker.h"
#include <random>
#include <thread>
#include <mutex>

namespace Pyxis
{
	World::World()
	{

	}

	World::~World()
	{
		for each (auto& pair in m_Chunks)
		{
			delete(pair.second);
		}
		/*for each (Chunk * chunk in m_Chunks_TR)
		{
			delete(chunk);
		}
		for each (Chunk * chunk in m_Chunks_BL)
		{
			delete(chunk);
		}
		for each (Chunk * chunk in m_Chunks_BR)
		{
			delete(chunk);
		}*/
	}

	void World::AddChunk(const glm::ivec2& chunkPos)
	{
		//make sure chunk doesn't already exist
		if (m_Chunks.find(chunkPos) == m_Chunks.end())
		{
			Chunk* chunk = new Chunk(chunkPos);
			m_Chunks[chunkPos] = chunk;
		}
	}

	Chunk* World::GetChunk(const glm::ivec2& chunkPos)
	{
		auto it = m_Chunks.find(chunkPos);
		if (it != m_Chunks.end())
			return it->second;
		AddChunk(chunkPos);
		return m_Chunks[chunkPos];
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

		for each (auto & pair in m_Chunks)
		{
			//BL
			UpdateChunkBucket(pair.second, 0, 0);
			UpdateChunkBucket(pair.second, 2, 0);
			UpdateChunkBucket(pair.second, 4, 0);
			UpdateChunkBucket(pair.second, 6, 0);
			UpdateChunkBucket(pair.second, 0, 2);
			UpdateChunkBucket(pair.second, 2, 2);
			UpdateChunkBucket(pair.second, 4, 2);
			UpdateChunkBucket(pair.second, 6, 2);
			UpdateChunkBucket(pair.second, 0, 4);
			UpdateChunkBucket(pair.second, 2, 4);
			UpdateChunkBucket(pair.second, 4, 4);
			UpdateChunkBucket(pair.second, 6, 4);
			UpdateChunkBucket(pair.second, 0, 6);
			UpdateChunkBucket(pair.second, 2, 6);
			UpdateChunkBucket(pair.second, 4, 6);
			UpdateChunkBucket(pair.second, 6, 6);
		}

		for each (auto & pair in m_Chunks)
		{
			//BR
			UpdateChunkBucket(pair.second, 0 + 1, 0);
			UpdateChunkBucket(pair.second, 2 + 1, 0);
			UpdateChunkBucket(pair.second, 4 + 1, 0);
			UpdateChunkBucket(pair.second, 6 + 1, 0);
			UpdateChunkBucket(pair.second, 0 + 1, 2);
			UpdateChunkBucket(pair.second, 2 + 1, 2);
			UpdateChunkBucket(pair.second, 4 + 1, 2);
			UpdateChunkBucket(pair.second, 6 + 1, 2);
			UpdateChunkBucket(pair.second, 0 + 1, 4);
			UpdateChunkBucket(pair.second, 2 + 1, 4);
			UpdateChunkBucket(pair.second, 4 + 1, 4);
			UpdateChunkBucket(pair.second, 6 + 1, 4);
			UpdateChunkBucket(pair.second, 0 + 1, 6);
			UpdateChunkBucket(pair.second, 2 + 1, 6);
			UpdateChunkBucket(pair.second, 4 + 1, 6);
			UpdateChunkBucket(pair.second, 6 + 1, 6);
		}

		for each (auto & pair in m_Chunks)
		{
			//TL
			UpdateChunkBucket(pair.second, 0, 0 + 1);
			UpdateChunkBucket(pair.second, 2, 0 + 1);
			UpdateChunkBucket(pair.second, 4, 0 + 1);
			UpdateChunkBucket(pair.second, 6, 0 + 1);
			UpdateChunkBucket(pair.second, 0, 2 + 1);
			UpdateChunkBucket(pair.second, 2, 2 + 1);
			UpdateChunkBucket(pair.second, 4, 2 + 1);
			UpdateChunkBucket(pair.second, 6, 2 + 1);
			UpdateChunkBucket(pair.second, 0, 4 + 1);
			UpdateChunkBucket(pair.second, 2, 4 + 1);
			UpdateChunkBucket(pair.second, 4, 4 + 1);
			UpdateChunkBucket(pair.second, 6, 4 + 1);
			UpdateChunkBucket(pair.second, 0, 6 + 1);
			UpdateChunkBucket(pair.second, 2, 6 + 1);
			UpdateChunkBucket(pair.second, 4, 6 + 1);
			UpdateChunkBucket(pair.second, 6, 6 + 1);
		}

		for each (auto & pair in m_Chunks)
		{
			//TR
			UpdateChunkBucket(pair.second, 0 + 1, 0 + 1);
			UpdateChunkBucket(pair.second, 2 + 1, 0 + 1);
			UpdateChunkBucket(pair.second, 4 + 1, 0 + 1);
			UpdateChunkBucket(pair.second, 6 + 1, 0 + 1);
			UpdateChunkBucket(pair.second, 0 + 1, 2 + 1);
			UpdateChunkBucket(pair.second, 2 + 1, 2 + 1);
			UpdateChunkBucket(pair.second, 4 + 1, 2 + 1);
			UpdateChunkBucket(pair.second, 6 + 1, 2 + 1);
			UpdateChunkBucket(pair.second, 0 + 1, 4 + 1);
			UpdateChunkBucket(pair.second, 2 + 1, 4 + 1);
			UpdateChunkBucket(pair.second, 4 + 1, 4 + 1);
			UpdateChunkBucket(pair.second, 6 + 1, 4 + 1);
			UpdateChunkBucket(pair.second, 0 + 1, 6 + 1);
			UpdateChunkBucket(pair.second, 2 + 1, 6 + 1);
			UpdateChunkBucket(pair.second, 4 + 1, 6 + 1);
			UpdateChunkBucket(pair.second, 6 + 1, 6 + 1);
		}


		//std::thread thread = std::thread(&World::UpdateChunk, this, m_Chunks[glm::ivec2(0,0)]);
		//thread.join();
		/*for (int x = 0; x < BUCKETSWIDTH; x++)
		{
			for (int y = 0; y < BUCKETSWIDTH; y++)
			{
				UpdateChunkBucket(m_Chunks[glm::ivec2(0, 0)], x, y);
			}
		}*/

		//m_Threads.clear();
		//for each (auto& pair in m_Chunks)
		//{
		//	Chunk* chunk = pair.second;
		//	m_Threads.push_back(std::thread(&World::UpdateChunk, this, chunk));
		//	/*m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 2, 0));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 4, 0));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 6, 0));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 0, 2));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 2, 2));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 4, 2));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 6, 2));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 0, 4));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 2, 4));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 4, 4));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 6, 4));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 0, 6));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 2, 6));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 4, 6));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 6, 6));*/
		//}

		//for each (std::thread& thread in m_Threads)
		//{
		//	thread.join();
		//}
		
		for each (auto & pair in m_Chunks)
		{
			pair.second->UpdateTexture();
		}
		

		//UpdateChunk(m_Chunks[{0, 0}]);

		m_UpdateBit = !m_UpdateBit;
	}

	/// <summary>
	/// 
	/// updating a chunk overview:
	/// copy the dirt rect state, and clear it so we can re-create it while updating
	/// loop over the elements, and update them. if the elements they are trying to swap with
	/// is out of bounds, then find the other chunk and get the element like that.
	/// because an element can belong to another chunk, there is a lot more conditional logic.
	/// const int BUCKETS = chunk->BUCKETS;
	/// const int BUCKETSIZE = chunk->BUCKETSIZE;
	/// </summary>
	/// <param name="chunk"></param>
	/// <param name="bucketX"></param>
	/// <param name="bucketY"></param>
	void World::UpdateChunkBucket(Chunk* chunk, int bucketX, int bucketY)
	{
		//defines for easy to read code
		#define SwapWithOther chunk->SetElement(xOther, yOther, currElement); chunk->SetElement(x, y, other); UpdateChunkDirtyRect(x, y, chunk);
		#define SwapWithOtherChunk otherChunk->m_Elements[indexOther] = currElement; chunk->SetElement(x, y, other); UpdateChunkDirtyRect(x, y, chunk);


		//copy the dirty rect
		std::pair<glm::ivec2, glm::ivec2> minmax = chunk->m_DirtyRects[bucketX + bucketY * BUCKETSWIDTH];

		//instead of resetting completely, just shrink by 1. This allows elements that cross borders to update, since otherwise the dirty rect would
		//forget about it instead of shrinking and still hitting it.
		chunk->m_DirtyRects[bucketX + bucketY * BUCKETSWIDTH] = std::pair<glm::ivec2, glm::ivec2>(
			minmax.first + 1, //min grows
			minmax.second - 1 //max shrinks
		);
		 
		//get the min and max
		//loop from min to max in both "axies"?
		bool directionBit = false;

		if (minmax.first.x <= minmax.second.x)
			if (minmax.first.y <= minmax.second.y)	
		for (int y = std::max(minmax.first.y, bucketY * BUCKETSIZE); y <= std::min(minmax.second.y, ((bucketY + 1) * BUCKETSIZE) - 1); y++) //going y then x so we do criss crossing 
		{
			directionBit = !directionBit;
			int startNum = directionBit ? std::max(minmax.first.x, bucketX * BUCKETSIZE) : std::min(minmax.second.x, ((bucketX + 1) * BUCKETSIZE) - 1);
			int compNumber = directionBit ? std::min(minmax.second.x, ((bucketX + 1) * BUCKETSIZE) - 1) + 1 : std::max(minmax.first.x, bucketX * BUCKETSIZE) - 1;
			//PX_TRACE("x is: {0}", x);
			for (int x = startNum; x != compNumber; directionBit ? x++ : x--)
			{
				//we now have an x and y of the element in the array, so update it
				Element& currElement = chunk->m_Elements[x + y * CHUNKSIZE];

				//skip if already updated
				if (currElement.m_Updated == m_UpdateBit) continue;
				//flip the update bit so we know we updated this element
				currElement.m_Updated = m_UpdateBit;

				int xOther = x;
				int yOther = y;

				//switch the behavior based on element type
				switch (currElement.m_Type)
				{
				case ElementType::solid:
					switch (currElement.m_SubType)
					{
					case ElementSubType::None:
						//check below, and move
						xOther = x;
						yOther = y - 1;
						if (IsInBounds(xOther, yOther))
						{
							//operate within the chunk, since we are in bounds
							Element other = chunk->GetElement(xOther, yOther);
							if (other.m_Type != ElementType::solid)
							{
								SwapWithOther;
								continue;
							}
						}
						else
						{
							//we need to get the element by finding the other chunk
							glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(xOther, yOther);
							Chunk* otherChunk = GetChunk(PixelToChunk(pixelSpace));
							int indexOther = ((xOther + CHUNKSIZE) % CHUNKSIZE) + ((yOther + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE;
							Element other = otherChunk->m_Elements[indexOther];
							if (other.m_Type != ElementType::solid)
							{
								//other element is air so we move to it
								SwapWithOtherChunk;
								continue;
							}
						}

						//next check bottom left and try to move
						xOther = x - 1;
						yOther = y - 1;
						if (IsInBounds(xOther, yOther))
						{
							//operate within the chunk, since we are in bounds
							Element other = chunk->GetElement(xOther, yOther);
							if (other.m_Type != ElementType::solid)
							{
								SwapWithOther;
								continue;
							}
						}
						else
						{
							//we need to get the element by finding the other chunk
							glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(xOther, yOther);
							Chunk* otherChunk = GetChunk(PixelToChunk(pixelSpace));
							int indexOther = ((xOther + CHUNKSIZE) % CHUNKSIZE) + ((yOther + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE;
							Element other = otherChunk->m_Elements[indexOther];
							if (other.m_Type != ElementType::solid)
							{
								SwapWithOtherChunk;
								continue;
							}
						}

						//next check bottom right and try to move
						xOther = x + 1;
						yOther = y - 1;
						if (IsInBounds(xOther, yOther))
						{
							//operate within the chunk, since we are in bounds
							Element other = chunk->GetElement(xOther, yOther);
							if (other.m_Type != ElementType::solid)
							{
								SwapWithOther;
								continue;
							}
						}
						else
						{
							//we need to get the element by finding the other chunk
							glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(xOther, yOther);
							Chunk* otherChunk = GetChunk(PixelToChunk(pixelSpace));
							int indexOther = ((xOther + CHUNKSIZE) % CHUNKSIZE) + ((yOther + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE;
							Element other = otherChunk->m_Elements[indexOther];
							if (other.m_Type != ElementType::solid)
							{
								SwapWithOtherChunk;
								continue;
							}
						}

						break;
					case ElementSubType::Static:
						//like stone, do nothing
						break;
					default:
						break;
					}
					

					break;
				case ElementType::liquid:
					switch (currElement.m_SubType)
					{
					case ElementSubType::None:
						Element other;
						//check below, and move
						xOther = x;
						yOther = y - 1;
						if (IsInBounds(xOther, yOther))
						{
							//operate within the chunk, since we are in bounds
							//Element& other = chunk->GetElement(xOther, yOther);
							other = chunk->m_Elements[xOther + yOther * CHUNKSIZE];
							if (other.m_Type == ElementType::gas || other.m_Type == ElementType::fire)
							{
								SwapWithOther;
								continue;
							}
						}
						else
						{
							//we need to get the element by finding the other chunk
							glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(xOther, yOther);
							Chunk* otherChunk = GetChunk(PixelToChunk(pixelSpace));
							int indexOther = ((xOther + CHUNKSIZE) % CHUNKSIZE) + ((yOther + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE;
							other = otherChunk->m_Elements[indexOther];
							if (other.m_Type == ElementType::gas || other.m_Type == ElementType::fire)
							{
								SwapWithOtherChunk;
								continue;
							}
						}
						
						int r = std::rand() & 1 ? 1 : -1;
						//int r = (x ^ 98252 + (m_UpdateBit * y) ^ 6234561) ? 1 : -1;

						//try left/right then bottom left/right
						xOther = x - r;
						yOther = y;
						if (IsInBounds(xOther, yOther))
						{
							//operate within the chunk, since we are in bounds
							//Element& other = chunk->GetElement(xOther, yOther);
							other = chunk->m_Elements[xOther + yOther * CHUNKSIZE];
							if (other.m_Type == ElementType::gas || other.m_Type == ElementType::fire)
							{
								SwapWithOther;
								continue;
							}
						}
						else
						{
							//we need to get the element by finding the other chunk
							glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(xOther, yOther);
							Chunk* otherChunk = GetChunk(PixelToChunk(pixelSpace));
							int indexOther = ((xOther + CHUNKSIZE) % CHUNKSIZE) + ((yOther + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE;
							other = otherChunk->m_Elements[indexOther];
							if (other.m_Type == ElementType::gas || other.m_Type == ElementType::fire)
							{
								SwapWithOtherChunk;
								continue;
							}
						}

						xOther = x + r;
						//yOther = y;
						if (IsInBounds(xOther, yOther))
						{
							//operate within the chunk, since we are in bounds
							//Element& other = chunk->GetElement(xOther, yOther);
							other = chunk->m_Elements[xOther + yOther * CHUNKSIZE];
							if (other.m_Type == ElementType::gas || other.m_Type == ElementType::fire)
							{
								SwapWithOther;
								continue;
							}
						}
						else
						{
							//we need to get the element by finding the other chunk
							glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(xOther, yOther);
							Chunk* otherChunk = GetChunk(PixelToChunk(pixelSpace));
							int indexOther = ((xOther + CHUNKSIZE) % CHUNKSIZE) + ((yOther + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE;
							other = otherChunk->m_Elements[indexOther];
							if (other.m_Type == ElementType::gas || other.m_Type == ElementType::fire)
							{
								SwapWithOtherChunk;
								continue;
							}
						}

						//bottom left/right
						xOther = x - r;
						yOther = y - 1;
						if (IsInBounds(xOther, yOther))
						{
							//operate within the chunk, since we are in bounds
							//Element& other = chunk->GetElement(xOther, yOther);
							other = chunk->m_Elements[xOther + yOther * CHUNKSIZE];
							if (other.m_Type == ElementType::gas || other.m_Type == ElementType::fire)
							{
								SwapWithOther;
								continue;
							}
						}
						else
						{
							//we need to get the element by finding the other chunk
							glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(xOther, yOther);
							Chunk* otherChunk = GetChunk(PixelToChunk(pixelSpace));
							int indexOther = ((xOther + CHUNKSIZE) % CHUNKSIZE) + ((yOther + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE;
							other = otherChunk->m_Elements[indexOther];
							if (other.m_Type == ElementType::gas || other.m_Type == ElementType::fire)
							{
								SwapWithOtherChunk;
								continue;
							}
						}

						xOther = x + r;
						//yOther = y - 1;
						if (IsInBounds(xOther, yOther))
						{
							//operate within the chunk, since we are in bounds
							//Element& other = chunk->GetElement(xOther, yOther);
							other = chunk->m_Elements[xOther + yOther * CHUNKSIZE];
							if (other.m_Type == ElementType::gas || other.m_Type == ElementType::fire)
							{
								SwapWithOther;
								continue;
							}
						}
						else
						{
							//we need to get the element by finding the other chunk
							glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(xOther, yOther);
							Chunk* otherChunk = GetChunk(PixelToChunk(pixelSpace));
							int indexOther = ((xOther + CHUNKSIZE) % CHUNKSIZE) + ((yOther + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE;
							other = otherChunk->m_Elements[indexOther];
							if (other.m_Type == ElementType::gas || other.m_Type == ElementType::fire)
							{
								SwapWithOtherChunk;
								continue;
							}
						}


						////check if there is no momentum and add some if needed
						//if (currElement.m_Vertical != 0 && currElement.m_Horizontal == 0) {
						//	currElement.m_Horizontal = m_UpdateBit ? 1 : -1;
						//	currElement.m_Vertical = 0;
						//}
						////skip if we are not needing to move
						//if (currElement.m_Horizontal == 0) continue;
						////try a side to move to based on horizontal
						//if (currElement.m_Horizontal < 0)
						//	//next check left and try to move
						//	xOther = x - 1;
						//else
						//	xOther = x + 1;
						//yOther = y;
						//if (IsInBounds(xOther, yOther))
						//{
						//	//operate within the chunk, since we are in bounds
						//	Element other = chunk->GetElement(xOther, yOther);
						//	if (other.m_Type == ElementType::gas || other.m_Type == ElementType::fire)
						//	{
						//		//other element is air so we move to it
						//		chunk->SetElement(xOther, yOther, currElement);
						//		chunk->SetElement(x, y, Element());
						//		UpdateChunkDirtyRect(x, y, chunk);
						//		continue;
						//	}
						//}
						//else
						//{
						//	//we need to get the element by finding the other chunk
						//	glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(xOther, yOther);
						//	Chunk* otherChunk = GetChunk(PixelToChunk(pixelSpace));
						//	int indexOther = ((xOther + CHUNKSIZE) % CHUNKSIZE) + ((yOther + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE;
						//	Element other = otherChunk->m_Elements[indexOther];
						//	if (other.m_Type == ElementType::gas || other.m_Type == ElementType::fire)
						//	{
						//		//other element is air so we move to it
						//		otherChunk->m_Elements[indexOther] = currElement;
						//		chunk->SetElement(x, y, other);
						//		UpdateChunkDirtyRect(x, y, chunk);
						//		continue;
						//	}
						//}
						////since the liquid didn't move, swap the horizontal
						//currElement.m_Horizontal = -currElement.m_Horizontal;

						
					}
					break;
				default:
					break;
				}

			}
		}
	}

	void World::UpdateChunkDirtyRect(int x, int y, Chunk* chunk)
	{
		//this needs to update the surrounding buckets if the coord is on the corresponding edge
		//ex: if on top edge, also update that bucket using this x/y. this is fine since it clamps to
		//bucket size anyway
		std::pair<glm::ivec2, glm::ivec2>* minmax = (chunk->m_DirtyRects + (x / BUCKETSIZE) + (y / BUCKETSIZE) * BUCKETSWIDTH);
		//update minimums
		if (x < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = x - chunk->m_DirtyRectBorderWidth;
		if (y < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = y - chunk->m_DirtyRectBorderWidth;
		//update maximums
		if (x > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = x + chunk->m_DirtyRectBorderWidth;
		if (y > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = y + chunk->m_DirtyRectBorderWidth;

		//there are only 8 cases, so i will use a switch statement with bits?
		int result = 0;
		if (y % BUCKETSIZE == BUCKETSIZE - 1) result |= 8; //top
		if (x % BUCKETSIZE == BUCKETSIZE - 1) result |= 4; //right
		if (y % BUCKETSIZE == 0)			  result |= 2; //bottom
		if (x % BUCKETSIZE == 0)			  result |= 1; //left

		if (result == 0) return;
		//since we are on an edge, see if we need to get a different chunk

		//SOMETHING IS WRONG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		//working on updating chunks
		Chunk* ChunkToUpdate;
		glm::ivec2 currentChunk = chunk->m_ChunkPos;
		int xOther = 0, yOther = 0;

		switch (result)
		{
		case 8:  // top

			//can optimize this if needed
			/*if (x >= CHUNKSIZE) currentChunk += glm::ivec2(1, 0);
			if (y >= CHUNKSIZE) currentChunk += glm::ivec2(0, 1);
			if (x < 0) currentChunk += glm::ivec2(-1, 0);
			if (y < 0) currentChunk += glm::ivec2(0, -1);*/
			if (y + 1 >= CHUNKSIZE) currentChunk += glm::ivec2(0, 1);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);
			xOther = (x + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y + 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			break;
		case 12: // top right
			
			//top
			if (y + 1 >= CHUNKSIZE) currentChunk += glm::ivec2(0, 1);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);
			xOther = (x + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y + 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			//top right
			currentChunk = chunk->m_ChunkPos;
			if (y + 1 >= CHUNKSIZE) currentChunk += glm::ivec2(0, 1);
			if (x + 1 >= CHUNKSIZE) currentChunk += glm::ivec2(1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);
			xOther = ((x + 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y + 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			//right
			currentChunk = chunk->m_ChunkPos;
			if (x + 1 >= CHUNKSIZE) currentChunk += glm::ivec2(1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);
			xOther = ((x + 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = (y + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}
			break;
		case 4:  // right
			if (x+1 >= CHUNKSIZE) currentChunk += glm::ivec2(1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);

			xOther = ((x + 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = (y + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			break;
		case 6:  // right bottom

			//right
			if (x + 1 >= CHUNKSIZE) currentChunk += glm::ivec2(1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);

			xOther = ((x + 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = (y + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			//br
			currentChunk = chunk->m_ChunkPos;
			if (y - 1 < 0) currentChunk += glm::ivec2(0, -1);
			if (x + 1 >= CHUNKSIZE) currentChunk += glm::ivec2(1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);
			xOther = ((x + 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y - 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			//bottom
			currentChunk = chunk->m_ChunkPos;
			currentChunk = chunk->m_ChunkPos;
			if (y - 1 < 0) currentChunk += glm::ivec2(0, -1);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);
			xOther = (x + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y - 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			break;
		case 2:  // bottom

			currentChunk = chunk->m_ChunkPos;
			if (y - 1 < 0) currentChunk += glm::ivec2(0, -1);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);

			xOther = (x + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y - 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			break;
		case 3:  // bottom left

			//bottom
			currentChunk = chunk->m_ChunkPos;
			if (y - 1 < 0) currentChunk += glm::ivec2(0, -1);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);

			xOther = (x + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y - 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			//bottom left
			currentChunk = chunk->m_ChunkPos;
			if (y - 1 < 0) currentChunk += glm::ivec2(0, -1);
			if (x - 1 < 0) currentChunk += glm::ivec2(-1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);
			xOther = ((x - 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y - 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			//left
			currentChunk = chunk->m_ChunkPos;
			if (x - 1 < 0) currentChunk += glm::ivec2(-1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);

			xOther = ((x - 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = (y + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			break;
		case 1:  // left
			if (x - 1 < 0) currentChunk += glm::ivec2(-1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);

			xOther = ((x - 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = (y + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}
			break;
		case 9:  // top left
			//left
			if (x - 1 < 0) currentChunk += glm::ivec2(-1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);

			xOther = ((x - 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = (y + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			//top left
			currentChunk = chunk->m_ChunkPos;
			if (y + 1 >= CHUNKSIZE) currentChunk += glm::ivec2(0, 1);
			if (x - 1 < 0) currentChunk += glm::ivec2(-1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);
			xOther = ((x - 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y + 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}
			break;

			//top
			currentChunk = chunk->m_ChunkPos;
			if (y + 1 >= CHUNKSIZE) currentChunk += glm::ivec2(0, 1);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);
			xOther = (x + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y + 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}
		}
	}

	/// <summary>
	/// DEPRECATED, no longer used
	/// </summary>
	void World::UpdateChunk(Chunk* chunk)
	{
		//updating a chunk overview:
		//copy the dirt rect state, and clear it so we can re-create it while updating
		//loop over the elements, and update them. if the elements they are trying to swap with
		//is out of bounds, then find the other chunk and get the element like that.
		//because an element can belong to another chunk, there is a lot more conditional logic.
		//const int BUCKETS = chunk->BUCKETS;
		//const int BUCKETSIZE = chunk->BUCKETSIZE;


		//copy the dirty rects
		std::pair<glm::ivec2, glm::ivec2> DirtyRects[BUCKETSWIDTH * BUCKETSWIDTH];
		memcpy(DirtyRects, chunk->m_DirtyRects, sizeof(DirtyRects));
		//fix this if uncommented
		//chunk->m_DirtyRects[bucketX + bucketY * BUCKETSWIDTH] = std::pair<glm::ivec2, glm::ivec2>(glm::ivec2(), glm::ivec2());
		chunk->ResetDirtyRects();
		//memset(chunk->m_DirtyRects, 0, sizeof(DirtyRects));

		//loop over each dirty rect
		for (int bx = 0; bx < BUCKETSWIDTH; bx++)
		{
			for (int by = 0; by < BUCKETSWIDTH; by++)
			{
				//get the min and max
				auto& minmax = DirtyRects[bx + by * BUCKETSWIDTH];
				Element currElement;
				//loop from min to max in both "axies"?
				for (int x = minmax.first.x; x <= minmax.second.x; x++)
				{
					for (int y = minmax.first.y; y <= minmax.second.y; y++)
					{
						//we now have an x and y of the element in the array, so update it
						currElement = chunk->m_Elements[x + y * CHUNKSIZE];

						//skip if already updated
						if (currElement.m_Updated == m_UpdateBit) continue;
						//flip the update bit so we know we updated this element
						currElement.m_Updated = m_UpdateBit;

						//switch the behavior based on element type
						switch (currElement.m_Type)
						{
						case ElementType::solid:

							//check below, and move
							if (IsInBounds(x, y - 1))
							{
								//operate within the chunk, since we are in bounds
								if (chunk->GetElement(x, y - 1).m_ID == 0)
								{
									//other element is air so we move to it
									chunk->SetElement(x, y - 1, currElement);
									chunk->SetElement(x, y, Element());
									chunk->UpdateDirtyRect(x, y);
									chunk->UpdateDirtyRect(x, y - 1);
									continue;
								}
							}
							else
							{
								//we need to get the element by finding the other chunk

							}

							break;
						default:
							break;
						}

					}
				}
			}
		}
	}

	void World::SetElement(const glm::ivec2& pixelPos, const Element& element)
	{
		//PX_TRACE("Setting element at ({0}, {1})", pixelPos.x, pixelPos.y);
		glm::ivec2 index = PixelToIndex(pixelPos);
		Element e = element;
		e.m_Updated = !m_UpdateBit;
		GetChunk(PixelToChunk(pixelPos))->SetElement(index.x, index.y, e);
	}

	void World::RenderWorld()
	{
		
		//PX_TRACE("Rendering world");
		for each (auto pair in m_Chunks)
		{
			//PX_TRACE("Drawing chunk {0}, {1}", pair.second->m_ChunkPos.x, pair.second->m_ChunkPos.y);
			Renderer2D::DrawQuad(glm::vec2(pair.second->m_ChunkPos.x + 0.5f, pair.second->m_ChunkPos.y + 0.5f), { 1,1 }, pair.second->m_Texture);
			
			//Renderer2D::DrawQuad(glm::vec3(pair.second->m_ChunkPos.x + 0.5f, pair.second->m_ChunkPos.y + 0.5f, 1.0f), {0.1f, 0.1f}, glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
		}
	}


	const bool World::IsInBounds(int x, int y)
	{
		//having y first might actually be faster, simply because things tend to fall
		if (y < 0 || y >= CHUNKSIZE) return false;
		if (x < 0 || x >= CHUNKSIZE) return false;
		return true;
	}

	//Helper to get a chunk from a world pixel position
	glm::ivec2 World::PixelToChunk(const glm::ivec2& pixelPos)
	{
		glm::ivec2 result;
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
		glm::ivec2 result;
		if (pixelPos.x < 0)
		{
			result.x = CHUNKSIZE - (std::abs(pixelPos.x) % CHUNKSIZE);
		}
		else result.x = pixelPos.x % CHUNKSIZE;
		if (pixelPos.y < 0)
		{
			result.y = CHUNKSIZE - (std::abs(pixelPos.y) % CHUNKSIZE);
		}
		else result.y = pixelPos.y % CHUNKSIZE;
		return result;
	}

}

