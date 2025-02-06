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
			glm::mat4 RotationMatrix = GetRotationMatrix();
			//movement
			glm::vec4 direction = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

			if (Pyxis::Input::IsKeyPressed(PX_KEY_W)) {
				direction += RotationMatrix * glm::vec4(0, 1, 0, 1);
			}
			if (Pyxis::Input::IsKeyPressed(PX_KEY_A)) {
				direction -= RotationMatrix * glm::vec4(1, 0, 0, 1);
			}
			if (Pyxis::Input::IsKeyPressed(PX_KEY_S)) {
				direction -= RotationMatrix * glm::vec4(0, 1, 0, 1);
			}
			if (Pyxis::Input::IsKeyPressed(PX_KEY_D)) {
				direction += RotationMatrix * glm::vec4(1, 0, 0, 1);
			}
			
			Translate((glm::vec3)direction * m_CameraSpeed * ts.GetSeconds());

		}

	};
	

}