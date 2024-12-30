#include "GameLayer.h"

#include <glm/gtc/type_ptr.hpp>

#include <Platform/OpenGL/OpenGLShader.h>
#include <chrono>
#include <variant>

#include "Pyxis/Renderer/UI.h"

namespace Pyxis
{

	GameLayer::GameLayer(std::string debugName)
		: Layer(debugName),
		m_OrthographicCameraController(2, 1 / 1, -100, 100)//, m_CurrentTickClosure()
	{

	}

	GameLayer::~GameLayer()
	{

	}

	void GameLayer::OnAttach()
	{

		PX_TRACE("Attached game layer");

		
		///////////////////////////////
		/// Init members & Renderer2D
		///////////////////////////////
		m_ViewportSize = { Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight() };
		Renderer2D::Init();
		m_Scene = CreateRef<Scene>();
		m_Scene->m_ActiveCamera = m_OrthographicCameraController.GetCamera();

		///////////////////////////////
		/// Panels (kinda debugging)
		///////////////////////////////
		m_Panels.push_back(CreateRef<ProfilingPanel>());
		Ref<SceneHierarchyPanel> sceneHeirarchyPanel = CreateRef<SceneHierarchyPanel>(m_Scene);
		m_Panels.push_back(sceneHeirarchyPanel);
		m_Panels.push_back(CreateRef<InspectorPanel>(sceneHeirarchyPanel));

		///////////////////////////////
		/// Debug / Testing
		///////////////////////////////
		Ref<UI::UINode> UI = CreateRef<UI::UINode>();
		m_Scene->AddNode(UI);
		auto rect = CreateRef<UI::UIRect>(glm::vec4(1));
		rect->m_Texture = Texture2D::Create("assets/textures/Test.png");
		UI->AddChild(rect);

		Ref<UI::UIButton> button = CreateRef<UI::UIButton>();
		button->Translate({ 0, 1, 1 });
		UI->AddChild(button);

		FontLibrary::AddFont("Aseprite", "assets/fonts/Aseprite.ttf");
		//FontLibrary::AddFont("Arial", "assets/fonts/arial.ttf");

		UI->AddChild(CreateRef<UI::UIText>(FontLibrary::GetFont("Aseprite")));


		FrameBufferSpecification fbspec;
		fbspec.Attachments = 
		{   
			{FrameBufferTextureFormat::RGBA8, FrameBufferTextureType::Color},
			{FrameBufferTextureFormat::R32UI, FrameBufferTextureType::Color},
			{FrameBufferTextureFormat::Depth, FrameBufferTextureType::Depth}
		};
		fbspec.Width = m_ViewportSize.x;
		fbspec.Height = m_ViewportSize.y;
		m_SceneFrameBuffer = FrameBuffer::Create(fbspec);
	}

	void GameLayer::OnDetach()
	{
		PX_TRACE("Detached game layer");
		//when detaching the game layer, it should be reset so the server can be joined again.
		m_World = nullptr;

	}

	/// <summary>
	/// This is the main update loop involving actually playing the game, input, controls, ect.
	/// Also includes rendering loop
	/// 
	/// Only run this if m_World is valid and ready to be played!
	/// The output of inputs is in m_CurrentTickClosure
	/// </summary>
	/// <param name="ts"></param>
	void GameLayer::GameUpdate(Timestep ts)
	{

		{
			PROFILE_SCOPE("Game Update");
			glm::ivec2 mousePixelPos = m_World->WorldToPixel(GetMousePosWorld());

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
				glm::ivec2 mousePos = GetMousePositionImGui();
				
				if (!(mousePos.x > m_ViewportSize.x || mousePos.x < 0 || mousePos.y > m_ViewportSize.y || mousePos.y < 0))
				{
					glm::vec2 mousePercent = (glm::vec2)mousePos / m_ViewportSize;
					auto vec = m_OrthographicCameraController.MousePercentToWorldPos(mousePercent.x, mousePercent.y);
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
							false);
					}
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

	}


