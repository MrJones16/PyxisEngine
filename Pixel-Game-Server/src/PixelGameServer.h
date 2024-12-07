#pragma once

#include "Pyxis.h"
#include "Pyxis/Network/NetworkServer.h"
#include "PixelNetworking.h"
#include "Pyxis/Core/OrthographicCameraController.h"
#include "World.h"

namespace Pyxis
{

	class PixelGameServer : public Pyxis::Layer, public Network::ServerInterface
	{
	public:
		PixelGameServer(uint16_t port);
		virtual ~PixelGameServer();

		virtual void OnAttach();
		virtual void OnDetatch();
		virtual void OnUpdate(Pyxis::Timestep ts) override;

		//virtual void OnUpdateOld(Pyxis::Timestep ts);
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Pyxis::Event& e) override;
		bool OnWindowResizeEvent(Pyxis::WindowResizeEvent& event);

	protected:
		//bool OnClientConnect(std::shared_ptr<Network::Connection<GameMessage>> client) override;
		void OnClientDisconnect(HSteamNetConnection client) override;
		//void OnMessage(std::shared_ptr<Network::Connection<GameMessage>> client, Network::Message< GameMessage>& msg) override;
		//void OnClientValidated(std::shared_ptr<Network::Connection<GameMessage>> client) override;
		void HandleTickClosure(MergedTickClosure& tc);
		void HandleMessages();

	private:
		Pyxis::OrthographicCameraController m_OrthographicCameraController;

		//STEAMTESTING
		uint16_t m_SteamPort;

		/// <summary>
		/// The authoritative world. 
		/// </summary>
		World m_World;

		/// <summary>
		/// the game tick the server is currently at
		/// </summary>
		uint64_t m_InputTick = 0;
		

		/// <summary>
		/// Map of clients and their client data to keep track of
		/// </summary>
		std::unordered_map<HSteamNetConnection, ClientData> m_ClientDataMap;
		//specifically for the use case of server side merged tick closures. uses the ID given to 
		std::unordered_map<uint64_t, HSteamNetConnection> m_ClientIDToHandleMap;

		///The current tick closure for the server.
		MergedTickClosure m_CurrentMergedTickClosure;

		//Time keeping for tick rate and sending of merged tick closures
		std::chrono::time_point<std::chrono::high_resolution_clock> m_UpdateTime = std::chrono::high_resolution_clock::now();
		float m_TickRate = 30.0f;




		//std::unordered_set<uint64_t> m_ClientsNeededForTick;
		//std::deque<MergedTickClosure> m_MTCDeque;

		//std::deque<MergedTickClosure> m_TickRequestStorage;
		//
		////delay making the server sleep, so the player count is updated and drawn at start.
		//int m_SleepDelay = 0;
		//int m_SleepDelayMax = 10000;

		

	};
}

