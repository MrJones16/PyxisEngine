#include "GameNode.h"

#include <glm/gtc/type_ptr.hpp>

#include <Platform/OpenGL/OpenGLShader.h>
#include <chrono>
#include <variant>
#include "MenuNode.h"


namespace Pyxis
{

	GameNode::GameNode(std::string debugName)
		: Node(debugName),
		m_KeyPressedReciever(this, &GameNode::OnKeyPressedEvent),
		m_MouseButtonPressedReciever(this, &GameNode::OnMouseButtonPressedEvent),
		m_MouseScrolledReciever(this, &GameNode::OnMouseScrolledEvent)
	{
		EventSignal::s_KeyPressedEventSignal.AddReciever(m_KeyPressedReciever);
		EventSignal::s_MouseButtonPressedEventSignal.AddReciever(m_MouseButtonPressedReciever);
		EventSignal::s_MouseScrolledEventSignal.AddReciever(m_MouseScrolledReciever);

		//Set up the UI Heirarchy since we have no scenes

		//I'm going to aim for a bottom hot bar for now.
		m_CameraController = CreateRef<OrthographicCameraControllerNode>();
		m_CameraController->SetMainCamera();
		m_CameraController->SetWidth(2);
		m_CameraController->Translate({ 0,1,0 });
		AddChild(m_CameraController);

		auto screenSpace = CreateRef<UI::ScreenSpace>();
		AddChild(screenSpace);

		m_Hotbar = CreateRef<UI::Canvas>();
		m_Hotbar->m_AutomaticSizing = true;
		m_Hotbar->m_AutomaticSizingPercent = { 1, 1 };//10 % height
		m_Hotbar->m_FixedSize.y = 112;
		//full x, 1/10th up the screen
		m_Hotbar->m_AutomaticPositioning = true;
		m_Hotbar->m_VerticalAlignment = UI::Down;
		m_Hotbar->CreateTextures("assets/textures/UI/GreenCanvas/", "GreenCanvasTile_", ".png");
		m_Hotbar->m_PPU = 32;
		screenSpace->AddChild(m_Hotbar);


		auto container = CreateRef<UI::Container>();
		container->m_AutomaticSizing = true;
		container->m_AutomaticSizingPercent = { 1,1 };
		container->m_AutomaticSizingOffset = { -32, -32 };
		container->m_Gap = 32;
		container->m_VerticalAlignment = UI::Center;
		container->Translate({ 0,0,-0.05f });
		m_Hotbar->AddChild(container);

		///Pause & Play Buttons
		auto ButtonHolder = CreateRef<UI::UIRect>("Button Holder");
		ButtonHolder->m_Enabled = false;
		ButtonHolder->m_Size = { 32, 32 };
		m_PlayButton = CreateRef<UI::Button>("Play Button", ResourceSystem::Load<Texture2DResource>("assets/Textures/UI/PlayButton.png"));
		m_PlayButton->SetFunction(std::bind(&GameNode::PlayButtonFunc, this));
		m_PlayButton->m_TextureResourcePressed = ResourceSystem::Load<Texture2DResource>("assets/Textures/UI/PlayButtonPressed.png");
		m_PlayButton->m_Size = { 32,32 };
		m_PlayButton->m_Enabled = false;
		ButtonHolder->AddChild(m_PlayButton);
		m_PauseButton = CreateRef<UI::Button>("Pause Button", ResourceSystem::Load<Texture2DResource>("assets/Textures/UI/PauseButton.png"));
		m_PauseButton->SetFunction(std::bind(&GameNode::PauseButtonFunc, this));
		m_PauseButton->m_TextureResourcePressed = ResourceSystem::Load<Texture2DResource>("assets/Textures/UI/PauseButtonPressed.png");
		m_PauseButton->m_Size = { 32,32 };
		ButtonHolder->AddChild(m_PauseButton);
		container->AddChild(ButtonHolder);

		//Brush Buttons
		auto BrushOptions = CreateRef<UI::UIRect>("Button Holder");
		BrushOptions->m_Enabled = false;
		BrushOptions->m_Size = { 72, 32 };
		auto brushCircle = CreateRef<UI::Button>("BrushCircle", ResourceSystem::Load<Texture2DResource>("assets/Textures/UI/BrushCircleButton.png"));
		brushCircle->SetFunction(std::bind(&GameNode::SetBrushType, this, BrushType::circle));
		brushCircle->m_TextureResourcePressed = ResourceSystem::Load<Texture2DResource>("assets/Textures/UI/BrushCircleButtonPressed.png");
		brushCircle->m_Size = { 32,32 };
		brushCircle->Translate({ -20, 0, 0 });
		BrushOptions->AddChild(brushCircle);

		auto brushSquare = CreateRef<UI::Button>("BrushSquare", ResourceSystem::Load<Texture2DResource>("assets/Textures/UI/BrushSquareButton.png"));
		brushSquare->SetFunction(std::bind(&GameNode::SetBrushType, this, BrushType::square));
		brushSquare->m_TextureResourcePressed = ResourceSystem::Load<Texture2DResource>("assets/Textures/UI/BrushSquareButtonPressed.png");
		brushSquare->m_Size = { 32,32 };
		brushSquare->Translate({ 20, 0, 0 });
		BrushOptions->AddChild(brushSquare);
		container->AddChild(BrushOptions);		

		auto ElementButtonContainer = CreateRef<UI::Container>("Element Buttons Container");
		//ElementButtonContainer->m_Size = {900, 32};
		ElementButtonContainer->m_AutomaticSizing = true;
		ElementButtonContainer->m_AutomaticSizingOffset = { -300, 0 };
		ElementButtonContainer->m_Gap = 0;
		for (int i = 0; i < m_World.m_ElementData.size(); i++)
		{
			ElementData& ed = m_World.m_ElementData[i];

			//abgr
			int r = (ed.color & 0x000000FF) >> 0;
			int g = (ed.color & 0x0000FF00) >> 8;
			int b = (ed.color & 0x00FF0000) >> 16;
			int a = (ed.color & 0xFF000000) >> 24;

			auto ElementTextButton = CreateRef<UI::TextButton>("ElementTextButton", FontLibrary::GetFont("Aseprite"), std::bind(&GameNode::SetBrushElement, this, i));
			ElementTextButton->m_TextureResource = ResourceSystem::Load<Texture2DResource>("assets/textures/UI/TextPlateWhite.png");
			ElementTextButton->m_TextureResourcePressed = ResourceSystem::Load<Texture2DResource>("assets/textures/UI/TextPlateWhitePressed.png");
			ElementTextButton->m_TextBorderSize = glm::vec2(6);
			ElementTextButton->m_Color = glm::vec4(r, g, b, a) / 255.0f;
			ElementTextButton->m_TextColor = glm::vec4(ElementTextButton->m_Color.r, ElementTextButton->m_Color.g, ElementTextButton->m_Color.b, 1);

			if (ElementTextButton->m_Color.r + ElementTextButton->m_Color.g + ElementTextButton->m_Color.b / 3.0f > 0.8f)
			{
				//element color is bright so make it darker
				ElementTextButton->m_TextColor = glm::vec4(ElementTextButton->m_Color.r * 0.1f, ElementTextButton->m_Color.g * 0.1f, ElementTextButton->m_Color.b * 0.1f, 1);
			}
			else
			{
				//its dark so make it brighter
				//ElementTextButton->m_TextColor = ElementTextButton->m_Color;
				for (int i = 0; i < 3 ;i++)
					ElementTextButton->m_TextColor[i] = std::min(ElementTextButton->m_TextColor[i] + 0.50f, 1.0f);
			}
			//ElementTextButton->m_TextColor.a = 255;
			ElementTextButton->m_PPU = 0.5f;
			ElementTextButton->UpdateSizeFromTexture();

			ElementTextButton->m_Text = ed.name;
			ElementTextButton->m_FontSize = 0.5f;
			ElementTextButton->Translate({ 0,0,-0.01f });			
			ElementButtonContainer->AddChild(ElementTextButton);
		}
		container->AddChild(ElementButtonContainer);

		//Quit Button		
		auto quitButton = CreateRef<UI::TextButton>("Quit Game Button", FontLibrary::GetFont("Aseprite"), std::bind(&GameNode::ReturnToMenu, this));
		quitButton->m_PPU = 0.5;
		quitButton->m_Text = "Quit Game";
		quitButton->m_TextColor = glm::vec4(255.0f / 255.0f, 221.0f / 255.0f, 159.0f / 255.0f, 1);
		quitButton->Translate({ 0,0,1 });
		quitButton->m_TextureResource = ResourceSystem::Load<Texture2DResource>("assets/textures/UI/ButtonWide.png");
		quitButton->m_TextureResourcePressed = ResourceSystem::Load<Texture2DResource>("assets/textures/UI/ButtonWidePressed.png");
		quitButton->UpdateSizeFromTexture();
		quitButton->m_TextBorderSize = glm::vec2(5,5);
		quitButton->m_TextOffset = { 0, 3, 0 };
		quitButton->m_TextOffsetPressed = { 0, 1, 0 };
		container->AddChild(quitButton);


		screenSpace->PropagateUpdate(); // simple way to tell the hotbar to fix itself
	}

