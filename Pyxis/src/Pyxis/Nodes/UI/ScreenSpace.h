#pragma once
#include <Pyxis/Nodes/UI/UIRect.h>
#include <Pyxis/Renderer/Camera.h>
#include <Pyxis/Events/EventSignals.h>
#include <Pyxis/Core/Application.h>

namespace Pyxis
{
	namespace UI
	{

		/// <summary>
		/// This class has the m_size of the screen, and scales all of it's children down to fit into it. 
		/// 
		/// so if you want to fill the bottom of the screen, it would be a rect with size {1920, 540 (50%)}, and translate it down by 0.25?
		/// 
		/// the Z axis in screen space is inverted, so a negative value is closer / viewed first
		/// </summary>
		class ScreenSpace : public UIRect
		{
		private:
			Reciever<void(WindowResizeEvent&)> m_WindowResizeReciever;
		public:
			ScreenSpace(const std::string& name = "ScreenSpace") : 
				UIRect(name),
				m_WindowResizeReciever(this, &ScreenSpace::OnWindowResizeEvent)
			{
				EventSignal::s_WindowResizeEventSignal.AddReciever(m_WindowResizeReciever);
				auto& window = Application::Get().GetWindow();
				m_Size = { window.GetWidth(), window.GetHeight() };
				m_Position = { 0,0,-0.5f };
				SetScale((glm::vec3(1) / glm::vec3(window.GetWidth() / 2.0f, window.GetHeight() / 2.0f, 1)));
			}

			ScreenSpace(UUID id) : UIRect(id),
				m_WindowResizeReciever(this, &ScreenSpace::OnWindowResizeEvent)
			{
				EventSignal::s_WindowResizeEventSignal.AddReciever(m_WindowResizeReciever);
				auto& window = Application::Get().GetWindow();
				m_Size = { window.GetWidth(), window.GetHeight() };
				m_Position = { 0,0,-0.5f };
				SetScale((glm::vec3(1) / glm::vec3(window.GetWidth() / 2.0f, window.GetHeight() / 2.0f, 1)));
			}

			virtual void OnRender() override
			{

			}

			//Serialize is the same as UIRect
			 
			//Deserialze
			virtual void Deserialize(json& j) override
			{
				UIRect::Deserialize(j);
				PropagateUpdate();
			}

			void OnWindowResizeEvent(WindowResizeEvent& e)
			{
				m_Size = { e.GetWidth(), e.GetHeight() };
				SetScale((glm::vec3(1) / glm::vec3(e.GetWidth() / 2.0f, e.GetHeight() / 2.0f, 1)));
				PropagateUpdate();
			}

			glm::mat4 GetWorldTransform() override
			{
				if (Camera::Main() != nullptr)
				{
					return glm::inverse(Camera::Main()->GetViewProjectionMatrix()) * m_LocalTransform;
				}
				else
				{
					return m_LocalTransform;
				}
			}


		};
		REGISTER_SERIALIZABLE_NODE(ScreenSpace);
	}
}
