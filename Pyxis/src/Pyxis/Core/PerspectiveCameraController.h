#pragma once
#include "Pyxis/Renderer/Camera.h"
#include "Pyxis/Core/Core.h"
#include "Pyxis/Core/Timestep.h"
#include "Pyxis/Core/Input.h"
#include "Pyxis/Core/InputCodes.h"
#include "Pyxis/Events/MouseEvent.h"
#include <glm/gtc/matrix_transform.hpp>

//currently not being used

namespace Pyxis
{
	class PerspectiveCameraController
	{
	public:
		PerspectiveCameraController(float aspect, float FOV, float nearClip, float farClip)
			: m_Camera(aspect, FOV, nearClip, farClip)
		{
			m_Camera.SetPosition({ 0, 0, -5 });
			m_Right   = { 1,0,0 };
			m_Up      = { 0,1,0 };
			m_Forward = { 0,0,1 };
			m_FOV = FOV;
			m_Yaw = 0;
			m_Pitch = 0;
			m_Roll = 0;
			m_CameraDirection = { 0,0,0 };
			m_CameraDirection.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
			m_CameraDirection.y = sin(glm::radians(m_Pitch));
			m_CameraDirection.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
			m_CameraDirection = glm::normalize(m_CameraDirection);
			m_Camera.SetRotation(glm::vec3(m_Yaw, m_Pitch, m_Roll));
			lastMousePos = {0,0};
		}

		PerspectiveCamera GetCamera() { return m_Camera; }

		void OnUpdate(Timestep ts)
		{
			//camera turning
			auto [mouseX, mouseY] = Input::GetMousePosition();

			float xDiff = mouseX - lastMousePos.x;
			float yDiff = mouseY - lastMousePos.y;
			lastMousePos = glm::vec2(mouseX, mouseY);
			if (Input::IsMouseButtonPressed(PX_MOUSE_BUTTON_RIGHT)) {
				m_Yaw += xDiff * m_Sensitivity;
				m_Pitch += yDiff * m_Sensitivity;
				if (m_Pitch > 89) m_Pitch = 89;
				if (m_Pitch < -89) m_Pitch = -89;
				m_Roll += 0;
				m_CameraDirection.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
				m_CameraDirection.y = sin(glm::radians(m_Pitch));
				m_CameraDirection.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
				m_CameraDirection = glm::normalize(m_CameraDirection);
				m_Camera.SetRotation(glm::vec3(-m_Yaw, m_Pitch, m_Roll));
			}

			glm::mat3 RotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-m_Yaw), glm::vec3(0.0f, 1.0f, 0.0f)) *
									   glm::rotate(glm::mat4(1.0f), glm::radians(m_Pitch), glm::vec3(1.0f, 0.0f, 0.0f)) *
									   glm::rotate(glm::mat4(1.0f), glm::radians(m_Roll), glm::vec3(0.0f, 0.0f, 1.0f));
			//movement
			glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);

			if (Pyxis::Input::IsKeyPressed(PX_KEY_W)) {
				direction += RotationMatrix * glm::vec3(0, 0, 1);
			}
			if (Pyxis::Input::IsKeyPressed(PX_KEY_A)) {
				direction += RotationMatrix * glm::vec3(1, 0, 0);
			}
			if (Pyxis::Input::IsKeyPressed(PX_KEY_S)) {
				direction -= RotationMatrix * glm::vec3(0, 0, 1);
			}
			if (Pyxis::Input::IsKeyPressed(PX_KEY_D)) {
				direction -= RotationMatrix * glm::vec3(1, 0, 0);
			}
			if (Pyxis::Input::IsKeyPressed(PX_KEY_SPACE)) {
				direction += RotationMatrix * glm::vec3(0, 1, 0);
			}
			if (Pyxis::Input::IsKeyPressed(PX_KEY_LEFT_SHIFT)) {
				direction -= RotationMatrix * glm::vec3(0, 1, 0);
			}

			//if (direction.x != 0 || direction.y != 0 || direction.z != 0)
				//direction = glm::normalize(direction);
			m_Camera.SetPosition(m_Camera.GetPosition() + direction * m_CameraSpeed * ts.GetSeconds());
		}

		void SetPosition(glm::vec3 position)
		{
			m_Camera.SetPosition(position);
		}

		glm::vec3 Right()
		{
			//return glm::vec3(m_Camera.GetRotationMatrix() * glm::vec4(1,0,0,1));
			return glm::cross(m_CameraDirection, { 0,1,0 });
		}

		glm::vec3 Up()
		{
			//return glm::vec3(m_Camera.GetRotationMatrix() * glm::vec4(0,1,0,1));
			return { 0,1,0 };
		}

		glm::vec3 Forward()
		{
			return m_CameraDirection;
			//return glm::vec3(m_Camera.GetRotationMatrix() * glm::vec4(0,0,1,1));
		}

		bool OnImGuiRender()
		{

		}

		void OnMouseScrolledEvent(MouseScrolledEvent& event)
		{
			if (event.GetYOffset() < 0)
			{
				if (Pyxis::Input::IsKeyPressed(PX_KEY_LEFT_SHIFT))
				{
					PX_CORE_INFO("SlowerCameraSpeed");
					m_CameraSpeed *= 0.9f;
					if (m_CameraSpeed < 2.5f) m_CameraSpeed = 2.5f;

				}
				else
				{
					PX_CORE_INFO("FasterCameraSpeed");
					m_CameraSpeed *= 1.1f;
					PX_CORE_INFO("Wider Cam");
					//float width = m_Camera.GetWidth();
					float FOV = m_Camera.GetFOV();
					FOV *= 1.1f;
					if (FOV > 170) FOV = 170;
					
					//m_Camera.SetWidth(width);
					//m_Camera.SetHeight(width * 9 / 16);
					m_Camera.SetFOV(FOV);
				}
			}
			else
			{
				if (Pyxis::Input::IsKeyPressed(PX_KEY_LEFT_SHIFT))
				{
					PX_CORE_INFO("FasterCameraSpeed");
					m_CameraSpeed *= 1.1f;
				}
				else {
					PX_CORE_INFO("SlowerCameraSpeed");
					m_CameraSpeed *= 0.9f;
					if (m_CameraSpeed < 2.5f) m_CameraSpeed = 2.5f;
					PX_CORE_INFO("Smaller Cam");
					//float width = m_Camera.GetWidth();
					float FOV = m_Camera.GetFOV();
					FOV *= 0.9f;
					if (FOV < 25.0f) FOV = 25.0f;
					//m_Camera.SetWidth(width);
					//m_Camera.SetHeight(width * 9 / 16);
					m_Camera.SetFOV(FOV);
				}
			}
		}

	private:
		PerspectiveCamera m_Camera;
		glm::vec3 m_Right;
		glm::vec3 m_Up;
		glm::vec3 m_Forward;

		glm::vec2 lastMousePos;

		float m_FOV;

		float m_Yaw;
		float m_Pitch;
		float m_Roll;

		glm::vec3 m_CameraDirection;

		float m_CameraSpeed = 1;
		float m_Sensitivity = 0.5f;
	};
}