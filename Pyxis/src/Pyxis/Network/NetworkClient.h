#pragma once

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include "NetworkMessage.h"
#include "NetworkThreadSafeQueue.h"

#ifndef PX_DEFAULT_PORT
	#define PX_DEFAULT_PORT 21218
#endif

namespace Pyxis
{
	namespace Network
	{
		class ClientInterface
		{
		public:

			/// <summary>
			/// Starts the client.
			/// 
			/// If there was a failure, the output reason would be written in m_ConnectionStatusMessage
			/// </summary>
			/// <returns>True if successful</returns>
			bool Connect(const SteamNetworkingIPAddr& serverAddr);

			/// <summary>
			/// Starts the client. Converts the string input to a GNS server address
			/// 
			/// If there was a failure, the output reason would be written in m_ConnectionStatusMessage
			/// </summary>
			/// <returns>True if successful</returns>
			bool Connect(const std::string& serverAddr);

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
			void SendMessageToServer(Message& message);

		public:
			inline uint64_t GetID() { return m_ID; };

			enum ConnectionStatus
			{
				Connecting, Connected, FailedToConnect, Disconnected, LostConnection
			};
			inline ConnectionStatus GetConnectionStatus() { return m_ConnectionStatus; };

			

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

			/// <summary>
			/// Called when the client successfully connects to a server
			/// </summary>
			virtual void OnConnectionSuccess();

			/// <summary>
			/// Called when an active connection is interrupted
			/// </summary>
			virtual void OnConnectionLost(const std::string& reasonText);

			/// <summary>
			/// Called when there is an issue connecting to a server
			/// 
			/// Could be a timeout, incorrect address, or any issue regaring calling Connect()
			/// </summary>
			virtual void OnConnectionFailure(const std::string& reasonText);
			
		protected:
			
			ConnectionStatus m_ConnectionStatus = ConnectionStatus::Disconnected;
			std::string m_ConnectionStatusMessage = "";
			uint64_t m_ID = 0;
			HSteamNetConnection m_hConnection;
			ISteamNetworkingSockets* m_pInterface;
			ISteamNetworkingUtils* m_pUtils;

		};
		//uint64_t m_ID = 0;

	}
}