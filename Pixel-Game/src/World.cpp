#include "World.h"
#include "ChunkWorker.h"
#include <thread>
#include <mutex>

namespace Pyxis
{
	World::World()
	{

	}

	void World::AddChunk(const glm::ivec2& chunkPos)
	{
		//make sure chunk doesn't already exist
		if (m_Chunks.find(chunkPos) == m_Chunks.end())
		{
			Chunk* chunk = new Chunk(chunkPos);
			m_Chunks[chunkPos] = chunk;
			//add to the other containers for fast multi-threadable updating
			if (chunkPos.x % 2 == 0)
			{
				//x is even
				if (chunkPos.y % 2 == 0)
				{
					//y is even
					m_Chunks_BL.push_back(chunk);

				}
				else
				{
					//y is odd
					m_Chunks_TL.push_back(chunk);
				}
			}
			else
			{
				//x is odd
				if (chunkPos.y % 2 == 0)
				{
					//y is even
					m_Chunks_BR.push_back(chunk);
				}
				else
				{
					//y is odd
					m_Chunks_TR.push_back(chunk);
				}
			}
		}
	}


	void World::UpdateWorld()
	{
		for each (Chunk* chunk in m_Chunks_TL)
		{
			UpdateChunk(chunk);
		}
		//ChunkWorker(this, m_Chunks[{0, 0}]);
		//std::thread thread = std::thread(ChunkWorker(), this, m_Chunks[glm::ivec2(0,0)]);
	}

	void World::UpdateChunk(Chunk* chunk)
	{

	}


	//Helper to get a chunk from a world pixel position
	glm::ivec2 World::WorldToChunk(const glm::ivec2& worldPos)
	{
		glm::ivec2 result;
		if (worldPos.x < 0)
		{
			result.x = (worldPos.x + 1) / CHUNKSIZE;
			result.x--;
		}
		else result.x = worldPos.x / CHUNKSIZE;
		if (worldPos.y < 0)
		{
			result.y = (worldPos.y + 1) / CHUNKSIZE;
			result.y--;
		}
		else result.y = worldPos.y / CHUNKSIZE;
		return result;
	}

	//Helper to get an index from a world pixel position
	glm::ivec2 World::WorldToIndex(const glm::ivec2& worldPos)
	{
		glm::ivec2 result;
		if (worldPos.x < 0)
		{
			result.x = CHUNKSIZE - (std::abs(worldPos.x) % CHUNKSIZE);
		}
		else result.x = worldPos.x % CHUNKSIZE;
		if (worldPos.y < 0)
		{
			result.y = CHUNKSIZE - (std::abs(worldPos.y) % CHUNKSIZE);
		}
		else result.y = worldPos.y % CHUNKSIZE;
		return result;
	}

}