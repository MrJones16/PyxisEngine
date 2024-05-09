#include "GameLayer.h"

#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include <Platform/OpenGL/OpenGLShader.h>
#include <chrono>



namespace Pyxis
{

	template<typename Fn>
	class Timer
	{
	public:
		Timer(const char* name, Fn&& func)
			: m_Name(name), m_Stopped(false), m_Func(func)
		{
			m_StartTimepoint = std::chrono::high_resolution_clock::now();
		}

		~Timer()
		{
			if (!m_Stopped)
				Stop();
		}

		void Stop()
		{
			auto endTimepoint = std::chrono::high_resolution_clock::now();
			long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
			long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

			m_Stopped = true;
			float duration = (end - start) * 0.001f;

			m_Func({ m_Name, duration });
			//std::cout << m_Name << ": Duration: " << duration << "milliseconds" << std::endl;
		}
	private:
		const char* m_Name;
		std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
		bool m_Stopped;
		Fn m_Func;
	};

#if PROFILING
#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](ProfileResult profileResult) {m_ProfilingPanel->m_ProfileResults.push_back(profileResult);})
#else
#define PROFILE_SCOPE(name)
#endif

	GameLayer::GameLayer()
		: Layer("GameLayer"),
		m_OrthographicCameraController(10, 1 / 1, -100, 100)
	{

	}

	void GameLayer::OnAttach()
	{
		m_World = CreateRef<World>();
		m_ViewportSize = { Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight() };
		//m_ProfileResults = std::vector<ProfileResult>();
		Renderer2D::Init();
		m_TestTexture = Texture2D::Create("assets/textures/pixel-space.png");
		//m_SpritesheetTexture = Texture2D::Create("assets/textures/ForestTileSet.png");
		//m_SubTextureTest = SubTexture2D::CreateFromCoords(m_SpritesheetTexture, { 2 ,0 }, { 16,16 });
		FrameBufferSpecification fbspec;
		fbspec.Width = 1280;
		fbspec.Height = 720;
		m_SceneFrameBuffer = FrameBuffer::Create(fbspec);

		m_ActiveScene = CreateRef<Scene>();

		//Create panels to add
		m_ProfilingPanel = CreateRef<ProfilingPanel>();
		m_Panels.push_back(m_ProfilingPanel);
		//auto hierarchyPanel = CreateRef<SceneHierarchyPanel>(m_ActiveScene);
		//m_Panels.push_back(hierarchyPanel);
		//auto inspectorPanel = CreateRef<InspectorPanel>(hierarchyPanel);
		//m_Panels.push_back(inspectorPanel);

		////Ref<Entity> entity = CreateRef<Entity>();
		//auto entity = CreateRef<EntityWithSprite>("test entity 1");
		//auto childEntity = CreateRef<EntityWithSprite>("test entity 1's Child");
		//entity->AddChild(childEntity);

		//m_ActiveScene->AddEntity(entity);
		//m_ActiveScene->AddEntity(CreateRef<EntityWithSprite>("foo"));
		//m_ActiveScene->AddEntity(CreateRef<EntityWithSprite>("bar"));


		//m_Chunk->SetElement({ 100,100 }, "Sand");
		
	}

	void GameLayer::OnDetatch()
	{
		
	}

	void GameLayer::OnUpdate(Timestep ts)
	{
		//PROFILE_SCOPE("GameLayer::OnUpdate");

		//update
		//if (m_SceneViewIsFocused)
		m_OrthographicCameraController.OnUpdate(ts);

		//rendering
		#if STATISTICS
		Renderer2D::ResetStats();
		#endif

		{
			PROFILE_SCOPE("Renderer Prep");
			//m_SceneFrameBuffer->Bind();
			RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1 });
			RenderCommand::Clear();
			Renderer2D::BeginScene((m_ActiveScene->m_ActiveCamera) ? *(m_ActiveScene->m_ActiveCamera) : m_OrthographicCameraController.GetCamera());
		}

		{
			PROFILE_SCOPE("Game Update");
			if (Input::IsMouseButtonPressed(0))
			{
				Element* element = new Element();
				element->m_ID = 1;
				//element->m_Name = "Sand";
				element->m_Type = ElementType::solid;
				element->m_SubType = ElementSubType::None;
				element->m_Tags = 0;
				element->m_Color = 0xFF00FFFF;
				element->m_Updated = !m_World->m_UpdateStatus;
				auto [x, y] = Pyxis::Input::GetMousePosition();

				auto vec = m_OrthographicCameraController.MouseToWorldPos(x, y);
				
				m_World->SetElement(m_World->WorldToPixel(vec), element);

				Ref<Chunk> chunk = m_World->GetChunk(m_World->WorldToChunk(vec));
				if (chunk != nullptr)
					chunk->UpdateTexture();
			}
			if (Input::IsMouseButtonPressed(1))
			{
				Element* element = new Element();
				element->m_ID = 2;
				//element->m_Name = "Sand";
				element->m_Type = ElementType::liquid;
				element->m_SubType = ElementSubType::None;
				element->m_Tags = 0;
				element->m_Color = 0xFFFF0000;
				element->m_Updated = !m_World->m_UpdateStatus;
				auto [x, y] = Pyxis::Input::GetMousePosition();

				auto vec = m_OrthographicCameraController.MouseToWorldPos(x, y);

				m_World->SetElement(m_World->WorldToPixel(vec), element);

				Ref<Chunk> chunk = m_World->GetChunk(m_World->WorldToChunk(vec));
				if (chunk != nullptr)
					chunk->UpdateTexture();

			}

			if (m_SimulationRunning)
				m_World->UpdateWorld();
		}

		{
			PROFILE_SCOPE("Renderer Draw");
			m_World->RenderWorld();
		}

		Renderer2D::EndScene();

		//m_SceneFrameBuffer->Unbind()
	}

	void GameLayer::OnImGuiRender()
	{
		if (ImGui::BeginMainMenuBar())
		{
			//ImGui::DockSpaceOverViewport();
			
			ImGui::EndMainMenuBar();
		}

		for each (auto panel in m_Panels)
		{
			panel->OnImGuiRender();
		}


#if STATISTICS
		Renderer2D::Statistics stats = Renderer2D::GetStats();
		ImGui::Begin("Rendering Statistics");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
		ImGui::End();
#endif
	}
	void GameLayer::OnEvent(Event& e)
	{
		PX_CORE_INFO("event: {0}", e);
		m_OrthographicCameraController.OnEvent(e);
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(PX_BIND_EVENT_FN(GameLayer::OnWindowResizeEvent));
		dispatcher.Dispatch<KeyPressedEvent>(PX_BIND_EVENT_FN(GameLayer::OnKeyPressedEvent));
	}

	bool GameLayer::OnWindowResizeEvent(WindowResizeEvent& event) {
		//m_OrthographicCameraController.SetAspect((float)event.GetHeight() / (float)event.GetWidth());
		m_ViewportSize = { event.GetWidth() , event.GetHeight() };
		Renderer::OnWindowResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		return false;
	}

	bool GameLayer::OnKeyPressedEvent(KeyPressedEvent& event) {
		if (event.GetKeyCode() == PX_KEY_SPACE)
		{
			m_SimulationRunning = !m_SimulationRunning;
		}
		if (event.GetKeyCode() == PX_KEY_RIGHT)
		{
			m_World->UpdateWorld();
		}
		if (event.GetKeyCode() == PX_KEY_C)
		{
			m_World->Clear();
			//m_Chunk->Clear();
		}
		return false;
	}

}