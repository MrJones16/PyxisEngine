#pragma once
#include <Pyxis/Nodes/ChainBody2D.h>

namespace Pyxis
{

	class ChunkChainBody : public ChainBody2D
	{
	public:
		glm::ivec2 m_ChunkOwnerPos = {0,0};

	public:
		ChunkChainBody(const std::string& name, b2BodyType type) : ChainBody2D(name, type)
		{

		}

		ChunkChainBody(UUID id) : ChainBody2D(id)
		{

		}

		~ChunkChainBody() 
		{

		}

		void Serialize(json& j) override
		{
			ChainBody2D::Serialize(j);
			j["Type"] = "ChunkChainBody";

			//Add new members
			j["ChunkOwnerPos"] = m_ChunkOwnerPos;
		}

		void Deserialize(json& j) override
		{
			ChainBody2D::Deserialize(j);

			//Deserialize new members
			m_ChunkOwnerPos = j["ChunkOwnerPos"];
		}
		
	};

	REGISTER_SERIALIZABLE_NODE(ChunkChainBody);
}
