#include "ChunkWorker.h"

namespace Pyxis
{
	void ChunkWorker::operator()(World& world, Chunk* chunk)
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
						if (currElement.m_Updated == world.m_UpdateBit) continue;
						//flip the update bit so we know we updated this element
						currElement.m_Updated = world.m_UpdateBit;

						//switch the behavior based on element type
						switch (currElement.m_Type)
						{
						case ElementType::solid:

							//check below, and move
							if (world.IsInBounds(x, y - 1))
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
}