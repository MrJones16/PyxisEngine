#pragma once

#include "Pyxis.h"
#include <Pyxis/Network/Network.h>
#include "Pyxis/Core/OrthographicCameraController.h"
#include "PixelClientInterface.h"
#include "World.h"

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
		

		int m_PlayerCount = 0;
		std::unordered_set<uint64_t> m_ClientsNeededForTick;
		std::deque<MergedTickClosure> m_MTCDeque;

		std::deque<MergedTickClosure> m_TickRequestStorage;
		
		//delay making the server sleep, so the player count is updated and drawn at start.
		int m_SleepDelay = 0;
		int m_SleepDelayMax = 10000;

	};
}

