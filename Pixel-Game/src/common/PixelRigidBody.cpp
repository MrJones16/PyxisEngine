#include "PixelRigidBody.h"

namespace Pyxis
{
	PixelRigidBody::PixelRigidBody()
	{

	}

	PixelRigidBody::PixelRigidBody(uint64_t uuid, const glm::ivec2& size, Element* ElementArray, b2BodyType type, b2World* world)
	{
		m_ID = uuid;
		m_Width = size.x;
		m_Height = size.y;
		m_Type = type;
		m_Origin = { m_Width / 2, m_Height / 2 };
		m_ElementArray = ElementArray;

		//create the base body of the whole pixel body
		b2BodyDef pixelBodyDef;

		//for the scaling of the box world and pixel bodies, every PPU pixels is 1 unit in the world space
		pixelBodyDef.position = { 0,0 };
		pixelBodyDef.type = type;
		
		if (world != nullptr)
		{
			CreateB2Body(world);
		}
	}

	PixelRigidBody::~PixelRigidBody()
	{
		delete[] m_ElementArray;
	}

	void PixelRigidBody::CreateB2Body(b2World* world)
	{

		//create the base body of the whole pixel body
		b2BodyDef pixelBodyDef;

		//for the scaling of the box world and pixel bodies, every PPU pixels is 1 unit in the world space
		pixelBodyDef.position = { 0,0 };
		pixelBodyDef.type = m_Type;
		m_B2Body = world->CreateBody(&pixelBodyDef);


		//BUG ERROR FIX TODO WRONG BROKEN
		// getcontour points is able to have repeating points, which is a no-no for box2d triangles / triangulation
		auto contour = GetContourPoints();
		if (contour.size() == 0)
		{
			world->DestroyBody(m_B2Body);
			delete this;
			return;
		}

		auto contourVector = SimplifyPoints(contour, 0, contour.size() - 1, 1.0f);
		//auto simplified = body->SimplifyPoints(contour);

		//run triangulation algorithm to create the needed triangles/fixtures
		std::vector<p2t::Point*> polyLine;
		for each (auto point in contourVector)
		{
			polyLine.push_back(new p2t::Point(point));
		}

		//create each of the triangles to comprise the body, each being a fixture
		p2t::CDT* cdt = new p2t::CDT(polyLine);
		cdt->Triangulate();
		auto triangles = cdt->GetTriangles();
		for each (auto triangle in triangles)
		{
			b2PolygonShape triangleShape;
			b2Vec2 points[3] = {
				{(float)((triangle->GetPoint(0)->x - m_Origin.x) / PPU), (float)((triangle->GetPoint(0)->y - m_Origin.y) / PPU)},
				{(float)((triangle->GetPoint(1)->x - m_Origin.x) / PPU), (float)((triangle->GetPoint(1)->y - m_Origin.y) / PPU)},
				{(float)((triangle->GetPoint(2)->x - m_Origin.x) / PPU), (float)((triangle->GetPoint(2)->y - m_Origin.y) / PPU)}
			};
			triangleShape.Set(points, 3);
			b2FixtureDef fixtureDef;
			fixtureDef.density = 1;
			fixtureDef.friction = 0.3f;
			fixtureDef.shape = &triangleShape;
			m_B2Body->CreateFixture(&fixtureDef);
		}
	}

	void PixelRigidBody::SetPixelPosition(const glm::ivec2& position)
	{
		m_B2Body->SetTransform({ (float)(position.x) / PPU, (float)(position.y) / PPU }, m_B2Body->GetAngle());
	}
	void PixelRigidBody::SetTransform(const glm::vec2& position, float rotation)
	{
		m_B2Body->SetTransform({ position.x, position.y }, rotation);
	}
	void PixelRigidBody::SetPosition(const glm::vec2& position)
	{
		m_B2Body->SetTransform({ position.x, position.y}, m_B2Body->GetAngle());
	}

	void PixelRigidBody::SetRotation(float rotation)
	{
		b2Vec2 position = m_B2Body->GetPosition();
		m_B2Body->SetTransform(position, rotation);
	}

