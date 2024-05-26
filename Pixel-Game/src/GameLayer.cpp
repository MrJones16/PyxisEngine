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
		m_ElementsMap["Air"] = Element();
		m_IndexToElement[0] = "Air";

		Element stone = Element();
		stone.m_Color = 0xFF888888;
		stone.m_Type = ElementType::solid;
		stone.m_SubType = ElementSubType::Static;
		stone.m_ID == 1;
		m_ElementsMap["Stone"] = stone;
		m_IndexToElement[1] = "Stone";


		Element sand = Element();
		sand.m_ID = 2;
		sand.m_Type = ElementType::solid;
		sand.m_SubType = ElementSubType::None;
		sand.m_Tags = 0;
		sand.m_Color = 0xFF00FFFF;
		m_ElementsMap["Sand"] = sand;
		m_IndexToElement[2] = "Sand";

		Element water = Element();
		water.m_ID = 3;
		water.m_Type = ElementType::liquid;
		water.m_SubType = ElementSubType::None;
		water.m_Tags = 0;
		water.m_Color = 0xFFFF0000;
		m_ElementsMap["Water"] = water;
		m_IndexToElement[3] = "Water";
		
	}

	void GameLayer::OnAttach()
	{
		//create the world
		m_World = CreateRef<World>();
		m_World->AddChunk(glm::ivec2(0, 0));

		//create a border around first chunk
		for (int i = 0; i < m_World->CHUNKSIZE; i++)
		{
			Element stone = m_ElementsMap["Stone"];
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

			PaintBrushHologram();
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
						ImGui::PushID(index);
						std::string name = (m_IndexToElement.find(index) != m_IndexToElement.end() ? m_IndexToElement[index] : "Air");
						if (index < m_IndexToElement.size() && ImGui::Selectable(name.c_str(), m_SelectedElementIndex == index, 0, ImVec2(50, 25)))
						{
							// Toggle clicked cell + toggle neighbors
							m_SelectedElementIndex = index;
							if (m_SelectedElementIndex >= m_IndexToElement.size())
							{
								m_SelectedElementIndex = 0;
							}
							
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
				Element element = m_ElementsMap[m_IndexToElement[m_SelectedElementIndex]];
				int r = element.m_Color & 0x000000FF;
				int g = (element.m_Color & 0x0000FF00) >> 8;
				int b = (element.m_Color & 0x00FF0000) >> 16;
				int a = (element.m_Color & 0xFF000000) >> 24;
				//randomize color
				#define random ((std::rand() % 40) - 20) //-20 to 20
				r = std::max(std::min(255, r + random), 0);
				g = std::max(std::min(255, g + random), 0);
				b = std::max(std::min(255, b + random), 0);
				//a = std::max(std::min(255, a + random), 0);
				
				element.m_Color = ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)g << 8) | ((uint32_t)r << 0);

				//set element and add chunk to update map
				m_World->SetElement(newPos, element);

				chunk = m_World->GetChunk(m_World->PixelToChunk(newPos));
				index = m_World->PixelToIndex(newPos);
				chunk->UpdateDirtyRect(index.x, index.y);
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
		std::unordered_map<glm::ivec2, Chunk*, HashVector> map;

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

				//get chunk, and skip if doesn't exist yet
				glm::ivec2 chunkPos = m_World->PixelToChunk(newPos);
				auto it = m_World->m_Chunks.find(chunkPos);
				if (it == m_World->m_Chunks.end())
					continue;

				//get chunk and index
				chunk = m_World->GetChunk(chunkPos);
				index = m_World->PixelToIndex(newPos);
				//set color and add to list to update texture at end
				uint32_t color = m_ElementsMap[m_IndexToElement[m_SelectedElementIndex]].m_Color;
				color &= 0x00FFFFFF;
				color += 0x88000000;
				chunk->m_PixelBuffer[index.x + index.y * CHUNKSIZE] = color;
				map[chunk->m_ChunkPos] = chunk;
			}
		}
		for each (auto & pair in map)
		{
			//pair.second->m_Texture->SetData(pair.second->m_PixelBuffer, sizeof(pair.second->m_PixelBuffer));
			pair.second->UpdateTextureForHologram();
		}
	}

}