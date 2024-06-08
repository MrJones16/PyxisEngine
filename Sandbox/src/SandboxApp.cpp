#include <Pyxis.h>
//---------- Entry Point ----------//
#include <Pyxis/Core/EntryPoint.h>

#include "Sandbox2D.h"

#include <ImGui/imgui.h>
#include "Pyxis/Renderer/Camera.h"
#include "Pyxis/Core/PerspectiveCameraController.h"
#include "Pyxis/Core/OrthographicCameraController.h"
#include <glm/gtc/type_ptr.hpp>

#include "Platform/OpenGL/OpenGLShader.h"

struct Particle
{
	glm::vec2 Position;
	glm::vec2 Velocity;
	float mass;

	glm::vec4 Color;
};

class ExampleLayer : public Pyxis::Layer
{
private:
	Pyxis::ShaderLibrary m_ShaderLibrary;
	Pyxis::Ref<Pyxis::FrameBuffer> m_FrameBuffer;
	Pyxis::Ref<Pyxis::VertexArray> m_VertexArray, m_VertexArrayCube, m_VertexArrayLine;
	Pyxis::Ref<Pyxis::VertexArray> m_ScreenVAO;
	Pyxis::PerspectiveCameraController m_PerspectiveCameraController;
	Pyxis::PerspectiveCamera m_Camera;
	Pyxis::OrthographicCameraController m_OrthographicCameraController;
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

	//game testing stuff
	std::list<Particle> m_Particles;
	float m_springForce = 20.0f;
	float m_dampForce = 1.0f;
	glm::vec2 anchorPos = glm::vec2(0);
	glm::vec2 m_WindowSize = glm::vec2(1920/2, 1080/2);

	//testing lines
	float lineVertices[6];

public:
	ExampleLayer() : Layer("Example"), 
		m_PerspectiveCameraController(1920 / 1080, 75.0f, 0.1f, 100.0f), 
		m_Camera(1920 / 1080, 75.0f, 0.1f, 100.0f),
		m_OrthographicCameraController(5, 9.0f / 16.0f, -100, 100)
	{
		m_CameraPosition = glm::vec3(0.0f, 0.0f, 5.0f);
		//m_CameraRotation = 0.0f;
		
		auto textureShader = m_ShaderLibrary.Load("Texture", "assets/shaders/Texture.glsl");
		std::dynamic_pointer_cast<Pyxis::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<Pyxis::OpenGLShader>(textureShader)->SetInt("u_TextureDiffuse", 0);

		auto SingleColorShader = m_ShaderLibrary.Load("SingleColorShader", "assets/shaders/SingleColorShader.glsl");
		std::dynamic_pointer_cast<Pyxis::OpenGLShader>(SingleColorShader)->Bind();
		std::dynamic_pointer_cast<Pyxis::OpenGLShader>(SingleColorShader)->SetFloat4("u_Color", m_SquareColor);

		auto RayMarchShader = m_ShaderLibrary.Load("RayMarch", "assets/shaders/RayMarch.glsl");
		auto Voxel = m_ShaderLibrary.Load("Voxel", "assets/shaders/Voxel.glsl");
		std::dynamic_pointer_cast<Pyxis::OpenGLShader>(Voxel)->SetInt("u_ColorTexture", 0);
		//RayMarchShader->Bind();
		//std::dynamic_pointer_cast<Pyxis::OpenGLShader>(RayMarchShader)->UploadUniformFloat2("u_Resolution", glm::vec2(1280, 720));

		m_VertexArray = Pyxis::VertexArray::Create();
		m_VertexArrayCube = Pyxis::VertexArray::Create();
		m_VertexArrayLine = Pyxis::VertexArray::Create();
		m_ScreenVAO = Pyxis::VertexArray::Create();

		float squareVertices[] =
		{
		    //x		y		z		u		v
			-0.75f, -0.75f,  0.0f,  0.0f,  0.0f, 
			 0.75f, -0.75f,  0.0f,  1.0f,  0.0f,
			 0.75f,  0.75f,  0.0f,  1.0f,  1.0f,
			-0.75f,  0.75f,  0.0f,  0.0f,  1.0f
		};

		float cubeVertices[] =
		{
			//x		y		z		u		v
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

		lineVertices[0] = 0;
		lineVertices[1] = 0;
		lineVertices[2] = 0;
		lineVertices[3] = 1;
		lineVertices[4] = 0;
		lineVertices[5] = 0;


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
		// Line
		/////////////////////
		Pyxis::BufferLayout lineLayout = {
			{Pyxis::ShaderDataType::Float3, "a_Position"},
		};
		Pyxis::Ref<Pyxis::VertexBuffer> lineVBO;
		lineVBO = Pyxis::VertexBuffer::Create(lineVertices, sizeof(lineVertices));
		lineVBO->SetLayout(lineLayout);
		m_VertexArrayLine->AddVertexBuffer(lineVBO);


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
		Pyxis::FrameBufferSpecification fbspec;
		fbspec.Width = 1920;
		fbspec.Height = 1080;
		m_FrameBuffer = Pyxis::FrameBuffer::Create(fbspec);
		Pyxis::Renderer::AddFrameBuffer(m_FrameBuffer);
		m_PerspectiveCameraController.SetPosition({ 0,0, 2 });

		std::srand(time(nullptr));

		m_Particles = std::list<Particle>(9);
		for (auto i = m_Particles.begin(); i != m_Particles.end(); i++)
		{
			i->mass = (std::rand() % 10);
			i->Color = glm::vec4((std::rand() % 100) / 100.0f, (std::rand() % 100) / 100.0f, (std::rand() % 100) / 100.0f, 1);
			i->Position = glm::vec2(((std::rand() % 20) - 10.0f) / 10.0f, ((std::rand() % 20) - 10.0f) / 10.0f);
			i->Velocity = glm::vec2(((std::rand() % 100) - 50.0f) / 100, ((std::rand() % 100) - 50.0f) / 100);
		}
	}
	

	void OnUpdate(Pyxis::Timestep ts) override
	{
		Pyxis::RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1 });
		Pyxis::RenderCommand::Clear();

		
		//m_Camera.SetRotation(m_CameraRotation);
		//Pyxis::Renderer::BeginScene(m_Camera);
		m_PerspectiveCameraController.OnUpdate(ts);
		m_OrthographicCameraController.OnUpdate(ts);
		
