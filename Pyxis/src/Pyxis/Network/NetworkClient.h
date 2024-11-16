#pragma once

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include "NetworkMessage.h"
#include "NetworkThreadSafeQueue.h"

namespace Pyxis
{
	namespace Network
	{
		class ClientInterface
		{
		public:
			/// <summary>
			/// Starts the client.
			/// </summary>
			/// <returns>True if successful</returns>
			bool Connect(const SteamNetworkingIPAddr& serverAddr);

			//STEAMFIX TODO
			//connect function!

			/// <summary>
			/// Main update loop for the client interface.
			/// Polls messages and connection changes.
			/// </summary>
			void UpdateInterface();

			/// <summary>
			/// Disconnect from the server.
			/// </summary>
			void Disconnect();

			void SendStringToServer(const std::string& stringMessage);

		public:
			inline uint64_t GetID() { return m_ID; };

		protected:

			inline static ClientInterface* s_pCallbackInstance = nullptr;
			
			/// <summary>
			/// Writes incoming messages to stdout
			/// </summary>
			void PollIncomingMessages();
			/// <summary>
			/// Function that will delegate connection changes
			/// </summary>
			/// <param name="pInfo"></param>
			void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);

			/// <summary>
			/// callback for 
			/// </summary>
			/// <param name="pInfo"></param>
			static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);

			/// <summary>
			/// Sets the static "Connection Change Callback" to this instance,
			/// and recieves the callback
			/// </summary>
			void PollConnectionStateChanges();
			
			uint64_t m_ID = 0;
			HSteamNetConnection m_hConnection;
			ISteamNetworkingSockets* m_pInterface;

		};
		//uint64_t m_ID = 0;

	}
}