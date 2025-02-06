#pragma once
#include "GameNode.h"
#include <Pyxis/Network/NetworkServer.h>
#include <steam/isteamfriends.h>


namespace Pyxis
{

	class HostedGameNode : public GameNode, public Network::ServerInterface
	{
	public:

		static const int MaxTickStorage = 500;


		HostedGameNode(std::string name = "Hosted Game Node") : GameNode(name) 
		{
			for (int x = -1; x <= 1; x++)
			{
				for (int y = -1; y <= 1; y++)
				{
					m_World.AddChunk({ x,y });
					m_World.GenerateChunk(m_World.GetChunk({ x,y }));
				}
			}
		}
		virtual ~HostedGameNode() = default;


		//////////////////////////////////////
		/// Node Functions
		//////////////////////////////////////
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnFixedUpdate() override;
		virtual void OnImGuiRender() override;


		//////////////////////////////////////
		/// Game / Multiplayer Functions
		//////////////////////////////////////
		void StartP2P(int virtualPort = 0);
		void StartIP(uint16_t port = PX_DEFAULT_PORT);
		void HandleMessages();

		//////////////////////////////////////
		/// Server Function Overrides
		//////////////////////////////////////
		void OnClientDisconnect(HSteamNetConnection& client) override;


		//Return to menu override
		virtual void ReturnToMenu() override;

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
