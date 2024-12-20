#include "PixelGameServer.h"

#include <imgui.h>

static const int MaxTickStorage = 500;


namespace Pyxis
{
	PixelGameServer::PixelGameServer(uint16_t port) : HostingGameLayer("Pyxis Server"),
		m_OrthographicCameraController(5, 9.0f / 16.0f, -100, 100),
		m_World()//m_World("../Pixel-Game/assets")
	{
		m_SteamPort = port;
		m_World.m_ServerMode = true;
	}

	PixelGameServer::~PixelGameServer()
	{
		//stop the server
		Stop();
	}

	void PixelGameServer::OnAttach()
	{
		//m_ProfileResults = std::vector<ProfileResult>();
		Pyxis::Renderer2D::Init();

		//setup the log so that imgui can display messages and not just the console
		// TODO: setup logs / sinks for spdlog
		//Pyxis::Log::GetClientLogger() ...

		//STEAMTESTING
		HostIP(m_SteamPort);
	}

	void PixelGameServer::OnDetatch()
	{

	}

	void PixelGameServer::OnUpdate(Pyxis::Timestep ts)
	{
		UpdateInterface();
		HandleMessages();

		auto time = std::chrono::high_resolution_clock::now();
		//only update if there are players
		if (m_ClientDataMap.size() > 0)
		if (m_TickRate > 0 &&
			std::chrono::time_point_cast<std::chrono::microseconds>(time).time_since_epoch().count()
			-
			std::chrono::time_point_cast<std::chrono::microseconds>(m_UpdateTime).time_since_epoch().count()
			>= (1.0f / m_TickRate) * 1000000.0f)
		{
			m_UpdateTime = time;

			//skip sending the message if we are waiting for a client to connect!
			if (m_DownloadingClients.empty())
			{
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

				if (m_TickRequestStorage.size() > MaxTickStorage) m_TickRequestStorage.pop_front();

				//process the mtc on the server side
				HandleTickClosure(m_CurrentMergedTickClosure);
				m_InputTick++;

				//reset the merged tick
				m_CurrentMergedTickClosure = MergedTickClosure();
			}
			
		}
		
	}

	void PixelGameServer::HandleMessages()
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
			case GameMessage::Client_RequestAllClientData:
			{
				PX_TRACE("Recieved request for client data from: [{0}]:{1}", msg->clientHConnection, m_ClientDataMap[msg->clientHConnection].m_Name);
				Network::Message clientDataMsg;
				clientDataMsg.header.id = static_cast<uint32_t>(GameMessage::Server_AllClientData);
				uint32_t numClients = 0;
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
				m_World.GetGameDataInit(gameDataInitMsg);
				gameDataInitMsg << m_InputTick;
				SendMessageToClient(msg->clientHConnection, gameDataInitMsg);

				//now lets populate a vector of messages to be sent,
				//being the chunks and pixel bodies
				m_DownloadingClients[msg->clientHConnection] = std::vector<Network::Message>();
				PX_WARN("Created a vector of messages for client");
				m_World.GetGameData(m_DownloadingClients[msg->clientHConnection]);
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
				m_World.ResetBox2D();
				b2ResetMsg << m_World.m_SimulationTick;
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
	

	void PixelGameServer::OnImGuiRender()
	{

		auto dock = ImGui::DockSpaceOverViewport(ImGui::GetID("MainDock"), (const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode);

		//top menu bar
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

		ImGui::SetNextWindowDockID(dock);
		if (ImGui::Begin("Server"))
		{
			//ImGui::Text("Sleep Timer");
			//ImGui::ProgressBar(static_cast<float>(m_SleepDelay) / static_cast<float>(m_SleepDelayMax));
			ImGui::Text(("Players:" + std::to_string(m_ClientDataMap.size())).c_str());
		}
		ImGui::End();

	}

	void PixelGameServer::OnEvent(Pyxis::Event& e)
	{
		//m_OrthographicCameraController.OnEvent(e);
		//Pyxis::EventDispatcher dispatcher(e);
		//dispatcher.Dispatch<Pyxis::WindowResizeEvent>(PX_BIND_EVENT_FN(Sandbox2D::OnWindowResizeEvent));
	}

	bool PixelGameServer::OnWindowResizeEvent(Pyxis::WindowResizeEvent& event) {
		//m_OrthographicCameraController.SetAspect((float)event.GetHeight() / (float)event.GetWidth());
		//Pyxis::Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
		////Pyxis::Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
		return false;
	}

	void PixelGameServer::DisconnectClient(HSteamNetConnection client, std::string Reason)
	{
		OnClientDisconnect(client);
		m_SteamNetworkingSockets->CloseConnection(client, 0, Reason.c_str(), true);
	}

	//bool PixelGameServer::OnClientConnect(std::shared_ptr<Network::Connection<GameMessage>> client)
	//{
	//	//I am able to block the incoming connection if i desire here
	//	m_PlayerCount++;

	//	return true;
	//}

	/*void PixelGameServer::OnClientDisconnect(std::shared_ptr<Network::Connection<GameMessage>> client)
	{
		PX_TRACE("Removing Client [{0}]", client->GetID());
		m_PlayerCount--;
		m_ClientsNeededForTick.erase(client->GetID());

		Network::Message<GameMessage> disconnectMsg;
		disconnectMsg.header.id = GameMessage::Server_ClientDisconnected;
		disconnectMsg << client->GetID();
		MessageAllClients(disconnectMsg, client);
		return;
	}*/

	
	void PixelGameServer::OnClientDisconnect(HSteamNetConnection client)
	{
		//remove client from map
		m_ClientDataMap.erase(client);

		if (m_DownloadingClients.contains(client))
		{
			PX_WARN("Popped DLCL Back. Size: {0}", m_DownloadingClients.size());
			m_DownloadingClients.erase(client);
		}
	}

}
