#pragma once

#include "Element.h"

#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_math.h>

namespace Pyxis
{
	class PixelRigidBody
	{
	public:

		PixelRigidBody();

		~PixelRigidBody();

		std::vector<glm::ivec2> GetContourPoints();

		std::vector<glm::ivec2> PixelRigidBody::SimplifyPoints(const std::vector<glm::ivec2>& contourVector, int startIndex, int endIndex, float threshold);

		int GetMarchingSquareCase(glm::ivec2 position);

		Element* m_ElementArray;
		std::vector<glm::ivec2> m_ContourVector;
		int m_Width;
		int m_Height;
		glm::ivec2 m_Origin;

		b2Body* m_B2Body;
	};
}