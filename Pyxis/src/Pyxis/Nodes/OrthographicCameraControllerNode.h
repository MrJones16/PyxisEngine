#pragma once

//#include <Pyxis/Renderer/Renderer2D.h>
//#include <glm/gtc/matrix_transform.hpp>
#include <Pyxis/Core/Input.h>

#include "Pyxis/Nodes/CameraNode.h"
#include <Pyxis/Events/EventSignals.h>

namespace Pyxis
{
	
	/// <summary>
	/// A Base UI Node that can act as a root
	/// </summary>
	class OrthographicCameraControllerNode : public CameraNode
	{
	public:
		float m_CameraSpeed = 0.5;
		float m_Sensitivity = 0.5f;

		Reciever<void(MouseScrolledEvent&)> m_MouseScrolledReciever;
		
	public:
		OrthographicCameraControllerNode(const std::string& name = "OrthographicCameraControllerNode") :
			CameraNode(name),
			m_MouseScrolledReciever(this, &OrthographicCameraControllerNode::OnMouseScrolledEvent)
		{
			EventSignal::s_MouseScrolledEventSignal.AddReciever(m_MouseScrolledReciever);
		};


		virtual ~OrthographicCameraControllerNode() = default;

		void OnMouseScrolledEvent(MouseScrolledEvent& e)
		{
			if (!Input::IsKeyPressed(PX_KEY_LEFT_CONTROL))
			{
				Zoom(1 - (e.GetYOffset() / 10));
			}
		}

		void Zoom(float multiplier)
		{
			SetWidth(m_Size.x * multiplier);
			SetHeight(m_Size.y * multiplier);
		}

		inline virtual void OnUpdate(Timestep ts) override
		{
			glm::mat3 RotationMatrix = GetRotationMatrix();
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

			Translate(direction * m_CameraSpeed * ts.GetSeconds());

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
			//	direction = glm::normalize(direction);
		}

	};
	

}