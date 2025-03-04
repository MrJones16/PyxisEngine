#include "MultiplayerGameNode.h"
#include <MenuNode.h>
namespace Pyxis
{

	MultiplayerGameNode::MultiplayerGameNode() : GameNode("Multiplayer Game Node")
	{
		//initialize the connecting screen nodes
		m_LSScreenSpace = Instantiate<UI::ScreenSpace>();
		m_LSScreenSpace->Translate({ 0,0,-0.2 });
		m_LSScreenSpace->m_Name = "Loading Screen Space";
		AddChild(m_LSScreenSpace);

		m_LSCanvas = Instantiate<UI::Canvas>();
		m_LSCanvas->CreateTextures("assets/textures/UI/GreenCanvas/", "GreenCanvasTile_", ".png");
		m_LSCanvas->m_AutomaticSizing = true;
		m_LSCanvas->m_PPU = 32;
		m_LSScreenSpace->AddChild(m_LSCanvas);


		m_LSText = Instantiate<UI::Text>(ResourceManager::Load<Font>("assets/fonts/Aseprite.ttf"));
		m_LSText->Translate({ 0,0,-0.01 });
		m_LSText->m_Text = "Connecting";
		m_LSText->m_Size = { 128, 32 };
		m_LSText->m_FontSize = 1;
		m_LSText->m_Alignment = UI::Center;
		m_LSText->m_MultiLine = false;
		m_LSCanvas->AddChild(m_LSText);

		m_LSButton = Instantiate<UI::TextButton>("Okay Button", ResourceManager::Load<Font>("assets/fonts/Aseprite.ttf"), std::bind(&MultiplayerGameNode::ReturnToMenu, this));
		m_LSButton->m_PPU = 0.5f;
		m_LSButton->m_Text = "Okay";
		m_LSButton->m_TextColor = glm::vec4(255.0f / 255.0f, 221.0f / 255.0f, 159.0f / 255.0f, 1);;
		m_LSButton->Translate({ 0,-64,-0.01 });
		m_LSButton->m_TextureResource = ResourceManager::Load<Texture2DResource>("assets/textures/UI/ButtonWide.png");
		m_LSButton->m_TextureResourcePressed = ResourceManager::Load<Texture2DResource>("assets/textures/UI/ButtonWidePressed.png");
		m_LSButton->UpdateSizeFromTexture();
		m_LSButton->m_TextBorderSize = glm::vec2(5, 5);
		m_LSButton->m_TextOffset = { 0, 3, -0.0001f };
		m_LSButton->m_TextOffsetPressed = { 0, 1, -0.0001f };
		m_LSButton->m_Enabled = false;
		m_LSCanvas->AddChild(m_LSButton);

		m_LSScreenSpace->PropagateUpdate();
	}

	void MultiplayerGameNode::ReturnToMenu()
	{
		Disconnect();
		m_MultiplayerState = MultiplayerState::Disconnected;
		auto menu = Instantiate<MenuNode>();
		QueueFreeHierarchy();
	}

	void MultiplayerGameNode::Connect(const std::string& AddressAndPort)
	{
		//max out input tick so we can discard early sent mtc's
		m_InputTick = -1;
		m_MultiplayerState = MultiplayerState::Connecting;
		ConnectIP(AddressAndPort);
	}

	void MultiplayerGameNode::Connect(SteamNetworkingIdentity& identity, int virtualPort)
	{
		//max out input tick so we can discard early sent mtc's
		m_InputTick = -1;
		m_MultiplayerState = MultiplayerState::Connecting;
		ConnectP2P(identity, virtualPort);
	}

	

	void MultiplayerGameNode::OnUpdate(Timestep ts)
	{
		UpdateInterface();

		if (m_ConnectionStatus == ConnectionStatus::Connecting || m_ConnectionStatus == ConnectionStatus::Connected)
		{
			HandleMessages();
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
			PROFILE_SCOPE("GameLayer::OnUpdate");
			GameUpdate(ts);
			for (auto& clientPair : m_ClientDataMap)
			{
				//draw the 3x3 square for each players cursor
				glm::vec3 worldPos = glm::vec3(clientPair.second.m_CursorWorldPosition.x, clientPair.second.m_CursorWorldPosition.y, 5);
				glm::vec2 size = glm::vec2(3.0f / CHUNKSIZE);
				Renderer2D::DrawQuad(worldPos, size, clientPair.second.m_Color);
			}

		}
	}

