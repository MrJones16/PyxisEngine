#include "pxpch.h"

#include "NetworkClient.h"

namespace Pyxis
{
	namespace Network
	{
		
		bool ClientInterface::Connect(const SteamNetworkingIPAddr& serverAddr)
		{

			if (m_ConnectionStatus == Connected) return true;
			// Select instance to use.  For now we'll always use the default.

			//initialze steam sockets
			SteamDatagramErrMsg errMsg;
			if (!GameNetworkingSockets_Init(nullptr, errMsg))
			{
				PX_CORE_ERROR("ClientInterface::Connect->GameNetworkingSockets_Init failed.  {0}", errMsg);
				m_ConnectionStatusMessage = "GNS_Init Failed.";
				m_ConnectionStatus = FailedToConnect;
				return false;
			}


			m_pInterface = SteamNetworkingSockets();


			//Start Connecting
			char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
			serverAddr.ToString(szAddr, sizeof(szAddr), true);
			PX_CORE_INFO("Connecting to server at {0}", szAddr);
			SteamNetworkingConfigValue_t opt;
			opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
			m_hConnection = m_pInterface->ConnectByIPAddress(serverAddr, 1, &opt);
			if (m_hConnection == k_HSteamNetConnection_Invalid)
			{
				PX_CORE_ERROR("Failed to create connection");
				m_ConnectionStatusMessage = "Failed to create connection";
				m_ConnectionStatus = ConnectionStatus::FailedToConnect;
				OnConnectionFailure(m_ConnectionStatusMessage);
				return false;
			}

			//Currently Connecting
			m_ConnectionStatusMessage = "Connecting";
			m_ConnectionStatus = Connecting;
			return true;
		}

		bool ClientInterface::Connect(const std::string& serverAddrString)
		{
			if (m_ConnectionStatus == Connected) return true;

			//initialze steam sockets
			SteamDatagramErrMsg errMsg;
			if (!GameNetworkingSockets_Init(nullptr, errMsg))
			{
				PX_CORE_ERROR("ClientInterface::Connect->GameNetworkingSockets_Init failed.  {0}", errMsg);
				m_ConnectionStatusMessage = "GNS_Init Failed.";
				m_ConnectionStatus = FailedToConnect;
				return false;
			}

			// Select instance to use.  For now we'll always use the default.
			m_pInterface = SteamNetworkingSockets();

			//Parse the string into a server address
			SteamNetworkingIPAddr serverAddr; serverAddr.Clear();
			if (serverAddr.IsIPv6AllZeros())
			{
				if (!serverAddr.ParseString(serverAddrString.c_str()))
				{
					m_ConnectionStatusMessage = "Invalid Server Address: " + serverAddrString;
					PX_CORE_ERROR(m_ConnectionStatusMessage);
					m_ConnectionStatus = ConnectionStatus::FailedToConnect;
					GameNetworkingSockets_Kill();
					OnConnectionFailure(m_ConnectionStatusMessage);
					return false;
				}
				if (serverAddr.m_port == 0)
					serverAddr.m_port = 21228;
			}

			//Start Connecting
			PX_CORE_INFO("Connecting to server at {0}", serverAddrString);
			SteamNetworkingConfigValue_t opt;
			opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
			m_hConnection = m_pInterface->ConnectByIPAddress(serverAddr, 1, &opt);
			if (m_hConnection == k_HSteamNetConnection_Invalid)
			{
				PX_CORE_ERROR("Failed to create connection");
				m_ConnectionStatusMessage = "Failed to create connection";
				m_ConnectionStatus = ConnectionStatus::FailedToConnect;
				GameNetworkingSockets_Kill();
				OnConnectionFailure(m_ConnectionStatusMessage);
				return false;
			}

			//Currently Connecting
			m_ConnectionStatusMessage = "Connecting";
			m_ConnectionStatus = Connecting;
			return true;
		}

		
		void ClientInterface::UpdateInterface()
		{
			if (m_ConnectionStatus == Connected || m_ConnectionStatus == Connecting)
			{
				PollIncomingMessages();
				PollConnectionStateChanges();
			}
		}

		
		void ClientInterface::Disconnect()
		{
			//Only disconnect if we are connected
			if (m_ConnectionStatus != ConnectionStatus::Connected)
			{
				m_ConnectionStatus = ConnectionStatus::Disconnected;
				return;
			}
			//Set our state to disconnected
			m_ConnectionStatus = ConnectionStatus::Disconnected;


			// Close the connection gracefully.
			// We use linger mode to ask for any remaining reliable data
			// to be flushed out.  But remember this is an application
			// protocol on UDP.  See ShutdownSteamDatagramConnectionSockets
			PX_CORE_TRACE("Disconnecting from chat server");
			m_pInterface->CloseConnection(m_hConnection, 0, "Goodbye", true);
		}

