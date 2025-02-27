#pragma once

#include <Pyxis.h>
#include "Element.h"
#include <box2d/b2_body.h>
#include "VectorHash.h"

static const int CHUNKSIZE = 64;

static const float PPU = 16.0f; // pixels per unit for box2d sim

namespace Pyxis
{
	struct DirtyRect
	{		
		glm::ivec2 min = {0,0};
		glm::ivec2 max = { 0,0 };
	};

	class Chunk
	{
	public:
		inline static bool s_DebugChunks = false;

		Chunk(glm::ivec2 chunkPos);
		~Chunk() = default;

		void Clear();

		Element& GetElement(int x, int y);
		Element& GetElement(const glm::ivec2& index);
		void SetElement(int x, int y, const Element& element);

		void UpdateDirtyRect(int x, int y);
		void ResetDirtyRect();

		void UpdateTexture();
		void UpdateWholeTexture();
		void RenderChunk();

		//whether or not this chunk has a static collider
		bool m_StaticColliderGenerated = false;
		bool m_StaticColliderChanged = true;
		void GenerateStaticCollider();

		void QueuePull(glm::ivec2 startPos, std::unordered_set<glm::ivec2, HashVector>& result, std::unordered_set<glm::ivec2, HashVector>& source);		
		std::vector<b2Vec2> GetContourPoints(const std::unordered_set<glm::ivec2, HashVector>& source);
		int GetMarchingSquareCase(const glm::ivec2& localPosition, const std::unordered_set<glm::ivec2, HashVector>& source);
		std::vector<b2Vec2> SimplifyPoints(const std::vector<b2Vec2>& contourVector, int startIndex, int endIndex, float threshold);

		
		//core chunk elements
		glm::ivec2 m_ChunkPos;
		Element m_Elements[CHUNKSIZE * CHUNKSIZE];

		//buckets for dirty rects
		int m_DirtyRectBorderWidth = 2;
		
		DirtyRect m_DirtyRect;

		bool m_PersistDirtyRect = false;

		//textures and rendering
		Ref<Texture2D> m_Texture;
		uint32_t m_PixelBuffer[CHUNKSIZE * CHUNKSIZE];

		b2Body* m_B2Body;

	};
}