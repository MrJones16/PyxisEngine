#include "Chunk.h"
#include "Pyxis/Renderer/Renderer2D.h"
#include <Pyxis/Game/Physics2D.h>
#include <box2d/b2_chain_shape.h>


namespace Pyxis
{
	Chunk::Chunk(glm::ivec2 chunkPos)
	{
		m_ChunkPos = chunkPos;
		for (int i = 0; i < CHUNKSIZE * CHUNKSIZE; i++)
		{
			m_Elements[i] = Element();
		}
		//create a static body for the chunk
		float RBToWorld = ((float)CHUNKSIZE / PPU);
		b2BodyDef bodyDef;
		bodyDef.type = b2_staticBody;
		bodyDef.position = b2Vec2(m_ChunkPos.x * RBToWorld, m_ChunkPos.y * RBToWorld);
		m_B2Body = Physics2D::GetWorld()->CreateBody(&bodyDef);

		//reset dirty rect
		ResetDirtyRect();


		m_Texture = Texture2D::Create(CHUNKSIZE, CHUNKSIZE);
		//set texture to fill color
		std::fill(m_PixelBuffer, m_PixelBuffer + (CHUNKSIZE * CHUNKSIZE), 0xFF000000);
		m_Texture->SetData(m_PixelBuffer, sizeof(m_PixelBuffer));

	}


	void Chunk::Clear()
	{
		for (int i = 0; i < CHUNKSIZE * CHUNKSIZE; i++)
		{
			m_Elements[i] = Element();
		}

		ResetDirtyRect();
		
		UpdateTexture();
		
	}

	Element& Chunk::GetElement(int x, int y)
	{
		return m_Elements[x + y * CHUNKSIZE];
	}

	Element& Chunk::GetElement(const glm::ivec2& index)
	{
		return m_Elements[index.x + index.y * CHUNKSIZE];
	}

	void Chunk::SetElement(int x, int y, const Element& element)
	{
		//test if we are placing a rigid element
		if (element.m_Rigid)
		{
			//we don't worry about updating the collider if we are setting a rigid element
			m_Elements[x + y * CHUNKSIZE] = element;
			return;
		}
		//test if we are replacing a rigid element
		if (m_Elements[x + y * CHUNKSIZE].m_Rigid)
		{
			m_Elements[x + y * CHUNKSIZE] = element;

			//we are replacing a rigid element, so we should set the new element to be rigid if it is a solid!
			ElementType newType = ElementData::GetElementData(m_Elements[x + y * CHUNKSIZE].m_ID).cell_type;
			if (newType == ElementType::solid)
				m_Elements[x + y * CHUNKSIZE].m_Rigid = true;

			return;
		}

		ElementType currType = ElementData::GetElementData(m_Elements[x + y * CHUNKSIZE].m_ID).cell_type;
		if (currType == ElementType::solid || currType == ElementType::movableSolid)
		{
			//we are currently solid, so see if we are changing to something that is not
			currType = ElementData::GetElementData(element.m_ID).cell_type;
			if (!(currType == ElementType::solid || currType == ElementType::movableSolid))
			{
				//we are no longer solid, so we need to update the collider
				m_StaticColliderChanged = true;
			}

			m_Elements[x + y * CHUNKSIZE] = element;

		}
		else
		{
			//we are not solid, so see if we will become one.
			currType = ElementData::GetElementData(element.m_ID).cell_type;
			if (currType == ElementType::solid || currType == ElementType::movableSolid)
			{
				//we are becoming solid, so we need to update the collider
				m_StaticColliderChanged = true;
			}

			m_Elements[x + y * CHUNKSIZE] = element;
		}		
	}

	/// <summary>
	/// updates the dirty rect for the chunk
	void Chunk::UpdateDirtyRect(int x, int y)
	{
		//update minimums
		if (x < m_DirtyRect.min.x + m_DirtyRectBorderWidth) m_DirtyRect.min.x = x - m_DirtyRectBorderWidth;
		if (y < m_DirtyRect.min.y + m_DirtyRectBorderWidth) m_DirtyRect.min.y = y - m_DirtyRectBorderWidth;
		//update maximums
		if (x > m_DirtyRect.max.x - m_DirtyRectBorderWidth) m_DirtyRect.max.x = x + m_DirtyRectBorderWidth;
		if (y > m_DirtyRect.max.y - m_DirtyRectBorderWidth) m_DirtyRect.max.y = y + m_DirtyRectBorderWidth;
	}

