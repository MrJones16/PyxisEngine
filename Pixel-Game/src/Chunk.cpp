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
		for (int bx = 0; bx < BUCKETS; bx++)
		{
			for (int by = 0; by < BUCKETS; by++)
			{
				auto& minmax = m_DirtyRects[bx + by * BUCKETS];
				minmax.first.x = bx * BUCKETSIZE;
				minmax.first.y = by * BUCKETSIZE;
				minmax.second.x = (bx * BUCKETSIZE) + BUCKETSIZE - 1;
				minmax.second.y = (by * BUCKETSIZE) + BUCKETSIZE - 1;
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
		for (int bx = 0; bx < BUCKETS; bx++)
		{
			for (int by = 0; by < BUCKETS; by++)
			{
				auto& minmax = m_DirtyRects[bx + by * BUCKETS];
				minmax.first.x = bx * BUCKETSIZE;
				minmax.first.y = by * BUCKETSIZE;
				minmax.second.x = (bx * BUCKETSIZE) + BUCKETSIZE - 1;
				minmax.second.y = (by * BUCKETSIZE) + BUCKETSIZE - 1;
			}
		}
		
		UpdateTexture();
	}

	/// <summary>
	/// updates one of the sub-dirty rects based on the input. 0->chunksize-1
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	void Chunk::UpdateDirtyRect(int x, int y)
	{
		auto& minmax = m_DirtyRects[(x / BUCKETSIZE) + (y / BUCKETSIZE) * BUCKETS];
		//update minimums
		if (x < minmax.first.x) minmax.first.x = x;
		if (y < minmax.first.y) minmax.first.y = y;
		//update maximums
		if (x > minmax.second.x) minmax.second.x = x;
		if (y > minmax.second.y) minmax.second.y = y;

	}


	void Chunk::UpdateTexture()
	{

		//add border to dirty rect
		//glm::ivec2 m_DirtyRectMinTemp = { std::max(0, m_DirtyRectMin.x - m_DirtyRectBorderWidth), std::max(0, m_DirtyRectMin.y - m_DirtyRectBorderWidth) };
		//glm::ivec2 m_DirtyRectMaxTemp = { std::min(CHUNKSIZE - 1, m_DirtyRectMax.x + m_DirtyRectBorderWidth), std::min(CHUNKSIZE - 1, m_DirtyRectMax.y + m_DirtyRectBorderWidth) };


		//dont clear image, just redraw updating section
		//std::fill(m_PixelBuffer, m_PixelBuffer + (CHUNKSIZE * CHUNKSIZE), 0xFF000000);

		bool dataChanged = false;

		for (int bx = 0; bx < BUCKETS; bx++)
		{
			for (int by = 0; by < BUCKETS; by++)
			{
				auto& minmax = m_DirtyRects[bx + by * BUCKETS];

				//if min.x <= max.x
				if (minmax.first.x <= minmax.second.x)
				{
					dataChanged = true;
					for (int x = minmax.first.x; x < minmax.second.x; x++)
					{
						//loop from min x to max x
						for (int y = minmax.first.y; y < minmax.second.y; y++)
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
			m_Texture->SetData(m_PixelBuffer, sizeof(m_PixelBuffer));
		}

		//bool debug = false;
		//if (debug)
		//{
		//	for (int x = 0; x < CHUNKSIZE; x++)
		//	{
		//		for (int y = 0; y < CHUNKSIZE; y++)
		//		{
		//			if (m_Elements[x + y * CHUNKSIZE] != nullptr)
		//			{
		//				m_PixelBuffer[x + y * CHUNKSIZE] = m_Elements[x + y * CHUNKSIZE]->m_Color;
		//			}
		//			else m_PixelBuffer[x + y * CHUNKSIZE] = 0xFF000000;
		//		}
		//	}
		//	//draw each edge
		//	for (int x = m_DirtyRectMinTemp.x; x <= m_DirtyRectMaxTemp.x; x++)
		//	{
		//		//top and bottom
		//		m_PixelBuffer[x + m_DirtyRectMinTemp.y * CHUNKSIZE] += 0x99222222;
		//		m_PixelBuffer[x + m_DirtyRectMaxTemp.y * CHUNKSIZE] += 0x99222222;
		//	}
		//	for (int y = m_DirtyRectMinTemp.y; y <= m_DirtyRectMaxTemp.y; y++)
		//	{
		//		//left and right
		//		m_PixelBuffer[m_DirtyRectMaxTemp.x + y * CHUNKSIZE] += 0x99222222;
		//		m_PixelBuffer[m_DirtyRectMinTemp.x + y * CHUNKSIZE] += 0x99222222;
		//	}
		//}
		//else
		//{
		//	for (int x = m_DirtyRectMinTemp.x; x <= m_DirtyRectMaxTemp.x; x++)
		//	{
		//		for (int y = m_DirtyRectMinTemp.y; y <= m_DirtyRectMaxTemp.y; y++)
		//		{
		//			if (m_Elements[x + y * CHUNKSIZE] != nullptr)
		//			{
		//				m_PixelBuffer[x + y * CHUNKSIZE] = m_Elements[x + y * CHUNKSIZE]->m_Color;
		//			}
		//			else m_PixelBuffer[x + y * CHUNKSIZE] = 0xFF000000;
		//		}
		//	}
		//}
		
	}

	void Chunk::RenderChunk()
	{
		
	}
}