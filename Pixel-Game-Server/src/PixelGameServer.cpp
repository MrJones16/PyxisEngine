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
		m_World("../Pixel-Game/assets")
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
		Stop();
	}

	void PixelGameServer::OnUpdate(Pyxis::Timestep ts)
	{
		//Pyxis::Renderer2D::BeginScene(m_OrthographicCameraController.GetCamera());
		 
		//check to make sure all the players are still connected
		for each (std::shared_ptr<Network::Connection<GameMessage>> client in m_DeqConnections)
		{
			if (!client->IsConnected())
			{
				//client is no longer connected, so remove them
				OnClientDisconnect(client);
				//client.reset();
				m_DeqConnections.erase(
					std::remove(m_DeqConnections.begin(), m_DeqConnections.end(), client), m_DeqConnections.end());
				
			}
		}

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



		while (m_MTCDeque.size() > 0 && m_MTCDeque.front().m_TickClosureCount >= m_PlayerCount)
		{
			Network::Message<GameMessage> msg;
			msg.header.id = GameMessage::Game_MergedTickClosure;

			MergedTickClosure& mtc = m_MTCDeque.front();
			msg << mtc.m_Data;
			msg << mtc.m_InputActionCount;
			msg << mtc.m_Tick;

			MessageAllClients(msg);
			m_GameTick++;
			m_World.HandleTickClosure(mtc);
			m_MTCDeque.pop_front();

			//now that i sent the merged packet to everyone, and i updated the authoritative world with
			//that same packet, ill update the world.
			if (m_World.m_Running)
				m_World.UpdateWorld();
		}



		//m_FrameBuffer->Unbind()
		//Pyxis::Renderer2D::EndScene();
	}

	void PixelGameServer::OnImGuiRender()
	{
		ImGui::DockSpaceOverViewport();
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
		/*Network::Message<GameMessage> msg;
		msg.header.id = GameMessage::Server_ClientConnected;
		client->Send(msg);*/

		return true;
	}

	void PixelGameServer::OnClientDisconnect(std::shared_ptr<Network::Connection<GameMessage>> client)
	{
		PX_TRACE("Removing Client [{0}]", client->GetID());
		m_PlayerCount--;
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
			//send the world, and rigid bodies

			///TODO need to put in actual world data, and possibly pause the game?
			Network::Message<GameMessage> msg;
			msg.header.id = GameMessage::Game_GameData;
			msg << m_GameTick;
			client->Send(msg);
			m_PlayerCount++;

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

			//std::cout << ID << "'s m_tick: " << tick << std::endl;

			if (m_MTCDeque.size() == 0)
			{
				//first merged tick closure, so just add it
				MergedTickClosure mtc;
				mtc.m_Tick = tick;
				mtc.AddTickClosure(tickClosure);
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
						m_MTCDeque.at(i).AddTickClosure(tickClosure);
						return;
					}
				}
				//didn't find the tick closure in the queue, so just add it
				MergedTickClosure mtc;
				mtc.m_Tick = tick;
				mtc.AddTickClosure(tickClosure);
				m_MTCDeque.push_back(mtc);
			}
			break;
		}
		}
		//no more code here, there is a return in the switch
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
