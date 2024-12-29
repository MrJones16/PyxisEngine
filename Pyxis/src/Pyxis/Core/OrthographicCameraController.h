#pragma once
#include "Pyxis/Renderer/Camera.h"
#include "Pyxis/Core/Core.h"
#include "Pyxis/Core/Timestep.h"
#include "Pyxis/Core/Input.h"
#include "Pyxis/Core/InputCodes.h"
#include "Pyxis/Events/MouseEvent.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Pyxis
{
	class OrthographicCameraController
	{
	public:
		float m_ZoomLevel = 1;
		float m_CameraSpeed = 0.5;
		float m_Sensitivity = 0.5f;

		OrthographicCameraController(float width, float aspect, float nearClip, float farClip)
			: m_Camera(CreateRef<OrthographicCamera>(width, aspect, nearClip, farClip))
		{
			m_Camera->SetPosition({ 0, 0, -5 });
			m_Camera->SetRotation({ 0,0,0 });
			m_Camera->SetWidth(width);
			m_Camera->SetHeight(width * aspect);
			m_Size = { width, width * aspect };
			m_ZoomLevel = 1;
			m_CameraDirection = m_Camera->GetRotationMatrix() * glm::vec3(0, 0, 1);
		}

		Ref<OrthographicCamera> GetCamera() { return m_Camera; }

		void OnUpdate(Timestep ts)
		{

			glm::mat3 RotationMatrix = m_Camera->GetRotationMatrix();
			//movement
			glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);

			//PX_CORE_INFO("angle is {0}", m_Camera->GetRotation().z);

			if (Pyxis::Input::IsKeyPressed(PX_KEY_W)) {
				direction += RotationMatrix * glm::vec3(0, 1, 0);
			}
			if (Pyxis::Input::IsKeyPressed(PX_KEY_A)) {
				direction -= RotationMatrix * glm::vec3(1, 0, 0);
			}
			if (Pyxis::Input::IsKeyPressed(PX_KEY_S)) {
				direction -= RotationMatrix * glm::vec3(0, 1, 0);
			}
			if (Pyxis::Input::IsKeyPressed(PX_KEY_D)) {
				direction += RotationMatrix * glm::vec3(1, 0, 0);
			}

			//rotation
			/*float angle = 0;
			BYTE center = 0;
			if (Pyxis::Input::IsKeyPressed(PX_KEY_Q)) {
				angle -= 30;
				center++;
			}
			if (Pyxis::Input::IsKeyPressed(PX_KEY_E)) {
				angle += 30;
				center++;
			}
			if (center == 2)
			{
				m_Camera->SetRotation(glm::vec3(0));
			}
			else m_Camera->SetRotation(m_Camera->GetRotation() + glm::vec3(0,0, angle * ts.GetSeconds()));*/

			//if (direction.x != 0 || direction.y != 0 || direction.z != 0)
				//direction = glm::normalize(direction);
			m_Camera->SetPosition(m_Camera->GetPosition() + direction * m_CameraSpeed * ts.GetSeconds());
		}

		glm::vec4 MousePercentToWorldPos(float x, float y)
		{
			Window& window = Application::Get().GetWindow();
			//from 0->1 to -1 -> 1
			x = (x - 0.5f) * 2.0f;
			y = (y - 0.5f) * 2.0f;

			x *= m_Camera->GetWidth() / 2;
			y *= m_Camera->GetHeight() / 2;

			glm::vec4 vec = glm::vec4(x, -y, 0, 1);
			vec = glm::translate(glm::mat4(1), m_Camera->GetPosition()) * vec;
			return vec;
		}


		/// <summary>
		/// 
		/// </summary>
		/// <param name="aspectRatio">height / width!</param>
		void SetAspect(float aspectRatio)
		{
			m_Camera->SetAspect(aspectRatio);
		}

		void OnResize(float width, float height)
		{
			PX_CORE_INFO("resizing camera");
			m_Size = { width, height };
			m_Camera->SetWidth(width * m_ZoomLevel);
			m_Camera->SetAspect(height / width);
			//m_Camera->SetHeight(height);
		}

		void SetPosition(glm::vec3 position)
		{
			m_Camera->SetPosition(position);
		}
		glm::vec3 GetPosition()
		{
			return m_Camera->GetPosition();
		}

		void SetRotation(glm::vec3 rotation)
		{
			m_Camera->SetRotation(rotation);
		}

		glm::mat4 GetViewProjectionMatrix()
		{
			return m_Camera->GetViewProjectionMatrix();
		}

		glm::vec3 Right()
		{
			//return glm::vec3(m_Camera->GetRotationMatrix() * glm::vec4(1,0,0,1));
			return glm::cross(m_CameraDirection, { 0,1,0 });
		}

		glm::vec3 Up()
		{
			//return glm::vec3(m_Camera->GetRotationMatrix() * glm::vec4(0,1,0,1));
			return { 0,1,0 };
		}

		glm::vec3 Forward()
		{
			return m_CameraDirection;
			//return glm::vec3(m_Camera->GetRotationMatrix() * glm::vec4(0,0,1,1));
		}

		void Zoom(float multiplyBy)
		{
			m_ZoomLevel *= multiplyBy;
			m_Camera->SetWidth(m_Size.x * m_ZoomLevel);
			m_Camera->SetHeight((m_Size.x * m_ZoomLevel) * m_Camera->GetAspect());
		}


		void OnEvent(Event& event)
		{
			if (event.GetEventType() == EventType::MouseScrolled)
			{
				OnMouseScrolledEvent((MouseScrolledEvent&)event);

			}
		}

		void OnMouseScrolledEvent(MouseScrolledEvent& event)
		{
			if (event.GetYOffset() < 0)
			{
				if (Pyxis::Input::IsKeyPressed(PX_KEY_LEFT_SHIFT))
				{
					//PX_CORE_INFO("SlowerCameraSpeed");
					m_CameraSpeed *= 0.9f;
					if (m_CameraSpeed < 2.5f) m_CameraSpeed = 2.5f;

				}
				else
				{
					//PX_CORE_INFO("FasterCameraSpeed");
					m_CameraSpeed *= 1.1f;
					//PX_CORE_INFO("Wider Cam");

					//camera size
					//float width = m_Camera->GetWidth();
					m_ZoomLevel *= 1.1f;
					m_Camera->SetWidth(m_Size.x * m_ZoomLevel);
					m_Camera->SetHeight((m_Size.x * m_ZoomLevel) * m_Camera->GetAspect());
				}
			}
			else
			{
				if (Pyxis::Input::IsKeyPressed(PX_KEY_LEFT_SHIFT))
				{
					//PX_CORE_INFO("FasterCameraSpeed");
					m_CameraSpeed *= 1.1f;
					if (m_CameraSpeed > 2.0f) m_CameraSpeed = 2.0f;
				}
				else {
					//PX_CORE_INFO("SlowerCameraSpeed");
					m_CameraSpeed *= 0.9f;
					if (m_CameraSpeed < 0.1f) m_CameraSpeed = 0.1f;
					//PX_CORE_INFO("Smaller Cam");

					//camera size
					m_ZoomLevel *= 0.9f;
					m_Camera->SetWidth(m_Size.x * m_ZoomLevel);
					m_Camera->SetHeight((m_Size.x * m_ZoomLevel) * m_Camera->GetAspect());
				}
			}
		}

	private:
		Ref<OrthographicCamera> m_Camera;

		glm::vec3 m_CameraDirection;

		glm::vec2 m_Size;

		
	};
}