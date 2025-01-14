#include "CameraNode.h"

namespace Pyxis
{
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

	const glm::mat3& CameraNode::GetRotationMatrix() const
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