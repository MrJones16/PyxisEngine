#include "Chunk.h"
#include "World.h"
#include "Pyxis/Renderer/Renderer2D.h"

namespace Pyxis
{
	Chunk::Chunk(glm::ivec2 chunkPos)
	{
		m_ChunkPos = chunkPos;
		for (int i = 0; i < CHUNKSIZE * CHUNKSIZE; i++)
		{

			m_Elements[i] = Element();
		}

		//reset dirty rects
		for (int bx = 0; bx < BUCKETSWIDTH; bx++)
		{
			for (int by = 0; by < BUCKETSWIDTH; by++)
			{
				auto& minmax = m_DirtyRects[bx + by * BUCKETSWIDTH];
				minmax.second.x = bx * BUCKETSIZE;
				minmax.second.y = by * BUCKETSIZE;
				minmax.first.x = (bx * BUCKETSIZE) + BUCKETSIZE - 1;
				minmax.first.y = (by * BUCKETSIZE) + BUCKETSIZE - 1;
			}
		}


		m_Texture = Texture2D::Create(CHUNKSIZE, CHUNKSIZE);
		//set texture to fill color
		std::fill(m_PixelBuffer, m_PixelBuffer + (CHUNKSIZE * CHUNKSIZE), 0xFF000000);
		m_Texture->SetData(m_PixelBuffer, sizeof(m_PixelBuffer));

	}


	void Chunk::Clear()
	{
		for (int i = 0; i < CHUNKSIZE * CHUNKSIZE; i++)
		{
			m_Elements[i] = Element();
		}

		//reset dirty rects
		for (int bx = 0; bx < BUCKETSWIDTH; bx++)
		{
			for (int by = 0; by < BUCKETSWIDTH; by++)
			{
				auto& minmax = m_DirtyRects[bx + by * BUCKETSWIDTH];
				minmax.first.x = bx * BUCKETSIZE;
				minmax.first.y = by * BUCKETSIZE;
				minmax.second.x = (bx * BUCKETSIZE) + BUCKETSIZE - 1;
				minmax.second.y = (by * BUCKETSIZE) + BUCKETSIZE - 1;
			}
		}
		
		UpdateTexture();
		
	}

	Element Chunk::GetElement(int x, int y)
	{
		return m_Elements[x + y * CHUNKSIZE];
	}

	void Chunk::SetElement(int x, int y, const Element& element)
	{
		//PX_TRACE("Element Set!");
		m_Elements[x + y * CHUNKSIZE] = element;
	}

	/// <summary>
	/// updates one of the sub-dirty rects based on the input. 0->chunksize-1
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	void Chunk::UpdateDirtyRect(int x, int y)
	{
		//this needs to update the surrounding buckets if the coord is on the corresponding edge
		//ex: if on top edge, also update that bucket using this x/y. this is fine since it clamps to
		//bucket size anyway
		auto& minmax = m_DirtyRects[(x / BUCKETSIZE) + (y / BUCKETSIZE) * BUCKETSWIDTH];
		//update minimums
		if (x < minmax.first.x + m_DirtyRectBorderWidth) minmax.first.x = x - m_DirtyRectBorderWidth;
		if (y < minmax.first.y + m_DirtyRectBorderWidth) minmax.first.y = y - m_DirtyRectBorderWidth;
		//update maximums
		if (x > minmax.second.x - m_DirtyRectBorderWidth) minmax.second.x = x + m_DirtyRectBorderWidth;
		if (y > minmax.second.y - m_DirtyRectBorderWidth) minmax.second.y = y + m_DirtyRectBorderWidth;
	}

	void Chunk::ResetDirtyRects()
	{
		//reset dirty rects
		for (int bx = 0; bx < BUCKETSWIDTH; bx++)
		{
			for (int by = 0; by < BUCKETSWIDTH; by++)
			{
				auto& minmax = m_DirtyRects[bx + by * BUCKETSWIDTH];
				minmax.first.x = (bx + 1) * BUCKETSIZE;
				minmax.first.y = (by + 1) * BUCKETSIZE;
				minmax.second.x = (bx * BUCKETSIZE) - 1;
				minmax.second.y = (by * BUCKETSIZE) - 1;
			}
		}
	}


	void Chunk::UpdateTexture()
	{
		bool debug = true;
		if (debug)
		{
			for (int x = 0; x < CHUNKSIZE; x++)
			{
				for (int y = 0; y < CHUNKSIZE; y++)
				{
					auto& minmax = m_DirtyRects[(x / BUCKETSIZE) + (y / BUCKETSIZE) * BUCKETSWIDTH];
					if (x == minmax.first.x || x == minmax.second.x)
					{
						if (y >= minmax.first.y && y <= minmax.second.y)
							m_PixelBuffer[x + y * CHUNKSIZE] = 0xFF77777777;
						else
							m_PixelBuffer[x + y * CHUNKSIZE] = m_Elements[x + y * CHUNKSIZE].m_Color;
						//draw gray for border
					}
					else if (y == minmax.first.y || y == minmax.second.y)
					{
						if (y >= minmax.first.y && y <= minmax.second.y)
							m_PixelBuffer[x + y * CHUNKSIZE] = 0xFF77777777;
						else
							m_PixelBuffer[x + y * CHUNKSIZE] = m_Elements[x + y * CHUNKSIZE].m_Color;
					}
					else m_PixelBuffer[x + y * CHUNKSIZE] = m_Elements[x + y * CHUNKSIZE].m_Color;
				}
			}
			m_Texture->SetData(m_PixelBuffer, sizeof(m_PixelBuffer));
		}
		else
		{
			bool dataChanged = false;

			for (int bx = 0; bx < BUCKETSWIDTH; bx++)
			{
				for (int by = 0; by < BUCKETSWIDTH; by++)
				{
					auto& minmax = m_DirtyRects[bx + by * BUCKETSWIDTH];

					//if min.x <= max.x
					if (minmax.first.x <= minmax.second.x)
					{
						dataChanged = true;
						for (int x = std::max(minmax.first.x, bx * BUCKETSIZE); x <= std::min(minmax.second.x, ((bx + 1) * BUCKETSIZE) - 1); x++)
						{
							//loop from min x to max x
							for (int y = std::max(minmax.first.y, by * BUCKETSIZE); y <= std::min(minmax.second.y, ((by + 1) * BUCKETSIZE) - 1); y++)
							{
								m_PixelBuffer[x + y * CHUNKSIZE] = m_Elements[x + y * CHUNKSIZE].m_Color;
							}
						}
					}
				}
			}
			if (dataChanged)
			{
				//PX_TRACE("Uploading texture");
				this;
				m_Texture->SetData(m_PixelBuffer, sizeof(m_PixelBuffer));
			}
		}

		
	}

	void Chunk::UpdateWholeTexture()
	{
		//set data first, then update the pixels. this allows you to draw over the texture without interupting the actual
		//elements in the map.
		
		for (int x = 0; x < CHUNKSIZE; x++)
		{
			for (int y = 0; y < CHUNKSIZE; y++)
			{
				m_PixelBuffer[x + y * CHUNKSIZE] = m_Elements[x + y * CHUNKSIZE].m_Color;
			}
		}
		m_Texture->SetData(m_PixelBuffer, sizeof(m_PixelBuffer));

	}

	void Chunk::RenderChunk()
	{
		
	}
}