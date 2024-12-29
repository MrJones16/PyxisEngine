#include "MultiplayerGameLayer.h"

namespace Pyxis
{
	MultiplayerGameLayer::MultiplayerGameLayer()
	{

	}

	MultiplayerGameLayer::~MultiplayerGameLayer()
	{

	}

	void MultiplayerGameLayer::OnUpdate(Timestep ts)
	{

		PROFILE_SCOPE("GameLayer::OnUpdate");
		m_OrthographicCameraController.OnUpdate(ts);
		m_Scene->Update(ts);

		UpdateInterface();

		if (m_ConnectionStatus == ConnectionStatus::Connecting || m_ConnectionStatus == ConnectionStatus::Connected)
		{
			HandleMessages();
		}

		//rendering
		#if STATISTICS
		Renderer2D::ResetStats();
		#endif

		{
			PROFILE_SCOPE("Renderer Prep");
			m_SceneFrameBuffer->Bind();
			RenderCommand::SetClearColor({ 198 / 255.0f, 239 / 255.0f, 249 / 255.0f, 1 });
			RenderCommand::Clear();
			Renderer2D::BeginScene(m_OrthographicCameraController.GetCamera());
		}
		

		if (m_MultiplayerState == MultiplayerState::Connected)
		{
			PROFILE_SCOPE("Network Update");
			//only run per tick rate
			auto time = std::chrono::high_resolution_clock::now();
			if (m_TickRate > 0 &&
				std::chrono::time_point_cast<std::chrono::microseconds>(time).time_since_epoch().count()
				-
				std::chrono::time_point_cast<std::chrono::microseconds>(m_UpdateTime).time_since_epoch().count()
				>= (1.0f / m_TickRate) * 1000000.0f)
			{
				PROFILE_SCOPE("Simulation Update");

				//reset world, then apply the tick closure
				if (m_World->m_SimulationTick == m_TickToResetBox2D)
				{
					m_World->ResetBox2D();
					m_TickToResetBox2D = -1;
				}

				m_UpdateTime = time;

				//for multiplayer, just send off your tick closure
				//(basically just your input shoved into a message)
				Network::Message msg;
				msg.header.id = static_cast<uint32_t>(GameMessage::Game_TickClosure);
				msg << m_CurrentTickClosure.m_Data;
				msg << m_CurrentTickClosure.m_InputActionCount;
				SendMessageToServer(msg);
				

				//reset tick closure
				m_CurrentTickClosure = TickClosure();

			}

			if (m_TickRateSlow > 0 &&
				std::chrono::time_point_cast<std::chrono::microseconds>(time).time_since_epoch().count()
				-
				std::chrono::time_point_cast<std::chrono::microseconds>(m_SlowUpdateTime).time_since_epoch().count()
				>= (1.0f / m_TickRateSlow) * 1000000.0f)
			{
				PROFILE_SCOPE("Slow Update");
				m_SlowUpdateTime = time;

				Network::Message mousePosMsg;
				mousePosMsg.header.id = static_cast<uint32_t>(GameMessage::Client_ClientDataMousePosition);
				glm::ivec2 mousePos = GetMousePositionImGui();
				glm::vec2 vec = m_OrthographicCameraController.MousePercentToWorldPos(mousePos.x, mousePos.y);
				m_ClientData.m_CursorWorldPosition = vec;
				mousePosMsg << vec;
				SendMessageToServer(mousePosMsg);
			}
		}

		if (m_MultiplayerState == MultiplayerState::Connected)
		{
			while (!m_MTCBuffer.empty())
			{
				if (m_MTCBuffer.front().m_Tick == m_InputTick)
				{
					//the front tick is what we want, so let's just handle it
					HandleTickClosure(m_MTCBuffer.front());
					m_MTCBuffer.pop_front();
					m_InputTick++;
				}
				else if (m_MTCBuffer.front().m_Tick < m_InputTick)
				{
					//this tick is old, so discard it
					m_MTCBuffer.pop_front();
				}
				else
				{
					//this tick must be for the future,
					// so lets request the missing one and wait
					if (m_LastRequestedTick != m_InputTick)
					{
						Network::Message requestMsg(static_cast<uint32_t>(GameMessage::Client_RequestMergedTick));
						requestMsg << m_InputTick;
						SendMessageToServer(requestMsg);
						PX_TRACE("MTC Skipped, Requesting Missing Tick");
					}
					break;
				}

			}
		}

		if (m_MultiplayerState == MultiplayerState::Connected && m_ConnectionStatus == ConnectionStatus::Connected)
		{
			GameUpdate(ts);
			for (auto& clientPair : m_ClientDataMap)
			{
				//draw the 3x3 square for each players cursor
				glm::vec3 worldPos = glm::vec3(clientPair.second.m_CursorWorldPosition.x, clientPair.second.m_CursorWorldPosition.y, 5);
				glm::vec2 size = glm::vec2(3.0f / CHUNKSIZE);
				Renderer2D::DrawQuad(worldPos, size, clientPair.second.m_Color);
			}

		}

		m_Scene->Render();

		Renderer2D::EndScene();

		m_SceneFrameBuffer->Unbind();
	}

