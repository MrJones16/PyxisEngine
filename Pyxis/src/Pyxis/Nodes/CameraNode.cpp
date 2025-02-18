#include "CameraNode.h"

namespace Pyxis
{
	void CameraNode::Serialize(json& j)
	{
		Node3D::Serialize(j);
		j["Type"] = "CameraNode"; // Override type identifier
		j["m_Size"] = m_Size;
		j["m_Aspect"] = m_Aspect;
		j["m_FOV"] = m_FOV;
		j["m_Near"] = m_Near;
		j["m_Far"] = m_Far;
	}
	void CameraNode::Deserialize(json& j)
	{
		// Deserialize base class (Node3D)
		Node3D::Deserialize(j);

		// Read CameraNode-specific properties
		if (j.contains("m_Size")) j.at("m_Size").get_to(m_Size);
		if (j.contains("m_Aspect")) j.at("m_Aspect").get_to(m_Aspect);
		if (j.contains("m_FOV")) j.at("m_FOV").get_to(m_FOV);
		if (j.contains("m_Near")) j.at("m_Near").get_to(m_Near);
		if (j.contains("m_Far")) j.at("m_Far").get_to(m_Far);

		//Recalculate projection mat with new data & for init
		RecalculateProjectionMatrix();
	}
	glm::vec2 CameraNode::MouseToWorldPos(glm::vec2 mousePos)
	{
		Window& window = Application::Get().GetWindow();
		mousePos.x /= (float)window.GetWidth();
		mousePos.y /= (float)window.GetHeight();
		//from 0->1 to -1 -> 1
		mousePos = (mousePos - 0.5f) * 2.0f;
		mousePos *= (m_Size / 2.0f);

		glm::vec4 vec = glm::vec4(mousePos.x, -mousePos.y, 0, 1);
		vec = glm::translate(glm::mat4(1), m_Position) * vec;
		return vec;
	}

	void CameraNode::RecalculateProjectionMatrix()
	{
		float height = m_FOV * m_Aspect;
		m_ProjectionMatrix = glm::ortho(-m_Size.x / 2, m_Size.x / 2, -m_Size.y / 2, m_Size.y / 2, m_Near, m_Far);
		RecalculateViewMatrix();
	}

	void CameraNode::RecalculateViewMatrix()
	{
		m_ViewMatrix = glm::inverse(GetWorldTransform());
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	const glm::mat3 CameraNode::GetRotationMatrix() const
	{
		return	glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.x), glm::vec3(-1.0f, 0.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.y), glm::vec3(0.0f, -1.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, -1.0f));
	};

	//void CameraNode::ResetLocalTransform()
	//{

	//}

	//void CameraNode::SetLocalTransform(const glm::mat4& transform)
	//{
	//}

	//void CameraNode::Translate(glm::vec3 translation)
	//{
	//	Node(translation);
	//	RecalculateViewMatrix();
	//}

	//void CameraNode::Rotate(glm::vec3 rotation)
	//{
	//	m_Rotation += rotation;
	//	m_LocalTransform = glm::rotate(m_LocalTransform, m_Rotation.x, { 1,0,0 });
	//	m_LocalTransform = glm::rotate(m_LocalTransform, m_Rotation.y, { 0,1,0 });
	//	m_LocalTransform = glm::rotate(m_LocalTransform, m_Rotation.z, { 0,0,1 });

	//	/*m_RotationMatrix =	glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.x), glm::vec3(-1.0f, 0.0f, 0.0f)) *
	//						glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.y), glm::vec3( 0.0f,-1.0f, 0.0f)) *
	//						glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.z), glm::vec3( 0.0f, 0.0f,-1.0f));*/
	//}

	//void CameraNode::Scale(glm::vec3 scale)
	//{
	//}

}