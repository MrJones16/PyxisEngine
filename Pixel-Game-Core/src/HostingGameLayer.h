#pragma once
#include "GameLayer.h"
#include <Pyxis/Network/NetworkServer.h>


namespace Pyxis
{

	class HostingGameLayer : public GameLayer, public Network::ServerInterface, public std::enable_shared_from_this<HostingGameLayer>
	{
	public:
		HostingGameLayer(std::string debugName = "Hosting Game Layer");
		virtual ~HostingGameLayer();

		//////////////////////////////////////
		/// Layer Functions
		//////////////////////////////////////
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;


		//////////////////////////////////////
		/// Game / Multiplayer Functions
		//////////////////////////////////////
		void StartP2P(int virtualPort = 0);
		void StartIP(uint16_t port = 21218);
		void HandleMessages();

	public:

	private:

		//////////////////////////////////////
		/// Multiplayer Hosting Variables
		//////////////////////////////////////

		MergedTickClosure m_CurrentMergedTickClosure;

		//map of clients world download progress / messages
		std::unordered_map<HSteamNetConnection, std::vector<Network::Message>> m_DownloadingClients;

		//a deque of the compressed mtc messages! allows for a smaller storage of the tick closures
		//and so they can be requested by a client if one goes missing
		std::deque<std::string> m_TickRequestStorage;

	};
}
