#include "PixelGameServer.h"

#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include <Platform/OpenGL/OpenGLShader.h>
#include <chrono>

namespace Pyxis
{
	PixelGameServer::PixelGameServer()
		: Layer("Sandbox"),
		m_OrthographicCameraController(5, 9.0f / 16.0f, -100, 100), m_ServerInterface(60000)
	{

	}

	void PixelGameServer::OnAttach()
	{
		//m_ProfileResults = std::vector<ProfileResult>();
		Pyxis::Renderer2D::Init();

		m_ServerInterface.Start();
	}

	void PixelGameServer::OnDetatch()
	{
		m_ServerInterface.Stop();
	}

	void PixelGameServer::OnUpdate(Pyxis::Timestep ts)
	{
		//Pyxis::Renderer2D::BeginScene(m_OrthographicCameraController.GetCamera());



		//server update
		m_ServerInterface.Update(-1, true);


		//m_FrameBuffer->Unbind()
		//Pyxis::Renderer2D::EndScene();
	}

	void PixelGameServer::OnImGuiRender()
	{
		ImGui::DockSpaceOverViewport();
	}

	void PixelGameServer::OnEvent(Pyxis::Event& e)
	{
		//m_OrthographicCameraController.OnEvent(e);
		//Pyxis::EventDispatcher dispatcher(e);
		//dispatcher.Dispatch<Pyxis::WindowResizeEvent>(PX_BIND_EVENT_FN(Sandbox2D::OnWindowResizeEvent));
	}

	bool PixelGameServer::OnWindowResizeEvent(Pyxis::WindowResizeEvent& event) {
		//m_OrthographicCameraController.SetAspect((float)event.GetHeight() / (float)event.GetWidth());
		//Pyxis::Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
		////Pyxis::Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
		return false;
	}
}
