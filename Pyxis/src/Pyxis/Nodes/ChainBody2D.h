#pragma once

#include <Pyxis/Nodes/RigidBody2D.h>
#include <box2d/b2_chain_shape.h>

namespace Pyxis
{

	class ChainBody2D : public RigidBody2D
	{
	public:
		ChainBody2D(const std::string& name, b2BodyType type) : RigidBody2D(name, type)
		{
			
		}

		ChainBody2D(UUID id) : RigidBody2D(id)
		{

		}

		void CreateLoop(const b2Vec2* vertices, int32 count)
		{
			b2ChainShape chainShape;
			chainShape.CreateLoop(vertices, count);

			b2FixtureDef fixtureDef;
			fixtureDef.density = 1;
			fixtureDef.friction = 0.3f;
			fixtureDef.shape = &chainShape;
			m_B2Body->CreateFixture(&fixtureDef);
		}

		void TransferWorld(b2World* world) override
		{
			if (m_HasBody)
			{
				//store data from body, then make a new one, then put the new data in.
				B2BodyStorage storage(m_B2Body);

				json body;

				//Serialize our chain data		
				b2Fixture* fixture = m_B2Body->GetFixtureList();
				while (fixture != nullptr)
				{
					b2ChainShape* chain = dynamic_cast<b2ChainShape*>(fixture->GetShape());
					if (chain)
					{
						json shapeData;
						shapeData["Type"] = "ChainShape";
						shapeData["VertexCount"] = chain->m_count;
						for (int i = 0; i < chain->m_count; i++)
						{
							shapeData["Vertices"].push_back(glm::vec2(chain->m_vertices[i].x, chain->m_vertices[i].y));
						}
						body["Shapes"] += shapeData;
					}
					fixture = fixture->GetNext();
				}

				//trick ourselves to make a new body even though we have one
				m_HasBody = false;
				CreateBody(world);
				storage.TransferData(m_B2Body);

				//deserialize the chain body
				auto& shapes = body["Shapes"];
				for (auto& shape : shapes)
				{
					if (shape["Type"] == "ChainShape")
					{
						b2ChainShape chain;
						std::vector<glm::vec2> vertices;
						
						int count = shape["VertexCount"];
						for (int i = 0; i < count; i++)
						{
							vertices.push_back(shape["Vertices"][i]);
						}
						chain.CreateLoop((b2Vec2*)vertices.data(), vertices.size() - 1);
						b2FixtureDef fixtureDef;
						fixtureDef.shape = &chain;
						m_B2Body->CreateFixture(&fixtureDef);
					}
				}

				PX_TRACE("Transferred body to new world. Position: ({0},{1})", m_B2Body->GetPosition().x, m_B2Body->GetPosition().y);
			}


			
			
		}

		void DebugDraw(float scaling, float z)
		{
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
					glm::vec3 start = glm::vec3(x1* scaling, y1 * scaling, z);
					glm::vec3 end = glm::vec3(x2 * scaling, y2 * scaling, z);

					Renderer2D::DrawLine(start, end);
					PX_CORE_TRACE("Drawing line from ({0},{1}) to ({2},{3})", start.x, start.y, end.x, end.y);
				}
				//draw the last line to connect the shape
				auto v = shape->m_vertices[shape->m_count - 1];
				float x1 = (T.q.c * v.x - T.q.s * v.y) + T.p.x;
				float y1 = (T.q.s * v.x + T.q.c * v.y) + T.p.y;

				auto e = shape->m_vertices[0];
				float x2 = (T.q.c * e.x - T.q.s * e.y) + T.p.x;
				float y2 = (T.q.s * e.x + T.q.c * e.y) + T.p.y;
				glm::vec3 start = glm::vec3(x1 * scaling, y1 * scaling, z);
				glm::vec3 end = glm::vec3(x2 * scaling, y2 * scaling, z);

				Renderer2D::DrawLine(start, end);

			}
		}

		void Serialize(json& j) override
		{
			RigidBody2D::Serialize(j);
			j["Type"] = "ChainBody2D";

			//serialize the chain body			
			b2Fixture* fixture = m_B2Body->GetFixtureList();
			while (fixture != nullptr)
			{
				b2ChainShape* chain = dynamic_cast<b2ChainShape*>(fixture->GetShape());
				if (chain)
				{
					json shapeData;
					shapeData["Type"] = "ChainShape";
					shapeData["VertexCount"] = chain->m_count;
					for (int i = 0; i < chain->m_count; i++)
					{
						shapeData["Vertices"].push_back(glm::vec2(chain->m_vertices[i].x, chain->m_vertices[i].y));
					}
					j["Shapes"] += shapeData;
				}
				fixture = fixture->GetNext();
			}
			
			
		}

		void Deserialize(json& j) override
		{
			RigidBody2D::Deserialize(j);

			//deserialize the chain body
			auto& shapes = j["Shapes"];
			for (auto& shape : shapes)
			{
				if (shape["Type"] == "ChainShape")
				{
					b2ChainShape chain;
					std::vector<glm::vec2> vertices;

					int count = shape["VertexCount"];
					for (int i = 0; i < count; i++)
					{
						vertices.push_back(shape["Vertices"][i]);
					}
					chain.CreateLoop((b2Vec2*)vertices.data(), vertices.size() - 1);

					b2FixtureDef fixtureDef;
					fixtureDef.shape = &chain;
					m_B2Body->CreateFixture(&fixtureDef);
				}
			}

		}

	};

	REGISTER_SERIALIZABLE_NODE(ChainBody2D);

}
