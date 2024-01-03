#include <Pyxis.h>

#include <ImGui/imgui.h>
#include "Pyxis/Renderer/Camera.h"
#include "Pyxis/Core/PerspectiveCameraController.h"
#include <glm/gtc/type_ptr.hpp>

#include "Platform/OpenGL/OpenGLShader.h"

class ExampleLayer : public Pyxis::Layer
{
private:
	Pyxis::ShaderLibrary m_ShaderLibrary;
	Pyxis::Ref<Pyxis::FrameBuffer> m_FrameBuffer;
	Pyxis::Ref<Pyxis::VertexArray> m_VertexArray, m_VertexArrayCube;
	Pyxis::Ref<Pyxis::VertexArray> m_ScreenVAO;
	Pyxis::PerspectiveCameraController m_PerspectiveCameraController;
	Pyxis::PerspectiveCamera m_Camera;
	//Pyxis::Material material;

	glm::vec3 m_CameraPosition;
	float m_CameraRotation;
	float m_CameraSpeed = 2.5f;
	float m_cameraRotationSpeed = 90.0f;

	glm::vec4 m_SquareColor = glm::vec4(1.0f);
	glm::vec3 m_SquarePosition = glm::vec3(0.0f);
	float m_SquareRotation = 0.0f;
	glm::vec3 m_SquareScale = glm::vec3(1.0f);

	Pyxis::Ref<Pyxis::Texture2D> m_Texture, m_MushroomTexture;
	Pyxis::Ref<Pyxis::Texture3D> m_Texture3D;

public:
	ExampleLayer() : Layer("Example"), m_PerspectiveCameraController(1920 / 1080, 75.0f, 0.1f, 100.0f), m_Camera(1920 / 1080, 75.0f, 0.1f, 100.0f)
	{
		m_CameraPosition = glm::vec3(0.0f, 0.0f, 5.0f);
		//m_CameraRotation = 0.0f;
		
		auto textureShader = m_ShaderLibrary.Load("Texture", "assets/shaders/Texture.glsl");
		std::dynamic_pointer_cast<Pyxis::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<Pyxis::OpenGLShader>(textureShader)->UploadUniformInt("u_TextureDiffuse", 0);

		auto SingleColorShader = m_ShaderLibrary.Load("SingleColorShader", "assets/shaders/SingleColorShader.glsl");
		std::dynamic_pointer_cast<Pyxis::OpenGLShader>(SingleColorShader)->Bind();
		std::dynamic_pointer_cast<Pyxis::OpenGLShader>(SingleColorShader)->UploadUniformFloat4("u_Color", m_SquareColor);

		auto RayMarchShader = m_ShaderLibrary.Load("RayMarch", "assets/shaders/RayMarch.glsl");
		auto Voxel = m_ShaderLibrary.Load("Voxel", "assets/shaders/Voxel.glsl");
		std::dynamic_pointer_cast<Pyxis::OpenGLShader>(Voxel)->UploadUniformInt("u_ColorTexture", 0);
		//RayMarchShader->Bind();
		//std::dynamic_pointer_cast<Pyxis::OpenGLShader>(RayMarchShader)->UploadUniformFloat2("u_Resolution", glm::vec2(1280, 720));

		m_VertexArray = Pyxis::VertexArray::Create();
		m_VertexArrayCube = Pyxis::VertexArray::Create();
		m_ScreenVAO = Pyxis::VertexArray::Create();

		float squareVertices[] =
		{
			-0.75f, -0.75f,  0.0f,  0.0f,  0.0f, 
			 0.75f, -0.75f,  0.0f,  1.0f,  0.0f,
			 0.75f,  0.75f,  0.0f,  1.0f,  1.0f,
			-0.75f,  0.75f,  0.0f,  0.0f,  1.0f
		};

		float cubeVertices[] =
		{
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
			 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

			-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
		};

		uint32_t cubeIndices[] =
		{
			0,1,2,
			3,4,5,

			6,7,8,
			9,10,11,

			12,13,14,
			15,16,17,

			18,19,20,
			21,22,23,

			24,25,26,
			27,28,29,

			30,31,32,
			33,34,35
		};

		float screenVertices[] =
		{
			-1, -1,  0,
			 1, -1,  0,
			 1,  1,  0,
			-1,  1,  0
		};

		uint32_t squareIndices[] =
		{
			0,1,2,
			0,2,3
		};

		/////////////////////
		// square
		/////////////////////

		Pyxis::Ref<Pyxis::VertexBuffer> SquareVBO;
		SquareVBO = Pyxis::VertexBuffer::Create(squareVertices, sizeof(squareVertices));
		Pyxis::BufferLayout layout = {
			{Pyxis::ShaderDataType::Float3, "a_Position"},
			{Pyxis::ShaderDataType::Float2, "a_TexCoord"},
		};
		SquareVBO->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(SquareVBO);

		Pyxis::Ref<Pyxis::IndexBuffer> indexBuffer;
		indexBuffer = Pyxis::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
		m_VertexArray->SetIndexBuffer(indexBuffer);


		/////////////////////
		// cube
		/////////////////////
		Pyxis::Ref<Pyxis::VertexBuffer> cubeVBO;
		cubeVBO = Pyxis::VertexBuffer::Create(cubeVertices, sizeof(cubeVertices));
		cubeVBO->SetLayout(layout);
		m_VertexArrayCube->AddVertexBuffer(cubeVBO);

		Pyxis::Ref<Pyxis::IndexBuffer> cubeIndexBuffer;
		cubeIndexBuffer = Pyxis::IndexBuffer::Create(cubeIndices, sizeof(cubeIndices) / sizeof(uint32_t));
		m_VertexArrayCube->SetIndexBuffer(cubeIndexBuffer);

		/////////////////////
		// screen quad
		/////////////////////
		Pyxis::Ref<Pyxis::VertexBuffer> ScreenVBO;
		ScreenVBO = Pyxis::VertexBuffer::Create(screenVertices, sizeof(screenVertices));
		Pyxis::BufferLayout positionLayout = {
			{Pyxis::ShaderDataType::Float3, "a_Position"}
		};
		ScreenVBO->SetLayout(positionLayout);
		m_ScreenVAO->AddVertexBuffer(ScreenVBO);

		Pyxis::Ref<Pyxis::IndexBuffer> screenIndexBuffer;
		screenIndexBuffer = Pyxis::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
		m_ScreenVAO->SetIndexBuffer(screenIndexBuffer);

		m_Texture = Pyxis::Texture2D::Create("assets/textures/Wall.png");
		m_MushroomTexture = Pyxis::Texture2D::Create("assets/textures/bluemush.png");

		m_FrameBuffer = Pyxis::FrameBuffer::Create(1920, 1080);
		Pyxis::Renderer::AddFrameBuffer(m_FrameBuffer);
		m_PerspectiveCameraController.SetPosition({ 0,0, 2 });
	}

