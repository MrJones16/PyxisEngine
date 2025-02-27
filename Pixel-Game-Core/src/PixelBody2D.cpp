#include "PixelBody2D.h"
#include <Pyxis/Game/Physics2D.h>

namespace Pyxis
{

	PixelBody2D::PixelBody2D(const std::string& name, b2BodyType type, World* world, std::vector<PixelBodyElement>& elements, bool CreatedFromSplit) : RigidBody2D(name, type),
		m_PXWorld(world)
	{
		//we need to convert the vector of elements to local positions to store them

		//find the width & height of the set of elements
		int minX = INT_MAX, maxX = INT_MIN, minY = INT_MAX, maxY = INT_MIN;
		for (auto& pbe : elements)
		{
			minX = std::min(pbe.worldPos.x, minX);
			maxX = std::max(pbe.worldPos.x, maxX);
			minY = std::min(pbe.worldPos.y, minY);
			maxY = std::max(pbe.worldPos.y, maxY);
		}
		m_Width = std::abs(maxX - minX) + 1;
		m_Height = std::abs(maxY - minY) + 1;
		glm::vec2 CenterPixelPos = glm::vec2((m_Width / 2.0f) + minX, (m_Height / 2.0f) + minY);
		glm::vec2 WorldPos = CenterPixelPos / (float)CHUNKSIZE;
		SetPosition(WorldPos);

		glm::ivec2 offset = glm::ivec2(minX, minY) + glm::ivec2(m_Width / 2, m_Height / 2);

		//use the minimum to get local positions from bottom left
		for (auto& pbe : elements)
		{
			glm::ivec2 localPos = pbe.worldPos - offset;
			m_Elements[localPos] = pbe;
		}
		if (CreatedFromSplit) m_InWorld = false; else m_InWorld = true;

		GeneratePixelBody(CreatedFromSplit);
	}

	PixelBody2D::PixelBody2D(UUID id) : RigidBody2D(id)
	{

	}

	void PixelBody2D::Serialize(json& j)
	{
		RigidBody2D::Serialize(j);
		j["Type"] = "PixelBody2D";

		//Position for pixelbody2d is different than normal
		j["m_Position"] = GetPosition();
		PX_TRACE("Serializing position: ({0},{1})", GetPosition().x, GetPosition().y);
		//j["m_Rotation"] = GetRotation();

		j["m_InWorld"] = m_InWorld;
		j["m_Width"] = m_Width;
		j["m_Height"] = m_Height;
		for (auto& [key, value] : m_Elements)
		{
			j["m_Elements"] += {{"Key", key}, { "Value", value } };
		}
		j["m_DebugDisplay"] = m_DebugDisplay;
	}

	void PixelBody2D::Deserialize(json& j)
	{
		RigidBody2D::Deserialize(j);

		//get the position
		glm::vec2 position;
		if (j.contains("m_Position")) j.at("m_Position").get_to(position);
		SetPosition(position);
		PX_TRACE("Deserializing position: ({0},{1})", position.x, position.y);

		//Extract new member variables
		if (j.contains("m_InWorld")) j.at("m_InWorld").get_to(m_InWorld);
		if (j.contains("m_Width")) j.at("m_Width").get_to(m_Width);
		if (j.contains("m_Height")) j.at("m_Height").get_to(m_Height);
		if (j.contains("m_Elements"))
		{
			for (auto& element : j.at("m_Elements"))
			{
				glm::ivec2 key;
				PixelBodyElement value;
				element.at("Key").get_to(key);
				element.at("Value").get_to(value);
				m_Elements[key] = value;
			}
		}
		if (j.contains("m_DebugDisplay")) j.at("m_DebugDisplay").get_to(m_DebugDisplay);

		if (!m_HasBody) CreateBody(Physics2D::GetWorld());
		GeneratePixelBody(true);
	}

