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
		

		//server update
		Update(-1, true);


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
		Network::Message<GameMessage> msg;
		msg.header.id = GameMessage::Server_ClientConnected;
		client->Send(msg);

		return true;
	}

	void PixelGameServer::OnClientDisconnect(std::shared_ptr<Network::Connection<GameMessage>> client)
	{
		PX_TRACE("Removing Client [{0}]", client->GetID());
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
			PX_TRACE("[{0}]: Registering client with ID", client->GetID());
			Network::Message<GameMessage> msg;
			msg.header.id = GameMessage::Server_ClientAssignID;
			msg << client->GetID();
			client->Send(msg);
			break;
		}
		case GameMessage::Game_AddPlayer:
		{
			PX_TRACE("[{0}]: Added a player", client->GetID());

			//send the message to the other players, ignoring sender
			MessageAllClients(msg, client);

			//create the player in the authority world
			uint64_t tick;
			uint64_t id;
			glm::vec2 position;
			msg >> position;
			msg >> id;
			msg >> tick;
			m_World.CreatePlayer(id, position);

			break;
		}
		case GameMessage::Message_All:
		{
			PX_TRACE("[{0}]: Message All", client->GetID());
			Network::Message<GameMessage> msg;
			msg.header.id = GameMessage::Server_Message;
			msg << client->GetID();
			MessageAllClients(msg, client);
			break;
		}
		}
		return;
	}

	void PixelGameServer::OnClientValidated(std::shared_ptr<Network::Connection<GameMessage>> client)
	{
		//now that the client has connected to the server and has been validated,
		//tell the client that they have been accepted / validated
		Network::Message<GameMessage> msg;
		msg.header.id = GameMessage::Server_ClientAccepted;
		msg << client->GetID();
		client->Send(msg);
	}
}
