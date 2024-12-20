#pragma once

//#include "NetworkThreadSafeQueue.h"
#include <steam/isteamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include "NetworkMessage.h"
#include <Pyxis/Core/Log.h>

#ifndef PX_DEFAULT_PORT
#define PX_DEFAULT_PORT 21218
#endif

#ifndef PX_DEFAULT_VIRTUAL_PORT
#define PX_DEFAULT_VIRTUAL_PORT 0
#endif


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
			/// Starts the server, and listens to the port set at instantiation.
			/// NOT P2P
			/// </summary>
			/// <returns>True if successful</returns>
			bool HostIP(uint16_t port = PX_DEFAULT_PORT);


			/// <summary>
			/// Starts a P2P Listening Server
			/// </summary>
			/// <param name="port">The virtual port for the P2P server</param>
			/// <returns></returns>
			bool HostP2P(int virtualPort = PX_DEFAULT_VIRTUAL_PORT);


			/// <summary>
			/// The main update loop for the server. It polls incoming messages, connection changes, and waits
			/// </summary>
			void UpdateInterface();
			void Stop();


			void SendStringToClient(HSteamNetConnection conn, const std::string& str);
			void SendStringToAllClients(const std::string& str, HSteamNetConnection except = k_HSteamNetConnection_Invalid);
			//Sending messages is volatile! it adds the ID to the message when sending
			//k_nSteamNetworkingSend_
			void SendMessageToClient(HSteamNetConnection conn, Message& message, int nSendFlags = k_nSteamNetworkingSend_Reliable);
			//k_nSteamNetworkingSend_
			void SendMessageToAllClients(Message& message, HSteamNetConnection except = k_HSteamNetConnection_Invalid, int nSendFlags = k_nSteamNetworkingSend_Reliable);
			//k_nSteamNetworkingSend_
			void SendCompressedStringToClient(HSteamNetConnection conn, std::string& compressedStr, int nSendFlags = k_nSteamNetworkingSend_Reliable);
			//k_nSteamNetworkingSend_
			void SendCompressedStringToAllClients(std::string& compressedStr, HSteamNetConnection except = k_HSteamNetConnection_Invalid, int nSendFlags = k_nSteamNetworkingSend_Reliable);
			bool PollMessage(Ref<Message>& MessageOut);
			
			void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);
			static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);
			void PollConnectionStateChanges();

			
			void DisconnectClient(HSteamNetConnection client, std::string Reason = "You were disconnected by the host");
			

		protected:
			//called when a client connects to the server
			inline virtual bool OnClientConnect(HSteamNetConnection& client);
			//called when a client appears to have disconnected
			inline virtual void OnClientDisconnect(HSteamNetConnection& client);
			//called when a message arrives
			//virtual void OnMessage(HSteamNetConnection client, Message<T>& msg);

		protected:

			static ServerInterface* s_pCallbackInstance;

			std::unordered_set<HSteamNetConnection> m_ClientsSet;

			SteamNetworkingIPAddr m_hLocalAddress;
			HSteamListenSocket m_ListeningSocket;
			HSteamNetPollGroup m_PollGroup;
			ISteamNetworkingSockets* m_SteamNetworkingSockets;
			ISteamNetworkingUtils* m_SteamNetworkingUtils;

			//thread safe queue for incoming message packets
			//ThreadSafeQueue<OwnedMessage<T>> m_QueueMessagesIn;

			

		};

	}
}