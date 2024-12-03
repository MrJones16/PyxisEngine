#pragma once

//#include "NetworkThreadSafeQueue.h"
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include "NetworkMessage.h"
#include <Pyxis/Core/Log.h>


namespace Pyxis
{
	namespace Network
	{

		class ServerInterface
		{
		public:
			ServerInterface();

			virtual ~ServerInterface();

			/// <summary>
			/// Starts the server. It initializes the SteamNetworkingSockets, 
			/// and listens to the port set at instantiation. 
			/// </summary>
			/// <returns>True if successful</returns>
			bool Start(uint16_t port = 0);

			/// <summary>
			/// The main update loop for the server. It polls incoming messages, connection changes, and waits
			/// </summary>
			void UpdateInterface();
			void Stop();


			void SendStringToClient(HSteamNetConnection conn, const std::string& str);
			void SendStringToAllClients(const std::string& str, HSteamNetConnection except = k_HSteamNetConnection_Invalid);
			void SendMessageToClient(HSteamNetConnection conn, Message& message);
			void SendMessageToAllClients(Message& message, HSteamNetConnection except = k_HSteamNetConnection_Invalid);
			void SendUnreliableMessageToClient(HSteamNetConnection conn, Message& message);
			void SendUnreliableMessageToAllClients(Message& message, HSteamNetConnection except = k_HSteamNetConnection_Invalid);
			bool PollMessage(Ref<Message>& MessageOut);
			void PollIncomingMessages();
			void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);
			static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);
			void PollConnectionStateChanges();
			

		protected:
			//called when a client connects to the server
			inline virtual bool OnClientConnect(HSteamNetConnection client);
			//called when a client appears to have disconnected
			inline virtual void OnClientDisconnect(HSteamNetConnection client);
			//called when a message arrives
			//virtual void OnMessage(HSteamNetConnection client, Message<T>& msg);

		protected:

			static ServerInterface* s_pCallbackInstance;

			SteamNetworkingIPAddr m_hLocalAddress;
			HSteamListenSocket m_hListenSock;
			HSteamNetPollGroup m_hPollGroup;
			ISteamNetworkingSockets* m_pInterface;
			ISteamNetworkingUtils* m_pUtils;

			std::map<HSteamNetConnection, uint64_t> m_mapClients;

			//thread safe queue for incoming message packets
			//ThreadSafeQueue<OwnedMessage<T>> m_QueueMessagesIn;

			// clients will be identified in the "wider system" via an ID
			uint64_t m_IDCounter = 10000;

			

		};

	}
}