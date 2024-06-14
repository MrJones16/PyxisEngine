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

#if PX_PROFILING
#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](ProfileResult profileResult) {m_ProfilingPanel->m_ProfileResults.push_back(profileResult);})
#else
#define PROFILE_SCOPE(name)
#endif

	GameLayer::GameLayer()
		: Layer("GameLayer"),
		m_OrthographicCameraController(2, 1 / 1, -100, 100)
	{

	}

	GameLayer::~GameLayer()
	{
		PX_TRACE("Game Layer Deleted");
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

		m_ViewportSize = { Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight() };
		Renderer2D::Init();

		m_ActiveScene = CreateRef<Scene>();

		//Create panels to add
		m_ProfilingPanel = CreateRef<ProfilingPanel>();
		m_Panels.push_back(m_ProfilingPanel);
		FrameBufferSpecification fbspec;
		fbspec.Width = m_ViewportSize.x;
		fbspec.Height = m_ViewportSize.y;
		m_SceneFrameBuffer = FrameBuffer::Create(fbspec);


	}

	void GameLayer::OnDetach()
	{
		PX_TRACE("Detatched game layer");
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
			m_SceneFrameBuffer->Bind();
			RenderCommand::SetClearColor({ 198 / 255.0f, 239/255.0f, 249/255.0f, 1 });
			RenderCommand::Clear();
			Renderer2D::BeginScene((m_ActiveScene->m_ActiveCamera) ? *(m_ActiveScene->m_ActiveCamera) : m_OrthographicCameraController.GetCamera());
		}

		{
			PROFILE_SCOPE("Game Update");
			
			auto [x, y] = GetMousePositionScene();
			auto vec = m_OrthographicCameraController.MouseToWorldPos(x, y);
			
			glm::ivec2 pixelPos = m_World->WorldToPixel(vec);

			auto chunkPos = m_World->PixelToChunk(pixelPos);
			auto it = m_World->m_Chunks.find(chunkPos);
			if (it != m_World->m_Chunks.end())
			{
				auto index = m_World->PixelToIndex(pixelPos);
				m_HoveredElement = it->second->m_Elements[index.x + index.y * CHUNKSIZE];
				if (m_HoveredElement.m_ID >= m_World->m_TotalElements)
				{
					//something went wrong? how?
					m_HoveredElement = Element();
				}
			}
			else
			{
				m_HoveredElement = Element();
			}
			

			if (Input::IsMouseButtonPressed(0) && !m_Hovering)
			{
				PaintElementAtCursor();
			}

			if (m_SimulationRunning)
			{
				//keep track of when the world was updated, and only update it if
				//enough time has passed for the next update to be ready
				auto time = std::chrono::high_resolution_clock::now();
				if (m_UpdatesPerSecond > 0 &&
					std::chrono::time_point_cast<std::chrono::microseconds>(time).time_since_epoch().count()
					-
					std::chrono::time_point_cast<std::chrono::microseconds>(m_UpdateTime).time_since_epoch().count()
					>= (1.0f / m_UpdatesPerSecond) * 1000000.0f)
				{
					m_World->UpdateWorld();
					m_UpdateTime = time;
				}

			}
		}

		{
			PROFILE_SCOPE("Renderer Draw");
			m_World->RenderWorld();
			PaintBrushHologram();
			//draw rigid body outline
			//horizontals
			glm::vec2 worldMin = glm::vec2((float)m_RigidMin.x / (float)CHUNKSIZE, (float)m_RigidMin.y / (float)CHUNKSIZE);
			glm::vec2 worldMax = glm::vec2((float)m_RigidMax.x / (float)CHUNKSIZE, (float)m_RigidMax.y / (float)CHUNKSIZE);
			Renderer2D::DrawLine({ worldMin.x, worldMin.y }, { worldMax.x, worldMin.y});
			Renderer2D::DrawLine({ worldMin.x, worldMax.y }, { worldMax.x, worldMax.y});
			//vertical lines
			Renderer2D::DrawLine({ worldMin.x, worldMin.y }, { worldMin.x, worldMax.y});
			Renderer2D::DrawLine({ worldMax.x, worldMin.y }, { worldMax.x, worldMax.y});
		}

		Renderer2D::EndScene();

		m_SceneFrameBuffer->Unbind();
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
				ImGui::Text("nothing here yet!");
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
		

		if (ImGui::Begin("Scene", (bool*)0, ImGuiWindowFlags_NoTitleBar))
		{
			m_SceneViewIsFocused = ImGui::IsWindowFocused();
			Application::Get().GetImGuiLayer()->BlockEvents(false);
			auto sceneViewSize = ImGui::GetContentRegionAvail();
			//ImGui::GetForegroundDrawList()->AddRect(minPos, maxPos, ImU32(0xFFFFFFFF));
			
			
			//PX_TRACE("offset: ({0},{1})", m_ViewportOffset.x, m_ViewportOffset.y);
			//PX_TRACE("window pos: ({0},{1})", ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
			//PX_TRACE("item rect min: ({0},{1})", ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y);
			//PX_TRACE("scene view size: ({0},{1})", sceneViewSize.x, sceneViewSize.y);
			ImGui::Image(
				(ImTextureID)m_SceneFrameBuffer->GetColorAttatchmentRendererID(),
				sceneViewSize,
				ImVec2(0, 1),
				ImVec2(1, 0),
				ImVec4(1, 1, 1, 1)
				//ImVec4(1, 1, 1, 1) border color
			);
			m_ViewportOffset = ImGui::GetItemRectMin();
			

			if (m_ViewportSize.x != sceneViewSize.x || m_ViewportSize.y != sceneViewSize.y)
			{
				m_OrthographicCameraController.SetAspect(sceneViewSize.y / sceneViewSize.x);
				m_SceneFrameBuffer->Resize((uint32_t)sceneViewSize.x, (uint32_t)sceneViewSize.y);
				m_ViewportSize = { sceneViewSize.x, sceneViewSize.y };
			}

			ImGui::End();
		}
		ImGui::PopStyleVar();

		
		if (ImGui::Begin("Settings"))
		{
			ImGui::SetNextItemOpen(true);
			if (ImGui::TreeNode("Simulation"))
			{
				ImGui::DragFloat("Updates Per Second", &m_UpdatesPerSecond, 1, 0, 244);
				ImGui::SetItemTooltip("Default: 60");
				if (m_SimulationRunning)
				{
					if (ImGui::Button("Pause"))
					{
						m_SimulationRunning = false;
					}
					ImGui::SetItemTooltip("Shortcut: Space");
				}
				else
				{
					if (ImGui::Button("Play"))
					{
						m_SimulationRunning = true;
					}
					ImGui::SetItemTooltip("Shortcut: Space");
					//ImGui::EndTooltip();
				}

				if (ImGui::Button("Clear"))
				{
					m_World->Clear();
				}

				if (ImGui::Button("Build Rigid Body"))
				{
					if (m_RigidMin.x < m_RigidMax.x)
					{
						m_World->CreatePixelRigidBody(m_RigidMin, m_RigidMax);

						m_RigidMin = { 9999999, 9999999 };
						m_RigidMax = { -9999999, -9999999 };

					}
				}

				if (ImGui::Button("Build Static Rigid Body"))
				{
					if (m_RigidMin.x < m_RigidMax.x)
					{
						m_World->CreatePixelRigidBody(m_RigidMin, m_RigidMax, b2_staticBody);

						m_RigidMin = { 9999999, 9999999 };
						m_RigidMax = { -9999999, -9999999 };

					}
				}
				
				
				ImGui::TreePop();
			}

			ImGui::SetNextItemOpen(true);
			if (ImGui::TreeNode("Hovered Element Properties"))
			{
				ElementData& elementData = m_World->m_ElementData[m_HoveredElement.m_ID];
				ImGui::Text("Element: %s", elementData.name.c_str());
				ImGui::Text("- Temperature: %f", m_HoveredElement.m_Temperature);

				ImGui::TreePop();
			}

			ImGui::SetNextItemOpen(true);
			if (ImGui::TreeNode("Brush Shape"))
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
				float width = ImGui::GetContentRegionAvail().x / 5;
				for (int y = 0; y < (m_World->m_TotalElements / 4) + 1; y++)
					for (int x = 0; x < 4; x++)
					{
						if (x > 0)
							ImGui::SameLine();
						int index = y * 4 + x;
						if (index >= m_World->m_TotalElements) continue;
						ImGui::PushID(index);
						auto color = m_World->m_ElementData[index].color;
						int r = (color & 0x000000FF) >> 0;
						int g = (color & 0x0000FF00) >> 8;
						int b = (color & 0x00FF0000) >> 16;
						int a = (color & 0xFF000000) >> 24;
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f));
						std::string name = (m_World->m_ElementData[index].name);
						;
						if (ImGui::Selectable(name.c_str(), m_SelectedElementIndex == index, 0, ImVec2(width, 25)))
						{
							// Toggle clicked cell
							m_SelectedElementIndex = index;
						}
						ImGui::PopStyleColor();
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
		//m_OrthographicCameraController.OnEvent(e);
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(PX_BIND_EVENT_FN(GameLayer::OnWindowResizeEvent));
		dispatcher.Dispatch<KeyPressedEvent>(PX_BIND_EVENT_FN(GameLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<MouseButtonPressedEvent>(PX_BIND_EVENT_FN(GameLayer::OnMouseButtonPressedEvent));
		dispatcher.Dispatch<MouseScrolledEvent>(PX_BIND_EVENT_FN(GameLayer::OnMouseScrolledEvent));
	}

	std::pair<float, float> GameLayer::GetMousePositionScene()
	{
		///if not using a framebuffer / imgui image, just use Pyxis::Input::GetMousePosition();
		
		float x = ImGui::GetMousePos().x;
		float y = ImGui::GetMousePos().y;
		x -= m_ViewportOffset.x;
		y -= m_ViewportOffset.y;
		
		//PX_TRACE("mouse position in scene: ({0},{1})", x, y);
		
		x = (x / m_ViewportSize.x) * Application::Get().GetWindow().GetWidth();
		y = (y / m_ViewportSize.y) * Application::Get().GetWindow().GetHeight();


		return std::pair<float, float>(x, y);
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
		if (event.GetMouseButton() == PX_MOUSE_BUTTON_FORWARD) // forward
		{
			m_BrushSize++;
		}
		if (event.GetMouseButton() == PX_MOUSE_BUTTON_BACK) // back
		{
			m_BrushSize--;
			if (m_BrushSize < 1) m_BrushSize == 1;
		}

		return false;
	}

	bool GameLayer::OnMouseScrolledEvent(MouseScrolledEvent& event)
	{
		if (Input::IsKeyPressed(PX_KEY_LEFT_CONTROL))
		{
			m_BrushSize += event.GetYOffset();
			if (m_BrushSize < 1) m_BrushSize = 1;
			if (m_BrushSize > 32) m_BrushSize = 32;
		}
		else if (Input::IsKeyPressed(PX_KEY_LEFT_ALT))
		{
			m_OrthographicCameraController.m_CameraSpeed *= 1 + (event.GetYOffset() / 10);
		}
		else
		{
			m_OrthographicCameraController.Zoom(1 - (event.GetYOffset() / 10));
		}
		
		return false;
	}

	void GameLayer::PaintElementAtCursor()
	{
		//paint a rigid body by tracking min and max
		std::unordered_map<glm::ivec2, Chunk*, HashVector> map;

		auto [x, y] = GetMousePositionScene();
		int max_width  = Application::Get().GetWindow().GetWidth();
		int max_height = Application::Get().GetWindow().GetHeight();
		if (x > max_width || x < 0 || y > max_height || y < 0 ) return;
		auto vec = m_OrthographicCameraController.MouseToWorldPos(x, y);
		glm::ivec2 pixelPos = m_World->WorldToPixel(vec);


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
				//element.m_BaseColor = Pyxis::RandomizeABGRColor(elementData.color, 20);

				chunk = m_World->GetChunk(m_World->PixelToChunk(newPos));
				index = m_World->PixelToIndex(newPos);
				elementData.UpdateElementData(element, index.x, index.y);
				element.m_Color = element.m_BaseColor;
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
				{
					m_World->SetElement(newPos, element);
					//update rigid min and max
					if (newPos.x < m_RigidMin.x) m_RigidMin.x = newPos.x;
					if (newPos.y < m_RigidMin.y) m_RigidMin.y = newPos.y;
					if (newPos.x > m_RigidMax.x) m_RigidMax.x = newPos.x;
					if (newPos.y > m_RigidMax.y) m_RigidMax.y = newPos.y;
				}

				
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

		auto [x, y] = GetMousePositionScene();
		auto vec = m_OrthographicCameraController.MouseToWorldPos(x, y);
		glm::ivec2 pixelPos = m_World->WorldToPixel(vec);

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