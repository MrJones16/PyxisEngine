#include "pxpch.h"
#include "Renderer2D.h"
#include "RenderCommand.h"

#include "VertexArray.h"
#include "Shader.h"

namespace Pyxis
{
	struct Renderer2DStorage
	{
		Ref<VertexArray> QuadVertexArray;
		Ref<Shader> SingleColorShader;
	};

	static Renderer2DStorage* s_Data; // can make this a pointer

	void Renderer2D::Init()
	{
		//initialize the renderer2d primitive things
		s_Data = new Renderer2DStorage();


		s_Data->QuadVertexArray = VertexArray::Create();
		//m_OrthographicCameraController = Pyxis::OrthographicCameraController(5, 9 / 16, -100, 100);
		s_Data->SingleColorShader = Shader::Create("assets/shaders/SingleColorShader.glsl");

		/////////////////////
		// square
		/////////////////////
		float squareVertices[] =
		{
			//x		y		z		u		v
			-0.75f, -0.75f,  0.0f,  0.0f,  0.0f,
			 0.75f, -0.75f,  0.0f,  1.0f,  0.0f,
			 0.75f,  0.75f,  0.0f,  1.0f,  1.0f,
			-0.75f,  0.75f,  0.0f,  0.0f,  1.0f
		};
		uint32_t squareIndices[] =
		{
			0,1,2,
			0,2,3
		};
		Ref<VertexBuffer> SquareVBO;
		SquareVBO = VertexBuffer::Create(squareVertices, sizeof(squareVertices));
		BufferLayout layout = {
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float2, "a_TexCoord"},
		};
		SquareVBO->SetLayout(layout);
		s_Data->QuadVertexArray->AddVertexBuffer(SquareVBO);

		Ref<IndexBuffer> indexBuffer;
		indexBuffer = IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
		s_Data->QuadVertexArray->SetIndexBuffer(indexBuffer);

		//frame buffer
		//m_FrameBuffer = FrameBuffer::Create(1920, 1080);
		//Renderer::AddFrameBuffer(m_FrameBuffer);
	}

	void Renderer2D::Shutdown()
	{
		delete s_Data;
	}

	void Renderer2D::BeginScene(const Pyxis::OrthographicCamera& camera)
	{
		s_Data->SingleColorShader->Bind();
		s_Data->SingleColorShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
		s_Data->SingleColorShader->SetMat4("u_Transform", glm::mat4(1));
	}

	void Renderer2D::EndScene()
	{
	}

	void Renderer2D::DrawQuad(const glm::vec2 position, const glm::vec2& size, const glm::vec4& color)
	{
		s_Data->SingleColorShader->Bind();
		s_Data->SingleColorShader->SetFloat4("u_Color", color);
		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);

	}

	void Renderer2D::DrawQuad(const glm::vec3 position, const glm::vec2& size, const glm::vec4& color)
	{
	}
}