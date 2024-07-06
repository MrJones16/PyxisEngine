#include "PixelGameServer.h"

#include <ImGui/imgui.h>
//#include <glm/gtc/type_ptr.hpp>

//#include <Platform/OpenGL/OpenGLShader.h>
#include <chrono>


namespace Pyxis
{
	PixelGameServer::PixelGameServer(uint16_t port)
		: Network::ServerInterface<GameMessage>(port), Layer("Pyxis Server"),
		m_OrthographicCameraController(5, 9.0f / 16.0f, -100, 100),
		m_World()//m_World("../Pixel-Game/assets")
	{
		m_World.m_ServerMode = true;
	}

	void PixelGameServer::OnAttach()
	{
		//m_ProfileResults = std::vector<ProfileResult>();
		Pyxis::Renderer2D::Init();

		//setup the log so that imgui can display messages and not just the console
		// TODO: setup logs / sinks for spdlog
		//Pyxis::Log::GetClientLogger() ...

		Start();
	}

	void PixelGameServer::OnDetatch()
	{
		//stop the server
		Stop();
	}

	void PixelGameServer::OnUpdate(Pyxis::Timestep ts)
	{
		//check to make sure all the players are still connected
		std::vector< std::shared_ptr<Network::Connection<GameMessage>>> clientsToRemove;
		for each (std::shared_ptr<Network::Connection<GameMessage>> client in m_DeqConnections)
		{
			if (!client->IsConnected())
			{
				clientsToRemove.push_back(client);
			}
		}
		//delete the clients after iterating
		for each (auto client in clientsToRemove)
		{
			//client is no longer connected, so remove them
			OnClientDisconnect(client);
			//client.reset();
			m_DeqConnections.erase(
				std::remove(m_DeqConnections.begin(), m_DeqConnections.end(), client), m_DeqConnections.end());
		}

		//server is sending merged closure ticks when first joining the server...

		//server update
		if (m_PlayerCount <= 0)
		{
			Update(-1, true);//i can save on resources when nobody is connected
		}
		else
		{
			Update(-1, false);
		}
		

		//attempt to run the merged tick closure at the front of the queue,
		
		if (m_MTCDeque.size() >= 200)
		{
			PX_TRACE("server fell behind!");
			PX_TRACE("Server is waiting on tick {0}", m_MTCDeque.front().m_Tick);
		}

		///if there are no players, 
		//todo: if a player joins, could they join in the middle of this?
		//they would be out of sync!
		if (m_PlayerCount == 0)
		{
			while (m_MTCDeque.size() > 0)
			{
				//handle the remaining merged tick closures
				MergedTickClosure& mtc = m_MTCDeque.front();
				m_InputTick++;
				m_World.HandleTickClosure(mtc);
				m_MTCDeque.pop_front();
			}
		}


		while (m_MTCDeque.size() > 0)
		{
			bool missingClient = false;
			for each (uint64_t id in m_ClientsNeededForTick)
			{
				if (m_MTCDeque.front().m_Clients.find(id) == m_MTCDeque.front().m_Clients.end())
				{
					//one of the clients we need are missing, so wait
					missingClient = true;
					//PX_TRACE("Waiting on client [{0}]", id);
					break;
				}
			}
			if (missingClient) break;

			//we have all the clients we need, so send the merged tick!
			Network::Message<GameMessage> msg;
			msg.header.id = GameMessage::Game_MergedTickClosure;

			MergedTickClosure& mtc = m_MTCDeque.front();
			msg << mtc.m_Data;
			msg << mtc.m_InputActionCount;
			msg << mtc.m_Tick;

			MessageAllClients(msg);
			m_World.HandleTickClosure(mtc);
			m_MTCDeque.pop_front();
			m_InputTick++;
		}
	}