	GameNode::~GameNode()
	{
		
	}


	/// <summary>
	/// This is the main update loop involving actually playing the game, input, controls, ect.
	/// Also includes rendering loop
	/// 
	/// Only run this if m_World is valid and ready to be played!
	/// The output of inputs is in m_CurrentTickClosure
	/// </summary>
	/// <param name="ts"></param>
	void GameNode::GameUpdate(Timestep ts)
	{

		m_Hovering = (Node::s_HoveredNodeID == GetID()) || (Node::s_HoveredNodeID == 0);
		//PX_TRACE("Hovered Node: {0}", Node::s_HoveredNodeID);

		{
			PROFILE_SCOPE("Game Update");
			glm::ivec2 mousePixelPos = m_World.WorldToPixel(GetMousePosWorld());

			auto chunkPos = m_World.PixelToChunk(mousePixelPos);
			auto it = m_World.m_Chunks.find(chunkPos);
			if (it != m_World.m_Chunks.end())
			{
				auto index = m_World.PixelToIndex(mousePixelPos);
				m_HoveredElement = it->second->m_Elements[index.x + index.y * CHUNKSIZE];
				if (m_HoveredElement.m_ID >= m_World.m_TotalElements)
				{
					//something went wrong? how?
					m_HoveredElement = Element();
				}
			}
			else
			{
				m_HoveredElement = Element();
			}


			if (Input::IsMouseButtonPressed(0) && m_Hovering)
			{
				glm::vec2 mousePos = GetMousePosWorld();
				glm::ivec2 pixelPos = m_World.WorldToPixel(mousePos);
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

		{
			PROFILE_SCOPE("Renderer Draw");
			m_World.RenderWorld();
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


	void GameNode::ClientImGuiRender()
	{
		if (ImGui::Begin("Game"))
		{
			ImGui::SetNextItemOpen(true);
			if (ImGui::TreeNode("Simulation"))
			{
				//ImGui::DragFloat("Updates Per Second", &m_UpdatesPerSecond, 1, 0, 244);
				//ImGui::SetItemTooltip("Default: 60");
				if (m_World.m_Running)
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
					m_World.m_DebugDrawColliders = !m_World.m_DebugDrawColliders;
				}
							
				if (m_BuildingRigidBody)
				{
					if (ImGui::Button("Build Rigid Body"))
					{
						srand(time(0));
						uint64_t uuid = std::rand();
						while (m_World.m_PixelBodyMap.find(uuid) != m_World.m_PixelBodyMap.end())
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
						while (m_World.m_PixelBodyMap.find(uuid) != m_World.m_PixelBodyMap.end())
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
					for each (auto body in m_World.m_PixelBodies)
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
				glm::ivec2 pixelPos = m_World.WorldToPixel(GetMousePosWorld());
				ImGui::Text(("(" + std::to_string(pixelPos.x) + ", " + std::to_string(pixelPos.y) + ")").c_str());
				ElementData& elementData = m_World.m_ElementData[m_HoveredElement.m_ID];
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
				for (int y = 0; y < (m_World.m_TotalElements / 4) + 1; y++)
					for (int x = 0; x < 4; x++)
					{
						if (x > 0)
							ImGui::SameLine();
						int index = y * 4 + x;
						if (index >= m_World.m_TotalElements) continue;
						ImGui::PushID(index);
						auto color = m_World.m_ElementData[index].color;
						int r = (color & 0x000000FF) >> 0;
						int g = (color & 0x0000FF00) >> 8;
						int b = (color & 0x00FF0000) >> 16;
						int a = (color & 0xFF000000) >> 24;
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f));
						std::string name = (m_World.m_ElementData[index].name);
						
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


	void GameNode::TextCentered(std::string text)
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

	//void GameNode::ConnectionUpdate()
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
	//			PX_TRACE("Applying tick {0} to sim {1}", m_MTCQueue.front().m_Tick, m_World.m_SimulationTick);
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

	//void GameNode::HandleMessages()
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
	//			m_World.DownloadWorldInit(*msg);
	//			uint32_t numPixelBodies;
	//			uint32_t numChunks;
	//			*msg >> numPixelBodies >> numChunks;

	//			m_DownloadTotal = static_cast<uint64_t>(numPixelBodies) + static_cast<uint64_t>(numChunks);
	//			m_DownloadCount = 0;

	//			PX_TRACE("Downloading game at tick [{0}], simulation tick [{1}]", m_InputTick, m_World.m_SimulationTick);
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
	//			m_World.DownloadWorld(*msg);
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
	//			PX_TRACE("current sim tick: {0}", m_World.m_SimulationTick);
	//			break;
	//		}

	//		default:
	//			break;
	//		}
	//	}
	//}


	void GameNode::HandleTickClosure(MergedTickClosure& tc)
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

					m_World.CreatePlayer(ID, pixelPos);
					break;
				}*/
				case InputAction::PauseGame:
				{
					Pause();
					break;
				}
				case InputAction::ResumeGame:
				{
					Play();
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
							auto& element = m_World.GetElement(pixelPos);
							auto& elementData = m_World.m_ElementData[element.m_ID];
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
							auto& element = m_World.GetElement(pixelPos);
							auto& elementData = m_World.m_ElementData[element.m_ID];
							if ((elementData.cell_type == ElementType::solid || elementData.cell_type == ElementType::movableSolid) && element.m_Rigid == false)
							{
								element.m_Rigid = true;
								//set the elements at the local position to be the element pulled from world
								elements[glm::ivec2(x - origin.x, y - origin.y)] = RigidBodyElement(element, pixelPos);
								m_World.SetElement(pixelPos, Element());
							}
						}
					}
					glm::ivec2 size = newMax - newMin;
					PX_TRACE("Mass is: {0}", mass);
					PixelRigidBody* body = new PixelRigidBody(UUID, size, elements, type, m_World.m_Box2DWorld);
					if (body->m_B2Body == nullptr)
					{
						PX_TRACE("Failed to create rigid body");
						continue;
					}
					else
					{
						m_World.m_PixelBodyMap[body->m_ID] = body;
						auto pixelPos = (newMin + newMax) / 2;
						if (width % 2 == 0) pixelPos.x += 1;
						if (height % 2 == 0) pixelPos.y += 1;
						body->SetPixelPosition(pixelPos);
						m_World.PutPixelBodyInWorld(*body);
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

					m_World.PaintBrushElement(pixelPos, elementID, brush, brushSize);
					break;
				}
				case Pyxis::InputAction::Input_StepSimulation:
				{
					PX_TRACE("input action: Input_StepSimulation");
					m_World.UpdateWorld();
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
					m_World.Clear();
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
		
		if (m_World.m_Running)
		{
			m_World.UpdateWorld();
		}
		else
		{
			m_World.UpdateTextures();
		}
	}

	glm::ivec2 GameNode::GetMousePositionImGui()
	{
		///if not using a framebuffer / imgui image, just use Pyxis::Input::GetMousePosition();
		//TODO: Set up ifdef for using imgui? or just stop using imgui... lol
		auto [mx, my] = ImGui::GetMousePos();

		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;

		return { (int)mx, (int)my };
	}

	glm::vec2 GameNode::GetMousePosWorld()
	{
		glm::vec2 mousePos = Input::GetMousePosition();
		Window& window = Application::Get().GetWindow();
		mousePos.x /= (float)window.GetWidth();
		mousePos.y /= (float)window.GetHeight();
		//from 0->1 to -1 -> 1
		mousePos = (mousePos - 0.5f) * 2.0f;
		mousePos *= (Camera::Main()->GetSize() / 2.0f);
		

		glm::vec4 vec = glm::vec4(mousePos.x, -mousePos.y, 0, 1);
		vec = glm::translate(glm::mat4(1), Camera::Main()->GetPosition()) * vec;
		return vec;
	}

	void GameNode::OnKeyPressedEvent(KeyPressedEvent& event) {
		if (event.GetKeyCode() == PX_KEY_F)
		{
			//SendStringToServer("Respects Paid!");
		}
		if (event.GetKeyCode() == PX_KEY_SPACE)
		{
			if (m_World.m_Running)
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

	}

	void GameNode::OnMouseButtonPressedEvent(MouseButtonPressedEvent& event)
	{
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
	}

	void GameNode::OnMouseScrolledEvent(MouseScrolledEvent& event)
	{
		if (Input::IsKeyPressed(PX_KEY_LEFT_CONTROL))
		{
			m_BrushSize += event.GetYOffset();
			if (m_BrushSize < 0) m_BrushSize = 0;
			if (m_BrushSize > 32) m_BrushSize = 32;
		}
		
		//do not end event here
	}

	void GameNode::PlayButtonFunc()
	{
		m_CurrentTickClosure.AddInputAction(InputAction::ResumeGame);
	}

	void GameNode::PauseButtonFunc()
	{
		m_CurrentTickClosure.AddInputAction(InputAction::PauseGame);
	}

	void GameNode::Play()
	{
		m_PlayButton->m_Enabled = false;
		m_PauseButton->m_Enabled = true; 
		m_World.m_Running = true;
		//PX_TRACE("Play!");
	}

	void GameNode::Pause()
	{
		m_PlayButton->m_Enabled = true;
		m_PauseButton->m_Enabled = false;
		m_World.m_Running = false;
		//PX_TRACE("Pause!");
	}

	void GameNode::ReturnToMenu()
	{
		auto menu = CreateRef<MenuNode>();
		m_Parent->AddChild(menu);
		QueueFree();
	}

	/*void GameNode::OnWindowResizeEvent(WindowResizeEvent& event)
	{
		glm::vec2 windowSize = { event.GetWidth(), event.GetHeight() };
		m_Hotbar->ResetLocalTransform();
		m_Hotbar->SetScale((glm::vec3(1) / glm::vec3(windowSize.x / 2.0f, windowSize.y / 2.0f, 1)));
		m_Hotbar->m_Size = { windowSize.x, windowSize.y * 0.1f };
		m_Hotbar->m_TextureScale = 32;
		m_Hotbar->UpdateCanvasTransforms();

	}*/


	void GameNode::PaintBrushHologram()
	{
		glm::ivec2 pixelPos = m_World.WorldToPixel(GetMousePosWorld());

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

				uint32_t color = m_World.m_ElementData[m_SelectedElementIndex].color;

				
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