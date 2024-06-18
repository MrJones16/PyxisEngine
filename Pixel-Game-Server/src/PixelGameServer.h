#pragma once

#include "Pyxis.h"
#include <Pyxis/Network/Network.h>
#include "Pyxis/Core/OrthographicCameraController.h"

namespace Pyxis
{
	class PixelGameServerInterface : public Pyxis::Network::ServerInterface<Network::CustomMessageTypes>
	{
	public:
		PixelGameServerInterface(uint16_t port) : Pyxis::Network::ServerInterface<Network::CustomMessageTypes>(port)
		{

		}
	protected:
		bool OnClientConnect(std::shared_ptr<Pyxis::Network::Connection<Network::CustomMessageTypes>> client) override
		{
			return true;
		}

		void OnClientDisconnect(std::shared_ptr<Pyxis::Network::Connection<Network::CustomMessageTypes>> client) override
		{
			return;
		}

		void OnMessage(std::shared_ptr<Pyxis::Network::Connection<Network::CustomMessageTypes>> client, Pyxis::Network::Message< Network::CustomMessageTypes>& msg) override
		{
			switch (msg.header.id)
			{
			case Network::CustomMessageTypes::ServerPing:
				PX_TRACE("[{0}]: Server Ping", client->GetID());
				//simply bounce the message back
				client->Send(msg);
				break;
			default:
				break;
			}
			return;
		}

	};

	class PixelGameServer : public Pyxis::Layer
	{
	public:
		PixelGameServer();
		virtual ~PixelGameServer() = default;

		virtual void OnAttach();
		virtual void OnDetatch();

		virtual void OnUpdate(Pyxis::Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Pyxis::Event& e) override;
		bool OnWindowResizeEvent(Pyxis::WindowResizeEvent& event);
	private:
		Pyxis::OrthographicCameraController m_OrthographicCameraController;

		PixelGameServerInterface m_ServerInterface;

	};
}

