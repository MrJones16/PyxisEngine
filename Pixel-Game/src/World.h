#pragma once

#include "Chunk.h"

namespace Pyxis
{
	class HashVector
	{
		size_t operator()(const glm::ivec2& vector)
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

		void AddChunk(const glm::ivec2& chunkPos);

		void UpdateWorld();
		void UpdateChunk(Chunk* chunk);



		//helper functions
		glm::ivec2 WorldToChunk(const glm::ivec2& worldPos);
		glm::ivec2 WorldToIndex(const glm::ivec2& worldPos);

		//std::vector<Chunk*> m_Chunks;
		std::unordered_map<glm::ivec2, Chunk*, HashVector> m_Chunks;

		std::vector<Chunk*> m_Chunks_TL;
		std::vector<Chunk*> m_Chunks_TR;
		std::vector<Chunk*> m_Chunks_BL;
		std::vector<Chunk*> m_Chunks_BR;

		std::vector<std::thread> m_Threads;

		bool m_UpdateBit = false;

	};
}