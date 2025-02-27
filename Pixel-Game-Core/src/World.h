#pragma once

#include "Chunk.h"
#include "Pyxis/FastNoiseLite/FastNoiseLite.h"
#include "VectorHash.h"

//all box2d things
#include <box2d/b2_world.h>
#include <box2d/b2_fixture.h>
//#include "PixelRigidBody.h"
#include <box2d/b2_math.h>


//networking game messages / input actions
#include "PixelNetworking.h"


namespace Pyxis
{

	class WakeUpQueryCallback : public b2QueryCallback
	{
	public:
		bool ReportFixture(b2Fixture* fixture)
		{
			b2Body* body = fixture->GetBody();
			//PX_TRACE("Found an object in the update region");
			body->SetAwake(true);
			// Return true to continue the query.
			return true;
		}
	};

	class FoundDynamicBodyQuery : public b2QueryCallback
	{
	public:
		bool& m_FoundBool;
		FoundDynamicBodyQuery(bool& foundBool) : m_FoundBool(foundBool) {};
		bool ReportFixture(b2Fixture* fixture)
		{
			b2Body* body = fixture->GetBody();
			if (body->GetType() == b2_dynamicBody)
			{
				//PX_TRACE("Dynamic body in the area.");
				m_FoundBool = true;

				//stop searching
				return false;
			}
			
			//continue searching.
			return true;
		}
	};

	class World
	{
	public:

		World(std::string assetPath = "assets", int seed = 1337);
		void Initialize(int worldSeed);

		bool LoadXMLElementData(std::string assetPath);

		enum class GameDataMsgType : uint8_t { pixelbody, chunk };
		void DownloadWorldInit(Network::Message& msg);
		void DownloadWorld(Network::Message& msg);
		void GetGameDataInit(Network::Message& msg);
		void GetGameData(std::vector<Network::Message>& messages);
		//void GetWorldData(Network::Message& msg);

		~World();

		Chunk* AddChunk(const glm::ivec2& chunkPos);
		Chunk* GetChunk(const glm::ivec2& chunkPos);
		void GenerateChunk(Chunk* chunk);

		Element& GetElement(const glm::ivec2& pixelPos);
		void SetElement(const glm::ivec2& pixelPos, const Element& element);
		void SetElementWithoutDirtyRectUpdate(const glm::ivec2& pixelPos, const Element& element);

		void PaintBrushElement(glm::ivec2 pixelPos, uint32_t elementID, BrushType brush, uint8_t brushSize);

		void UpdateWorld();
		void UpdateTextures();
		void UpdateChunk(Chunk* chunk);
		void UpdateChunkDirtyRect(int x, int y, Chunk* chunk);

		void Clear();
		void RenderWorld();

		void TestStaticColliders()
		{
			for (auto& [key, chunk] : m_Chunks)
			{
				chunk->GenerateStaticCollider();
			}
		}

	public:
		void ResetBox2D();
		//PixelRigidBody* CreatePixelRigidBody(uint64_t uuid, const glm::ivec2& size, Element* ElementArray, b2BodyType type = b2_dynamicBody);
		//void PutPixelBodyInWorld(PixelRigidBody& body);

	public:
		//moved to game layer and server respectively
		//void HandleTickClosure(MergedTickClosure& tc);
		//Player* CreatePlayer(uint64_t playerID, glm::ivec2 position);


		//helper functions

		
		void SeedRandom(int xPos, int yPos);
		static const bool IsInBounds(int x, int y);
		glm::ivec2 WorldToPixel(const glm::vec2& worldPos);
		glm::ivec2 PixelToChunk(const glm::ivec2& pixelPos);
		glm::ivec2 PixelToIndex(const glm::ivec2& pixelPos);

		//
		std::unordered_map<glm::ivec2, Chunk*, HashVector> m_Chunks;

		//keeping track of theads to join them
		//std::vector<std::thread> m_Threads;
		

		//extra data needed
		bool m_Running = true;				// Needs to be synchronized
		bool m_UpdateBit = false;			// Needs to be synchronized
		bool m_Error = false;

		//temps
		bool m_DebugDrawColliders = false;

		//server mode ignores textures!
		bool m_ServerMode = false;

		//world settings, for generation and gameplay?
		int m_WorldSeed = 1337;				// Needs to be synchronized
		uint64_t m_SimulationTick = 0;	// Needs to be synchronized
		FastNoiseLite m_HeightNoise;
		FastNoiseLite m_CaveNoise;

	};
}
