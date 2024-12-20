#pragma once

#include "HostingGameLayer.h"

namespace Pyxis
{

	class PixelGameServer : public HostingGameLayer
	{
	public:
		PixelGameServer(uint16_t port = PX_DEFAULT_PORT);
		virtual ~PixelGameServer();

		virtual void OnAttach();
		virtual void OnDetatch();
		virtual void OnUpdate(Pyxis::Timestep ts) override;

		//virtual void OnUpdateOld(Pyxis::Timestep ts);
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Pyxis::Event& e) override;
		bool OnWindowResizeEvent(Pyxis::WindowResizeEvent& event);

	protected:
		void DisconnectClient(HSteamNetConnection client, std::string Reason);

		//bool OnClientConnect(std::shared_ptr<Network::Connection<GameMessage>> client) override;
		//void OnClientDisconnect(HSteamNetConnection client) override;
		//void OnMessage(std::shared_ptr<Network::Connection<GameMessage>> client, Network::Message< GameMessage>& msg) override;
		//void OnClientValidated(std::shared_ptr<Network::Connection<GameMessage>> client) override;
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
		
		std::unordered_map < HSteamNetConnection, std::vector<Network::Message>> m_DownloadingClients;
		///The current tick closure for the server.
		MergedTickClosure m_CurrentMergedTickClosure;

		//Time keeping for tick rate and sending of merged tick closures
		std::chrono::time_point<std::chrono::high_resolution_clock> m_UpdateTime = std::chrono::high_resolution_clock::now();
		float m_TickRate = 30.0f;

		//a deque of the compressed mtc messages! allows for a smaller storage of the tick closures
		//and so they can be requested by a client if one goes missing
		std::deque<std::string> m_TickRequestStorage;

		

	};
}