	void PixelRigidBody::SetAngularVelocity(float velocity)
	{
		m_B2Body->SetAngularVelocity(velocity);
	}

	void PixelRigidBody::SetLinearVelocity(const b2Vec2& velocity)
	{
		m_B2Body->SetLinearVelocity(velocity);
	}
	
	

	/// <summary>
	/// gathers the outline points of the rigid body, and returns a vector of
	/// size 0 if there was a failure
	/// 
	/// currently tries to grab as many diagonals as possible, but could be better to switch the two special cases to be inverted
	/// to ignore diagonals if possible
	/// </summary>
	/// <returns></returns>
	std::vector<p2t::Point> PixelRigidBody::GetContourPoints()
	{
		//run marching squares on element array to find all vertices
		//inspired by https://www.emanueleferonato.com/2013/03/01/using-marching-squares-algorithm-to-trace-the-contour-of-an-image/
		std::vector<p2t::Point> ContourVector;

		glm::ivec2 cursor = { m_Width - 1,m_Height - 1 };
		//scroll through element array until you find an element to start at
		while (m_ElementArray[cursor.x + cursor.y * m_Width].m_ID == 0)
		{
			cursor.x--;
			if (cursor.x < 0)
			{
				cursor.x = m_Width - 1;
				cursor.y--;
				if (cursor.y < 0)
				{
					//ran out of pixels to test for! so the entire array is air...
					return ContourVector;
				}

			}
		}

		//found starting element by scrolling backwards through array
		glm::ivec2 start = cursor;
		glm::ivec2 step = { 1, 0 };
		glm::ivec2 prev = { 1, 0 };

		//moving in a counter-clockwise fashion
		bool closedLoop = false;
		while (!closedLoop)
		{
			int caseValue = GetMarchingSquareCase(cursor);
			switch (caseValue)
			{
			case 1:
			case 5:
			case 13:
				/* going UP with these cases:

							+---+---+   +---+---+   +---+---+
							| 1 |   |   | 1 |   |   | 1 |   |
							+---+---+   +---+---+   +---+---+
							|   |   |   | 4 |   |   | 4 | 8 |
							+---+---+  	+---+---+  	+---+---+

				*/
				step.x = 0;
				step.y = 1;
				break;

				
			case 8:
			case 10:
			case 11:
				/* going DOWN with these cases:

							+---+---+   +---+---+   +---+---+
							|   |   |   |   | 2 |   | 1 | 2 |
							+---+---+   +---+---+   +---+---+
							|   | 8 |   |   | 8 |   |   | 8 |
							+---+---+  	+---+---+  	+---+---+

				*/
				step.x = 0;
				step.y = -1;
				break;

			case 4:
			case 12:
			case 14:
				/* going LEFT with these cases:

							+---+---+   +---+---+   +---+---+
							|   |   |   |   |   |   |   | 2 |
							+---+---+   +---+---+   +---+---+
							| 4 |   |   | 4 | 8 |   | 4 | 8 |
							+---+---+  	+---+---+  	+---+---+

				*/
				step.x = -1;
				step.y = 0;
				break;

			case 2:
			case 3:
			case 7:
				/* going RIGHT with these cases:

							+---+---+   +---+---+   +---+---+
							|   | 2 |   | 1 | 2 |   | 1 | 2 |
							+---+---+   +---+---+   +---+---+
							|   |   |   |   |   |   | 4 |   |
							+---+---+  	+---+---+  	+---+---+

				*/
				step.x = 1;
				step.y = 0;
				break;
				
			case 6:
				/* special saddle point case 1:

				+---+---+
				|   | 2 |
				+---+---+
				| 4 |   |
				+---+---+

				going LEFT if coming from UP
				else going RIGHT

				*/
				if (prev.x == 0 && prev.y == -1) {
					step.x = -1;
					step.y = 0;
				}
				else {
					step.x = 1;
					step.y = 0;
				}
				break;

			case 9:
				/* special saddle point case 2:

					+---+---+
					| 1 |   |
					+---+---+
					|   | 8 |
					+---+---+

					going UP if coming from RIGHT
					else going DOWN

					*/
				if (prev.x == -1 && prev.y == 0) {
					step.x = 0;
					step.y = 1;
				}
				else {
					step.x = 0;
					step.y = -1;
				}
				break;
			}
			// saving contour point
			ContourVector.push_back(p2t::Point(cursor.x, cursor.y));
			// moving onto next point
			cursor += step;
			prev = step;
			//  drawing the line
			// if we returned to the first point visited, the loop has finished
			if (cursor == start) {
				closedLoop = true;
			}
		}
		return ContourVector;
	}


