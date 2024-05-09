#pragma once

#include <Pyxis.h>
#include "Element.h"



namespace Pyxis
{
	class Chunk
	{
	public:
		static const int CHUNKSIZE = 512;
		static const int BUCKETSIZE = 64;
		static const int BUCKETS = CHUNKSIZE / BUCKETSIZE;

		Chunk(glm::ivec2 chunkPos);
		~Chunk() = default;

		void Clear();

		void UpdateDirtyRect(int x, int y);

		void UpdateTexture();
		void RenderChunk();


		//Pixel m_PixelsUpdateBuffer

		
		//core chunk elements
		glm::ivec2 m_ChunkPos;
		Element m_Elements[CHUNKSIZE * CHUNKSIZE];

		//buckets for dirty rects
		int m_DirtyRectBorderWidth = 2;
		std::pair<glm::ivec2, glm::ivec2> m_DirtyRects[BUCKETS * BUCKETS];

		bool m_PersistDirtyRect = false;

		//textures and rendering
		Ref<Texture2D> m_Texture;
		uint32_t m_PixelBuffer[CHUNKSIZE * CHUNKSIZE];

	};
}