	void OnUpdate(Pyxis::Timestep ts) override
	{
		Pyxis::RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1 });
		Pyxis::RenderCommand::Clear();

		
		//m_Camera.SetRotation(m_CameraRotation);
		//Pyxis::Renderer::BeginScene(m_Camera);
		m_PerspectiveCameraController.OnUpdate(ts);
		
		Pyxis::Renderer::BeginScene(m_PerspectiveCameraController.GetCamera());

		//auto textureShader = m_ShaderLibrary.Get("Texture");
		//m_Texture->Bind();
		//Pyxis::Renderer::Submit(textureShader, m_VertexArray);

		auto RayMarchShader = m_ShaderLibrary.Get("RayMarch");
		auto VoxelShader = m_ShaderLibrary.Get("Voxel");
		//auto SingleColorShader = m_ShaderLibrary.Get("SingleColorShader");
		m_FrameBuffer->Bind();
		Pyxis::RenderCommand::Clear();
		Pyxis::Renderer::Submit(RayMarchShader, m_VertexArrayCube, glm::mat4(1.0f));
		//Pyxis::Renderer::Submit(RayMarchShader, m_ScreenVAO);
		m_FrameBuffer->Unbind();

		/*m_FrameBuffer->Bind();

		Pyxis::RenderCommand::Clear();

		auto SingleColorShader = m_ShaderLibrary.Get("SingleColorShader");
		std::dynamic_pointer_cast<Pyxis::OpenGLShader>(SingleColorShader)->Bind();
		std::dynamic_pointer_cast<Pyxis::OpenGLShader>(SingleColorShader)->UploadUniformFloat4("u_Color", m_SquareColor);

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, m_SquarePosition);
		transform = glm::rotate(transform, glm::radians(-m_SquareRotation), { 0,0,1 });
		transform = glm::scale(transform, m_SquareScale);
		Pyxis::Renderer::Submit(SingleColorShader, m_VertexArray, transform);

		auto textureShader = m_ShaderLibrary.Get("Texture");
		m_Texture->Bind();
		Pyxis::Renderer::Submit(textureShader, m_VertexArray);

		m_MushroomTexture->Bind();
		Pyxis::Renderer::Submit(textureShader, m_VertexArray, glm::translate(glm::mat4(1.0f), {1.0f,0.0f,-1.0f}));


		
		Pyxis::RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1 });
		Pyxis::RenderCommand::Clear();
		Pyxis::Renderer::Submit(textureShader, m_VertexArray, glm::translate(glm::mat4(1.0f), { 1.0f,0.0f,-1.0f }));
		m_FrameBuffer->Unbind();*/

		Pyxis::Renderer::EndScene();

	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Inspector");
		if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_Framed))
		{
			ImGui::Text("Camera Transform");
			ImGui::InputFloat3("Position", glm::value_ptr(m_CameraPosition), "%.2f");
			ImGui::SliderFloat("Rotation", &m_CameraRotation, -180, 180, "%.1f");
		}
		if (ImGui::CollapsingHeader("Square Editor"))
		{
			ImGui::Text("Square Transform");
			ImGui::ColorEdit4("SquareColor", glm::value_ptr(m_SquareColor));
			ImGui::InputFloat3("SquarePosition", glm::value_ptr(m_SquarePosition), "%.2f");
			ImGui::SliderFloat("SquareRotation", &m_SquareRotation, -180, 180, "%.1f");
			ImGui::InputFloat3("SquareScale", glm::value_ptr(m_SquareScale), "%.1f");
		}
		ImGui::End();

		ImGui::Begin("Scene");
		{
			
			//ImGui::GetForegroundDrawList()->AddRect(ImVec2(0, 0), windowSize, ImU32(0xFFFFFFFF));
			ImGui::BeginChild("GameRender");
			ImVec2 windowSize = ImGui::GetContentRegionMax();
			PX_CORE_INFO("{0}, {1}", windowSize.x, windowSize.y);
			Pyxis::Renderer::OnWindowResize(windowSize.x, windowSize.y);
			ImGui::Image(
				(ImTextureID)m_FrameBuffer->GetFrameBufferTexture()->GetID(),
				ImGui::GetContentRegionAvail(),
				ImVec2(0, 1),
				ImVec2(1, 0),
				ImVec4(1, 1, 1, 1),
				ImVec4(1, 1, 1, 1)
			);
		}
		ImGui::EndChild();
		ImGui::End();
	}

	void OnEvent(Pyxis::Event& event) override
	{
		Pyxis::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Pyxis::KeyPressedEvent>(PX_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<Pyxis::MouseScrolledEvent>(PX_BIND_EVENT_FN(ExampleLayer::OnMouseScrolledEvent));
		dispatcher.Dispatch<Pyxis::WindowResizeEvent>(PX_BIND_EVENT_FN(ExampleLayer::OnWindowResizeEvent));
		//PX_TRACE("{0}", event);
	}

	bool OnKeyPressedEvent(Pyxis::KeyPressedEvent& event) {
		//PX_CORE_INFO("Pressed Key");
		return false;
	}

	bool OnMouseScrolledEvent(Pyxis::MouseScrolledEvent& event) {
		m_PerspectiveCameraController.OnMouseScrolledEvent(event);
		return false;
	}

	bool OnWindowResizeEvent(Pyxis::WindowResizeEvent& event) {
		//Pyxis::Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
		return false;
	}


};

class Sandbox : public Pyxis::Application {
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}
	~Sandbox()
	{

	}
};

Pyxis::Application* Pyxis::CreateApplication() {
	return new Sandbox();
}