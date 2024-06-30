#include "PixelRigidBody.h"

namespace Pyxis
{

	PixelRigidBody::PixelRigidBody(uint64_t uuid, const glm::ivec2& size, std::unordered_map<glm::ivec2, RigidBodyElement, HashVector> elements, b2BodyType type, b2World* world)
		: m_ID(uuid), m_Width(size.x), m_Height(size.y), m_Type(type),
		m_Origin(size / 2), m_Elements(elements)
	{

		//create the base body of the whole pixel body
		b2BodyDef pixelBodyDef;

		//for the scaling of the box world and pixel bodies, every PPU pixels is 1 unit in the world space
		pixelBodyDef.position = { 0,0 };
		pixelBodyDef.type = type;
		
		if (world != nullptr)
		{
			if (!CreateB2Body(world))
			{
				PX_ERROR("Failed to create the b2 body for pixel body {0}", m_ID);
				m_B2Body = nullptr;
			}
			
		}
	}

	/// <summary>
	/// this does not remove the m_b2body! you need to call
	/// destroybody on the b2world.
	/// </summary>
	PixelRigidBody::~PixelRigidBody()
	{

	}

	bool PixelRigidBody::CreateB2Body(b2World* world)
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
			return false;
		}
		std::vector<p2t::Point> contourVector;
		if (contour.size() > 20)
		{
			contourVector = SimplifyPoints(contour, 0, contour.size() - 1, 1.0f);
		}
		else
		{
			contourVector = contour;
		}
		
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
				{(float)((triangle->GetPoint(0)->x) / PPU), (float)((triangle->GetPoint(0)->y) / PPU)},
				{(float)((triangle->GetPoint(1)->x) / PPU), (float)((triangle->GetPoint(1)->y) / PPU)},
				{(float)((triangle->GetPoint(2)->x) / PPU), (float)((triangle->GetPoint(2)->y) / PPU)}
			};
			triangleShape.Set(points, 3);
			b2FixtureDef fixtureDef;
			fixtureDef.density = 1;
			fixtureDef.friction = 0.3f;
			fixtureDef.shape = &triangleShape;
			m_B2Body->CreateFixture(&fixtureDef);
		}
		return true;
	}


	/// <summary>
	/// returns a vector of any new pixel rigid bodies that
	/// might have been made from splitting
	/// </summary>
	std::vector<PixelRigidBody*> PixelRigidBody::RecreateB2Body(unsigned int randSeed, b2World* world)
	{
		//same as creating a b2body, but we have to remove the previous one, and 
		// look out for multiple bodies since a split
		// could have occured
		// AND re-configure the local positions / origins...
		//TODO: Re-do the origin and local points...

		//the vector of new bodies made by the split
		std::vector<PixelRigidBody*> result;

		//step 1 make sure we aren't completely erased!
		if (m_Elements.size() == 0)
		{
			//we have been erased, so delete myself!
			world->DestroyBody(m_B2Body);
			m_B2Body = nullptr;
			return result;
		}
		
		//store the momentum data of the body
		PixelBodyData data;
		data.position = m_B2Body->GetPosition();
		data.angle = m_B2Body->GetAngle();
		data.linearVelocity = m_B2Body->GetLinearVelocity();
		data.angularVelocity = m_B2Body->GetAngularVelocity();

		//copy the elements of the main starting body
		std::unordered_map<glm::ivec2, RigidBodyElement, HashVector> elementsCopy = m_Elements;

		//pull the first body out, we will make this the new main body
		std::vector<glm::ivec2> thisLocalSet = PullContinuousElements(elementsCopy);
		std::vector<glm::ivec2> thisWorldSet;
		// we now have a set of all the local points for the main body
		// we will iterate over them, find the min/max, middle, and create a new
		// body for it!
		glm::ivec2 pixelPos = GetPixelPosition();
		
		//get the radius around the world point to scope to ish
		int maxSize = std::max(m_Width, m_Height) * 2;

		//we won't reset the main body until after, because we will
		//use it for the element data!
		std::srand(randSeed);
		while (elementsCopy.size() > 0)
		{
			//there are still other rigid bodies to be created!

			//local set will hold the local indices to the elements map
			std::vector<glm::ivec2> localSet = PullContinuousElements(elementsCopy);
			//world set will hold the world positions, for use after
			std::vector<glm::ivec2> worldSet;
			
			
			
			glm::ivec2 worldMin = pixelPos + maxSize;
			glm::ivec2 worldMax = pixelPos - maxSize;
			for (int i = 0; i < localSet.size(); i++)
			{
				//TODO: test changing the pullcontinuous to give world in the first place?
				worldSet.push_back(m_Elements[localSet[i]].worldPos);
				
				//it is now a world pos!
				if (worldSet[i].x < worldMin.x) worldMin.x = worldSet[i].x;
				if (worldSet[i].y < worldMin.y) worldMin.y = worldSet[i].y;
				if (worldSet[i].x > worldMax.x) worldMax.x = worldSet[i].x;
				if (worldSet[i].y > worldMax.y) worldMax.y = worldSet[i].y;
			}
			//we now have the world positions, and the min/max!
			int width = (worldMax.x - worldMin.x) + 1;
			int height = (worldMax.y - worldMin.y) + 1;
			glm::ivec2 origin = { width / 2, height / 2 };
			//we have to create an entire new rigid body!
			
			uint64_t newID = std::rand() * 0xfacebeef; //random number cause i can
			std::unordered_map<glm::ivec2, RigidBodyElement, HashVector> subElements;
			glm::ivec2 worldCenter = worldMin + origin;
			for (int i = 0; i < localSet.size(); i++)
			{
				subElements[worldSet[i] - worldCenter] = RigidBodyElement(m_Elements[localSet[i]]);
			}
			result.push_back(new PixelRigidBody(newID, glm::ivec2(width, height), subElements, m_Type, world));
			result.back()->SetPixelPosition(worldCenter);
			//TODO: alter velocities based on distance to original center
			//for more realistic slicing
			result.back()->SetLinearVelocity(data.linearVelocity);
			result.back()->SetAngularVelocity(data.angularVelocity);
			//ignore pos and angle as they are reset
		}
		//since we are finished with creating the sub-bodies, finish updating our own

		//set the min max inverted so i can find the new min/max
		glm::ivec2 worldMin = pixelPos + maxSize;
		glm::ivec2 worldMax = pixelPos - maxSize;
		for (int i = 0; i < thisLocalSet.size(); i++)
		{
			//TODO: test changing the pullcontinuous to give world in the first place?
			thisWorldSet.push_back(m_Elements[thisLocalSet[i]].worldPos);

			//it is now a world pos!
			if (thisWorldSet[i].x < worldMin.x) worldMin.x = thisWorldSet[i].x;
			if (thisWorldSet[i].y < worldMin.y) worldMin.y = thisWorldSet[i].y;
			if (thisWorldSet[i].x > worldMax.x) worldMax.x = thisWorldSet[i].x;
			if (thisWorldSet[i].y > worldMax.y) worldMax.y = thisWorldSet[i].y;
		}
		//we now have the world positions, and the min/max!
		m_Width = (worldMax.x - worldMin.x) + 1;
		m_Height = (worldMax.y - worldMin.y) + 1;
		m_Origin = { m_Width / 2, m_Height / 2 };

		std::unordered_map<glm::ivec2, RigidBodyElement, HashVector> newElements;
		glm::ivec2 worldCenter = worldMin + m_Origin;
		for (int i = 0; i < thisLocalSet.size(); i++)
		{
			newElements[thisWorldSet[i] - worldCenter] = RigidBodyElement(m_Elements[thisLocalSet[i]]);
		}
		//finally, erase the old storage, and use the new one!
		m_Elements = newElements;


		
		world->DestroyBody(m_B2Body);
		if (CreateB2Body(world))
		{
			SetPixelPosition(worldCenter);
			m_B2Body->SetLinearVelocity(data.linearVelocity);
			m_B2Body->SetAngularVelocity(data.angularVelocity);
		}
		return result;
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

	glm::ivec2 PixelRigidBody::GetPixelPosition()
	{
		auto pos = m_B2Body->GetPosition();
		return glm::ivec2(pos.x * PPU, pos.y * PPU);
	}
	
	

	/// <summary>
	/// gathers the COUNTERCLOCKWISE outline points of the rigid body, and 
	/// returns a vector of size 0 if there was a failure
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

		glm::ivec2 cursor = glm::ivec2(m_Width,m_Height) - m_Origin; // get top right local pos
		//scroll through element array until you find an element to start at
		while (m_Elements.find(cursor) == m_Elements.end())
		{
			cursor.x--;
			if (cursor.x < -m_Origin.x)
			{
				cursor.x = (m_Width - 1) - m_Origin.x;
				cursor.y--;
				if (cursor.y < -m_Origin.y)
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


	int PixelRigidBody::GetMarchingSquareCase(glm::ivec2 localPosition)
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
		auto it = m_Elements.find(localPosition + glm::ivec2(0, 0));
		if (it != m_Elements.end()) result += 8;

		it = m_Elements.find(localPosition + glm::ivec2(-1, 0));
		if (it != m_Elements.end()) result += 4;

		it = m_Elements.find(localPosition + glm::ivec2(0, 1));
		if (it != m_Elements.end()) result += 2;

		it = m_Elements.find(localPosition + glm::ivec2(-1, 1));
		if (it != m_Elements.end()) result += 1;

		return result;
	}

	/// <summary>
	/// takes a map of local points, floodfills from the first element, and returns what it pulled out
	/// as a vector of the local positions.
	/// 
	/// !REMOVES things from the map!
	/// </summary>
	/// <param name="elements"></param>
	/// <returns></returns>
	std::vector<glm::ivec2> PixelRigidBody::PullContinuousElements(std::unordered_map<glm::ivec2, RigidBodyElement, HashVector>& elements)
	{
		std::vector<glm::ivec2> result;
		FloodPull(elements.begin()->first, result, elements);
		return result;
	}

	void PixelRigidBody::FloodPull(glm::ivec2 pos, std::vector<glm::ivec2>& result, std::unordered_map<glm::ivec2, RigidBodyElement, HashVector>& elements)
	{
		if (elements.find(pos) != elements.end())
		{
			//there is an element here, so add it and recursively call neighbors
			result.push_back(pos);
			elements.erase(pos);
			FloodPull(pos + glm::ivec2(1, 0), result, elements);
			FloodPull(pos + glm::ivec2(-1, 0), result, elements);
			FloodPull(pos + glm::ivec2(0, 1), result, elements);
			FloodPull(pos + glm::ivec2(0, -1), result, elements);
		}
	}


	Player::Player(uint64_t uuid, const glm::ivec2& size, std::unordered_map<glm::ivec2, RigidBodyElement, HashVector> elements, b2BodyType type, b2World* world)
		: PixelRigidBody(uuid, size, elements, type, world)
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