	void MultiplayerGameNode::OnFixedUpdate()
	{
		if (m_MultiplayerState == MultiplayerState::Connected)
		{
			PROFILE_SCOPE("Simulation Update");

			//reset world, then apply the tick closure
			if (m_World.m_SimulationTick == m_TickToResetBox2D)
			{
				m_World.ResetBox2D();
				m_TickToResetBox2D = -1;
			}

			//for multiplayer, just send off your tick closure
			//(basically just your input shoved into a message)
			Network::Message msg;
			msg.header.id = static_cast<uint32_t>(GameMessage::Game_TickClosure);
			msg << m_CurrentTickClosure.m_Data;
			msg << m_CurrentTickClosure.m_InputActionCount;
			SendMessageToServer(msg);

			//reset tick closure
			m_CurrentTickClosure = TickClosure();

			//update mouse position for others
			Network::Message mousePosMsg;
			mousePosMsg.header.id = static_cast<uint32_t>(GameMessage::Client_ClientDataMousePosition);
			m_ClientData.m_CursorWorldPosition = GetMousePosWorld();
			mousePosMsg << m_ClientData.m_CursorWorldPosition;
			SendMessageToServer(mousePosMsg);
		}

		
	}

	void MultiplayerGameNode::OnRender()
	{
		if (m_MultiplayerState == MultiplayerState::DownloadingWorld)
		{
			m_LSText->m_Text = "Downloading World: { " + std::to_string(((float)m_DownloadCount / (float)m_DownloadTotal) * 100.0f)  + "% }";
		}
			
	}

	void MultiplayerGameNode::OnConnectionSuccess()
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
		
		m_LSText->m_Text = "Requesting Client Data";
	}
	
	void MultiplayerGameNode::OnConnectionLost(const std::string& reasonText)
	{
		////m_ConnectionStatus = ConnectionStatus::Disconnected;
		
		//m_MultiplayerState = MultiplayerState::Disconnected;

		m_LSCanvas->m_Enabled = true;
		m_LSText->m_Enabled = true;
		m_LSButton->m_Enabled = true;
		m_LSText->m_Text = "Connection Lost";
		
	}

	void MultiplayerGameNode::OnConnectionFailure(const std::string& reasonText)
	{
		//same as lost
		m_LSCanvas->m_Enabled = true;
		m_LSText->m_Enabled = true;
		m_LSButton->m_Enabled = true;
		m_LSText->m_Text = "Connection Failed";
	}

	void MultiplayerGameNode::HandleMessages()
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
				m_LSText->m_Text = "Downloading World...";
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

				m_World.DownloadWorldInit(*msg);
				uint32_t numPixelBodies;
				uint32_t numChunks;
				*msg >> numPixelBodies >> numChunks;

				m_DownloadTotal = static_cast<uint64_t>(numPixelBodies) + static_cast<uint64_t>(numChunks);
				m_DownloadCount = 0;

				PX_TRACE("Downloading game at tick [{0}], simulation tick [{1}]", m_InputTick, m_World.m_SimulationTick);
				if (m_DownloadTotal == 0)
				{
					PX_TRACE("The world is empty! hop on in!");
					//tell the server we are finished, so it can resume the game
					Network::Message finishedDownloadMsg;
					finishedDownloadMsg.header.id = static_cast<uint32_t>(GameMessage::Client_GameDataComplete);
					SendMessageToServer(finishedDownloadMsg);
					//we are connected!
					m_MultiplayerState = MultiplayerState::Connected;
					m_LSCanvas->m_Enabled = false;
					m_LSText->m_Enabled = false;
					m_LSButton->m_Enabled = false;
				}
				else
				{
					PX_TRACE("Expecting {0} PixelBodies and {1} Chunks totalling {2} messages", numPixelBodies, numChunks, m_DownloadTotal);
				}
				break;
			}
			case GameMessage::Server_GameDataRigidBody:
			case GameMessage::Server_GameDataChunk:
			{

				//we recieved a pixel body, so lets count it, and add it to the world
				m_DownloadCount++;
				m_World.DownloadWorld(*msg);
				if (m_DownloadCount == m_DownloadTotal)
				{					
					PX_INFO("Downloading World: [100%]");
					//tell the server we are finished, so it can resume the game
					Network::Message gameDataCompleteMsg;
					gameDataCompleteMsg.header.id = static_cast<uint32_t>(GameMessage::Client_GameDataComplete);
					SendMessageToServer(gameDataCompleteMsg);

					//now that we have finished recieving the game data and loading the world, start the game!
					m_MultiplayerState = MultiplayerState::Connected;
					m_LSCanvas->m_Enabled = false;
					m_LSText->m_Enabled = false;
					m_LSButton->m_Enabled = false;
				}
				else
				{
					Network::Message gameDataRecievedMsg;
					gameDataRecievedMsg.header.id = static_cast<uint32_t>(GameMessage::Client_GameDataRecieved);
					SendMessageToServer(gameDataRecievedMsg);
					//PX_INFO("Downloading World: [{0}%]", (double)m_DownloadCount / (double)m_DownloadTotal);
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
				PX_TRACE("current sim tick: {0}", m_World.m_SimulationTick);
				break;
			}

			default:
				break;
			}
		}
	}


}
