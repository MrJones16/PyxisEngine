#include "pxpch.h"

#include "NetworkClient.h"

namespace Pyxis
{
	namespace Network
	{
		
		bool ClientInterface::Connect(const SteamNetworkingIPAddr& serverAddr)
		{
			//steamfix, maybe move init to a higher level?
			SteamDatagramErrMsg errMsg;
			if (!GameNetworkingSockets_Init(nullptr, errMsg))
			{
				PX_CORE_ERROR("SteamServer::Start->GameNetworkingSockets_Init failed.  {0}", errMsg);
				return false;
			}
			// Select instance to use.  For now we'll always use the default.
			m_pInterface = SteamNetworkingSockets();

			// Start connecting
			char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
			serverAddr.ToString(szAddr, sizeof(szAddr), true);
			PX_CORE_INFO("Connecting to chat server at {0}", szAddr);
			SteamNetworkingConfigValue_t opt;
			opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
			m_hConnection = m_pInterface->ConnectByIPAddress(serverAddr, 1, &opt);
			if (m_hConnection == k_HSteamNetConnection_Invalid)
			{
				PX_CORE_ERROR("Failed to create connection");
				return false;
			}
			return true;
		}

		
		void ClientInterface::UpdateInterface()
		{
			PollIncomingMessages();
			PollConnectionStateChanges();
		}

		
		void ClientInterface::Disconnect()
		{
			PX_CORE_TRACE("Disconnecting from chat server");

			// Close the connection gracefully.
			// We use linger mode to ask for any remaining reliable data
			// to be flushed out.  But remember this is an application
			// protocol on UDP.  See ShutdownSteamDatagramConnectionSockets
			m_pInterface->CloseConnection(m_hConnection, 0, "Goodbye", true);
		}

		
		void ClientInterface::SendStringToServer(const std::string& stringMessage)
		{
			m_pInterface->SendMessageToConnection(m_hConnection, stringMessage.c_str(), (uint32)stringMessage.length(), k_nSteamNetworkingSend_Reliable, nullptr);
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
				break;

			case k_ESteamNetworkingConnectionState_ClosedByPeer:
			case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
			{
				//quit

				// Print an appropriate message
				if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting)
				{
					// Note: we could distinguish between a timeout, a rejected connection,
					// or some other transport problem.
					PX_CORE_TRACE("We sought the remote host, yet our efforts were met with defeat.  ({0})", pInfo->m_info.m_szEndDebug);
				}
				else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
				{
					PX_CORE_TRACE("Alas, troubles beset us; we have lost contact with the host.  ({0})", pInfo->m_info.m_szEndDebug);
				}
				else
				{
					// NOTE: We could check the reason code for a normal disconnection
					PX_CORE_TRACE("The host hath bidden us farewell.  ({0})", pInfo->m_info.m_szEndDebug);
				}

				// Clean up the connection.  This is important!
				// The connection is "closed" in the network sense, but
				// it has not been destroyed.  We must close it on our end, too
				// to finish up.  The reason information do not matter in this case,
				// and we cannot linger because it's already closed on the other end,
				// so we just pass 0's.
				m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
				m_hConnection = k_HSteamNetConnection_Invalid;
				break;
			}

			case k_ESteamNetworkingConnectionState_Connecting:
				// We will get this callback when we start connecting.
				// We can ignore this.
				break;

			case k_ESteamNetworkingConnectionState_Connected:
				PX_CORE_INFO("Connected to server OK");
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