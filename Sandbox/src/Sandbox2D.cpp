#include "Sandbox2D.h"

#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include <Platform/OpenGL/OpenGLShader.h>


Sandbox2D::Sandbox2D()
	: Layer("Sandbox"), 
	m_OrthographicCameraController(5, 9.0f / 16.0f, -100, 100)
{

}

void Sandbox2D::OnAttach()
{
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
	Pyxis::Renderer2D::BeginScene(m_OrthographicCameraController.GetCamera());
	//m_FrameBuffer->Bind();
	Pyxis::RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1 });
	Pyxis::RenderCommand::Clear();

	//m_SingleColorShader->SetFloat4("u_Color", m_TestColor);

	Pyxis::Renderer2D::DrawQuad({ 0.0f,0.0f }, { 1.0f,1.0f }, m_TestColor);
	//Pyxis::Renderer::Submit(m_SingleColorShader, m_SquareVertexArray, glm::mat4(1));

	//m_FrameBuffer->Unbind()
	Pyxis::Renderer2D::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Testing");

	ImGui::ColorEdit4("SquareColor", glm::value_ptr(m_TestColor));
	ImGui::End();

	//ImGui::Begin("Scene");
	//{
	//	//ImGui::GetForegroundDrawList()->AddRect(ImVec2(0, 0), windowSize, ImU32(0xFFFFFFFF));
	//	ImGui::BeginChild("GameRender");
	//	ImVec2 windowSize = ImGui::GetContentRegionMax();

	//	m_OrthographicCameraController.SetAspect(windowSize.y / windowSize.x);
	//	//PX_CORE_INFO("{0}, {1}", windowSize.x, windowSize.y);
	//	Pyxis::Renderer::OnWindowResize(windowSize.x, windowSize.y);
	//	ImGui::Image(
	//		(ImTextureID)m_FrameBuffer->GetFrameBufferTexture()->GetID(),
	//		ImGui::GetContentRegionAvail(),
	//		ImVec2(0, 1),
	//		ImVec2(1, 0),
	//		ImVec4(1, 1, 1, 1)
	//		//ImVec4(1, 1, 1, 1) border color
	//	);
	//}
	//ImGui::EndChild();
	//ImGui::End();
}

void Sandbox2D::OnEvent(Pyxis::Event& e)
{
	m_OrthographicCameraController.OnEvent(e);
}
