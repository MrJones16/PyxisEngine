#include "GameLayer.h"

#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include <Platform/OpenGL/OpenGLShader.h>
#include <chrono>
#include <variant>

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
		m_OrthographicCameraController(2, 1 / 1, -100, 100), m_CurrentTickClosure()
	{
		
	}

	GameLayer::~GameLayer()
	{
		PX_TRACE("Game Layer Deleted");
	}

	void GameLayer::OnAttach()
	{
		//Connect to the server:
		if (!m_ClientInterface.Connect("127.0.0.1", 21218)) {
			PX_ERROR("Failed to connect to the server...");
			Application::Get().Sleep(2000);
			Application::Get().Close();
			return;
		}


		m_ViewportSize = { Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight() };
		Renderer2D::Init();

		//m_ActiveScene = CreateRef<Scene>();

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
		PX_TRACE("Detached game layer");
	}


	void GameLayer::OnUpdate(Timestep ts)
	{
		//PROFILE_SCOPE("GameLayer::OnUpdate");

		HandleMessages();

		if (m_Connecting)
		{
			//still connecting...

			//while we are connecting, if we have the world data, it is still our
			//job to catch up to the server, which means 
			if (!m_WaitForWorldData)
			{
				//we are no longer waiting on the world data, so its time to catch up
				//all the ticks we missed while getting game data
				if (m_MTCQueue.size() > 0)
				{
					//as the client, i now need to remove the input actions from the 
					//latency queue for the tick i recieved, but not
					//in this section where i'm not sending any ticks


					if (m_MTCQueue.front().m_Tick < m_InputTick)
					{
						//we were sent a merged tick closure before we requested data, so ignore this since
						//the world state we recieved already had this tick applied
						m_MTCQueue.pop_front();
						return;
					}
					m_LatencyStateReset = true;
					PX_TRACE("Applying tick {0} to sim {1}", m_MTCQueue.front().m_Tick, m_World->m_SimulationTick);
					HandleTickClosure(m_MTCQueue.front());
					m_MTCQueue.pop_front();
					m_InputTick++;
				}
				if (m_InputTick == m_TickToEnter)
				{
					PX_TRACE("Reached Tick: {0}, I'm caught up!", m_TickToEnter);
					//we loaded the world, and we are caught up to what the
					//server has sent, since we reached the tick to enter

					//begin the game and start sending ticks! 
					//the server is waiting for us!
					m_Connecting = false;
					m_SimulationRunning = true;
				}
				
			}
			return;
		}

		//go through all the recieved merged tick closures and resolve them
		while (m_MTCQueue.size() > 0)
		{
			//TODO
			//as the client, i now need to remove the input actions from the 
			//latency queue for the tick i recieved
			m_LatencyStateReset = true;

			//skip any tick closures we have already finished, since we might get duplicates
			if (m_MTCQueue.front().m_Tick <= m_LatestMergedTick) {
				m_MTCQueue.pop_front();
				continue;
			}

			//if the tick closure we are doing is one for the future, we need to ask
			//the server to re-send the missing closure
			if (m_MTCQueue.front().m_Tick > m_LatestMergedTick + 1) {
				//ask the server for the missing tick...
				Network::Message<GameMessage> msg;
				msg.header.id = GameMessage::Client_RequestMergedTick;
				msg << m_LatestMergedTick + uint64_t(1);
				m_ClientInterface.SendUDP(msg);
				break; // we need to break out of while loop to recieve the new message
			}

			//reset world, then apply the tick closure
			if (m_World->m_SimulationTick == m_TickToResetBox2D)
			{
				m_World->ResetBox2D();
				m_TickToResetBox2D = -1;
			}
			
			HandleTickClosure(m_MTCQueue.front());
			m_LatestMergedTick = m_MTCQueue.front().m_Tick;
			//PX_TRACE("Handling a tick closure for tick: {0}!", m_MTCQueue.front().m_Tick);
			m_MTCQueue.pop_front();
			//dont increment game tick, because that is the "heartbeat" of constant input messages to align
			//the game tick doesn't represent how many times the simulation has been stepped, but the amount
			//of input messages sent
		}

		//TODO: replace this with actual input queue, and expected ping time.
		{
			//just using the latency queue limit, as that will replace this in the future
			if (m_InputTick - m_LatestMergedTick > m_LatencyQueueLimit)
			{


				//we are ahead by the limit, so disable ourselves from adding any more until we let everyone else catch back up,
				//at least until we hit the length we want to be at for the expected ping,
				//for now, just clear our newest inputs, so we don't send a fat tick closure, and wait till we get half in queue

				m_WaitingForOthers = true;
			}

			if (m_InputTick - m_LatestMergedTick < m_LatencyQueueLimit / 2)
			{
				//we let everyone get caught up at least half way
				m_WaitingForOthers = false;
			}

			if (m_WaitingForOthers)
			{
				m_CurrentTickClosure = TickClosure();
			}
		}
		


		if (m_LatencyStateReset)
		{
			//i need to reset the latency state, and show what the world 
			//should look like with the latency inputs
			//m_LatencyWorld = m_World;
		}

		//handling messages will clear the latency state, as well as
		//remove the input that was added, so now i get my new inputs for
		//the next tick, and add them to the queue

		if (m_LatencyInputQueue.size() >= m_LatencyQueueLimit)
		{
			//this will be important for not spamming the server!
			// 
			//we hit the limit of prediction, so skip furthering the simulation
			return;
		}


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
			Renderer2D::BeginScene(m_OrthographicCameraController.GetCamera());
		}

		{
			PROFILE_SCOPE("Game Update");
			
			auto [x, y] = GetMousePositionScene();
			auto vec = m_OrthographicCameraController.MouseToWorldPos(x, y);
			
			glm::ivec2 mousePixelPos = m_World->WorldToPixel(vec);

			auto chunkPos = m_World->PixelToChunk(mousePixelPos);
			auto it = m_World->m_Chunks.find(chunkPos);
			if (it != m_World->m_Chunks.end())
			{
				auto index = m_World->PixelToIndex(mousePixelPos);
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
				auto [x, y] = GetMousePositionScene();
				int max_width = Application::Get().GetWindow().GetWidth();
				int max_height = Application::Get().GetWindow().GetHeight();
				if (x > max_width || x < 0 || y > max_height || y < 0) return;
				auto vec = m_OrthographicCameraController.MouseToWorldPos(x, y);
				glm::ivec2 pixelPos = m_World->WorldToPixel(vec);
				if (m_BuildingRigidBody)
				{
					if (pixelPos.x < m_RigidMin.x) m_RigidMin.x = pixelPos.x;
					if (pixelPos.x > m_RigidMax.x) m_RigidMax.x = pixelPos.x;
					if (pixelPos.y < m_RigidMin.y) m_RigidMin.y = pixelPos.y;
					if (pixelPos.y > m_RigidMax.y) m_RigidMax.y = pixelPos.y;
				}
				else
				{
					m_CurrentTickClosure.AddInputAction(
						InputAction::Input_Place, 
						(uint8_t)m_BrushSize,
						(uint16_t)m_BrushType,
						(uint32_t)m_SelectedElementIndex,
						pixelPos, 
						m_ClientInterface.GetID());
				}
			}

			if (m_LatencyInputQueue.size() < m_LatencyQueueLimit && !m_WaitingForOthers)
			{
				//keep track of how fast we should be sending our input action updates
				auto time = std::chrono::high_resolution_clock::now();
				if (m_UpdatesPerSecond > 0 &&
					std::chrono::time_point_cast<std::chrono::microseconds>(time).time_since_epoch().count()
					-
					std::chrono::time_point_cast<std::chrono::microseconds>(m_UpdateTime).time_since_epoch().count()
					>= (1.0f / m_UpdatesPerSecond) * 1000000.0f)
				{
					//add the current mouse pos as a input action, to show other players
					//where you are looking
					m_CurrentTickClosure.AddInputAction(InputAction::Input_MousePosition, m_ClientInterface.GetID(), mousePixelPos);


					Network::Message<GameMessage> msg;
					msg.header.id = GameMessage::Game_TickClosure;
					msg << m_CurrentTickClosure.m_Data;
					msg << m_CurrentTickClosure.m_InputActionCount;
					msg << m_InputTick;
					msg << m_ClientInterface.GetID();
					m_ClientInterface.SendUDP(msg);
					//PX_TRACE("sent tick closure for tick {0}", m_InputTick);

					m_InputTick++;

					//reset tick closure
					m_CurrentTickClosure = TickClosure();
					//m_CurrentTickClosure.m_Tick = m_InputTick;
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

			for each (auto playerCursor in m_PlayerCursors)
			{
				//skip your own mouse pos
				if (playerCursor.first == m_ClientInterface.GetID()) continue;
				//draw the 3x3 square for each players cursor
				glm::vec3 worldPos = glm::vec3((float)playerCursor.second.pixelPosition.x / CHUNKSIZE, (float)playerCursor.second.pixelPosition.y / CHUNKSIZE, 5);
				glm::vec2 size = glm::vec2(3.0f / CHUNKSIZE);
				Renderer2D::DrawQuad(worldPos, size, playerCursor.second.color);
			}
		}

		Renderer2D::EndScene();

		m_SceneFrameBuffer->Unbind();
	}

	void GameLayer::OnImGuiRender()
	{
		auto dock = ImGui::DockSpaceOverViewport((const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode);

		if (m_Connecting)
		{
			//show that we are connecting
			ImGui::SetNextWindowDockID(dock);
			std::string text = "Connecting To Server...";
			if (ImGui::Begin("Connecting Screen", (bool*)0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
			{
				TextCentered(text);
			}
			ImGui::End();
			return;
		}
		//ImGui::ShowDemoWindow();
		
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
		
		ImGui::SetNextWindowDockID(dock);
		if (ImGui::Begin("Scene", (bool*)0, ImGuiWindowFlags_NoTitleBar))
		{
			m_SceneViewIsFocused = ImGui::IsWindowFocused();
			Application::Get().GetImGuiLayer()->BlockEvents(false);
			auto sceneViewSize = ImGui::GetContentRegionAvail();
			//ImGui::GetForegroundDrawList()->AddRect(minPos, maxPos, ImU32(0xFFFFFFFF));
			ImGui::Image(
				(ImTextureID)m_SceneFrameBuffer->GetColorAttachmentRendererID(),
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
				//ImGui::DragFloat("Updates Per Second", &m_UpdatesPerSecond, 1, 0, 244);
				//ImGui::SetItemTooltip("Default: 60");
				if (m_World->m_Running)
				{
					if (ImGui::Button("Pause"))
					{
						m_CurrentTickClosure.AddInputAction(InputAction::PauseGame, m_ClientInterface.GetID());
					}
					ImGui::SetItemTooltip("Shortcut: Space");
				}
				else
				{
					if (ImGui::Button("Play"))
					{
						m_CurrentTickClosure.AddInputAction(InputAction::ResumeGame, m_ClientInterface.GetID());
					}
					ImGui::SetItemTooltip("Shortcut: Space");
					//ImGui::EndTooltip();
				}

				if (ImGui::Button("Clear"))
				{
					m_CurrentTickClosure.AddInputAction(InputAction::ClearWorld);
				}

				if (ImGui::Button("Toggle Collider View"))
				{
					m_World->m_DebugDrawColliders = !m_World->m_DebugDrawColliders;
				}
				
				if (m_BuildingRigidBody)
				{
					if (ImGui::Button("Build Rigid Body"))
					{
						srand(time(0));
						uint64_t uuid = std::rand();
						while (m_World->m_PixelBodyMap.find(uuid) != m_World->m_PixelBodyMap.end())
						{
							srand(time(0));
							uint64_t uuid = std::rand();
						}
						if (m_RigidMin.x < m_RigidMax.x)
							m_CurrentTickClosure.AddInputAction(InputAction::TransformRegionToRigidBody, b2_dynamicBody, m_RigidMin, m_RigidMax, uuid);
						m_RigidMin = { 9999999, 9999999 };
						m_RigidMax = { -9999999, -9999999 };
					}
					if (ImGui::Button("Build Kinematic Rigid Body"))
					{
						srand(time(0));
						uint64_t uuid = std::rand();
						while (m_World->m_PixelBodyMap.find(uuid) != m_World->m_PixelBodyMap.end())
						{
							srand(time(0));
							uint64_t uuid = std::rand();
						}
						if (m_RigidMin.x < m_RigidMax.x)
							m_CurrentTickClosure.AddInputAction(InputAction::TransformRegionToRigidBody, b2_kinematicBody, m_RigidMin, m_RigidMax, uuid);
						m_RigidMin = { 9999999, 9999999 };
						m_RigidMax = { -9999999, -9999999 };
					}

					if (ImGui::Button("Stop Building Rigid Body"))
					{
						m_RigidMin = { 9999999, 9999999 };
						m_RigidMax = { -9999999, -9999999 };
						m_BuildingRigidBody = false;
					}
				}
				else
				{
					if (ImGui::Button("Start Building Rigid Body"))
					{
						m_BuildingRigidBody = true;
					}
				}

				/*ImGui::SliderFloat("Douglas-Peucker Threshold", &m_DouglasThreshold, 0, 2);

				if (ImGui::Button("UpdateOutline"))
				{
					for each (auto body in m_World->m_PixelBodies)
					{
						auto contour = body->GetContourPoints();
						body->m_ContourVector = body->SimplifyPoints(contour, 0, contour.size() - 1, m_DouglasThreshold);
					}
				}*/
				
				
				ImGui::TreePop();
			}

			ImGui::SetNextItemOpen(true);
			if (ImGui::TreeNode("Hovered Element Properties"))
			{
				auto [x, y] = GetMousePositionScene();
				auto vec = m_OrthographicCameraController.MouseToWorldPos(x, y);
				glm::ivec2 pixelPos = m_World->WorldToPixel(vec);
				ImGui::Text(("(" + std::to_string(pixelPos.x) + ", " + std::to_string(pixelPos.y) + ")").c_str());
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

	void GameLayer::TextCentered(std::string text)
	{
		float win_width = ImGui::GetWindowSize().x;
		float text_width = ImGui::CalcTextSize(text.c_str()).x;

		// calculate the indentation that centers the text on one line, relative
		// to window left, regardless of the `ImGuiStyleVar_WindowPadding` value
		float text_indentation = (win_width - text_width) * 0.5f;

		// if text is too long to be drawn on one line, `text_indentation` can
		// become too small or even negative, so we check a minimum indentation
		float min_indentation = 20.0f;
		if (text_indentation <= min_indentation) {
			text_indentation = min_indentation;
		}
		ImGui::SameLine(text_indentation);
		ImGui::PushTextWrapPos(win_width - text_indentation);
		ImGui::TextWrapped(text.c_str());
		ImGui::PopTextWrapPos();
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

	void GameLayer::HandleMessages()
	{
		if (m_ClientInterface.IsConnected())
		{
			while (!m_ClientInterface.Incoming().empty())
			{
				auto msg = m_ClientInterface.Incoming().pop_front().msg;
				switch (msg.header.id)
				{
				case GameMessage::Ping:
				{
					break;
				}
				case GameMessage::Server_ClientAccepted:
				{
					//Server has told us we are validated! so ask it for my ID
					Network::Message<GameMessage> msg;
					msg.header.id = GameMessage::Client_RegisterWithServer;
					m_ClientInterface.Send(msg);

					break;
				}
				case GameMessage::Server_ClientAssignID:
				{
					//server responded with our ID 
					uint64_t id;
					msg >> id;
					m_ClientInterface.SetID(std::move(id));
					
					//now ask the server for the game data
					Network::Message<GameMessage> msg;
					msg.header.id = GameMessage::Game_RequestGameData;
					m_ClientInterface.Send(msg);

					break;
				}
				case GameMessage::Server_ClientConnected:
				{
					//someone else joined, so we will:
					// * keep track of their mouse position to draw it
					//
					uint64_t id;
					msg >> id;

					m_PlayerCursors[id] = PlayerCursor(id);
					break;
				}
				//update ping case? for better prediction
				case GameMessage::Game_GameData:
				{
					msg >> m_InputTick;
					CreateWorld();
					msg >> m_World->m_Running;
					m_World->LoadWorld(msg);
					m_LatestMergedTick = m_InputTick;
					
					PX_TRACE("loaded game at tick {0}", m_InputTick);
					PX_TRACE("Game simulation tick is {0}", m_World->m_SimulationTick);

					//now that i have finished downloading the world
					// (which might have taken a while)
					// tell the server i am done

					
					m_WaitForWorldData = false;
					//now catch up from all the inputs sent while loading
					HandleMessages();
					//now that i caught up on all the messages, be finished connecting
					//and start sending my own ticks, and make others wait
					Network::Message<GameMessage> msg;
					msg.header.id = GameMessage::Game_Loaded;
					m_ClientInterface.Send(msg);
					break;
				}
				case GameMessage::Game_TickToEnter:
				{
					msg >> m_TickToEnter;
					PX_TRACE("Tick to enter at: {0}", m_TickToEnter);
					break;
				}
				case GameMessage::Game_ResetBox2D:
				{
					//gather all rigid body data, store it, and reload it!
					//this has to be done once we are finished with all the previously
					//collected mtc's, so we will mark when we are supposed to
					//reset, and do it then!
					msg >> m_TickToResetBox2D;
					PX_TRACE("Sim Tick to reset at: {0}", m_TickToResetBox2D);
					PX_TRACE("current sim tick: {0}", m_World->m_SimulationTick);
					break;
				}
				case GameMessage::Game_MergedTickClosure:
				{
					MergedTickClosure mtc;
					msg >> mtc.m_Tick;
					msg >> mtc.m_InputActionCount;
					msg >> mtc.m_Data;
					
					if (m_MTCQueue.size() > 0 && m_MTCQueue.front().m_Tick > mtc.m_Tick)
					{
						m_MTCQueue.push_front(std::move(mtc));
						//add to the front since we are probably a catch-up tick
					}
					else
					{
						//throw on the back
						m_MTCQueue.push_back(std::move(mtc));
					}

					break;
				}
				default:
					break;
				}
			}
		}
		else
		{
			//lost connection
			PX_WARN("Lost connection to server.");
			//Application::Get().Sleep(2000);
			Application::Get().Close();
		}
	}

	void GameLayer::HandleTickClosure(MergedTickClosure& tc)
	{
		for (int i = 0; i < tc.m_InputActionCount; i++)
		{
			InputAction IA;
			tc >> IA;
			switch (IA)
			{
			case InputAction::Add_Player:
			{
				uint64_t ID;
				tc >> ID;

				glm::ivec2 pixelPos;
				tc >> pixelPos;

				m_World->CreatePlayer(ID, pixelPos);
				break;
			}
			case InputAction::PauseGame:
			{
				uint64_t ID;
				tc >> ID;
				m_World->m_Running = false;
				break;
			}
			case InputAction::ResumeGame:
			{
				uint64_t ID;
				tc >> ID;
				m_World->m_Running = true;
				break;
			}
			case InputAction::TransformRegionToRigidBody:
			{
				uint64_t ID;
				tc >> ID;

				glm::ivec2 maximum;
				tc >> maximum;

				glm::ivec2 minimum;
				tc >> minimum;

				b2BodyType type;
				tc >> type;

				int width = (maximum.x - minimum.x) + 1;
				int height = (maximum.y - minimum.y) + 1;
				if (width * height <= 0) break;
				glm::ivec2 newMin = maximum;
				glm::ivec2 newMax = minimum;
				//iterate over section and find the width, height, center, ect
				int mass = 0;
				for (int x = 0; x < width; x++)
				{
					for (int y = 0; y < height; y++)
					{
						glm::ivec2 pixelPos = glm::ivec2(x + minimum.x, y + minimum.y);
						auto& element = m_World->GetElement(pixelPos);
						auto& elementData = m_World->m_ElementData[element.m_ID];
						if ((elementData.cell_type == ElementType::solid || elementData.cell_type == ElementType::movableSolid) && element.m_Rigid == false)
						{
							if (pixelPos.x < newMin.x) newMin.x = pixelPos.x;
							if (pixelPos.y < newMin.y) newMin.y = pixelPos.y;
							if (pixelPos.x > newMax.x) newMax.x = pixelPos.x;
							if (pixelPos.y > newMax.y) newMax.y = pixelPos.y;
							mass++;
						}
					}
				}
				if (mass < 2) continue;//skip if we are 1 element or 0
				PX_TRACE("transforming {0} elements to a rigid body", mass);

				width = (newMax.x - newMin.x) + 1;
				height = (newMax.y - newMin.y) + 1;

				glm::ivec2 origin = { width / 2, height / 2 };
				std::unordered_map<glm::ivec2, RigidBodyElement, HashVector> elements;
				for (int x = 0; x < width; x++)
				{
					for (int y = 0; y < height; y++)
					{
						glm::ivec2 pixelPos = { x + newMin.x, y + newMin.y };

						//loop over every element, grab it, and make it rigid if it is a movable Solid
						auto& element = m_World->GetElement(pixelPos);
						auto& elementData = m_World->m_ElementData[element.m_ID];
						if ((elementData.cell_type == ElementType::solid || elementData.cell_type == ElementType::movableSolid) && element.m_Rigid == false)
						{
							element.m_Rigid = true;
							//set the elements at the local position to be the element pulled from world
							elements[glm::ivec2(x - origin.x, y - origin.y)] = RigidBodyElement(element, pixelPos);
							m_World->SetElement(pixelPos, Element());
						}
					}
				}
				glm::ivec2 size = newMax - newMin;
				PX_TRACE("Mass is: {0}", mass);
				PixelRigidBody* body = new PixelRigidBody(ID, size, elements, type, m_World->m_Box2DWorld);
				if (body->m_B2Body == nullptr)
				{
					PX_TRACE("Failed to create rigid body");
					continue;
				}
				else
				{
					m_World->m_PixelBodyMap[body->m_ID] = body;
					auto pixelPos = (newMin + newMax) / 2;
					if (width % 2 == 0) pixelPos.x += 1;
					if (height % 2 == 0) pixelPos.y += 1;
					body->SetPixelPosition(pixelPos);
					m_World->PutPixelBodyInWorld(*body);
				}


				break;
			}
			case Pyxis::InputAction::Input_Move:
			{
				//PX_TRACE("input action: Input_Move");
				break;
			}
			case Pyxis::InputAction::Input_Place:
			{
				//PX_TRACE("input action: Input_Place");
				uint64_t id;
				glm::ivec2 pixelPos;
				uint32_t elementID;
				BrushType brush;
				uint8_t brushSize;
				tc >> id >> pixelPos >> elementID >> brush >> brushSize;

				m_World->PaintBrushElement(pixelPos, elementID, brush, brushSize);
				break;
			}
			case Pyxis::InputAction::Input_StepSimulation:
			{
				PX_TRACE("input action: Input_StepSimulation");
				m_World->UpdateWorld();
				break;
			}
			case InputAction::Input_MousePosition:
			{
				glm::ivec2 mousePos;
				uint64_t ID;
				tc >> mousePos >> ID;
				if (mousePos.x < -2000000000) break;

				m_PlayerCursors[ID].pixelPosition = mousePos;
				break;
			}
			case InputAction::ClearWorld:
			{
				m_World->Clear();
				break;
			}
			default:
			{
				PX_TRACE("input action: default?");
				break;
			}
			}
		}
		if (m_World->m_Running)
		{
			m_World->UpdateWorld();
		}
		else
		{
			m_World->UpdateTextures();
		}
	}

	bool GameLayer::CreateWorld()
	{
		//create the world
		m_World = CreateRef<World>();
		if (m_World->m_Error)
		{
			PX_ERROR("Failed to create the world");
			Application::Get().Sleep(2000);
			Application::Get().Close();
			return false;
		}
		return true;
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
			if (m_World->m_Running)
			{
				m_CurrentTickClosure.AddInputAction(InputAction::PauseGame, m_ClientInterface.GetID());
			}
			else 
			{
				m_CurrentTickClosure.AddInputAction(InputAction::ResumeGame, m_ClientInterface.GetID());
			}
		}
		if (event.GetKeyCode() == PX_KEY_RIGHT)
		{
			m_CurrentTickClosure.AddInputAction(InputAction::Input_StepSimulation);
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