#pragma once

#include "Element.h"
#include "VectorHash.h"

#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_math.h>
#include <poly2tri.h>
#include <box2d/b2_world.h>

namespace Pyxis
{
	/// <summary>
	/// this is based on how many pixels represent 1 unit.
	/// if an object had a velocity of -10, it would fall 10 pixels at
	/// a PPU of 1, whereas it would fall 100 pixels at a PPU of 10;
	/// 
	/// basically, what matters is this is the value of an average
	/// width / size pixel body. like a 1x1 box is this wide/tall
	/// 
	/// I have it at 16 cause a 16x16 box is a box to me!
	/// </summary>
	static const float PPU = 16.0f; // pixels per unit for box2d sim

	struct RigidBodyElement
	{
		RigidBodyElement() {}
		RigidBodyElement(Element e)
			: element(e)
		{}
		RigidBodyElement(Element e, glm::ivec2 worldPosition)
			: element(e), worldPos(worldPosition)
		{}
		Element element = Element();
		bool hidden = false;
		glm::ivec2 worldPos = { 0,0 };
	};

	struct PixelBodyData
	{
		b2Vec2 linearVelocity;
		float angularVelocity;
		float angle;
		b2Vec2 position;
	};

	class PixelRigidBody
	{
	public:
		PixelRigidBody(uint64_t uuid, const glm::ivec2& size, std::unordered_map<glm::ivec2, RigidBodyElement, HashVector> elements, b2BodyType type = b2_dynamicBody, b2World* world = nullptr);
		~PixelRigidBody();

		bool CreateB2Body(b2World* world);
		std::vector<PixelRigidBody*> RecreateB2Body(unsigned int randSeed, b2World* world);

		void SetPixelPosition(const glm::ivec2& position);
		void SetTransform(const glm::vec2& position, float rotation);
		void SetPosition(const glm::vec2& position);
		void SetRotation(float rotation);
		void SetAngularVelocity(float velocity);
		void SetLinearVelocity(const b2Vec2& velocity);

		glm::ivec2 GetPixelPosition();

	public:
		//algorithms for creating a box2d body
		std::vector<p2t::Point> GetContourPoints();
		std::vector<p2t::Point> SimplifyPoints(const std::vector<p2t::Point>& contourVector, int startIndex, int endIndex, float threshold);
		int GetMarchingSquareCase(glm::ivec2 position);

		std::vector<glm::ivec2> PullContinuousElements(std::unordered_map<glm::ivec2, RigidBodyElement, HashVector>& elements);
		void FloodPull(glm::ivec2 pos, std::vector<glm::ivec2>& result, std::unordered_map<glm::ivec2, RigidBodyElement, HashVector>& elements);

	public:
		int m_Width = 0;
		int m_Height = 0;

		/// <summary>
		/// m_Elements holds the LOCAL positions based on the origin!
		/// to index like an array, add the origin back
		/// </summary>
		std::unordered_map<glm::ivec2, RigidBodyElement, HashVector> m_Elements;
		glm::ivec2 m_Origin = {0,0};

		uint64_t m_ID = 0;

		b2BodyType m_Type = b2_dynamicBody;
		b2Body* m_B2Body = nullptr;
		bool m_InWorld = false;

		//std::vector<p2t::Point> m_ContourVector;
	};

	class Player : public PixelRigidBody
	{
	public:
		Player(uint64_t uuid, const glm::ivec2& size, std::unordered_map<glm::ivec2, RigidBodyElement, HashVector> elements, b2BodyType type, b2World* world);
		~Player();

	public:
		void SetPosition(glm::vec2 position);
	};
}