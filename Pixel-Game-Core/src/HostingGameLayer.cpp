#include "HostingGameLayer.h"

static const int MaxTickStorage = 500;

namespace Pyxis
{

	HostingGameLayer::HostingGameLayer(std::string debugName) : GameLayer(debugName)
	{

	}

	HostingGameLayer::~HostingGameLayer()
	{

	}

	void HostingGameLayer::OnUpdate(Timestep ts)
	{
		PROFILE_SCOPE("GameLayer::OnUpdate");
		m_OrthographicCameraController.OnUpdate(ts);
		m_Scene->Update(ts);

		UpdateInterface();

		HandleMessages();

		//rendering
		#if STATISTICS
		Renderer2D::ResetStats();
		#endif

		{
			PROFILE_SCOPE("Renderer Prep");
			m_SceneFrameBuffer->Bind();
			RenderCommand::SetClearColor({ 198 / 255.0f, 239 / 255.0f, 249 / 255.0f, 1 });
			RenderCommand::Clear();
			Renderer2D::BeginScene(m_OrthographicCameraController.GetCamera().get());
		}


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

				m_UpdateTime = time;

				//skip sending the message if we are waiting for a client to connect!
				if (m_DownloadingClients.empty())
				{
					//first, since we are also playing, put our tick closure into the mtc
					m_CurrentMergedTickClosure.AddTickClosure(m_CurrentTickClosure, k_HSteamNetConnection_Invalid);

					//pack the merged tick into a message and send to all clients
					Network::Message msg(static_cast<uint32_t>(GameMessage::Game_MergedTickClosure));
					msg << m_CurrentMergedTickClosure.m_Data;
					msg << m_CurrentMergedTickClosure.m_ClientCount;
					msg << m_InputTick;

					//send the messages unreliably, since i have mtc recovery set up already!
					SendMessageToAllClients(msg, 0, k_nSteamNetworkingSend_Unreliable);

					//add the compressed message into the tick storage
					m_TickRequestStorage.emplace_back();
					msg.Compressed(m_TickRequestStorage.back());

					//limit the tick storage to "MaxTickStorage"
					if (m_TickRequestStorage.size() > MaxTickStorage) m_TickRequestStorage.pop_front();

					//process the mtc on the our end
					HandleTickClosure(m_CurrentMergedTickClosure);
					m_InputTick++;

					//reset the merged tick
					m_CurrentMergedTickClosure = MergedTickClosure();

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
					mousePosMsg.header.id = static_cast<uint32_t>(GameMessage::Server_ClientDataMousePosition);
					glm::ivec2 mousePos = GetMousePositionImGui();
					glm::vec2 vec = m_OrthographicCameraController.MousePercentToWorldPos(mousePos.x, mousePos.y);
					m_ClientData.m_CursorWorldPosition = vec;
					//invalid for the server
					HSteamNetConnection serverConn = k_HSteamNetConnection_Invalid;
					mousePosMsg << vec;
					mousePosMsg << serverConn;
					SendMessageToAllClients(mousePosMsg);
				}
			}
		}

		for (auto& clientPair : m_ClientDataMap)
		{
			if (clientPair.first == 0) continue;
			//draw the 3x3 square for each players cursor
			glm::vec3 worldPos = glm::vec3(clientPair.second.m_CursorWorldPosition.x, clientPair.second.m_CursorWorldPosition.y, 5);
			glm::vec2 size = glm::vec2(3.0f / CHUNKSIZE);
			Renderer2D::DrawQuad(worldPos, size, clientPair.second.m_Color);
		}

		GameUpdate(ts);

		m_Scene->Render();

		Renderer2D::EndScene();

		m_SceneFrameBuffer->Unbind();
	}

	void HostingGameLayer::OnImGuiRender()
	{
		auto dock = ImGui::DockSpaceOverViewport(ImGui::GetID("MainDock"), (const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode);
		ClientImGuiRender(dock);
	}

	/// <summary>
	/// Begin Hosting on the given virtual port, and starting your own game
	/// </summary>
	/// <param name="virtualPort"></param>
	void HostingGameLayer::StartP2P(int virtualPort)
	{
		CreateWorld();
		SteamFriends()->SetRichPresence("status", "Hosting a game of Pyxis!");
		SteamFriends()->SetRichPresence("connect", "gameinfo");
		HostP2P(virtualPort);
	}

	/// <summary>
	/// Begin Hosting on the given port, not over steam, and run game as well
	/// </summary>
	/// <param name="virtualPort"></param>
	void HostingGameLayer::StartIP(uint16_t port)
	{
		CreateWorld();
		HostIP(port);
	}

	void HostingGameLayer::HandleMessages()
	{
		Ref<Network::Message> msg;
		while (PollMessage(msg))
		{
			switch (static_cast<GameMessage>(msg->header.id))
			{
			case GameMessage::Client_ClientData:
			{
				//add the client data to a map so we can send it to new players later
				PX_TRACE("Recieved client data from client: [{0}]:{1}", msg->clientHConnection, m_ClientDataMap[msg->clientHConnection].m_Name);
				m_ClientDataMap[msg->clientHConnection];
				*msg >> m_ClientDataMap[msg->clientHConnection];
				break;
			}
			case GameMessage::Client_ClientDataMousePosition:
			{
				//when sent to clients, we include the HSteamNetConnection, and when clients send to us it is just the world position
				
				//Update the position on our end, and send it as unreliable to other clients
				m_ClientDataMap[msg->clientHConnection];
				*msg >> m_ClientDataMap[msg->clientHConnection].m_CursorWorldPosition;
				Network::Message mousePosMsg;
				mousePosMsg.header.id = static_cast<uint32_t>(GameMessage::Server_ClientDataMousePosition);
				mousePosMsg << m_ClientDataMap[msg->clientHConnection].m_CursorWorldPosition;
				mousePosMsg << msg->clientHConnection;
				SendMessageToAllClients(mousePosMsg, msg->clientHConnection, k_nSteamNetworkingSend_Unreliable);
				break;
			}
			case GameMessage::Client_RequestAllClientData:
			{
				PX_TRACE("Recieved request for client data from: [{0}]:{1}", msg->clientHConnection, m_ClientDataMap[msg->clientHConnection].m_Name);
				Network::Message clientDataMsg;
				clientDataMsg.header.id = static_cast<uint32_t>(GameMessage::Server_AllClientData);
				uint32_t numClients = 0;
				m_ClientDataMap[0] = m_ClientData;
				for (auto& clientPair : m_ClientDataMap)
				{
					if (clientPair.first != msg->clientHConnection)
					{
						clientDataMsg << clientPair.second;
						clientDataMsg << clientPair.first;
						numClients++;
					}
				}
				clientDataMsg << numClients;
				SendMessageToClient(msg->clientHConnection, clientDataMsg);
				PX_TRACE("Sent All Client Data to {0}", msg->clientHConnection);
				break;
			}
			case GameMessage::Client_RequestMergedTick:
			{
				//assuming there are 66 mtc's stored
				//m_inputtick is then 67
				//if tick 65 was requested, then that would be located at position 64
				//which is 66 - (67 - 65)
				//         66 -  2 = 64
				//that is the reasoning for the calculation below
				//basically, the difference between the current tick and how far back
				//the request is, is how far back from the end of the storage to grab from


				uint64_t tick;
				*msg >> tick;
				int diff = m_InputTick - tick;
				int position = m_TickRequestStorage.size() - diff;
				if (position < 0 || position >= m_TickRequestStorage.size())
				{
					//Requested tick does not exist!
					PX_WARN("Requested Tick Not Found, setting client to be out of sync!");
					DisconnectClient(msg->clientHConnection, "Client Became Desynced (Requested a tick we no longer had!)");
				}
				else
					SendCompressedStringToClient(msg->clientHConnection, m_TickRequestStorage.at(position));

				break;
			}
			case GameMessage::Client_RequestGameData:
			{
				//TODO: 
				//maybe send some info to other clients? so their game doesn't just randomly stop...


				// begin by halting the game for this client to join the world
				// then, send the initializing message, saying how many chunks need
				// to be sent, and how many rigid bodies?
				// then send all the chunks individually and send pixel bodies in groups as well? or maybe individually.


				//now, lets send that initial message describing how many chunks we will send
				//and how many pixel bodies there are.
				Network::Message gameDataInitMsg;
				m_World->GetGameDataInit(gameDataInitMsg);
				gameDataInitMsg << m_InputTick;
				SendMessageToClient(msg->clientHConnection, gameDataInitMsg);

				//now lets populate a vector of messages to be sent,
				//being the chunks and pixel bodies
				//this is also how we track pausing to let someone download
				m_DownloadingClients[msg->clientHConnection] = std::vector<Network::Message>();
				PX_WARN("Created a vector of messages for client");
				m_World->GetGameData(m_DownloadingClients[msg->clientHConnection]);
				//send the first and wait for it to be acknowledged

				if (!m_DownloadingClients[msg->clientHConnection].empty())
				{
					PX_TRACE("Sent GameDataMsg: ID[{0}], Size[{1}]", m_DownloadingClients[msg->clientHConnection].back().header.id, m_DownloadingClients[msg->clientHConnection].back().size());
					SendMessageToClient(msg->clientHConnection, m_DownloadingClients[msg->clientHConnection].back());
					m_DownloadingClients[msg->clientHConnection].pop_back();
				}

				//as soon as a new client joins the game,
				//we have to reset everyones box2d simulation, so send that now so everyone stays in sync!
				Network::Message b2ResetMsg;
				b2ResetMsg.header.id = static_cast<uint32_t>(GameMessage::Game_ResetBox2D);
				m_World->ResetBox2D();
				b2ResetMsg << m_World->m_SimulationTick;
				SendMessageToAllClients(b2ResetMsg, msg->clientHConnection);
				break;
			}
			case GameMessage::Client_GameDataRecieved:
			{
				PX_TRACE("Sent GameDataMsg: ID[{0}], Size[{1}]", m_DownloadingClients[msg->clientHConnection].back().header.id, m_DownloadingClients[msg->clientHConnection].back().size());
				SendMessageToClient(msg->clientHConnection, m_DownloadingClients[msg->clientHConnection].back());
				m_DownloadingClients[msg->clientHConnection].pop_back();
				PX_WARN("Sent GameDataPacket. Remaining: {0}", m_DownloadingClients[msg->clientHConnection].size());
				break;
			}
			case GameMessage::Client_GameDataComplete:
			{
				//the connecting client finished loading the world, so lets resume! 
				if (m_DownloadingClients[msg->clientHConnection].empty())
				{
					PX_WARN("Erased Client. DLCL Size: {0}", m_DownloadingClients.size());
					m_DownloadingClients.erase(msg->clientHConnection);
				}
				break;
			}
			case GameMessage::Game_TickClosure:
			{
				//merge the recieved tick closure into our current merged tick closure!

				TickClosure tc;
				*msg >> tc.m_InputActionCount;
				*msg >> tc.m_Data;
				m_CurrentMergedTickClosure.AddTickClosure(tc, msg->clientHConnection);
				//finished! we just wait till it's time to send the message, 
				break;
			}
			}

		}
	}

	void HostingGameLayer::OnClientDisconnect(HSteamNetConnection& client)
	{
		if (m_ClientDataMap.contains(client))
			m_ClientDataMap.erase(client);
	}

	

}