	void PixelGameServer::OnImGuiRender()
	{
		//we are a server, do no imgui rendering!
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

	bool PixelGameServer::OnClientConnect(std::shared_ptr<Network::Connection<GameMessage>> client)
	{
		//I am able to block the incoming connection if i desire here
		m_PlayerCount++;

		return true;
	}

	void PixelGameServer::OnClientDisconnect(std::shared_ptr<Network::Connection<GameMessage>> client)
	{
		PX_TRACE("Removing Client [{0}]", client->GetID());
		m_PlayerCount--;
		m_ClientsNeededForTick.erase(client->GetID());
		return;
	}

	void PixelGameServer::OnMessage(std::shared_ptr<Network::Connection<GameMessage>> client, Network::Message<GameMessage>& msg)
	{
		switch (msg.header.id)
		{
		case GameMessage::Ping:
		{
			PX_TRACE("[{0}]: Server Ping", client->GetID());
			//simply bounce the message back
			client->Send(msg);
			break;
		}
		case GameMessage::Client_RegisterWithServer:
		{
			//client is asking for an ID, so give it it's ID
			PX_TRACE("[{0}]: Registering client with ID", client->GetID());
			Network::Message<GameMessage> msg;
			msg.header.id = GameMessage::Server_ClientAssignID;
			msg << client->GetID();
			client->Send(msg);
			break;
		}
		case GameMessage::Game_RequestGameData:
		{
			//client is requesting the world data, so send it!
			//send the world and rigid body data
			Network::Message<GameMessage> msg;
			msg.header.id = GameMessage::Game_GameData;
			//in order to send the game data, i'll pause the game for now while its sending?
			//or maybe not, and ill just keep updating and sending the client the 
			//merged ticks so it can catch up afterwards
			m_World.GetWorldData(msg);
			msg << m_World.m_Running;
			msg << m_InputTick;
			client->Send(msg);

			//as soon as a new client joins the game,
			//we have to reset everyones box2d simulation, so send that now so everyone stays in sync!
			Network::Message<GameMessage> b2msg;
			b2msg.header.id = GameMessage::Game_ResetBox2D;
			m_World.ResetBox2D();
			b2msg << m_World.m_SimulationTick;
			MessageAllClients(b2msg, client);

			break;
		}
		case GameMessage::Game_Loaded:
		{
			//the server now expects that client to send ticks
			m_ClientsNeededForTick.insert(client->GetID());
			PX_TRACE("Starting to need client {0} at game tick {1}", client->GetID(), m_InputTick);
			Network::Message<GameMessage> msg;
			msg.header.id = GameMessage::Game_TickToEnter;
			msg << m_InputTick;
			client->Send(msg);
			break;
		}
		case GameMessage::Game_TickClosure:
		{

			//merge the tick closure with other clients, and send it back if all merged

			TickClosure tickClosure;
			uint64_t ID;
			uint64_t tick;
			msg >> ID;
			msg >> tick;
			msg >> tickClosure.m_InputActionCount;
			msg >> tickClosure.m_Data;

			if (m_MTCDeque.size() == 0)
			{
				//first merged tick closure, so just add it
				MergedTickClosure mtc;
				mtc.m_Tick = tick;
				mtc.AddTickClosure(tickClosure, client->GetID());
				m_MTCDeque.push_back(mtc);
			}
			else
			{
				//there are already tick closures, so find the one to merge with or make
				uint64_t oldestTick = m_MTCDeque.front().m_Tick;
				for (int i = 0; i < m_MTCDeque.size(); i++)
				{
					if (tick == m_MTCDeque.at(i).m_Tick)
					{
						//found the same tick in the queue, so add it
						m_MTCDeque.at(i).AddTickClosure(tickClosure, client->GetID());
						return;
					}
				}
				//didn't find the tick closure in the queue, so just add it
				MergedTickClosure mtc;
				mtc.m_Tick = tick;
				mtc.AddTickClosure(tickClosure, client->GetID());
				m_MTCDeque.push_back(mtc);
			}
			break;
		}
		}
		//no more code here, there is a return in the switch statement
		return;
	}

	void PixelGameServer::OnClientValidated(std::shared_ptr<Network::Connection<GameMessage>> client)
	{
		//now that the client has connected to the server and has been validated,
		//tell the client that they have been accepted / validated
		Network::Message<GameMessage> msg;
		msg.header.id = GameMessage::Server_ClientAccepted;
		client->Send(msg);
	}
}
