#pragma once

#include "Chunk.h"
#include "Pyxis/FastNoiseLite/FastNoiseLite.h"

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
		void GenerateChunk(Chunk* chunk);

		void UpdateWorld();
		void UpdateChunk(Chunk* chunk);
		void UpdateChunkBucket(Chunk* chunk, int bucketX, int bucketY);
		void UpdateChunkDirtyRect(int x, int y, Chunk* chunk);

		void Clear();

		Element GetElementByName(std::string elementName);
		void SetElement(const glm::ivec2& pixelPos, const Element& element);

		void RenderWorld();



		//helper functions
		static const bool IsInBounds(int x, int y);
		glm::ivec2 WorldToPixel(const glm::vec2& worldPos);
		glm::ivec2 PixelToChunk(const glm::ivec2& pixelPos);
		glm::ivec2 PixelToIndex(const glm::ivec2& pixelPos);

		bool StringContainsTag(const std::string& string);
		std::string TagFromString(const std::string& stringWithTag);
		std::string ReplaceTagInString(const std::string& stringToFill, const std::string& name);


		//world settings, for generation and gameplay?
		int m_WorldSeed = 1337;
		FastNoiseLite m_HeightNoise;
		FastNoiseLite m_CaveNoise;

		//
		std::unordered_map<glm::ivec2, Chunk*, HashVector> m_Chunks;

		//keeping track of theads to join them
		std::vector<std::thread> m_Threads;

		//keeping track of element data, tags, reactions, ect
		int m_TotalElements = 0;
		std::vector<ElementData> m_ElementData;
		std::vector<Reaction> m_Reactions;
		std::unordered_map<std::string, std::vector<uint32_t>> m_TagElements;
		std::unordered_map<std::string, uint32_t> m_ElementIDs;
		std::vector<std::unordered_map<uint32_t, ReactionResult>> m_ReactionLookup;

		//extra data needed
		bool m_UpdateBit = false;
		bool m_Error = false;

	};
}