	void Chunk::ResetDirtyRect()
	{
		m_DirtyRect.min.x = CHUNKSIZE - 1;
		m_DirtyRect.min.y = CHUNKSIZE - 1;
		m_DirtyRect.max.x = 0;
		m_DirtyRect.max.y = 0;		
	}


	void Chunk::UpdateTexture()
	{		
		bool dataChanged = false;

		//if min.x <= max.x
		if (m_DirtyRect.min.x <= m_DirtyRect.max.x)
		{
			dataChanged = true;
			for (int x = std::max(m_DirtyRect.min.x, 0); x <= std::min(m_DirtyRect.max.x, CHUNKSIZE - 1); x++)
			{
				//loop from min x to max x
				for (int y = std::max(m_DirtyRect.min.y, 0); y <= std::min(m_DirtyRect.max.y, CHUNKSIZE - 1); y++)
				{
					m_PixelBuffer[x + y * CHUNKSIZE] = m_Elements[x + y * CHUNKSIZE].m_Color;
				}
			}
		}
			
		if (dataChanged)
		{
			//PX_TRACE("Uploading texture");
			this;
			m_Texture->SetData(m_PixelBuffer, sizeof(m_PixelBuffer));
		}
		
	}

	void Chunk::UpdateWholeTexture()
	{
		//set data first, then update the pixels. this allows you to draw over the texture without interupting the actual
		//elements in the map.
		
		for (int x = 0; x < CHUNKSIZE; x++)
		{
			for (int y = 0; y < CHUNKSIZE; y++)
			{
				m_PixelBuffer[x + y * CHUNKSIZE] = m_Elements[x + y * CHUNKSIZE].m_Color;
			}
		}
		m_Texture->SetData(m_PixelBuffer, sizeof(m_PixelBuffer));

	}