	void GameLayer::ClientImGuiRender(ImGuiID dockID)
	{
		m_Hovering = ImGui::IsWindowHovered();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
		ImGui::SetNextWindowDockID(dockID);
		if (ImGui::Begin("Scene", (bool*)0, ImGuiWindowFlags_NoTitleBar))
		{
			m_SceneViewIsFocused = ImGui::IsWindowFocused();

			auto viewportOffset = ImGui::GetCursorScreenPos();
			auto newViewportSize = ImGui::GetContentRegionAvail();

			m_ViewportBounds[0] = { viewportOffset.x, viewportOffset.y };
			m_ViewportBounds[1] = { viewportOffset.x + newViewportSize.x, viewportOffset.y + newViewportSize.y };

			//PX_CORE_WARN("Min Bounds: ({0},{1})", m_ViewportBounds[0].x, m_ViewportBounds[0].y);
			//PX_CORE_WARN("Max Bounds: ({0},{1})", m_ViewportBounds[1].x, m_ViewportBounds[1].y);

			Application::Get().GetImGuiLayer()->BlockEvents(false);
			//ImGui::GetForegroundDrawList()->AddRect(minPos, maxPos, ImU32(0xFFFFFFFF));
			ImGui::Image(
				(ImTextureID)(m_SceneFrameBuffer->GetColorAttachmentRendererID(0)),
				newViewportSize,
				ImVec2(0, 1),
				ImVec2(1, 0),
				ImVec4(1, 1, 1, 1)
				//ImVec4(1, 1, 1, 1) border color
			);
			m_ViewportOffset = ImGui::GetItemRectMin();
		
			if (m_ViewportSize.x != newViewportSize.x || m_ViewportSize.y != newViewportSize.y)
			{
				m_ViewportSize = { newViewportSize.x, newViewportSize.y };
				m_OrthographicCameraController.SetAspect(m_ViewportSize.y / m_ViewportSize.x);
				m_SceneFrameBuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			}
		
			ImGui::End();
		}
		ImGui::PopStyleVar();

		if (ImGui::Begin("Game"))
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
						m_CurrentTickClosure.AddInputAction(InputAction::PauseGame);
					}
					ImGui::SetItemTooltip("Shortcut: Space");
				}
				else
				{
					if (ImGui::Button("Play"))
					{
						m_CurrentTickClosure.AddInputAction(InputAction::ResumeGame);
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
				glm::ivec2 pixelPos = m_World->WorldToPixel(GetMousePosWorld());
				ImGui::Text(("(" + std::to_string(pixelPos.x) + ", " + std::to_string(pixelPos.y) + ")").c_str());
				ElementData& elementData = m_World->m_ElementData[m_HoveredElement.m_ID];
				ImGui::Text("Element: %s", elementData.name.c_str());
				ImGui::Text("- Temperature: %f", m_HoveredElement.m_Temperature);
			
				ImGui::TreePop();
			}
			
			ImGui::SetNextItemOpen(true);
			if (ImGui::TreeNode("Building Mode"))
			{
				if (ImGui::Selectable("~", m_BuildMode == BuildMode::Normal, 0, ImVec2(25, 25)))
				{
					m_BuildMode = BuildMode::Normal;
				}
				if (ImGui::Selectable("()", m_BuildMode == BuildMode::Dynamic, 0, ImVec2(25, 25)))
				{
					m_BuildMode = BuildMode::Dynamic;
				}
				if (ImGui::Selectable("[]", m_BuildMode == BuildMode::Kinematic, 0, ImVec2(25, 25)))
				{
					m_BuildMode = BuildMode::Kinematic;
				}
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
			
		/*if (ImGui::Begin("NetworkDebug"))
		{
			ImGui::Text(("Input Tick:" + std::to_string(m_InputTick)).c_str());
			ImGui::Text(("Last Recieved Merged Tick: " + std::to_string(d_LastRecievedInputTick)).c_str());
		}
		ImGui::End();*/
			
		for (Ref<Panel> panel : m_Panels)
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
		dispatcher.Dispatch<MouseButtonReleasedEvent>(PX_BIND_EVENT_FN(GameLayer::OnMouseButtonReleasedEvent));
		dispatcher.Dispatch<MouseScrolledEvent>(PX_BIND_EVENT_FN(GameLayer::OnMouseScrolledEvent));
	}

	//void GameLayer::ConnectionUpdate()
	//{
	//	//still connecting...

	//	//make sure the socket is still open, and if it isn't then return to main menu
	//	if (!m_ClientInterface.IsConnected())
	//	{
	//		m_ConnectionStatus = FailedToConnect;
	//		return;
	//	}

	//	//since we know we are connected still, call handle messages.
	//	//cant be called before since we aren't always connected to something
	//	HandleMessages();

	//	//while we are connecting, if we have the world data, it is still our
	//	//job to catch up to the server, which means 
	//	if (!m_WaitForWorldData)
	//	{
	//		//we are no longer waiting on the world data, so its time to catch up
	//		//all the ticks we missed while getting game data
	//		if (m_MTCQueue.size() > 0)
	//		{
	//			//as the client, i now need to remove the input actions from the 
	//			//latency queue for the tick i recieved, but not
	//			//in this section where i'm not sending any ticks


	//			if (m_MTCQueue.front().m_Tick < m_InputTick)
	//			{
	//				//we were sent a merged tick closure before we requested data, so ignore this since
	//				//the world state we recieved already had this tick applied
	//				m_MTCQueue.pop_front();
	//				return;
	//			}
	//			m_LatencyStateReset = true;
	//			PX_TRACE("Applying tick {0} to sim {1}", m_MTCQueue.front().m_Tick, m_World->m_SimulationTick);
	//			HandleTickClosure(m_MTCQueue.front());
	//			m_MTCQueue.pop_front();
	//			m_InputTick++;
	//		}
	//		if (m_InputTick == m_TickToEnter)
	//		{
	//			PX_TRACE("Reached Tick: {0}, I'm caught up!", m_TickToEnter);
	//			//we loaded the world, and we are caught up to what the
	//			//server has sent, since we reached the tick to enter

	//			//begin the game and start sending ticks! 
	//			//the server is waiting for us!
	//			m_ConnectionStatus = Connected;
	//			m_SimulationRunning = true;
	//		}

	//	}
	//}

	//void GameLayer::HandleMessages()
	//{
	//	Ref<Network::Message> msg;
	//	while (PollMessage(msg))
	//	{
	//		//PX_TRACE("Recieved message from Server");
	//		switch (static_cast<GameMessage>(msg->header.id))
	//		{
	//		case GameMessage::Server_ClientData:
	//		{
	//			HSteamNetConnection clientID;
	//			*msg >> clientID;
	//			PX_TRACE("Recieved client Data for client {0}", clientID);
	//			m_ClientDataMap[clientID];
	//			*msg >> m_ClientDataMap[clientID];
	//			break;
	//		}
	//		case GameMessage::Server_AllClientData:
	//		{
	//			uint32_t numClients;
	//			*msg >> numClients;
	//			PX_TRACE("Recieved all client Data. Player count: {0}", numClients);
	//			for (int i = 0; i < numClients; i++)
	//			{
	//				HSteamNetConnection clientID;
	//				*msg >> clientID;
	//				m_ClientDataMap[clientID];
	//				*msg >> m_ClientDataMap[clientID];
	//			}
	//			//now that we recieved the other player data, lets request the game data
	//			Network::Message requestGameDataMsg;
	//			requestGameDataMsg.header.id = static_cast<uint32_t>(GameMessage::Client_RequestGameData);
	//			SendMessageToServer(requestGameDataMsg);
	//			PX_TRACE("Requesting Game Data");
	//			m_MultiplayerState = MultiplayerState::DownloadingWorld;
	//			break;
	//		}
	//		case GameMessage::Server_ClientDisconnected:
	//		{
	//			HSteamNetConnection clientID;
	//			*msg >> clientID;

	//			m_ClientDataMap.erase(clientID);
	//			break;
	//		}
	//		case GameMessage::Server_GameDataInit:
	//		{
	//			PX_TRACE("Recieved Game Data");
	//			*msg >> m_InputTick;
	//			CreateWorld();
	//			
	//			m_World->DownloadWorldInit(*msg);
	//			uint32_t numPixelBodies;
	//			uint32_t numChunks;
	//			*msg >> numPixelBodies >> numChunks;

	//			m_DownloadTotal = static_cast<uint64_t>(numPixelBodies) + static_cast<uint64_t>(numChunks);
	//			m_DownloadCount = 0;

	//			PX_TRACE("Downloading game at tick [{0}], simulation tick [{1}]", m_InputTick, m_World->m_SimulationTick);
	//			if (m_DownloadTotal == 0)
	//			{
	//				PX_TRACE("The world is empty! hop on in!");
	//				//tell the server we are finished, so it can resume the game
	//				Network::Message finishedDownloadMsg;
	//				finishedDownloadMsg.header.id = static_cast<uint32_t>(GameMessage::Client_GameDataComplete);
	//				SendMessageToServer(finishedDownloadMsg);
	//				//we are connected!
	//				m_MultiplayerState = MultiplayerState::Connected;
	//			}
	//			else
	//			{
	//				PX_TRACE("Expecting {0} PixelBodies and {1} Chunks totalling {2} messages", numPixelBodies, numChunks, m_DownloadTotal);
	//			}
	//			break;
	//		}
	//		case GameMessage::Server_GameDataPixelBody:
	//		case GameMessage::Server_GameDataChunk:
	//		{
	//			
	//			//we recieved a pixel body, so lets count it, and add it to the world
	//			m_DownloadCount++;
	//			m_World->DownloadWorld(*msg);
	//			if (m_DownloadCount == m_DownloadTotal)
	//			{
	//				PX_INFO("Downloading World: [100%]");
	//				//tell the server we are finished, so it can resume the game
	//				Network::Message gameDataCompleteMsg;
	//				gameDataCompleteMsg.header.id = static_cast<uint32_t>(GameMessage::Client_GameDataComplete);
	//				SendMessageToServer(gameDataCompleteMsg);

	//				//now that we have finished recieving the game data and loading the world, start the game!
	//				m_MultiplayerState = MultiplayerState::Connected;
	//			}
	//			else
	//			{
	//				Network::Message gameDataRecievedMsg;
	//				gameDataRecievedMsg.header.id = static_cast<uint32_t>(GameMessage::Client_GameDataRecieved);
	//				SendMessageToServer(gameDataRecievedMsg);
	//				PX_INFO("Downloading World: [{0}%]", (double)m_DownloadCount / (double)m_DownloadTotal);
	//			}
	//			break;
	//		}
	//		case GameMessage::Game_MergedTickClosure:
	//		{
	//			MergedTickClosure mtc;
	//			*msg >> mtc.m_Tick;
	//			*msg >> mtc.m_ClientCount;
	//			*msg >> mtc.m_Data;
	//			d_LastRecievedInputTick = mtc.m_Tick;

	//			//if this tick is the current one, handle it immediately,
	//			// don't need to push it to the front of the buffer
	//			if (mtc.m_Tick == m_InputTick)
	//			{
	//				HandleTickClosure(mtc);
	//				m_InputTick++;
	//			}
	//			else if (mtc.m_Tick == m_InputTick)
	//			{
	//				//also, if the tick is old just discard it.
	//				//we only want to hand onto new ones.
	//			}
	//			else
	//			{
	//				//this is a future tick, so lets throw it on the stack
	//				m_MTCBuffer.push_back(mtc);
	//			}

	//			break;
	//		}
	//		case GameMessage::Game_ResetBox2D:
	//		{
	//			//gather all rigid body data, store it, and reload it!
	//			//this has to be done once we are finished with all the previously
	//			//collected mtc's, so we will mark when we are supposed to
	//			//reset, and do it then!
	//			*msg >> m_TickToResetBox2D;
	//			PX_TRACE("Sim Tick to reset at: {0}", m_TickToResetBox2D);
	//			PX_TRACE("current sim tick: {0}", m_World->m_SimulationTick);
	//			break;
	//		}

	//		default:
	//			break;
	//		}
	//	}
	//}


	void GameLayer::HandleTickClosure(MergedTickClosure& tc)
	{
		for (int i = 0; i < tc.m_ClientCount; i++)
		{
			HSteamNetConnection clientID;
			tc >> clientID;
			uint32_t inputActionCount;
			tc >> inputActionCount;
			for (int i = 0; i < inputActionCount; i++)
			{
				InputAction IA;
				tc >> IA;
				switch (IA)
				{
				/*case InputAction::Add_Player:
				{
					uint64_t ID;
					tc >> ID;

					glm::ivec2 pixelPos;
					tc >> pixelPos;

					m_World->CreatePlayer(ID, pixelPos);
					break;
				}*/
				case InputAction::PauseGame:
				{
					m_World->m_Running = false;
					break;
				}
				case InputAction::ResumeGame:
				{
					m_World->m_Running = true;
					break;
				}
				case InputAction::TransformRegionToRigidBody:
				{
					uint64_t UUID;
					tc >> UUID;

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
					PixelRigidBody* body = new PixelRigidBody(UUID, size, elements, type, m_World->m_Box2DWorld);
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
					bool rigid;
					glm::ivec2 pixelPos;
					uint32_t elementID;
					BrushType brush;
					uint8_t brushSize;
					tc >> rigid >> pixelPos >> elementID >> brush >> brushSize;

					m_World->PaintBrushElement(pixelPos, elementID, brush, brushSize);
					break;
				}
				case Pyxis::InputAction::Input_StepSimulation:
				{
					PX_TRACE("input action: Input_StepSimulation");
					m_World->UpdateWorld();
					break;
					break;
				}
				case InputAction::Input_MousePosition:
				{
					//add new mouse position to data
					glm::vec2 mousePos;
					HSteamNetConnection clientID;
					tc >> mousePos >> clientID;
					m_ClientDataMap[clientID].m_CursorWorldPosition = mousePos;
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

	glm::ivec2 GameLayer::GetMousePositionImGui()
	{
		///if not using a framebuffer / imgui image, just use Pyxis::Input::GetMousePosition();
		//TODO: Set up ifdef for using imgui? or just stop using imgui... lol
		auto [mx, my] = ImGui::GetMousePos();

		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;

		return { (int)mx, (int)my };
	}

	glm::vec2 GameLayer::GetMousePosWorld()
	{
		glm::ivec2 mousePos = GetMousePositionImGui();
		glm::vec2 mousePercent = (glm::vec2)mousePos / m_ViewportSize;
		return m_OrthographicCameraController.MousePercentToWorldPos(mousePercent.x, mousePercent.y);
	}

	bool GameLayer::OnWindowResizeEvent(WindowResizeEvent& event) {
		m_OrthographicCameraController.SetAspect((float)event.GetHeight() / (float)event.GetWidth());
		m_ViewportSize = { event.GetWidth() , event.GetHeight() };
		Renderer::OnWindowResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		return false;
	}

	bool GameLayer::OnKeyPressedEvent(KeyPressedEvent& event) {
		if (event.GetKeyCode() == PX_KEY_F)
		{
			//SendStringToServer("Respects Paid!");
		}
		if (event.GetKeyCode() == PX_KEY_SPACE)
		{
			if (m_World->m_Running)
			{
				m_CurrentTickClosure.AddInputAction(InputAction::PauseGame);
			}
			else 
			{
				m_CurrentTickClosure.AddInputAction(InputAction::ResumeGame);
			}
		}
		if (event.GetKeyCode() == PX_KEY_RIGHT)
		{
			m_CurrentTickClosure.AddInputAction(InputAction::Input_StepSimulation);
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
		//let the UI keep track of what has been pressed, so that way buttons can be on release!
		UI::UINode::s_MousePressedNodeID = m_Scene->m_HoveredNodeID;

		//see if the node is valid, and if it is then send the event through.
		if (Node::Nodes.contains(m_Scene->m_HoveredNodeID))
		{
			//the hovered node is valid
			if (UI::UINode* uinode = dynamic_cast<UI::UINode*>(Node::Nodes[m_Scene->m_HoveredNodeID]))
			{
				uinode->OnMousePressed(event.GetMouseButton());
			}

		}

		//PX_TRACE(event.GetMouseButton());
		if (event.GetMouseButton() == PX_MOUSE_BUTTON_FORWARD) // forward
		{
			m_BrushSize++;
			if (m_BrushSize > 32) m_BrushSize = 32;
		}
		if (event.GetMouseButton() == PX_MOUSE_BUTTON_BACK) // back
		{
			m_BrushSize--;
			if (m_BrushSize < 0.0f) m_BrushSize = 0;
		}


		return false;
	}

	bool GameLayer::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& event)
	{
		if (Node::Nodes.contains(m_Scene->m_HoveredNodeID))
		{
			//the hovered node is valid
			if (UI::UINode* uinode = dynamic_cast<UI::UINode*>(Node::Nodes[m_Scene->m_HoveredNodeID]))
			{
				uinode->OnMouseReleased(event.GetMouseButton());
				if (UI::UINode::s_MousePressedNodeID == m_Scene->m_HoveredNodeID)
					uinode->OnClick();
			}
			
					

		}

		//do not end event here
		return false;
	}

	bool GameLayer::OnMouseScrolledEvent(MouseScrolledEvent& event)
	{
		if (Input::IsKeyPressed(PX_KEY_LEFT_CONTROL))
		{
			m_BrushSize += event.GetYOffset();
			if (m_BrushSize < 0) m_BrushSize = 0;
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
		
		//do not end event here
		return false;
		return false;
	}


	void GameLayer::PaintBrushHologram()
	{
		glm::ivec2 pixelPos = m_World->WorldToPixel(GetMousePosWorld());

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
				glm::vec4 vecColor = glm::vec4(r,g,b,std::fmax(a * 0.5f, 0.25f));
				
				//draw square at that pixel
				float pixelSize = 1.0f / (float)CHUNKSIZE;
				//
				Renderer2D::DrawQuad((glm::vec3(newPos.x, newPos.y, 0) / (float)CHUNKSIZE) + glm::vec3(pixelSize / 2, pixelSize / 2, 0.05f), glm::vec2(pixelSize, pixelSize), vecColor);
				
			}
		}
		
	}


}