	std::vector<p2t::Point> PixelRigidBody::SimplifyPoints(const std::vector<p2t::Point>& contourVector, int startIndex, int endIndex, float threshold)
	{
		float maxDist = 0.0f;
		int maxIndex = 0;
		float dividend = (contourVector[endIndex].x - contourVector[startIndex].x);
		if (dividend != 0)
		{
			float m = (contourVector[endIndex].y - contourVector[startIndex].y) / dividend;
			float b = contourVector[startIndex].y - m * contourVector[startIndex].x;
			float dividend2 = ((m * m) + 1);
			for (int i = startIndex + 1; i < endIndex; i++)
			{
				float distToLine = std::abs((-m * contourVector[i].x) + contourVector[i].y - b) / std::sqrt(dividend2);
				if (distToLine > maxDist) {
					maxDist = distToLine;
					maxIndex = i;
				}
			}
		}
		else
		{
			for (int i = startIndex + 1; i < endIndex; i++)
			{
				float distToLine = contourVector[i].x - contourVector[startIndex].x;
				if (distToLine > maxDist) {
					maxDist = distToLine;
					maxIndex = i;
				}
			}
		}
				
		if (maxDist > threshold)
		{
			//do another simplify to the left and right, and combine them as the result
			auto result = SimplifyPoints(contourVector, startIndex, maxIndex, threshold);
			auto right = SimplifyPoints(contourVector, maxIndex, endIndex, threshold); 
			for (int i = 1; i < right.size(); i++)
			{
				result.push_back(right[i]);
			}
			return result;
		}

		auto endResult = std::vector<p2t::Point>();
		endResult.push_back(contourVector[startIndex]);
		endResult.push_back(contourVector[endIndex]);
		PX_TRACE("Removed Points");
		return endResult;
	}


	int PixelRigidBody::GetMarchingSquareCase(glm::ivec2 position)
	{
		/*

			checking the 2x2 pixel grid, assigning these values to each pixel, if not transparent

			+---+---+
			| 1 | 2 |
			+---+---+
			| 4 | 8 | <- current pixel (position.x,position.y)
			+---+---+

		*/

		int result = 0;
		if ((position.x) >= 0 && (position.x) < m_Width && (position.y) >= 0 && (position.y) < m_Height)
		{
			if (m_ElementArray[position.x + position.y * m_Width].m_ID != 0) result += 8;
		}
		if ((position.x - 1) >= 0 && (position.x - 1) < m_Width && (position.y) >= 0 && (position.y) < m_Height)
		{
			if (m_ElementArray[(position.x - 1) + (position.y) * m_Width].m_ID != 0) result += 4;
		}
		if ((position.x) >= 0 && (position.x) < m_Width && (position.y + 1) >= 0 && (position.y + 1) < m_Height)
		{
			if (m_ElementArray[position.x + (position.y + 1) * m_Width].m_ID != 0) result += 2;
		}
		if ((position.x - 1) >= 0 && (position.x - 1) < m_Width && (position.y + 1) >= 0 && (position.y + 1) < m_Height)
		{
			if (m_ElementArray[(position.x - 1) + (position.y + 1) * m_Width].m_ID != 0) result += 1;
		}

		return result;
	}


	Player::Player()
	{
	}
	Player::~Player()
	{
	}
	void Player::SetPosition(glm::vec2 position)
	{
		m_B2Body->SetTransform({ position.x, position.y }, m_B2Body->GetAngle());
	}
}