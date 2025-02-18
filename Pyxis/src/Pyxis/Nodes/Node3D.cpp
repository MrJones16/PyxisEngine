#include "Node3D.h"
#include "Node3D.h"
#include "Node3D.h"
#include "Node3D.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_decompose.hpp"

namespace Pyxis
{
	void Node3D::UpdateLocalTransform()
	{
		//PX_TRACE("Updated Transform");
		m_LocalTransform = glm::translate(glm::mat4(1), m_Position);
		m_LocalTransform = glm::rotate(m_LocalTransform, glm::radians(m_Rotation.x), { -1, 0, 0 });
		m_LocalTransform = glm::rotate(m_LocalTransform, glm::radians(m_Rotation.y), { 0,-1, 0 });
		m_LocalTransform = glm::rotate(m_LocalTransform, glm::radians(m_Rotation.z), { 0, 0,-1 });
		m_LocalTransform = glm::scale(m_LocalTransform, m_Scale);
	}

	void Node3D::OnInspectorRender()
	{
		if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::DragFloat3("Position", glm::value_ptr(m_Position)))
			{
				UpdateLocalTransform();
			}
			if (ImGui::DragFloat3("Rotation", glm::value_ptr(m_Rotation)))
			{
				UpdateLocalTransform();
			}
			if (ImGui::DragFloat3("Scale", glm::value_ptr(m_Scale)))
			{
				UpdateLocalTransform();
			}
			ImGui::TreePop();
		}
	}

	void Node3D::Serialize(json& j)
	{
		j["m_Position"] = m_Position;
		j["m_Rotation"] = m_Rotation;
		j["m_Scale"] = m_Scale;
		Node::Serialize(j);
		j["Type"] = "Node3D"; // Override type identifier
	}

	void Node3D::Deserialize(json& j)
	{
		if (j.contains("m_Position")) j.at("m_Position").get_to(m_Position);
		if (j.contains("m_Rotation")) j.at("m_Rotation").get_to(m_Rotation);
		if (j.contains("m_Scale")) j.at("m_Scale").get_to(m_Scale);
		UpdateLocalTransform();
		Node::Deserialize(j);
	}

	glm::mat4 Node3D::GetWorldTransform()
	{
		if (Node3D* parent3D = dynamic_cast<Node3D*>(m_Parent))
		{
			//TODO: Test if this is the correct order
			return parent3D->GetWorldTransform() * m_LocalTransform;
		}
		else
		{
			return m_LocalTransform;
		}

	}

	void Node3D::ResetLocalTransform()
	{
		m_Position = glm::vec3(0);
		m_Scale = glm::vec3(1);
		m_Rotation = glm::vec3(0);
		m_LocalTransform = glm::mat4(1);
	}
	glm::mat4& Node3D::GetLocalTransform()
	{
		return m_LocalTransform;
	}
	void Node3D::SetLocalTransform(const glm::mat4& transform)
	{
		glm::quat rotation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(transform, m_Scale, rotation, m_Position, skew, perspective);

		m_Rotation = -glm::degrees(glm::eulerAngles(rotation));

		UpdateLocalTransform();
	}




	void Node3D::Translate(const glm::vec3& translation)
	{
		m_Position += translation;
		UpdateLocalTransform();
	}
	void Node3D::SetPosition(const glm::vec3& position)
	{
		m_Position = position;
		UpdateLocalTransform();
	}


	void Node3D::Rotate(const glm::vec3& rotation)
	{
		m_Rotation += rotation;
		UpdateLocalTransform();
	}
	void Node3D::SetRotation(const glm::vec3& rotation)
	{
		m_Rotation = rotation;
		UpdateLocalTransform();
	}


	void Node3D::Scale(const glm::vec3& scale)
	{
		m_Scale *= scale;
		UpdateLocalTransform();
	}
	void Node3D::SetScale(const glm::vec3& scale)
	{
		m_Scale = scale;
		UpdateLocalTransform();
	}
}