	void PixelBody2D::OnPhysicsUpdate()
	{
		///when we update as a pixel body, we have to take all the elements of our body out of the simulation,
		///and put them back in the newly calculated positions.
		///
		
		if (!m_B2Body->IsAwake())
		{
			//skip sleeping bodies
			return;
		}

		if (m_HasBody)
		{
			ExitWorld();

			UpdateElementPositions();

			if (m_RecreateBody)
			{
				GeneratePixelBody();
				m_RecreateBody = false;
			}			

			EnterWorld();
		}

		
	}

	void PixelBody2D::OnInspectorRender()
	{
		RigidBody2D::OnInspectorRender();
		ImGui::Checkbox("Debug Display", &m_DebugDisplay);
	}

	void PixelBody2D::OnRender()
	{
		if (m_DebugDisplay)
		{
			//draw center position
			Renderer2D::DrawQuad({ GetPosition().x, GetPosition().y, 20 }, glm::vec2(0.75f / 256.0f, 0.75f / 256.0f), { 1,1,1,1 });

			float scaling = PPU / (float)CHUNKSIZE;
			
			auto& T = m_B2Body->GetTransform();
			for (auto fixture = m_B2Body->GetFixtureList(); fixture != nullptr; fixture = fixture->GetNext())
			{
				auto shape = (b2PolygonShape*)(fixture->GetShape());
				for (int i = 0; i < shape->m_count - 1; i++)
				{
					auto v = shape->m_vertices[i];
					float x1 = (T.q.c * v.x - T.q.s * v.y) + T.p.x;
					float y1 = (T.q.s * v.x + T.q.c * v.y) + T.p.y;

					auto e = shape->m_vertices[i + 1];
					float x2 = (T.q.c * e.x - T.q.s * e.y) + T.p.x;
					float y2 = (T.q.s * e.x + T.q.c * e.y) + T.p.y;
					glm::vec3 start = glm::vec3(x1, y1, 10) * scaling;
					glm::vec3 end = glm::vec3(x2, y2, 10) * scaling;

					Renderer2D::DrawLine(start, end);
				}
				//draw the last line to connect the shape
				auto v = shape->m_vertices[shape->m_count - 1];
				float x1 = (T.q.c * v.x - T.q.s * v.y) + T.p.x;
				float y1 = (T.q.s * v.x + T.q.c * v.y) + T.p.y;

				auto e = shape->m_vertices[0];
				float x2 = (T.q.c * e.x - T.q.s * e.y) + T.p.x;
				float y2 = (T.q.s * e.x + T.q.c * e.y) + T.p.y;
				glm::vec3 start = glm::vec3(x1, y1, 10) * scaling;
				glm::vec3 end = glm::vec3(x2, y2, 10) * scaling;

				Renderer2D::DrawLine(start, end);

			}
		}
		
		
	}



	void PixelBody2D::EnterWorld()
	{
		if (m_Elements.size() == 0)
		{
			QueueFree();
			return;
		}

		//chunkloading
		//get current chunk we are in, then get the chunks around us
		glm::vec2 pixelPos = GetPosition() * (float)CHUNKSIZE;
		glm::ivec2 chunkPos = m_PXWorld->PixelToChunk(glm::ivec2(pixelPos.x, pixelPos.y));
		for (int x = -1; x <= 1; x++)
		{
			for (int y = -1; y <= 1; y++)
			{
				glm::ivec2 checkPos = chunkPos + glm::ivec2(x, y);
				if (!m_PXWorld->m_Chunks.contains(checkPos))
				{
					m_PXWorld->AddChunk(checkPos);
				}
			}
		}


		//int minX = INT_MAX, maxX = INT_MIN, minY = INT_MAX, maxY = INT_MIN;

		for (auto& mappedElement : m_Elements)
		{
			//minX = std::min(mappedElement.second.worldPos.x, minX);
			//maxX = std::max(mappedElement.second.worldPos.x, maxX);
			//minY = std::min(mappedElement.second.worldPos.y, minY);
			//maxY = std::max(mappedElement.second.worldPos.y, maxY);
 			if (m_PXWorld->GetElement(mappedElement.second.worldPos).m_ID != 0)
			{
				//we are replacing something that is in the way!

				//TODO: instead of always hiding, throw liquids ect as a particle!
				mappedElement.second.hidden = true;

			}
			else
			{

				if (std::abs(m_B2Body->GetAngularVelocity()) > 0.01f || m_B2Body->GetLinearVelocity().LengthSquared() > 0.01f)
				{
					//we are moving, so update dirty rect
					//PX_TRACE("we are moving!: angular: {0}, linear: {1}", pair.second->m_B2Body->GetAngularVelocity(), pair.second->m_B2Body->GetLinearVelocity().LengthSquared());
					m_PXWorld->SetElement(mappedElement.second.worldPos, mappedElement.second.element);
				}
				else
				{
					//we are still, so stop updating region!
					m_PXWorld->SetElementWithoutDirtyRectUpdate(mappedElement.second.worldPos, mappedElement.second.element);
				}

				//we are no longer hidden!
				mappedElement.second.hidden = false;

			}
		}
	}

