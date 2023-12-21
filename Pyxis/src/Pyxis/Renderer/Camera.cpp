#include "pxpch.h"
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Pyxis
{
	OrthographicCamera::OrthographicCamera(float width = 2, float height = 2, float nearClip = -100.0f, float farClip = 100.0f)
		: m_ProjectionMatrix(glm::ortho(-width / 2, width / 2, -height / 2, height / 2, nearClip, farClip)),
		m_ViewMatrix(1.0f), m_Position(0.0f),
		m_Width(width), m_Height(height), m_Near(nearClip), m_Far(farClip)
	{
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::RecalculateProjectionMatrix()
	{
		m_ProjectionMatrix = glm::ortho(-m_Width / 2, m_Width / 2, -m_Height / 2, m_Height / 2, m_Near, m_Far);
		RecalculateViewMatrix();
	}

	void OrthographicCamera::RecalculateViewMatrix()
	{
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, m_Position);
		transform = glm::rotate(transform, glm::radians(-m_Rotation), glm::vec3(0,0,1.0f));
		//scale

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}