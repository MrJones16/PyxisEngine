#pragma once

#include "Element.h"

#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_math.h>
#include <poly2tri/poly2tri.h>

namespace Pyxis
{
	class PixelRigidBody
	{
	public:

		PixelRigidBody();
		~PixelRigidBody();

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