	void PixelBody2D::ExitWorld()
	{
		//center of the pixel body in the world
		glm::ivec2 centerPixelWorld = { m_B2Body->GetPosition().x * PPU, m_B2Body->GetPosition().y * PPU };

		//keep list of elements to take out after iteration
		std::vector<glm::ivec2> elementsToRemove;

		//loop over all elements, and attempt to take them out of the world
		for (auto& mappedElement : m_Elements)
		{
			if (mappedElement.second.hidden) // make sure we aren't hidden before trying to pull
				continue;

			//the world position of the element is already known, so just try to grab it
			Element& worldElement = m_PXWorld->GetElement(mappedElement.second.worldPos);
			ElementData& elementData = ElementData::GetElementData(worldElement.m_ID);

			if (mappedElement.second.element.m_ID != worldElement.m_ID)// || !worldElement.m_Rigid TODO re-implement rigid?
			{

				//element has changed over the last update, could have been removed
				//or melted or something of the sort.
				//we need to re-create our rigid body!

				if (elementData.cell_type == ElementType::solid || (elementData.cell_type == ElementType::movableSolid && worldElement.m_Rigid))
				{
					//replaced element is able to continue being part of the solid!
					//this could be the player replacing the blocks, or a solid block
					//reacts with something and stays solid, like getting stained or something
					//idk
					//either way, in this situation we just pull the new element
					m_Elements[mappedElement.first].element = worldElement;
					m_PXWorld->SetElementWithoutDirtyRectUpdate(mappedElement.second.worldPos, Element());
				}
				else
				{
					//the element that has taken over the spot is not able to be
					//a solid, so we need to re-construct the rigid body without that
					//element! so we leave it in the sim, and erase the previous from
					//the body
					elementsToRemove.push_back(mappedElement.first);
				}
				//PX_TRACE("Element changed over update at ({0},{1})", mappedElement.second.worldPos.x, mappedElement.second.worldPos.y);
			}
			else
			{
				//element should be the same, so nothing has changed, pull the element out
				m_Elements[mappedElement.first].element = worldElement;
				m_PXWorld->SetElementWithoutDirtyRectUpdate(mappedElement.second.worldPos, Element());
			}
		}

		//now that we pulled all the elements out, try to re-construct the body if needed:
		if (elementsToRemove.size() > 0)
		{
			//we need to reconstruct!

			//remove the outdated elements
			for (auto& localPos : elementsToRemove)
			{
				m_Elements.erase(localPos);
			}

			m_RecreateBody = true;
		}
	}

