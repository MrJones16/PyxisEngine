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

		template<typename T>
		class ServerInterface
		{
		public:
			ServerInterface(uint16_t port);

			virtual ~ServerInterface();

			/// <summary>
			/// Starts the server. It initializes the SteamNetworkingSockets, 
			/// and listens to the port set at instantiation. 
			/// </summary>
			/// <returns>True if successful</returns>
			bool Start();

			/// <summary>
			/// The main update loop for the server. It polls incoming messages, connection changes, and waits
			/// </summary>
			void Update();
			void Stop();

			// send a message to a specific client
			void SendStringToClient(HSteamNetConnection conn, const char* str);
			void SendStringToAllClients(const char* str, HSteamNetConnection except = k_HSteamNetConnection_Invalid);
			void PollIncomingMessages();
			void SetClientNick(HSteamNetConnection hConn, const char* nick);
			void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);
			static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);
			void PollConnectionStateChanges();
			//void MessageClient(std::shared_ptr<Connection<T>> client, const Message<T>& msg);
		protected:
			//called when a client connects to the server
			inline virtual bool OnClientConnect(std::shared_ptr<Connection<T>> client);
			//called when a client appears to have disconnected
			inline virtual void OnClientDisconnect(std::shared_ptr<Connection<T>> client);
			//called when a message arrives
			virtual void OnMessage(std::shared_ptr<Connection<T>> client, Message<T>& msg);

		protected:

			static ServerInterface* s_pCallbackInstance;

			HSteamListenSocket m_hListenSock;
			HSteamNetPollGroup m_hPollGroup;
			ISteamNetworkingSockets* m_pInterface;

			struct Client_t
			{
				uint64_t m_ID;
				std::string m_sNick;
			};

			std::map<HSteamNetConnection, Client_t > m_mapClients;

			//thread safe queue for incoming message packets
			ThreadSafeQueue<OwnedMessage<T>> m_QueueMessagesIn;

			// clients will be identified in the "wider system" via an ID
			uint64_t m_IDCounter = 10000;

			

		};

	}
}