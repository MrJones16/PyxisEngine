#include "pxpch.h"
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Pyxis
{
	PerspectiveCamera::PerspectiveCamera(float aspect, float FOV, float nearClip, float farClip)
		: Camera(),
		m_Position(0.0f, 0.0f, 0.0f), m_Rotation(0.0f, 0.0f, 0.0f)
	{
		m_Aspect = aspect;
		m_FOV = FOV;
		m_LockAspect = true;
		m_Size.x = FOV;
		m_Size.y = FOV * aspect;
		m_Near = nearClip;
		m_Far = farClip;
		if (s_MainCamera == nullptr)
		{
			s_MainCamera = this;
		}

		m_ProjectionMatrix = glm::perspective(glm::radians(FOV), aspect, nearClip, farClip);
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, m_Position);
		//transform = glm::rotate(transform, glm::radians(-m_Rotation), glm::vec3(0, 0, 1.0f));
		//scale

		m_ViewMatrix = glm::inverse(transform);
		//m_ViewMatrix = transform;
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;

		

		m_RotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.x), glm::vec3(0.0f, 1.0f, 0.0f)) *
						   glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.y), glm::vec3(1.0f, 0.0f, 0.0f)) *
						   glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		//m_RotationMatrix = glm::rotate(m_RotationMatrix, glm::radians(m_Rotation.x), { 1,0,0 });
		//m_RotationMatrix = glm::rotate(m_RotationMatrix, glm::radians(m_Rotation.y), { 0,1,0 });
		//m_RotationMatrix = glm::rotate(m_RotationMatrix, glm::radians(m_Rotation.z), { 0,0,1 });
	}

	void PerspectiveCamera::RecalculateProjectionMatrix()
	{
		m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_Aspect, m_Near, m_Far);
		RecalculateViewMatrix();
	}

	void PerspectiveCamera::RecalculateViewMatrix()
	{
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, m_Position);
		//transform = glm::rotate(transform, glm::radians(m_Rotation.x), glm::vec3(0, 1, 0));;
		//transform = glm::rotate(transform, glm::radians(m_Rotation.y), glm::vec3(0, 1, 0));;
		//transform = glm::rotate(transform, glm::radians(m_Rotation.z), glm::vec3(0, 1, 0));;
		//scale

		m_RotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.x), glm::vec3(0.0f, 1.0f, 0.0f)) *
						   glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.y), glm::vec3(1.0f, 0.0f, 0.0f)) *
						   glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		//transform = transform * m_RotationMatrix;
		//m_ViewMatrix = glm::inverse(transform);

		glm::vec3 CameraDirection = glm::vec3();
		/*CameraDirection.x = cos(glm::radians(m_Rotation.x)) * cos(glm::radians(m_Rotation.y));
		CameraDirection.y = sin(glm::radians(m_Rotation.y));
		CameraDirection.z = sin(glm::radians(m_Rotation.x)) * cos(glm::radians(m_Rotation.y));
		CameraDirection = glm::normalize(CameraDirection);*/
		CameraDirection = m_RotationMatrix * glm::vec3(0, 0, 1);
		
		
		m_ViewMatrix = glm::lookAt(m_Position, m_Position + CameraDirection, glm::vec3(0, 1, 0));
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;

	}


	OrthographicCamera::OrthographicCamera(float width = 2, float aspect = 9.0f / 16.0f, float nearClip = -100.0f, float farClip = 100.0f)
		: Camera(),
		m_Position(0.0f), m_Rotation(0.0f)
	{
		m_Size = glm::vec2(width, width * aspect);
		m_Aspect = aspect;
		m_FOV = 90;
		m_Near = nearClip;
		m_Far = farClip;

		m_ProjectionMatrix = glm::ortho(-m_Size.x / 2, m_Size.x / 2, -(m_Size.y) / 2, (m_Size.y) / 2, m_Near, m_Far);
		m_ViewMatrix = glm::mat4(1);
		m_RotationMatrix = glm::rotate(glm::mat4(1), glm::radians(-m_Rotation.z), glm::vec3(0, 0, 1.0f));
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::RecalculateProjectionMatrix()
	{
		m_ProjectionMatrix = glm::ortho(-m_Size.x / 2, m_Size.x / 2, -(m_Size.y) / 2, (m_Size.y) / 2, m_Near, m_Far);
		RecalculateViewMatrix();
	}

	void OrthographicCamera::RecalculateViewMatrix()
	{
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, m_Position);
		transform = glm::rotate(transform, glm::radians(-m_Rotation.z), glm::vec3(0,0,1.0f));
		//scale

		m_RotationMatrix = glm::rotate(glm::mat4(1), glm::radians(-m_Rotation.z), glm::vec3(0, 0, 1.0f));

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	

}