	void PixelBody2D::UpdateElementPositions()
	{
		//center of the pixel body in the world
		b2Vec2 b2Position = m_B2Body->GetPosition();
		glm::ivec2 centerPixelWorld = { b2Position.x * PPU, b2Position.y * PPU };
		if (b2Position.x < 0) centerPixelWorld.x -= 1;
		if (b2Position.y < 0) centerPixelWorld.y -= 1;

		float angle = m_B2Body->GetAngle();
		auto rotationMatrix = glm::mat2x2(1);

		//if angle gets above 45 degrees, apply a 90 deg rotation first
		while (angle > 0.78539816339f)
		{
			angle -= 1.57079632679f;
			rotationMatrix *= glm::mat2x2(0, -1, 1, 0);
		}
		while (angle < -0.78539816339f)
		{
			angle += 1.57079632679f;
			rotationMatrix *= glm::mat2x2(0, 1, -1, 0);
		}

		float A = -std::tan(angle / 2);
		float B = std::sin(angle);
		auto horizontalSkewMatrix = glm::mat2x2(1, 0, A, 1);//0 a
		auto verticalSkewMatrix = glm::mat2x2(1, B, 0, 1);// b 0
		for (auto& mappedElement : m_Elements)
		{
			// find the element in the world by using the transform of the body
			// and the stored local position 			
			glm::ivec2 skewedPos = mappedElement.first * rotationMatrix;

			//horizontal skew:
			int horizontalSkewAmount = (float)skewedPos.y * A;
			skewedPos.x += horizontalSkewAmount;

			//vertical skew
			int skewAmount = (float)skewedPos.x * B;
			skewedPos.y += skewAmount;

			//horizontal skew:
			horizontalSkewAmount = (float)skewedPos.y * A;
			skewedPos.x += horizontalSkewAmount;
			
			mappedElement.second.worldPos = glm::ivec2(skewedPos.x, skewedPos.y) + centerPixelWorld;
		}
	}

