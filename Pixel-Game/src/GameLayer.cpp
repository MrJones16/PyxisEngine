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
		//create the world
		m_World = CreateRef<World>();
		if (m_World->m_Error)
		{
			PX_ERROR("Failed to create the world");
			return;
		}
		m_World->AddChunk(glm::ivec2(0, 0));


		//create a border around first chunk
		Element stone = Element();
		stone.m_ID = m_World->m_ElementIDs["stone"];
		ElementData& elementdata = m_World->m_ElementData[stone.m_ID];
		stone.m_Color = elementdata.color;
		stone.m_Updated = !m_World->m_UpdateBit;
		for (int i = 0; i < m_World->CHUNKSIZE; i++)
		{
			m_World->SetElement({ i, 0 }, stone);//bottom
			m_World->SetElement({ i, CHUNKSIZE - 1 }, stone);//top
			m_World->SetElement({ 0, i }, stone);//left
			m_World->SetElement({ CHUNKSIZE - 1, i }, stone);//right
		}

		m_ViewportSize = { Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight() };
		Renderer2D::Init();

		m_ActiveScene = CreateRef<Scene>();

		//Create panels to add
		m_ProfilingPanel = CreateRef<ProfilingPanel>();
		m_Panels.push_back(m_ProfilingPanel);
		
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
			if (Input::IsMouseButtonPressed(0) && !m_Hovering)
			{
				PaintElementAtCursor();
			}
			/*if (Input::IsMouseButtonPressed(1))
			{
				auto [x, y] = Pyxis::Input::GetMousePosition();
				auto vec = m_OrthographicCameraController.MouseToWorldPos(x, y);
				glm::ivec2 pixelPos = glm::ivec2(vec.x * CHUNKSIZE, vec.y * CHUNKSIZE);
				m_World->SetElement(pixelPos, m_ElementsMap["Stone"]);
				Chunk* chunk = m_World->GetChunk(m_World->PixelToChunk(pixelPos));
				glm::ivec2 index = m_World->PixelToIndex(pixelPos);
				chunk->UpdateDirtyRect(index.x, index.y);

				chunk->UpdateTexture();
			}*/

			if (m_SimulationRunning)
				m_World->UpdateWorld();
		}

		{
			PROFILE_SCOPE("Renderer Draw");
			m_World->RenderWorld();
			PaintBrushHologram();
		}

		Renderer2D::EndScene();

		//m_SceneFrameBuffer->Unbind()
	}

	void GameLayer::OnImGuiRender()
	{
		//ImGui::ShowDemoWindow();
		
		ImGui::DockSpaceOverViewport((const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode);

		m_Hovering = ImGui::IsWindowHovered();
		

		if (ImGui::BeginMainMenuBar())
		{
			//ImGui::DockSpaceOverViewport();
			if (ImGui::BeginMenu("File"))
			{
				ImGui::Text("Welcome to the file menu.");
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		
		if (ImGui::Begin("Brush Settings"))
		{

			ImGui::SetNextItemOpen(true);
			if (ImGui::TreeNode("Bush Shape"))
			{
				if (ImGui::Selectable("Circle", m_BrushType == BrushType::circle))
				{
					m_BrushType = BrushType::circle;
				}
				if (ImGui::Selectable("Square", m_BrushType == BrushType::square))
				{
					m_BrushType = BrushType::square;
				}
				ImGui::TreePop();
			}
			

			ImGui::DragFloat("Brush Size", &m_BrushSize, 1.0f, 1.0f, 10.0f);


			ImGui::SetNextItemOpen(true);
			if (ImGui::TreeNode("Element Type"))
			{
				for (int y = 0; y < 4; y++)
					for (int x = 0; x < 4; x++)
					{
						if (x > 0)
							ImGui::SameLine();
						int index = y * 4 + x;
						if (index >= m_World->m_TotalElements) continue;
						ImGui::PushID(index);
						std::string name = (m_World->m_ElementData[index].name);
						if (ImGui::Selectable(name.c_str(), m_SelectedElementIndex == index, 0, ImVec2(50, 25)))
						{
							// Toggle clicked cell
							m_SelectedElementIndex = index;
						}
						ImGui::PopID();
					}
				ImGui::TreePop();
			}
			
		}
		ImGui::End();

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
		//PX_CORE_INFO("event: {0}", e);
		m_OrthographicCameraController.OnEvent(e);
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(PX_BIND_EVENT_FN(GameLayer::OnWindowResizeEvent));
		dispatcher.Dispatch<KeyPressedEvent>(PX_BIND_EVENT_FN(GameLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<MouseButtonPressedEvent>(PX_BIND_EVENT_FN(GameLayer::OnMouseButtonPressedEvent));
	}

	bool GameLayer::OnWindowResizeEvent(WindowResizeEvent& event) {
		m_OrthographicCameraController.SetAspect((float)event.GetHeight() / (float)event.GetWidth());
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
			///m_World->Clear();
			//m_Chunk->Clear();
		}
		if (event.GetKeyCode() == PX_KEY_V)
		{
			Window& window = Application::Get().GetWindow();
			window.SetVSync(!window.IsVSync());
		}
		//switching brushes
		if (event.GetKeyCode() == PX_KEY_Z)
		{
			int type = ((int)m_BrushType) - 1;
			if (type < 0) type = 0;
			m_BrushType = (BrushType)type;
		}
		if (event.GetKeyCode() == PX_KEY_C)
		{
			int type = ((int)m_BrushType) + 1;
			if (type >= (int)BrushType::end) type = (int)BrushType::end - 1;
			m_BrushType = BrushType(type);
		}

		return false;
	}

	bool GameLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& event)
	{
		//PX_TRACE(event.GetMouseButton());
		if (event.GetMouseButton() == 4) // forward
		{
			m_BrushSize++;
		}
		if (event.GetMouseButton() == 3) // back
		{
			m_BrushSize--;
			if (m_BrushSize <= 0) m_BrushSize == 1;
		}

		return false;
	}

	void GameLayer::PaintElementAtCursor()
	{
		std::unordered_map<glm::ivec2, Chunk*, HashVector> map;

		auto [x, y] = Pyxis::Input::GetMousePosition();
		auto vec = m_OrthographicCameraController.MouseToWorldPos(x, y);
		glm::ivec2 pixelPos = glm::ivec2(vec.x * CHUNKSIZE, vec.y * CHUNKSIZE);


		glm::ivec2 newPos = pixelPos;
		for (int x = -m_BrushSize; x <= m_BrushSize; x++)
		{
			for (int y = -m_BrushSize; y <= m_BrushSize; y++)
			{
				newPos = pixelPos + glm::ivec2(x,y);
				Chunk* chunk;
				glm::ivec2 index;
				switch (m_BrushType)
				{
				case BrushType::circle:
					//limit brush to circle
					if (std::sqrt((float)(x * x) + (float)(y * y)) >= m_BrushSize) continue;
					break;
				case BrushType::square:
					break;
				}
				//get element / color
				Element element = Element();
				ElementData& elementData = m_World->m_ElementData[m_SelectedElementIndex];
				element.m_ID = m_SelectedElementIndex;
				element.m_Updated = !m_World->m_UpdateBit;
				element.m_BaseColor = World::RandomizeABGRColor(elementData.color, 20);
				element.m_Color = element.m_BaseColor;

				chunk = m_World->GetChunk(m_World->PixelToChunk(newPos));
				index = m_World->PixelToIndex(newPos);
				chunk->UpdateDirtyRect(index.x, index.y);

				//set element and add chunk to update map
				if (m_World->m_ElementIDs["debug_heat"] == m_SelectedElementIndex)
				{
					//add heat
					chunk->m_Elements[index.x + index.y * CHUNKSIZE].m_Temperature++;
				}
				else if (m_World->m_ElementIDs["debug_cool"] == m_SelectedElementIndex)
				{
					chunk->m_Elements[index.x + index.y * CHUNKSIZE].m_Temperature--;
				}
				else
					m_World->SetElement(newPos, element);

				
				map[chunk->m_ChunkPos] = chunk;
			}
		}
		for each(auto& pair in map)
		{
			pair.second->UpdateTexture();
		}
	}

	void GameLayer::PaintBrushHologram()
	{

		auto [x, y] = Pyxis::Input::GetMousePosition();
		auto vec = m_OrthographicCameraController.MouseToWorldPos(x, y);
		glm::ivec2 pixelPos = glm::ivec2(vec.x * CHUNKSIZE, vec.y * CHUNKSIZE);

		glm::ivec2 newPos = pixelPos;
		for (int x = -m_BrushSize; x <= m_BrushSize; x++)
		{
			for (int y = -m_BrushSize; y <= m_BrushSize; y++)
			{
				newPos = pixelPos + glm::ivec2(x, y);
				Chunk* chunk;
				glm::ivec2 index;
				switch (m_BrushType)
				{
				case BrushType::circle:
					//limit to circle
					if (std::sqrt((float)(x * x) + (float)(y * y)) >= m_BrushSize) continue;
					break;
				case BrushType::square:
					//don't need to skip any
					break;
				}

				uint32_t color = m_World->m_ElementData[m_SelectedElementIndex].color;
				
				float r = float(color & 0x000000FF) / 255.0f;
				float g = float((color & 0x0000FF00) >> 8) / 255.0f;
				float b = float((color & 0x00FF0000) >> 16) / 255.0f;
				float a = float((color & 0xFF000000) >> 24) / 255.0f;
				glm::vec4 vecColor = glm::vec4(r,g,b,a * 0.5f);
				
				//draw square at that pixel
				float pixelSize = 1.0f / (float)CHUNKSIZE;
				//
				Renderer2D::DrawQuad((glm::vec3(newPos.x, newPos.y, 0) / (float)CHUNKSIZE) + glm::vec3(pixelSize / 2, pixelSize / 2, 5), glm::vec2(pixelSize, pixelSize), vecColor);
				
			}
		}
		
	}


}