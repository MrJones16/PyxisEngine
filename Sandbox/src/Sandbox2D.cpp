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
}

void Sandbox2D::OnDetatch()
{
}

void Sandbox2D::OnUpdate(Pyxis::Timestep ts)
{
	//update
	m_OrthographicCameraController.OnUpdate(ts);

	//rendering
	
	{
		Pyxis::RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1 });
		Pyxis::RenderCommand::Clear();
		Pyxis::Renderer2D::BeginScene(m_OrthographicCameraController.GetCamera());
	}

	{
		Pyxis::Renderer2D::DrawRotatedQuad(m_TestPosition, m_TestSize, glm::radians(m_TestRotation), m_TestColor);

		for (float x = -5.0f; x < 5.0f; x += 0.5f)
		{
			for (float y = -5.0f; y < 5.0f; y += 0.5f)
			{
				Pyxis::Renderer2D::DrawQuad({ x, y , -1}, { 0.08f, 0.08f }, { (x + 5) / 10, (y + 5) / 10 , 1, 1 });
			}
		}
		Pyxis::Renderer2D::DrawQuad({ 2.0f, 0.0f, 0.0f }, { 1.0f ,1.0f }, m_TestTexture);

		Pyxis::Renderer2D::DrawQuad({ 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, m_SubTextureTest);
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
	//Pyxis::EventDispatcher dispatcher(e);
	//dispatcher.Dispatch<Pyxis::WindowResizeEvent>(PX_BIND_EVENT_FN(Sandbox2D::OnWindowResizeEvent));
}

bool Sandbox2D::OnWindowResizeEvent(Pyxis::WindowResizeEvent& event) {
	//m_OrthographicCameraController.SetAspect((float)event.GetHeight() / (float)event.GetWidth());
	//Pyxis::Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
	////Pyxis::Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
	return false;
}