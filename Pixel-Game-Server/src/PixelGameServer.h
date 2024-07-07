#pragma once

#include "Pyxis.h"
#include <Pyxis/Network/Network.h>
#include "Pyxis/Core/OrthographicCameraController.h"
#include "common/PixelClientInterface.h"
#include "common/World.h"

namespace Pyxis
{

	class PixelGameServer : public Pyxis::Layer, public Network::ServerInterface<GameMessage>
	{
	public:
		PixelGameServer(uint16_t port);
		virtual ~PixelGameServer() = default;

		virtual void OnAttach();
		virtual void OnDetatch();

		virtual void OnUpdate(Pyxis::Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Pyxis::Event& e) override;
		bool OnWindowResizeEvent(Pyxis::WindowResizeEvent& event);

	protected:
		bool OnClientConnect(std::shared_ptr<Network::Connection<GameMessage>> client) override;
		void OnClientDisconnect(std::shared_ptr<Network::Connection<GameMessage>> client) override;
		void OnMessage(std::shared_ptr<Network::Connection<GameMessage>> client, Network::Message< GameMessage>& msg) override;
		void OnClientValidated(std::shared_ptr<Network::Connection<GameMessage>> client) override;
		void HandleTickClosure(MergedTickClosure& tc);

	private:
		Pyxis::OrthographicCameraController m_OrthographicCameraController;

		/// <summary>
		/// The authoritative world. 
		/// </summary>
		World m_World;
		/// <summary>
		/// the game tick the server is currently at
		/// </summary>
		uint64_t m_InputTick = 0;
		//PixelServerInterface m_ServerInterface;

		int m_PlayerCount = 0;
		std::unordered_set<uint64_t> m_ClientsNeededForTick;
		std::deque<MergedTickClosure> m_MTCDeque;
		//std::unordered_map<uint64_t, MergedTickClosure> m_TickClosureMap;

	};
}