		void ClientInterface::OnConnectionSuccess()
		{

		}
		
		void ClientInterface::OnConnectionLost(const std::string& reasonText)
		{

		}

		void ClientInterface::OnConnectionFailure(const std::string& reasonText)
		{

		}

		void ClientInterface::SendStringToServer(const std::string& stringMessage)
		{
			m_pInterface->SendMessageToConnection(m_hConnection, stringMessage.c_str(), (uint32)stringMessage.length(), k_nSteamNetworkingSend_Reliable, nullptr);
			//ISteamNetworkingUtils::AllocateMessage
			
		}

		
		void ClientInterface::PollIncomingMessages()
		{
			while (true)
			{
				ISteamNetworkingMessage* pIncomingMsg = nullptr;
				int numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection, &pIncomingMsg, 1);
				if (numMsgs == 0)
					break;
				if (numMsgs < 0)
					PX_CORE_ERROR("Error checking for messages");
				
				// Just echo anything we get from the server
				//fwrite(pIncomingMsg->m_pData, 1, pIncomingMsg->m_cbSize, stdout);
				std::string message(reinterpret_cast<const char*>(pIncomingMsg->m_pData), pIncomingMsg->m_cbSize);
				PX_CORE_TRACE("{0}", message);
				fputc('\n', stdout);

				// We don't need this anymore.
				pIncomingMsg->Release();
			}
		}

		
		void ClientInterface::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
		{
			assert(pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid);

			// What's the state of the connection?
			switch (pInfo->m_info.m_eState)
			{
			case k_ESteamNetworkingConnectionState_None:
				// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
				//m_ConnectionStatus = ConnectionStatus::Disconnected;
				//m_ConnectionStatusMessage = "We closed the connection";
				break;

			case k_ESteamNetworkingConnectionState_ClosedByPeer:
			case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
			{
				//quit?

				// Print an appropriate message

				//We were connecting when we encountered a problem
				if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting)
				{
					// Note: we could distinguish between a timeout, a rejected connection,
					// or some other transport problem.
					m_ConnectionStatus = ConnectionStatus::FailedToConnect;
					m_ConnectionStatusMessage = "We sought the remote host, yet our efforts were met with defeat.  (" + (std::string)(pInfo->m_info.m_szEndDebug) + ")";
					PX_CORE_TRACE("{0}", m_ConnectionStatusMessage);
					OnConnectionFailure(m_ConnectionStatusMessage);
				}
				else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
				{
					m_ConnectionStatus = ConnectionStatus::LostConnection;
					m_ConnectionStatusMessage = "Alas, troubles beset us; we have lost contact with the host. (" + (std::string)(pInfo->m_info.m_szEndDebug) + ")";
					PX_CORE_TRACE(m_ConnectionStatusMessage);
					OnConnectionLost(m_ConnectionStatusMessage);
				}
				else
				{
					// NOTE: We could check the reason code for a normal disconnection
					//NOTE?? is this case even possible to reach? i don't think so
					m_ConnectionStatus = ConnectionStatus::Disconnected;
					m_ConnectionStatusMessage = "The host hath bidden us farewell: (" + (std::string)(pInfo->m_info.m_szEndDebug) + ")";
					PX_CORE_TRACE(m_ConnectionStatusMessage);
				}

				// Clean up the connection.  This is important!
				// The connection is "closed" in the network sense, but
				// it has not been destroyed.  We must close it on our end, too
				// to finish up.  The reason information do not matter in this case,
				// and we cannot linger because it's already closed on the other end,
				// so we just pass 0's.
				m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
				m_hConnection = k_HSteamNetConnection_Invalid;

				GameNetworkingSockets_Kill();

				break;
			}

			case k_ESteamNetworkingConnectionState_Connecting:
				// We will get this callback when we start connecting.
				// We can ignore this.
				break;

			case k_ESteamNetworkingConnectionState_Connected:
				m_ConnectionStatus = ConnectionStatus::Connected;
				m_ConnectionStatusMessage = "Connected to server";
				OnConnectionSuccess();
				PX_CORE_INFO("Connected to server");
				break;

			default:
				// Silences -Wswitch
				break;
			}
		}

		
		void ClientInterface::SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
		{
			s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
		}

		
		void ClientInterface::PollConnectionStateChanges()
		{
			s_pCallbackInstance = this;
			m_pInterface->RunCallbacks();
		}
	}
}