		Pyxis::Renderer::BeginScene(m_OrthographicCameraController.GetCamera());

		//auto textureShader = m_ShaderLibrary.Get("Texture");
		//m_Texture->Bind();
		//Pyxis::Renderer::Submit(textureShader, m_VertexArray);

		auto RayMarchShader = m_ShaderLibrary.Get("RayMarch");
		auto VoxelShader = m_ShaderLibrary.Get("Voxel");
		auto SingleColorShader = m_ShaderLibrary.Get("SingleColorShader");

		//m_FrameBuffer->Bind();
		//Pyxis::RenderCommand::Clear();
		////Pyxis::Renderer::Submit(RayMarchShader, m_VertexArrayCube, glm::mat4(1.0f));
		//Pyxis::Renderer::Submit(RayMarchShader, m_VertexArrayCube, glm::mat4(1.0f));
		////Pyxis::Renderer::Submit(RayMarchShader, m_ScreenVAO);
		//m_FrameBuffer->Unbind();

		m_FrameBuffer->Bind();

		Pyxis::RenderCommand::Clear();

		SingleColorShader->Bind();
		SingleColorShader->SetFloat4("u_Color", m_SquareColor);

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, m_SquarePosition);
		transform = glm::rotate(transform, glm::radians(-m_SquareRotation), { 0,0,1 });
		transform = glm::scale(transform, m_SquareScale);

		auto textureShader = m_ShaderLibrary.Get("Texture");
		m_MushroomTexture->Bind();
		Pyxis::Renderer::Submit(textureShader, m_VertexArray, glm::translate(glm::mat4(1.0f), { 1.0f,0.0f,-1.0f }));

		//draw spring anchors
		SingleColorShader->Bind();
		glm::mat4 anchorTransform = glm::mat4(1);
		anchorTransform = glm::scale(anchorTransform, glm::vec3(0.2f, 0.2f, 0.2f));
		Pyxis::Renderer::Submit(SingleColorShader, m_VertexArray, anchorTransform);

		anchorTransform = glm::translate(glm::mat4(1), glm::vec3(anchorPos.x, anchorPos.y, 0));
		anchorTransform = glm::scale(anchorTransform, glm::vec3(0.2f, 0.2f, 0.2f));
		Pyxis::Renderer::Submit(SingleColorShader, m_VertexArray, anchorTransform);

		
		//update particles
		for (auto&& p : m_Particles)
		{
			p.Position += p.Velocity * ts.GetSeconds();

			//adding forces//

			//add gravity
			p.Velocity -= glm::vec2(0, 9.8f * ts.GetSeconds());

			//calculate spring force
			float force = (1.0f / p.mass) * m_springForce;
			bool underTension = false;

			//add spring to anchor
			glm::vec2 direction = anchorPos - p.Position;
			float dist = GetVectorLength(direction);
			//direction /= dist;
			if (dist > 1)
			{
				p.Velocity += direction * force * ts.GetSeconds();
				underTension = true;
			}

			//add spring to center
			direction = glm::vec2(0) - p.Position;
			dist = GetVectorLength(direction);
			//direction /= dist;
			if (dist > 1)
			{
				p.Velocity += direction * force * ts.GetSeconds();
				underTension = true;
			}

			//dampening force
			if (underTension)
			p.Velocity -= p.Velocity * m_dampForce * ts.GetSeconds();
			

			//draw lines to anchors
			//draw line
			SingleColorShader->Bind();
			SingleColorShader->SetFloat4("u_Color", glm::vec4(0, 0, 1, 1));
			
			for each (auto var in m_VertexArrayLine->GetVertexBuffers())
			{
				//x1
				lineVertices[0] = p.Position.x;
				//y1
				lineVertices[1] = p.Position.y;
				//x2
				lineVertices[3] = 0;
				//y2
				lineVertices[4] = 0;
				var->SetData(lineVertices, sizeof(lineVertices));
				Pyxis::Renderer::SubmitLine(SingleColorShader, m_VertexArrayLine);

				//x1
				lineVertices[0] = p.Position.x;
				//y1
				lineVertices[1] = p.Position.y;
				//x2 (to anchor)
				lineVertices[3] = anchorPos.x;
				//y2
				lineVertices[4] = anchorPos.y;
				var->SetData(lineVertices, sizeof(lineVertices));
				Pyxis::Renderer::SubmitLine(SingleColorShader, m_VertexArrayLine);
			}
			
		}

		

		//draw particles
		
		Pyxis::Renderer::PreBatch(SingleColorShader, m_VertexArray);
		glm::mat4 particleTransform;
		for (auto&& p : m_Particles)
		{
			particleTransform = glm::translate(glm::mat4(1), glm::vec3(p.Position.x, p.Position.y, 0));
			particleTransform = glm::scale(particleTransform, glm::vec3(0.1f, 0.1f, 0.1f));
			SingleColorShader->SetFloat4("u_Color", p.Color);
			Pyxis::Renderer::Submit(SingleColorShader, m_VertexArray, particleTransform);
		}



		m_FrameBuffer->Unbind();

		Pyxis::Renderer::EndScene();

	}

	virtual void OnImGuiRender() override
	{
		ImGui::Begin("Inspector");
		if (ImGui::CollapsingHeader("Constants"))
		{
			ImGui::SliderFloat("Spring Force", &m_springForce, 0, 100, "%.01f");
			ImGui::SliderFloat("Damp Force", &m_dampForce, 0, 10, "%.01f");
		}
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
			
			m_OrthographicCameraController.SetAspect(windowSize.y / windowSize.x);
			//PX_CORE_INFO("{0}, {1}", windowSize.x, windowSize.y);
			Pyxis::Renderer::OnWindowResize(windowSize.x, windowSize.y);
			ImGui::Image(
				(ImTextureID)m_FrameBuffer->GetColorAttatchmentRendererID(),
				ImGui::GetContentRegionAvail(),
				ImVec2(0, 1),
				ImVec2(1, 0),
				ImVec4(1, 1, 1, 1)
				//ImVec4(1, 1, 1, 1) border color
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
		dispatcher.Dispatch<Pyxis::MouseButtonPressedEvent>(PX_BIND_EVENT_FN(ExampleLayer::OnMouseButtonPressedEvent));
		dispatcher.Dispatch<Pyxis::WindowResizeEvent>(PX_BIND_EVENT_FN(ExampleLayer::OnWindowResizeEvent));
		//PX_TRACE("{0}", event);
	}

	bool OnKeyPressedEvent(Pyxis::KeyPressedEvent& event) {
		//PX_CORE_INFO("Pressed Key");
		return false;
	}

	bool OnMouseScrolledEvent(Pyxis::MouseScrolledEvent& event) {
		m_PerspectiveCameraController.OnMouseScrolledEvent(event);
		m_OrthographicCameraController.OnMouseScrolledEvent(event);
		return false;
	}
	bool OnMouseButtonPressedEvent(Pyxis::MouseButtonPressedEvent& event) {
		if (event.GetMouseButton() == 0)
		{
			//LMB
			auto [x, y] = Pyxis::Input::GetMousePosition();

			x = ((x / m_WindowSize.x) * 2) - 1;
			y = ((y / m_WindowSize.y) * 2) - 1;

			x *= m_OrthographicCameraController.GetCamera().GetWidth() / 2;
			y *= m_OrthographicCameraController.GetCamera().GetHeight() / 2;
			//PX_CORE_TRACE("screen mouse pos: ({0}, {1})", x, y);
			
			
			anchorPos = glm::vec2(x, y);
			glm::vec4 vec = glm::vec4(x, -y, 0, 1);
			vec = glm::translate(glm::mat4(1), m_OrthographicCameraController.GetPosition()) * vec;
			//anchorPos = (glm::inverse(m_OrthographicCameraController.GetViewProjectionMatrix()) * vec);
			anchorPos = vec;
			PX_CORE_TRACE("World mouse pos: ({0}, {1})", anchorPos.x, anchorPos.y);
		}
		return false;
	}

	bool OnWindowResizeEvent(Pyxis::WindowResizeEvent& event) {
		m_WindowSize = glm::vec2(event.GetWidth(), event.GetHeight());
		//Pyxis::Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
		return false;
	}

	float GetVectorLength(glm::vec2 vector)
	{
		return std::sqrt(vector.x * vector.x + vector.y * vector.y);
	}


};

class Sandbox : public Pyxis::Application {
public:
	Sandbox()
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}
	~Sandbox()
	{

	}
};

Pyxis::Application* Pyxis::CreateApplication() {
	return new Sandbox();
}