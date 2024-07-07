#include "EditorLayer.h"

#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include <Platform/OpenGL/OpenGLShader.h>
#include <chrono>

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/InspectorPanel.h"

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
#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](ProfileResult profileResult) {m_ProfileResults.push_back(profileResult);})
#else
#define PROFILE_SCOPE(name)
#endif

	EditorLayer::EditorLayer()
		: Layer("EditorLayer"),
		m_OrthographicCameraController(5, 9.0f / 16.0f, -100, 100)
	{

	}

	void EditorLayer::OnAttach()
	{
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
		auto hierarchyPanel = CreateRef<SceneHierarchyPanel>(m_ActiveScene);
		m_Panels.push_back(hierarchyPanel);
		auto inspectorPanel = CreateRef<InspectorPanel>(hierarchyPanel);
		m_Panels.push_back(inspectorPanel);

		//Ref<Entity> entity = CreateRef<Entity>();
		auto entity = CreateRef<EntityWithSprite>("test entity 1");
		auto childEntity = CreateRef<EntityWithSprite>("test entity 1's Child");
		entity->AddChild(childEntity);

		m_ActiveScene->AddEntity(entity);
		m_ActiveScene->AddEntity(CreateRef<EntityWithSprite>("foo"));
		m_ActiveScene->AddEntity(CreateRef<EntityWithSprite>("bar"));

	}

	void EditorLayer::OnDetatch()
	{
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		PROFILE_SCOPE("EditorLayer::OnUpdate");
		//update
		if (m_SceneViewIsFocused)
			m_OrthographicCameraController.OnUpdate(ts);

		//rendering
#if STATISTICS
		Renderer2D::ResetStats();
#endif

		{
			PROFILE_SCOPE("Renderer Prep");
			m_SceneFrameBuffer->Bind();
			RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1 });
			RenderCommand::Clear();

			Renderer2D::BeginScene((m_ActiveScene->m_ActiveCamera) ? *(m_ActiveScene->m_ActiveCamera) : m_OrthographicCameraController.GetCamera());
		}

		{
			PROFILE_SCOPE("Game Update");
			m_ActiveScene->Update(ts);
		}

		//{
		//	PROFILE_SCOPE("Renderer Draw");
		//	Renderer2D::DrawRotatedQuad(m_TestPosition, m_TestSize, glm::radians(m_TestRotation), m_TestColor);

		//	for (float x = -5.0f; x <= 5.0f; x += 0.5f)
		//	{
		//		for (float y = -5.0f; y <= 5.0f; y += 0.5f)
		//		{
		//			Renderer2D::DrawQuad({ x, y , -1 }, { 0.08f, 0.08f }, { (x + 5) / 10, (y + 5) / 10 , 1, 1 });
		//		}
		//	}
		//	Renderer2D::DrawQuad({ 2.0f, 0.0f, 0.0f }, { 1.0f ,1.0f }, m_TestTexture);

		//	//Renderer2D::DrawQuad({ 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, m_SubTextureTest);
		//}

		//m_FrameBuffer->Unbind()
		Renderer2D::EndScene();
		m_SceneFrameBuffer->Unbind();
	}

	void EditorLayer::OnImGuiRender()
	{
		if (ImGui::BeginMainMenuBar())
		{
			ImGui::DockSpaceOverViewport();
			
			ImGui::EndMainMenuBar();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });

		if (ImGui::Begin("Scene"))
		{
			m_SceneViewIsFocused = ImGui::IsWindowFocused();
			Application::Get().GetImGuiLayer()->BlockEvents(!ImGui::IsWindowFocused());
			//ImGui::GetForegroundDrawList()->AddRect(ImVec2(0, 0), windowSize, ImU32(0xFFFFFFFF));
			ImVec2 windowSize = ImGui::GetContentRegionAvail();
			ImGui::Image(
				(ImTextureID)m_SceneFrameBuffer->GetColorAttachmentRendererID(),
				windowSize,
				ImVec2(0, 1),
				ImVec2(1, 0),
				ImVec4(1, 1, 1, 1)
				//ImVec4(1, 1, 1, 1) border color
			);

			if (m_ViewportSize.x != windowSize.x || m_ViewportSize.y != windowSize.y)
			{
				m_OrthographicCameraController.SetAspect(windowSize.y / windowSize.x);
				m_SceneFrameBuffer->Resize((uint32_t)windowSize.x, (uint32_t)windowSize.y);
				m_ViewportSize = { windowSize.x, windowSize.y };
			}
			
			ImGui::End();
		}
		ImGui::PopStyleVar();

		for each (auto panel in m_Panels)
		{
			panel->OnImGuiRender();
		}


		ImGui::Begin("Testing");

		ImGui::ColorEdit4("SquareColor", glm::value_ptr(m_TestColor), 0.1f);
		ImGui::DragFloat3("SquarePosition", glm::value_ptr(m_TestPosition), 0.1f);
		ImGui::DragFloat2("SquareSize", glm::value_ptr(m_TestSize), 0.1f);
		ImGui::DragFloat("SquareRotation", &m_TestRotation, 0.1f);
		ImGui::End();

		//profiling
#if PROFILING
		const int AverageAmount = 100;
		ImGui::Begin("Profiler");

		if (m_ProfileAverageValue.empty())
		{
			for (auto& result : m_ProfileResults)
			{
				m_ProfileAverageValue.push_back(result.Time);
			}
		}
		auto avgVal = m_ProfileAverageValue.begin();
		//update the average with current time
		for (auto& result : m_ProfileResults)
		{
			*avgVal = *avgVal + result.Time;
			avgVal++;
		}
		//check for every 100th update, and update storage
		if (m_ProfileAverageCount >= AverageAmount)
		{
			m_ProfileAverageValueStorage.clear();
			avgVal = m_ProfileAverageValue.begin();
			for (auto& result : m_ProfileResults)
			{
				//set the storage
				m_ProfileAverageValueStorage.push_back(*avgVal / AverageAmount);
				avgVal++;
			}
			m_ProfileAverageValue.clear();
			m_ProfileAverageCount = 0;
		}
		//loop back through profiles and print times
		auto avgValFromStorage = m_ProfileAverageValueStorage.begin();
		for (auto& result : m_ProfileResults)
		{
			//write the average
			char label[50];
			strcpy(label, "%.3fms ");
			strcat(label, result.Name);
			ImGui::Text(label, *avgValFromStorage);
			avgValFromStorage++;
		}
		m_ProfileAverageCount++;
		m_ProfileResults.clear();
		ImGui::End();
#endif

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

	void EditorLayer::OnEvent(Event& e)
	{
		PX_CORE_INFO("event: {0}", e);
		m_OrthographicCameraController.OnEvent(e);
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(PX_BIND_EVENT_FN(EditorLayer::OnWindowResizeEvent));
	}

	bool EditorLayer::OnWindowResizeEvent(WindowResizeEvent& event) {
		//m_OrthographicCameraController.SetAspect((float)event.GetHeight() / (float)event.GetWidth());
		//Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
		//Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
		return false;
	}

}