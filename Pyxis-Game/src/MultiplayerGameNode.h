#pragma once

#include "GameNode.h"
#include <Pyxis/Network/NetworkClient.h>
#include <steam/isteamfriends.h>

namespace Pyxis
{
	class MultiplayerGameNode : public GameNode, public Network::ClientInterface
	{
	public:
		MultiplayerGameNode();
		virtual ~MultiplayerGameNode() = default;
		
		virtual void OnUpdate(Timestep ts) override;

		virtual void OnFixedUpdate() override;

		virtual void OnRender() override;

		Ref<UI::ScreenSpace> m_LSScreenSpace;
		Ref<UI::Canvas> m_LSCanvas;
		Ref<UI::Text> m_LSText;
		Ref<UI::Button> m_LSButton;
		virtual void ReturnToMenu() override;

		//////////////////////////////////////
		/// Multiplayer Functions
		//////////////////////////////////////
		void Connect(const std::string& AddressAndPort);
		void Connect(SteamNetworkingIdentity& identity, int virtualPort = 0);
		void OnConnectionSuccess() override;
		void OnConnectionLost(const std::string& reasonText) override;
		void OnConnectionFailure(const std::string& reasonText) override;
		void HandleMessages();


	public:

	private:


		//////////////////////////////////////
		/// Multiplayer Variables
		//////////////////////////////////////
		enum MultiplayerState
		{
			Disconnected,
			Connecting,
			GatheringPlayerData,
			DownloadingWorld,
			CatchingUp,
			Connected,
		};
		MultiplayerState m_MultiplayerState = Disconnected;


		//////////////////////////////////////
		/// Client Variables
		//////////////////////////////////////

		//used for limiting requests for missing ticks
		uint64_t m_LastRequestedTick = -1;

		//buffer to hold incoming MTC's, since we could get one out of order
		std::deque<MergedTickClosure> m_MTCBuffer;

		//simulation tick to reset world at, for when other players join the same server as you.
		uint64_t m_TickToResetBox2D = -1;

		//for keeping track of world download progress
		uint64_t m_DownloadCount = 0;
		uint64_t m_DownloadTotal = 0;


	};
}
