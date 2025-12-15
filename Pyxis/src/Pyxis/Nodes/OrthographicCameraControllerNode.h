#pragma once

#include <Pyxis/Core/Input.h>
#include "Pyxis/Nodes/PixelCameraNode.h"
#include <Pyxis/Events/EventSignals.h>

namespace Pyxis
{
	
	/// <summary>
	/// A Base UI Node that can act as a root
	/// </summary>
	class OrthographicCameraControllerNode : public PixelCameraNode
	{
	public:
		float m_CameraSpeed = 64;
		float m_Sensitivity = 0.5f;

		Reciever<void(MouseScrolledEvent&)> m_MouseScrolledReciever;
		
	public:
		OrthographicCameraControllerNode(const std::string& name = "OrthographicCameraControllerNode") :
			PixelCameraNode(name),
			m_MouseScrolledReciever(this, &OrthographicCameraControllerNode::OnMouseScrolledEvent)
		{
			EventSignal::s_MouseScrolledEventSignal.AddReciever(m_MouseScrolledReciever);
		};

		OrthographicCameraControllerNode(UUID id) :
			PixelCameraNode(id),
			m_MouseScrolledReciever(this, &OrthographicCameraControllerNode::OnMouseScrolledEvent)
		{
			EventSignal::s_MouseScrolledEventSignal.AddReciever(m_MouseScrolledReciever);
		};


		virtual ~OrthographicCameraControllerNode() = default;

		//Serialization
		virtual void Serialize(json& j)
		{
			PixelCameraNode::Serialize(j);
			j["Type"] = "OrthographicCameraControllerNode"; // Override type identifier
			j["m_CameraSpeed"] = m_CameraSpeed;
			j["m_Sensitivity"] = m_Sensitivity;
		}

		virtual void Deserialize(json& j)
		{
			PixelCameraNode::Deserialize(j);
			if (j.contains("m_CameraSpeed")) j.at("m_CameraSpeed").get_to(m_CameraSpeed);
			if (j.contains("m_Sensitivity")) j.at("m_Sensitivity").get_to(m_Sensitivity);
		}

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
	
	REGISTER_SERIALIZABLE_NODE(OrthographicCameraControllerNode);
}