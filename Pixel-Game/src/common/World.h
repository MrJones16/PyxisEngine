#pragma once

#include "Chunk.h"
#include "Pyxis/FastNoiseLite/FastNoiseLite.h"

//all box2d things
#include <box2d/b2_world.h>
#include "PixelRigidBody.h"
#include <box2d/b2_math.h>

namespace Pyxis
{
	enum class InputAction : uint32_t
	{
		Add_Player,
		Remove_Player,
		PauseGame, ResumeGame,
		TransformRigidBody,
		Input_Move,
		Input_Place,
		Input_StepSimulation,
	};

	//class InputActionData : 

	class TickClosure
	{
	public:
		//stored first, all the data for an action, then what kind of action it is
		std::vector<uint8_t> m_Data;
		//the number of input actions to apply
		uint32_t m_InputActionCount = 0;

	public:
		template<typename... Arguments>
		void AddInputAction(InputAction action, Arguments ... args)
		{
			AddInputActionData(args...);
			*(this) << action;
			m_InputActionCount++;
		}

		/// <summary>
		/// Pushes any "POD" like data into the message buffer
		/// </summary>
		template<typename DataType>
		friend TickClosure& operator << (TickClosure& tc, const DataType& data)
		{
			//chack that the type of the data being pushed is trivially copyable
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into message body vector");

			//cache current vector size, as this is where we will add the data
			size_t i = tc.m_Data.size();

			//resize the vector by the size of the data being pushed
			tc.m_Data.resize(tc.m_Data.size() + sizeof(DataType));

			//physically copy the data into the newly allocated vector space
			std::memcpy(tc.m_Data.data() + i, &data, sizeof(DataType));

			//return message so it can be "chained"
			return tc;
		}

		template<typename DataType>
		friend TickClosure& operator >> (TickClosure& tc, DataType& data)
		{
			//chack that the data is easily copyable
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to have been in a body vector");

			//cache the location towards the end of the vector where the pulled data starts
			size_t dataSize = sizeof(DataType);
			size_t i = tc.m_Data.size() - dataSize;

			//physically copy the memory to the DataType variable
			std::memcpy(&data, tc.m_Data.data() + i, sizeof(DataType));

			//shrink the vector;
			tc.m_Data.resize(i);

			//return the target message so it can be "chained"
			return tc;
		}

	private:
		template<typename DataType>
		void AddInputActionData(DataType data)
		{
			*(this) << data;
		}

		template<typename DataType, typename... Arguments>
		void AddInputActionData(DataType data, Arguments ... args)
		{
			*(this) << data;
			AddInputActionData(args...);
		}

	};

	class MergedTickClosure : public TickClosure
	{
	public:
		//the tick this action should occur
		uint64_t m_Tick = 0;

		uint16_t m_TickClosureCount = 0;
		inline void AddTickClosure(TickClosure& tickClosure)
		{
			size_t i = m_Data.size(); // get current end point of data
			m_Data.resize(m_Data.size() + tickClosure.m_Data.size());
			memcpy(m_Data.data() + i, tickClosure.m_Data.data(), tickClosure.m_Data.size());
			m_InputActionCount += tickClosure.m_InputActionCount;
			m_TickClosureCount++;
		}
	};

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

	
	static const int CHUNKSIZE = 512;
	static const float PPU = 15.0f; // pixels per unit for box2d sim


	class World
	{
	public:

		World(std::string assetPath = "assets");
		bool LoadElementData(std::string assetPath);
		void BuildReactionTable();
		~World();

		void AddChunk(const glm::ivec2& chunkPos);
		Chunk* GetChunk(const glm::ivec2& chunkPos);
		void GenerateChunk(Chunk* chunk);

		void UpdateWorld();
		void UpdateTextures();
		void UpdateChunkBucket(Chunk* chunk, int bucketX, int bucketY);
		void UpdateChunkDirtyRect(int x, int y, Chunk* chunk);

		void Clear();
		void RenderWorld();

	public:
		Element GetElementByName(std::string elementName, int x, int y);
		Element& GetElement(const glm::ivec2& pixelPos);
		void SetElement(const glm::ivec2& pixelPos, const Element& element);
		void CreatePixelRigidBody(uint32_t uuid, const glm::ivec2& size, Element* ElementArray, glm::ivec2 pixelPos, b2BodyType type = b2_dynamicBody);

	public:
		void HandleTickClosure(MergedTickClosure& tc);
		Player* CreatePlayer(uint32_t playerID, glm::vec2 position);


		//helper functions
		static const bool IsInBounds(int x, int y);
		glm::ivec2 WorldToPixel(const glm::vec2& worldPos);
		glm::ivec2 PixelToChunk(const glm::ivec2& pixelPos);
		glm::ivec2 PixelToIndex(const glm::ivec2& pixelPos);

		bool StringContainsTag(const std::string& string);
		std::string TagFromString(const std::string& stringWithTag);
		std::string ReplaceTagInString(const std::string& stringToFill, const std::string& name);


		b2World* m_Box2DWorld;
		std::unordered_map<uint32_t, PixelRigidBody*> m_PixelBodyMap;
		//std::vector<PixelRigidBody*> m_PixelBodies;

		//
		std::unordered_map<glm::ivec2, Chunk*, HashVector> m_Chunks;

		//keeping track of theads to join them
		//std::vector<std::thread> m_Threads;

		//keeping track of element data, tags, reactions, ect
		int m_TotalElements = 0;
		std::vector<ElementData> m_ElementData;
		std::vector<Reaction> m_Reactions;
		std::unordered_map<std::string, std::vector<uint32_t>> m_TagElements;
		std::unordered_map<std::string, uint32_t> m_ElementIDs;
		std::vector<std::unordered_map<uint32_t, ReactionResult>> m_ReactionLookup;

		//extra data needed
		bool m_Running = true;
		bool m_UpdateBit = false;
		bool m_Error = false;
		//server mode ignores textures!
		bool m_ServerMode = false;

		//world settings, for generation and gameplay?
		int m_WorldSeed = 1337;
		FastNoiseLite m_HeightNoise;
		FastNoiseLite m_CaveNoise;

	};
}