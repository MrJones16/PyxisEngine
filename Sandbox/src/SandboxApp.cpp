#include <Pyxis.h>

#include <ImGui/imgui.h>
#include "Pyxis/Renderer/Camera.h"
#include <glm/gtc/type_ptr.hpp>

class ExampleLayer : public Pyxis::Layer
{
private:
	std::shared_ptr<Pyxis::Shader> m_SingleColorShader;
	std::shared_ptr<Pyxis::Shader> m_Shader;

	std::shared_ptr<Pyxis::VertexArray> m_VertexArray;
	std::shared_ptr<Pyxis::VertexArray> m_SquareVertexArray;
	Pyxis::OrthographicCamera m_Camera;

	glm::vec3 m_CameraPosition;
	float m_CameraRotation;
	float m_CameraSpeed = 2.5f;
	float m_cameraRotationSpeed = 90.0f;

	glm::vec4 m_SquareColor = glm::vec4(1.0f);
	glm::vec3 m_SquarePosition = glm::vec3(0.0f);
	float m_SquareRotation = 0.0f;
	glm::vec3 m_SquareScale = glm::vec3(1.0f);

public:
	ExampleLayer() : Layer("Example"), m_Camera(3.2f, 1.8f, -1.0f, 1.0f)
	{
		m_CameraPosition = glm::vec3(0.0f);
		m_CameraRotation = 0.0f;
		std::string vertexSource = R"(
			#version 460
			
			layout (location = 0) in vec3 a_Position;
			layout (location = 1) in vec4 a_Color;

			out vec4 v_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			void main()
			{
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0f);
				v_Color = a_Color;
			}
		)";
		std::string positionvertexSource = R"(
			#version 460
			
			layout (location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			void main()
			{
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0f);
			}
		)";
		std::string fragmentSource = R"(
			#version 460
			
			layout (location = 0) out vec4 color;
			in vec4 v_Color;

			void main()
			{
				color = v_Color;
			}
		)";

		std::string singlecolorfragmentSource = R"(
			#version 460
			
			layout (location = 0) out vec4 color;

			uniform vec4 u_Color;

			void main()
			{
				color = u_Color;
			}
		)";
		m_SingleColorShader.reset(Pyxis::Shader::Create(positionvertexSource, singlecolorfragmentSource));
		m_Shader.reset(Pyxis::Shader::Create(vertexSource, fragmentSource));

		m_VertexArray.reset(Pyxis::VertexArray::Create());

		float vertices[] =
		{
			-0.5f, -0.5f,  0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			 0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			 0.0f,  0.5f,  0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		};
		std::shared_ptr<Pyxis::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Pyxis::VertexBuffer::Create(vertices, sizeof(vertices)));
		Pyxis::BufferLayout layout = {
			{Pyxis::ShaderDataType::Float3, "a_Position"},
			{Pyxis::ShaderDataType::Float4, "a_Color"},
		};
		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		//create indices and gen/bind element(index) buffer
		uint32_t indices[3] =
		{
			0,1,2
		};
		std::shared_ptr<Pyxis::IndexBuffer> indexBuffer;
		indexBuffer.reset(Pyxis::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		//making a square
		float squareVertices[] =
		{
			-0.75f, -0.75f,  0.0f,
			 0.75f, -0.75f,  0.0f,
			 0.75f,  0.75f,  0.0f,
			-0.75f,  0.75f,  0.0f
		};
		uint32_t squareIndices[] =
		{
			0,1,2,
			0,2,3
		};

		m_SquareVertexArray.reset(Pyxis::VertexArray::Create());
		std::shared_ptr<Pyxis::VertexBuffer> squareVB;
		squareVB.reset(Pyxis::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		Pyxis::BufferLayout squareLayout = {
			{Pyxis::ShaderDataType::Float3, "a_Position"}
		};
		squareVB->SetLayout(squareLayout);
		m_SquareVertexArray->AddVertexBuffer(squareVB);

		std::shared_ptr<Pyxis::IndexBuffer> squareIB;
		squareIB.reset(Pyxis::IndexBuffer::Create(squareIndices, 6));
		m_SquareVertexArray->SetIndexBuffer(squareIB);
	}

	void OnUpdate(Pyxis::Timestep ts) override
	{
		Pyxis::RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1 });
		Pyxis::RenderCommand::Clear();

		glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
		
		if (Pyxis::Input::IsKeyPressed(PX_KEY_W)) {
			direction.y += 1;
		}
		if (Pyxis::Input::IsKeyPressed(PX_KEY_A)) {
			direction.x -= 1;
		}
		if (Pyxis::Input::IsKeyPressed(PX_KEY_S)) {
			direction.y -= 1;
		}
		if (Pyxis::Input::IsKeyPressed(PX_KEY_D)) {
			direction.x += 1;
		}
		if (Pyxis::Input::IsKeyPressed(PX_KEY_Q)) {
			m_CameraRotation -= m_cameraRotationSpeed * ts;
		}
		if (Pyxis::Input::IsKeyPressed(PX_KEY_E)) {
			m_CameraRotation += m_cameraRotationSpeed * ts;
		}


		if (direction.x != 0 && direction.y != 0)
			direction = glm::normalize(direction);
		m_CameraPosition += direction * m_CameraSpeed * ts.GetSeconds();
		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);
		Pyxis::Renderer::BeginScene(m_Camera);

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, m_SquarePosition);
		transform = glm::rotate(transform, glm::radians(-m_SquareRotation), {0,0,1});
		transform = glm::scale(transform, m_SquareScale);
		m_SingleColorShader->Bind();
		m_SingleColorShader->UploadUniformFloat4("u_Color", m_SquareColor);
		Pyxis::Renderer::Submit(m_SingleColorShader, m_SquareVertexArray, transform);

		Pyxis::Renderer::Submit(m_Shader, m_VertexArray);

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
	}

	void OnEvent(Pyxis::Event& event) override
	{
		Pyxis::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Pyxis::KeyPressedEvent>(PX_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<Pyxis::MouseScrolledEvent>(PX_BIND_EVENT_FN(ExampleLayer::OnMouseScrolledEvent));
		//PX_TRACE("{0}", event);
	}

	bool OnKeyPressedEvent(Pyxis::KeyPressedEvent& event) {
		//PX_CORE_INFO("Pressed Key");
		return false;
	}

	bool OnMouseScrolledEvent(Pyxis::MouseScrolledEvent& event) {
		if (event.GetYOffset() < 0)
		{
			if (Pyxis::Input::IsKeyPressed(PX_KEY_LEFT_SHIFT))
			{
				PX_CORE_INFO("SlowerCameraSpeed");
				m_CameraSpeed *= 0.9f;
				if (m_CameraSpeed < 2.5f) m_CameraSpeed = 2.5f;
				
			}
			else
			{
				PX_CORE_INFO("FasterCameraSpeed");
				m_CameraSpeed *= 1.1f;
				PX_CORE_INFO("Wider Cam");
				float width = m_Camera.GetWidth();
				width *= 1.1;
				m_Camera.SetWidth(width);
				m_Camera.SetHeight(width * 9 / 16);
			}
		}
		else
		{
			if (Pyxis::Input::IsKeyPressed(PX_KEY_LEFT_SHIFT))
			{
				PX_CORE_INFO("FasterCameraSpeed");
				m_CameraSpeed *= 1.1f;
			}
			else {
				PX_CORE_INFO("SlowerCameraSpeed");
				m_CameraSpeed *= 0.9f;
				if (m_CameraSpeed < 2.5f) m_CameraSpeed = 2.5f;
				PX_CORE_INFO("Smaller Cam");
				float width = m_Camera.GetWidth();
				width *= 0.9;
				if (width < 3.2f) width = 3.2f;
				m_Camera.SetWidth(width);
				m_Camera.SetHeight(width * 9 / 16);
			}
			
		}
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