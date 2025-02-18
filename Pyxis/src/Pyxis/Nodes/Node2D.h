#pragma once

#include <Pyxis/Nodes/Node.h>
#include <box2d/b2_body.h>
#include <glm/mat2x2.hpp>

namespace Pyxis
{
	

	class Node2D : public Node
	{
	protected:
		glm::vec2 m_Position = glm::vec2(0);
		float m_Layer = 0;
		//In radians
		float m_Rotation = 0;

	public:

		Node2D(const std::string& name = "Node2D") : Node(name)
		{

		}

		Node2D(UUID id) : Node(id)
		{

		}


		virtual glm::mat4 GetWorldTransform()
		{
			glm::mat4 localTransform = glm::translate(glm::mat4(), glm::vec3(m_Position.x, m_Position.y, m_Layer));
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

		void Serialize(json& j) override
		{
			Node::Serialize(j);
			j["Type"] = "Node2D"; // Override type identifier
			j["m_Position"] = m_Position;
			j["m_Layer"] = m_Layer;
			j["m_Rotation"] = m_Rotation;
		}
		void Deserialize(json& j) override
		{
			Node::Deserialize(j);
			if (j.contains("m_Position")) j.at("m_Position").get_to(m_Position);
			if (j.contains("m_Layer")) j.at("m_Layer").get_to(m_Layer);
			if (j.contains("m_Rotation")) j.at("m_Rotation").get_to(m_Rotation);
		}


		virtual void Translate(const glm::vec2& translation)
		{
			m_Position += translation;
		}
		virtual void SetPosition(const glm::vec2& position)
		{
			m_Position = position;
		}
		virtual glm::vec2 GetPosition()
		{
			return m_Position;
		}
		

		virtual void Rotate(const float radians)
		{
			m_Rotation += radians;
		}
		virtual void SetRotation(const float radians)
		{
			m_Rotation = radians;
		}
		virtual float GetRotation() 
		{ 
			return m_Rotation; 
		}

	};

	REGISTER_SERIALIZABLE_NODE(Node2D);
}