	void MultiplayerGameLayer::OnImGuiRender()
	{
		auto dock = ImGui::DockSpaceOverViewport(ImGui::GetID("MainDock"), (const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode);

		//we are playing multiplayer,
		switch (m_ConnectionStatus)
		{
		case Network::ClientInterface::ConnectionStatus::Connecting:
		{
			//we are still connecting
			ImGui::SetNextWindowDockID(dock);
			if (ImGui::Begin("Connecting"))
			{
				ImGui::Text(GetConnectionStatusMessage().c_str());
				ImGui::End();
			}
			break;
		}
		case Network::ClientInterface::ConnectionStatus::Disconnected:
		{
			m_MultiplayerState = MultiplayerState::Disconnected;
			Ref<MultiplayerGameLayer> ref = shared_from_this();
			Application::Get().PopLayerQueue(ref);
		}
		case Network::ClientInterface::ConnectionStatus::LostConnection:
		case Network::ClientInterface::ConnectionStatus::FailedToConnect:
		{
			ImGui::SetNextWindowDockID(dock);
			if (ImGui::Begin("Connection Failed", (bool*)0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
			{
				ImGui::Text("Connection Failed");

				ImGui::Text(GetConnectionStatusMessage().c_str());
				if (ImGui::Button("Okay"))
				{
					Disconnect();
					m_MultiplayerState = MultiplayerState::Disconnected;
					Ref<MultiplayerGameLayer> ref = shared_from_this();
					Application::Get().PopLayerQueue(ref);

				}
			}
			ImGui::End();
			break;
		}
		case Network::ClientInterface::ConnectionStatus::Connected:
		{

			switch (m_MultiplayerState)
			{
			case MultiplayerState::GatheringPlayerData:
			{
				ImGui::SetNextWindowDockID(dock);
				if (ImGui::Begin("Connecting To Server"))
				{
					ImGui::Text("Gathering Player Data");
					ImGui::End();
				}

				break;
			}
			case MultiplayerState::DownloadingWorld:
			{
				ImGui::SetNextWindowDockID(dock);
				if (ImGui::Begin("Connecting To Server"))
				{
					ImGui::Text("Downloading World");
					ImGui::ProgressBar((float)m_DownloadCount / (float)m_DownloadTotal);
					ImGui::End();
				}
				break;
			}
			case MultiplayerState::Connected:
			{
				ClientImGuiRender(dock);
				break;
			}
			}
			break;
		}

		}
	}

	void MultiplayerGameLayer::Connect(const std::string& AddressAndPort)
	{
		//max out input tick so we can discard early sent mtc's
		m_InputTick = -1;
		m_MultiplayerState = MultiplayerState::Connecting;
		ConnectIP(AddressAndPort);
	}

	void MultiplayerGameLayer::Connect(SteamNetworkingIdentity& identity, int virtualPort)
	{
		//max out input tick so we can discard early sent mtc's
		m_InputTick = -1;
		m_MultiplayerState = MultiplayerState::Connecting;
		ConnectP2P(identity, virtualPort);
	}

	void MultiplayerGameLayer::OnConnectionSuccess()
	{
		Network::Message clientDataMsg;
		clientDataMsg.header.id = static_cast<uint32_t>(GameMessage::Client_ClientData);
		clientDataMsg << m_ClientData;
		PX_TRACE("Sending my client data to server");
		SendMessageToServer(clientDataMsg);

		m_MultiplayerState = MultiplayerState::GatheringPlayerData;
		Network::Message msg;
		msg.header.id = static_cast<uint32_t>(GameMessage::Client_RequestAllClientData);
		PX_TRACE("Requesting all client data");
		SendMessageToServer(msg);
	}

	void MultiplayerGameLayer::OnConnectionLost(const std::string& reasonText)
	{
		//m_ConnectionStatus = ConnectionStatus::Disconnected;
		m_MultiplayerState = MultiplayerState::Disconnected;
	}

	void MultiplayerGameLayer::HandleMessages()
	{
		Ref<Network::Message> msg;
		while (PollMessage(msg))
		{
			//PX_TRACE("Recieved message from Server");
			switch (static_cast<GameMessage>(msg->header.id))
			{
			case GameMessage::Server_ClientData:
			{
				HSteamNetConnection clientID;
				*msg >> clientID;
				PX_TRACE("Recieved client Data for client {0}", clientID);
				m_ClientDataMap[clientID];
				*msg >> m_ClientDataMap[clientID];
				break;
			}
			case GameMessage::Server_ClientDataMousePosition:
			{
				//Update the position on our end
				HSteamNetConnection otherClient;
				*msg >> otherClient;
				//we will use Invalid connection handle as the server's handle
				if (otherClient == k_HSteamNetConnection_Invalid)
				{
					m_ClientDataMap[k_HSteamNetConnection_Invalid];
					*msg >> m_ClientDataMap[k_HSteamNetConnection_Invalid].m_CursorWorldPosition;
				}
				else
				{
					//we know it is another player's
					m_ClientDataMap[otherClient];
					*msg >> m_ClientDataMap[otherClient].m_CursorWorldPosition;
				}
				
				break;
			}
			case GameMessage::Server_AllClientData:
			{
				uint32_t numClients;
				*msg >> numClients;
				PX_TRACE("Recieved all client Data. Player count: {0}", numClients);
				for (int i = 0; i < numClients; i++)
				{
					HSteamNetConnection clientID;
					*msg >> clientID;
					m_ClientDataMap[clientID];
					*msg >> m_ClientDataMap[clientID];
				}
				//now that we recieved the other player data, lets request the game data
				Network::Message requestGameDataMsg;
				requestGameDataMsg.header.id = static_cast<uint32_t>(GameMessage::Client_RequestGameData);
				SendMessageToServer(requestGameDataMsg);
				PX_TRACE("Requesting Game Data");
				m_MultiplayerState = MultiplayerState::DownloadingWorld;
				break;
			}
			case GameMessage::Server_ClientDisconnected:
			{
				HSteamNetConnection clientID;
				*msg >> clientID;

				m_ClientDataMap.erase(clientID);
				break;
			}
			case GameMessage::Server_GameDataInit:
			{
				PX_TRACE("Recieved Game Data");
				*msg >> m_InputTick;
				CreateWorld();

				m_World->DownloadWorldInit(*msg);
				uint32_t numPixelBodies;
				uint32_t numChunks;
				*msg >> numPixelBodies >> numChunks;

				m_DownloadTotal = static_cast<uint64_t>(numPixelBodies) + static_cast<uint64_t>(numChunks);
				m_DownloadCount = 0;

				PX_TRACE("Downloading game at tick [{0}], simulation tick [{1}]", m_InputTick, m_World->m_SimulationTick);
				if (m_DownloadTotal == 0)
				{
					PX_TRACE("The world is empty! hop on in!");
					//tell the server we are finished, so it can resume the game
					Network::Message finishedDownloadMsg;
					finishedDownloadMsg.header.id = static_cast<uint32_t>(GameMessage::Client_GameDataComplete);
					SendMessageToServer(finishedDownloadMsg);
					//we are connected!
					m_MultiplayerState = MultiplayerState::Connected;
				}
				else
				{
					PX_TRACE("Expecting {0} PixelBodies and {1} Chunks totalling {2} messages", numPixelBodies, numChunks, m_DownloadTotal);
				}
				break;
			}
			case GameMessage::Server_GameDataPixelBody:
			case GameMessage::Server_GameDataChunk:
			{

				//we recieved a pixel body, so lets count it, and add it to the world
				m_DownloadCount++;
				m_World->DownloadWorld(*msg);
				if (m_DownloadCount == m_DownloadTotal)
				{
					PX_INFO("Downloading World: [100%]");
					//tell the server we are finished, so it can resume the game
					Network::Message gameDataCompleteMsg;
					gameDataCompleteMsg.header.id = static_cast<uint32_t>(GameMessage::Client_GameDataComplete);
					SendMessageToServer(gameDataCompleteMsg);

					//now that we have finished recieving the game data and loading the world, start the game!
					m_MultiplayerState = MultiplayerState::Connected;
				}
				else
				{
					Network::Message gameDataRecievedMsg;
					gameDataRecievedMsg.header.id = static_cast<uint32_t>(GameMessage::Client_GameDataRecieved);
					SendMessageToServer(gameDataRecievedMsg);
					PX_INFO("Downloading World: [{0}%]", (double)m_DownloadCount / (double)m_DownloadTotal);
				}
				break;
			}
			case GameMessage::Game_MergedTickClosure:
			{
				MergedTickClosure mtc;
				*msg >> mtc.m_Tick;
				*msg >> mtc.m_ClientCount;
				*msg >> mtc.m_Data;
				d_LastRecievedInputTick = mtc.m_Tick;

				//if this tick is the current one, handle it immediately,
				// don't need to push it to the front of the buffer
				if (mtc.m_Tick == m_InputTick)
				{
					HandleTickClosure(mtc);
					m_InputTick++;
				}
				else if (mtc.m_Tick == m_InputTick)
				{
					//also, if the tick is old just discard it.
					//we only want to hand onto new ones.
				}
				else
				{
					//this is a future tick, so lets throw it on the stack
					m_MTCBuffer.push_back(mtc);
				}

				break;
			}
			case GameMessage::Game_ResetBox2D:
			{
				//gather all rigid body data, store it, and reload it!
				//this has to be done once we are finished with all the previously
				//collected mtc's, so we will mark when we are supposed to
				//reset, and do it then!
				*msg >> m_TickToResetBox2D;
				PX_TRACE("Sim Tick to reset at: {0}", m_TickToResetBox2D);
				PX_TRACE("current sim tick: {0}", m_World->m_SimulationTick);
				break;
			}

			default:
				break;
			}
		}
	}
	
}