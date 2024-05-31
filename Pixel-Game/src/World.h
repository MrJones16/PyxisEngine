#pragma once

#include "Chunk.h"

namespace Pyxis
{
	/*template<>
	struct std::hash<glm::ivec2>
	{
		std::size_t operator()(const glm::ivec2& v) const
		{

		}
	};*/

	class HashVector
	{
	public:
		size_t operator()(glm::ivec2 vector) const
		{
			size_t seed = 0;
			seed = vector.x + 0xae3779b9 + (seed << 6) + (seed >> 2);
			seed ^= vector.y + 0x87d81ab8 + (seed << 7) + (seed >> 3);
			return seed;
		}
	};

	class World
	{
	public:
		static const int CHUNKSIZE = 512;

		World();
		bool LoadElementData();
		void BuildReactionTable();
		~World();

		void AddChunk(const glm::ivec2& chunkPos);
		Chunk* GetChunk(const glm::ivec2& chunkPos);

		void UpdateWorld();
		void UpdateChunk(Chunk* chunk);
		void UpdateChunkBucket(Chunk* chunk, int bucketX, int bucketY);
		void UpdateChunkDirtyRect(int x, int y, Chunk* chunk);

		void SetElement(const glm::ivec2& pixelPos, const Element& element);

		void RenderWorld();



		//helper functions
		static const bool IsInBounds(int x, int y);
		glm::ivec2 PixelToChunk(const glm::ivec2& pixelPos);
		glm::ivec2 PixelToIndex(const glm::ivec2& pixelPos);

		//std::vector<Chunk*> m_Chunks;
		std::unordered_map<glm::ivec2, Chunk*, HashVector> m_Chunks;

		/*std::vector<Chunk*> m_Chunks_TL;
		std::vector<Chunk*> m_Chunks_TR;
		std::vector<Chunk*> m_Chunks_BL;
		std::vector<Chunk*> m_Chunks_BR;*/

		std::vector<std::thread> m_Threads;

		int m_TotalElements = 0;
		std::vector<ElementData> m_ElementData;
		std::unordered_map<std::string, uint32_t> m_ElementIDs;
		std::vector<std::unordered_map<uint32_t, ReactionResult>> m_ReactionLookup;
		 
		
		

		bool m_UpdateBit = false;
		bool m_Error = false;

	};
}