	void PixelBody2D::GeneratePixelBody(bool SkipCalculations)
	{

		//step 1 make sure we aren't completely erased!
		if (m_Elements.size() == 0)
		{
			return;
		}

		//if we weren't created from a split, we need to check if we have been split or are not 
		//contiguous to begin with
		if (!SkipCalculations)
		{
			b2Vec2 linearVelocity = m_B2Body->GetLinearVelocity();
			float angularVelocity = m_B2Body->GetAngularVelocity();


			//get the full set of elements for the seed pull set
			std::unordered_set<glm::ivec2, HashVector> source;
			for (auto& pair : m_Elements)
			{
				source.insert(pair.first);
			}
			//we need to split the elements that are continuous into seperate bodies, so we
			//use a flood algorithm to pull contiguous vertices out
			std::vector<std::unordered_set<glm::ivec2, HashVector>> areas = GetContiguousAreas(source);

			//we need to make the split bodies first, because m_elements contains the actual element data
			while (areas.size() > 1)
			{
				//we have a split body to make!

				//get a list of pixel body elements to turn into the new pixelbody2d
				std::vector<PixelBodyElement> elements;
				for (auto& pos : areas.back())
				{
					elements.push_back(m_Elements[pos]);
				}
				//const std::string& name, b2BodyType type, World* world, std::vector<PixelBodyElement>& elements, bool CreatedFromSplit = false);
				//auto newPixelBody2D = PixelBody2D(m_Name, GetType(), m_PXWorld, elements, true);
				auto newPixelBody2D = Instantiate<PixelBody2D>(m_Name, GetType(), m_PXWorld, elements, true);
				newPixelBody2D->m_B2Body->SetLinearVelocity(linearVelocity);
				newPixelBody2D->m_B2Body->SetAngularVelocity(angularVelocity);
				newPixelBody2D->EnterWorld();

				if (m_Parent != nullptr)
				{
					m_Parent->AddChild(newPixelBody2D);
				}
				

				areas.pop_back();				
			}
			//we only have our area left (or maybe we never even split)


			//find the new width, height & center of the set of elements
			int minX = INT_MAX, maxX = INT_MIN, minY = INT_MAX, maxY = INT_MIN;
			for (auto& localPos : areas[0])
			{
				PixelBodyElement& e = m_Elements[localPos];
				minX = std::min(e.worldPos.x, minX);
				maxX = std::max(e.worldPos.x, maxX);
				minY = std::min(e.worldPos.y, minY);
				maxY = std::max(e.worldPos.y, maxY);
			}
			m_Width = std::abs(maxX - minX) + 1;
			m_Height = std::abs(maxY - minY) + 1;

			glm::vec2 CenterPixelPos = glm::vec2((m_Width / 2.0f) + minX, (m_Height / 2.0f) + minY);			
			SetPosition(CenterPixelPos / (float)CHUNKSIZE);

			glm::ivec2 offset = glm::ivec2(minX, minY) + glm::ivec2(m_Width / 2, m_Height / 2);

			//use the minimum to get local positions from bottom left
			std::unordered_map<glm::ivec2, PixelBodyElement, HashVector> newElements;
			for (auto& oldPos : areas[0])
			{
				auto& pbe = m_Elements[oldPos];
				glm::ivec2 localPos = pbe.worldPos - offset;
				newElements[localPos] = pbe;
			}
			
			m_Elements = std::move(newElements);
		}

		//now that we have the width, height, and contiguous elements of the pixel body, we can generate the fixtures and such

		//clear any previous fixtures
		auto fixture = m_B2Body->GetFixtureList();
		if (fixture != nullptr) do
		{
			b2Fixture* next = fixture->GetNext();
			m_B2Body->DestroyFixture(fixture);
			fixture = next;
		} while (fixture != nullptr);


		// BUG ERROR FIX TODO WRONG BROKEN
		// getcontour points is able to have repeating points, which is a no-no for box2d triangles / triangulation
		auto contour = GetContourPoints();
		if (contour.size() == 0)
		{
			QueueFree();
			PX_ASSERT(false, "PixelBody2D Error: Contour gathered was size 0");
			return;
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

		if (contourVector.size() < 3) return;

		//run triangulation algorithm to create the needed triangles/fixtures
		std::vector<p2t::Point*> polyLine;
		for (auto& point : contourVector)
		{
			polyLine.push_back(new p2t::Point(point));
		}

		//create each of the triangles to comprise the body, each being a fixture
		p2t::CDT* cdt = new p2t::CDT(polyLine);
		cdt->Triangulate();
		auto triangles = cdt->GetTriangles();
		for (auto& triangle : triangles)
		{
			b2PolygonShape triangleShape;
			b2Vec2 points[3] = {
				{(float)((triangle->GetPoint(0)->x) / PPU), (float)((triangle->GetPoint(0)->y) / PPU)},
				{(float)((triangle->GetPoint(1)->x) / PPU), (float)((triangle->GetPoint(1)->y) / PPU)},
				{(float)((triangle->GetPoint(2)->x) / PPU), (float)((triangle->GetPoint(2)->y) / PPU)}
			};
			if (points[0] == points[1] || points[0] == points[2] || points[1] == points[2]) continue; // degenerate polygon, so skip.
			triangleShape.Set(points, 3);
			b2FixtureDef fixtureDef;
			fixtureDef.density = 1;
			fixtureDef.friction = 0.3f;
			fixtureDef.shape = &triangleShape;
			m_B2Body->CreateFixture(&fixtureDef);
		}
		
	}


	void PixelBody2D::TransferWorld(b2World* world)
	{
		RigidBody2D::TransferWorld(world);
		GeneratePixelBody(true);
	}

	glm::mat4 PixelBody2D::GetWorldTransform()
	{
		//b2 position is different than world position.
		auto pos = m_B2Body->GetPosition();
		pos *= B2ToWorld;//convert from b2 position to world position
		glm::mat4 localTransform = glm::translate(glm::mat4(), glm::vec3(pos.x, pos.y, m_Layer));
		localTransform = glm::rotate(localTransform, m_Rotation, { 0,0,-1 });
		if (Node2D* parent2D = dynamic_cast<Node2D*>(m_Parent))
		{
			return parent2D->GetWorldTransform() * localTransform;
		}
		else
		{
			return localTransform;
		}
	}

	void PixelBody2D::Translate(const glm::vec2& translation)
	{
		m_Position += translation;
		m_B2Body->SetTransform(b2Vec2(m_Position.x * WorldToB2, m_Position.y * WorldToB2), m_Rotation);
	}
	void PixelBody2D::SetPosition(const glm::vec2& position)
	{
		m_Position = position;
		m_B2Body->SetTransform(b2Vec2( m_Position.x * WorldToB2, m_Position.y * WorldToB2) , m_Rotation);
	}
	glm::vec2 PixelBody2D::GetPosition()
	{
		b2Vec2 vec = m_B2Body->GetPosition();
		return glm::vec2(vec.x, vec.y) * B2ToWorld;
	}

	glm::vec2 PixelBody2D::GetB2Position()
	{
		b2Vec2 vec = m_B2Body->GetPosition();
		return glm::vec2(vec.x, vec.y);
	}

	void PixelBody2D::Rotate(const float radians)
	{
		m_Rotation += radians;
		m_B2Body->SetTransform({ m_Position.x, m_Position.y }, m_Rotation);
	}
	void PixelBody2D::SetRotation(const float radians)
	{
		m_Rotation = radians;
		m_B2Body->SetTransform({ m_Position.x, m_Position.y }, m_Rotation);
	}
	float PixelBody2D::GetRotation()
	{
		return m_B2Body->GetAngle();
	}


	std::vector<std::unordered_set<glm::ivec2, HashVector>> PixelBody2D::GetContiguousAreas(std::unordered_set<glm::ivec2, HashVector> source)
	{
		std::vector<std::unordered_set<glm::ivec2, HashVector>> result;

		while (source.size() > 0)
		{
			//init a new set in the vector
			result.push_back(std::unordered_set<glm::ivec2, HashVector>());
			//populate it using the source and queue pull
			QueuePull({ 0,0 }, result.back(), source);
		}

		return result;
	}


	void PixelBody2D::QueuePull(glm::ivec2 startPos, std::unordered_set<glm::ivec2, HashVector>& result, std::unordered_set<glm::ivec2, HashVector>& source)
	{
		if (source.size() == 0) return;

		//make sure the start pos is valid
		if (!source.contains(startPos))
		{
			startPos = *source.begin();
		}

		//we make a queue of positions to check
		std::queue<glm::ivec2> queue;
		std::unordered_set<glm::ivec2, HashVector> visited;

		queue.push(startPos);
		visited.insert(startPos);
		result.insert(startPos);
		source.erase(startPos);//since we know it is valid, and it won't be reached by loop

		const glm::ivec2 neighbors[] =
		{
			{ 1, 0},
			{-1, 0},
			{ 0, 1},
			{ 0,-1},
		};

		while (queue.size() > 0)
		{			
			// check neighbors to see if we need to add them to the queue, or
			//have already visited them
			glm::ivec2 position = queue.front();
			queue.pop();

			for (int i = 0; i < 4; i++)
			{
				glm::ivec2 pos = position + neighbors[i];
				if (!visited.contains(pos) && source.contains(pos))
				{
					queue.push(pos);
					result.insert(pos);
					source.erase(pos);
				}
				visited.insert(pos);
			}			
		}

		//done!

	}

	std::vector<p2t::Point> PixelBody2D::GetContourPoints()
	{
		//run marching squares on element array to find all vertices
		//inspired by https://www.emanueleferonato.com/2013/03/01/using-marching-squares-algorithm-to-trace-the-contour-of-an-image/
		std::vector<p2t::Point> ContourVector;

		int halfWidth = (m_Width / 2) + 2;
		int halfHeight = (m_Height / 2) + 2;
		glm::ivec2 cursor = glm::ivec2(halfWidth, halfHeight); // get top right local pos
		//scroll through element array until you find an element to start at
		while (!m_Elements.contains(cursor))
		{
			cursor.x--;
			if (cursor.x < -halfWidth)
			{
				cursor.x = halfWidth;
				cursor.y--;
				if (cursor.y < -halfHeight)
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

	int PixelBody2D::GetMarchingSquareCase(glm::ivec2 localPosition)
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

	std::vector<p2t::Point> PixelBody2D::SimplifyPoints(const std::vector<p2t::Point>& contourVector, int startIndex, int endIndex, float threshold)
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
		//PX_TRACE("Removed Points");
		return endResult;
	}

}
