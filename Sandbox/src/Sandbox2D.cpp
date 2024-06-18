#include "Sandbox2D.h"

#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include <Platform/OpenGL/OpenGLShader.h>
#include <chrono>



Sandbox2D::Sandbox2D()
	: Layer("Sandbox"), 
	m_OrthographicCameraController(5, 9.0f / 16.0f, -100, 100)
{

}

void Sandbox2D::OnAttach()
{
	//m_ProfileResults = std::vector<ProfileResult>();
	Pyxis::Renderer2D::Init();

	m_ClientInterface.Connect("127.0.0.1", 60000);

}

void Sandbox2D::OnDetatch()
{
	m_ClientInterface.Disconnect();
}

void Sandbox2D::OnUpdate(Pyxis::Timestep ts)
{
	//update
	m_OrthographicCameraController.OnUpdate(ts);



	//update the client connection
	if (m_ClientInterface.IsConnected())
	{
		if (!m_ClientInterface.Incoming().empty())
		{
			auto msg = m_ClientInterface.Incoming().pop_front().msg;
			switch (msg.header.id)
			{
			case Pyxis::Network::CustomMessageTypes::ServerAccept:
			{
				//server has responded to a ping request
				PX_TRACE("Server Accepted Connection");
				
			}
			break;
			case Pyxis::Network::CustomMessageTypes::ServerPing:
			{
				std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
				std::chrono::system_clock::time_point timeThen;
				msg >> timeThen;
				PX_TRACE("Ping: {0}", std::chrono::duration<double>(timeNow - timeThen).count());
			}
			break;
			case Pyxis::Network::CustomMessageTypes::ServerMessage:
			{
				uint32_t clientID;
				msg >> clientID;
				PX_TRACE("Hello from [{0}]", clientID);
			}
			break;
			}
		}
	}
	else
	{
		//lost connection to server
		PX_INFO("Server Down");
		Pyxis::Application::Get().Close();
	}
	
	

	//rendering
	
	{
		Pyxis::RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1 });
		Pyxis::RenderCommand::Clear();
		Pyxis::Renderer2D::BeginScene(m_OrthographicCameraController.GetCamera());
	}

	{

		for (float x = -5.0f; x < 5.0f; x += 0.25f)
		{
			for (float y = -5.0f; y < 5.0f; y += 0.25f)
			{
				Pyxis::Renderer2D::DrawQuad({ x, y , -1}, { 0.08f, 0.08f }, { (x + 5) / 10, (y + 5) / 10 , 1, 1 });
			}
		}
	}

	//m_FrameBuffer->Unbind()
	Pyxis::Renderer2D::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	
}

void Sandbox2D::OnEvent(Pyxis::Event& e)
{
	//m_OrthographicCameraController.OnEvent(e);
	Pyxis::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Pyxis::KeyPressedEvent>(PX_BIND_EVENT_FN(Sandbox2D::OnKeyPressedEvent));
	//dispatcher.Dispatch<Pyxis::WindowResizeEvent>(PX_BIND_EVENT_FN(Sandbox2D::OnWindowResizeEvent));
}

bool Sandbox2D::OnKeyPressedEvent(Pyxis::KeyPressedEvent& event) {
	if (event.GetKeyCode() == PX_KEY_P)
	{
		m_ClientInterface.PingServer();
	}
	if (event.GetKeyCode() == PX_KEY_M)
	{
		m_ClientInterface.MessageAll();

	}
	//m_OrthographicCameraController.SetAspect((float)event.GetHeight() / (float)event.GetWidth());
	//Pyxis::Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
	////Pyxis::Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
	return false;
}

bool Sandbox2D::OnWindowResizeEvent(Pyxis::WindowResizeEvent& event) {
	//m_OrthographicCameraController.SetAspect((float)event.GetHeight() / (float)event.GetWidth());
	//Pyxis::Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
	////Pyxis::Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
	return false;
}