	void Chunk::RenderChunk()
	{
		if (s_DebugChunks)
		{
			//draw center position
			glm::vec4 chunkStatusColor = {1,1,1,0.2f};
			if (m_StaticColliderGenerated) chunkStatusColor = { 0,0,1,0.2f };
			if (m_StaticColliderGenerated && m_StaticColliderChanged) chunkStatusColor = { 0,1,1,0.2f };
			Renderer2D::DrawQuad({ m_ChunkPos.x + 0.5f, m_ChunkPos.y + 0.5f, 20 }, glm::vec2(0.1f), chunkStatusColor);

			float scaling = PPU / (float)CHUNKSIZE;
			//Draw the collider
			auto& T = m_B2Body->GetTransform();
			for (auto fixture = m_B2Body->GetFixtureList(); fixture != nullptr; fixture = fixture->GetNext())
			{
				auto shape = (b2ChainShape*)(fixture->GetShape());
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


			glm::vec2 min = {std::max(m_DirtyRect.min.x, 0), std::max(m_DirtyRect.min.y, 0) };
			glm::vec2 max = { std::min(m_DirtyRect.max.x, CHUNKSIZE - 1), std::min(m_DirtyRect.max.y, CHUNKSIZE - 1) };
			if (!(min.x > max.x || min.y > max.y))
			{
				float width = (max.x - min.x) + 1;
				float height = (max.y - min.y) + 1;
				
				float posX = (min.x + (width / 2.0f)) / (float)CHUNKSIZE;
				float posY = (min.y + (height / 2.0f)) / (float)CHUNKSIZE;

				posX += m_ChunkPos.x;
				posY += m_ChunkPos.y;

				Renderer2D::DrawQuad({ posX, posY, 30 }, {width / (float)CHUNKSIZE, height / (float)CHUNKSIZE}, { 1,0,0,0.2f });
			}
			
		}
	}

	void Chunk::GenerateStaticCollider()
	{
		m_StaticColliderGenerated = true;
		//clear any previous fixtures
		auto fixture = m_B2Body->GetFixtureList();
		if (fixture != nullptr) do
		{
			b2Fixture* next = fixture->GetNext();
			m_B2Body->DestroyFixture(fixture);
			fixture = next;
		} while (fixture != nullptr);

		std::unordered_set<glm::ivec2, HashVector> source;
		for (int i = 0; i < CHUNKSIZE * CHUNKSIZE; i++)
		{
			ElementData& ed = ElementData::GetElementData(m_Elements[i].m_ID);
			if (!m_Elements[i].m_Rigid && ed.cell_type == ElementType::solid)
			{
				source.insert({ i % CHUNKSIZE, i / CHUNKSIZE });
			}
		}
		std::list<std::unordered_set<glm::ivec2, HashVector>> areas;
		while (source.size() > 0)
		{
			std::unordered_set<glm::ivec2, HashVector> result;
			QueuePull(*source.begin(), result, source);
			areas.push_back(result);
		}

		while (!areas.empty())
		{
			auto contour = GetContourPoints(areas.back());
			areas.pop_back();

			


			// BUG ERROR FIX TODO WRONG BROKEN
			// getcontour points is able to have repeating points, which is a no-no for box2d triangles / triangulation
			if (contour.size() == 0)
			{
				
				PX_ASSERT(false, "PixelBody2D Error: Contour gathered was size 0");
				continue;
			}

			std::vector<b2Vec2> contourVector;
			if (contour.size() > 20)
			{
				contourVector = SimplifyPoints(contour, 0, contour.size() - 1, 1.0f);
			}
			else
			{
				contourVector = contour;
			}

			//scale points to PPU
			for (auto& point : contourVector)
			{
				point.x /= PPU;
				point.y /= PPU;
			}

			if (contourVector.size() >= 3)
			{
				b2ChainShape chainShape;
				chainShape.CreateLoop((b2Vec2*)contourVector.data(), contourVector.size());

				b2FixtureDef fixtureDef;
				fixtureDef.density = 1;
				fixtureDef.friction = 0.3f;
				fixtureDef.shape = &chainShape;
				m_B2Body->CreateFixture(&fixtureDef);
			}

		}

	}


	void Chunk::QueuePull(glm::ivec2 startPos, std::unordered_set<glm::ivec2, HashVector>& result, std::unordered_set<glm::ivec2, HashVector>& source)
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
	int Chunk::GetMarchingSquareCase(const glm::ivec2& localPosition, const std::unordered_set<glm::ivec2, HashVector>& source)
	{
		int result = 0;
		if (source.contains(localPosition + glm::ivec2(0, 0))) result += 8;
		if (source.contains(localPosition + glm::ivec2(-1, 0))) result += 4;
		if (source.contains(localPosition + glm::ivec2(0, 1))) result += 2;
		if (source.contains(localPosition + glm::ivec2(-1, 1))) result += 1;
		return result;
	}
	std::vector<b2Vec2> Chunk::SimplifyPoints(const std::vector<b2Vec2>& contourVector, int startIndex, int endIndex, float threshold)
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

		auto endResult = std::vector<b2Vec2>();
		endResult.push_back(contourVector[startIndex]);
		endResult.push_back(contourVector[endIndex]);
		//PX_TRACE("Removed Points");
		return endResult;
	}
	std::vector<b2Vec2> Chunk::GetContourPoints(const std::unordered_set<glm::ivec2, HashVector>& source)
	{
		//run marching squares on element array to find all vertices
		//inspired by https://www.emanueleferonato.com/2013/03/01/using-marching-squares-algorithm-to-trace-the-contour-of-an-image/
		std::vector<b2Vec2> ContourVector;

		
		glm::ivec2 cursor = glm::ivec2(CHUNKSIZE - 1, CHUNKSIZE - 1); // get top right local pos
		//scroll through element array until you find an element to start at
		while (!source.contains(cursor))
		{
			cursor.x--;
			if (cursor.x < 0)
			{
				cursor.x = CHUNKSIZE - 1;
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
			int caseValue = GetMarchingSquareCase(cursor, source);
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
			ContourVector.push_back(b2Vec2(cursor.x, cursor.y));
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
}