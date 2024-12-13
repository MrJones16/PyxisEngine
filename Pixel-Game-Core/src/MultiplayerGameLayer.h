#pragma once

#include "GameLayer.h"
#include <Pyxis/Network/NetworkClient.h>

namespace Pyxis
{
	class MultiplayerGameLayer : public GameLayer, public Network::ClientInterface, public std::enable_shared_from_this<MultiplayerGameLayer>
	{
	public:
		MultiplayerGameLayer();
		virtual ~MultiplayerGameLayer();

		//////////////////////////////////////
		/// Layer Functions
		//////////////////////////////////////
		//virtual void OnAttach() override;
		//virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		//virtual void OnEvent(Event& e) override;


		//////////////////////////////////////
		/// Multiplayer Functions
		//////////////////////////////////////
		void ConnectIP(const std::string& AddressAndPort);
		void ConnectP2P();
		void OnConnectionSuccess() override;
		void OnConnectionLost(const std::string& reasonText) override;
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
