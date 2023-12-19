#include "pxpch.h"
#include "Application.h"

#include "Input.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Pyxis 
{

	Application* Application::s_Instance = nullptr;

	

	Application::Application() {

		PX_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallBack(BIND_EVENT_FN(Application::OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		m_VertexArray.reset(VertexArray::Create());

		float vertices[] =
		{
			-0.5f, -0.5f,  0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			 0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			 0.0f,  0.5f,  0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		};
		std::shared_ptr<VertexBuffer> vertexBuffer;
		vertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
		BufferLayout layout = {
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float4, "a_Color"},
		};
		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		//create indices and gen/bind element(index) buffer
		uint32_t indices[3] =
		{
			0,1,2
		};
		std::shared_ptr<IndexBuffer> indexBuffer;
		indexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
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

		m_SquareVA.reset(VertexArray::Create());
		std::shared_ptr<VertexBuffer> squareVB;
		squareVB.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		BufferLayout squareLayout = {
			{ShaderDataType::Float3, "a_Position"}
		};
		squareVB->SetLayout(squareLayout);
		m_SquareVA->AddVertexBuffer(squareVB);

		std::shared_ptr<IndexBuffer> squareIB;
		squareIB.reset(IndexBuffer::Create(squareIndices, 6));
		m_SquareVA->SetIndexBuffer(squareIB);

		std::string vertexSource = R"(
			#version 460
			
			layout (location = 0) in vec3 a_Position;
			layout (location = 1) in vec4 a_Color;

			out vec4 v_Color;

			void main()
			{
				gl_Position = vec4(a_Position, 1.0f);
				v_Color = a_Color;
			}
		)";
		std::string positionvertexSource = R"(
			#version 460
			
			layout (location = 0) in vec3 a_Position;

			void main()
			{
				gl_Position = vec4(a_Position, 1.0f);
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

		std::string orangefragmentSource = R"(
			#version 460
			
			layout (location = 0) out vec4 color;

			void main()
			{
				color = vec4(1.0f, 0.7f, 0.0f, 1.0f);
			}
		)";

		m_Shader.reset(Shader::Create(vertexSource, fragmentSource));
		m_OrangeShader.reset(Shader::Create(positionvertexSource, orangefragmentSource));
	}

	Application::~Application() {
		
	};

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}

	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
	}

	void Application::Run() {
		while (m_Running)
		{

			glClearColor(0.2f, 0.2f, 0.2f, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			m_OrangeShader->Bind();
			m_SquareVA->Bind();
			glDrawElements(GL_TRIANGLES, m_SquareVA->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

			m_Shader.get()->Bind();
			m_VertexArray->Bind();
			glDrawElements(GL_TRIANGLES, m_VertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

			

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();


			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

}