#pragma once

#include "Element.h"

#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_math.h>
#include <poly2tri/poly2tri.h>

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


	class PixelRigidBody
	{
	public:

		PixelRigidBody();
		~PixelRigidBody();

		void SetPixelPosition(const glm::ivec2& position);
		void SetPosition(const glm::vec2& position);
		void SetRotation(float rotation);
		void SetAngularVelocity(float velocity);
		void SetLinearVelocity(const b2Vec2& velocity);

	public:
		//algorithms for creating a box2d body
		std::vector<p2t::Point> GetContourPoints();
		std::vector<p2t::Point> PixelRigidBody::SimplifyPoints(const std::vector<p2t::Point>& contourVector, int startIndex, int endIndex, float threshold);
		int GetMarchingSquareCase(glm::ivec2 position);

	public:
		int m_Width;
		int m_Height;

		Element* m_ElementArray;
		glm::ivec2 m_Origin;

		b2Body* m_B2Body;

		//std::vector<p2t::Point> m_ContourVector;
	};

	class Player : public PixelRigidBody
	{
	public:
		Player();
		~Player();

	public:
		void SetPosition(glm::vec2 position);
	};
}