#include "pxpch.h"

#include "NetworkClient.h"

namespace Pyxis
{
	namespace Network
	{
		ClientInterface::ClientInterface()
		{
			m_SteamNetworkingSockets = SteamNetworkingSockets();
			m_SteamNetworkingUtils = SteamNetworkingUtils();
			m_hConnection = k_HSteamNetConnection_Invalid;
		}

		ClientInterface::~ClientInterface()
		{
			//SteamAPI_Shutdown();
		}

		bool ClientInterface::ConnectIP(const SteamNetworkingIPAddr& serverAddr)
		{

			if (m_ConnectionStatus == Connected) return true;
			// Select instance to use.  For now we'll always use the default.

			////initialze steam sockets
			//SteamDatagramErrMsg errMsg;
			//if (!GameNetworkingSockets_Init(nullptr, errMsg))
			//{
			//	PX_CORE_ERROR("ClientInterface::Connect->GameNetworkingSockets_Init failed.  {0}", errMsg);
			//	m_ConnectionStatusMessage = "GNS_Init Failed.";
			//	m_ConnectionStatus = FailedToConnect;
			//	return false;
			//}


			m_SteamNetworkingSockets = SteamNetworkingSockets();
			//Start Connecting
			char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
			serverAddr.ToString(szAddr, sizeof(szAddr), true);
			PX_CORE_INFO("Connecting to server at {0}", szAddr);
			SteamNetworkingConfigValue_t opt;
			opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
			m_hConnection = m_SteamNetworkingSockets->ConnectByIPAddress(serverAddr, 1, &opt);
			if (m_hConnection == k_HSteamNetConnection_Invalid)
			{
				PX_CORE_ERROR("Failed to create connection");
				m_ConnectionStatusMessage = "Failed to create connection";
				m_ConnectionStatus = ConnectionStatus::FailedToConnect;
				OnConnectionFailure(m_ConnectionStatusMessage);
				//GameNetworkingSockets_Kill();
				return false;
			}

			//Currently Connecting
			m_ConnectionStatusMessage = "Connecting";
			m_ConnectionStatus = Connecting;
			return true;
		}

		bool ClientInterface::ConnectIP(const std::string& serverAddrString)
		{
			if (m_ConnectionStatus == Connected) return true;		

			// Select instance to use.  For now we'll always use the default.
			
			//Parse the string into a server address
			SteamNetworkingIPAddr serverAddr; serverAddr.Clear();
			if (serverAddr.IsIPv6AllZeros())
			{
				if (!serverAddr.ParseString(serverAddrString.c_str()))
				{
					m_ConnectionStatusMessage = "Invalid Server Address: " + serverAddrString;
					PX_CORE_ERROR(m_ConnectionStatusMessage);
					m_ConnectionStatus = ConnectionStatus::FailedToConnect;
					OnConnectionFailure(m_ConnectionStatusMessage);
					//GameNetworkingSockets_Kill();
					return false;
				}
				if (serverAddr.m_port == 0)
					serverAddr.m_port = 21228;
			}

			//Start Connecting
			PX_CORE_INFO("Connecting to server at {0}", serverAddrString);
			SteamNetworkingConfigValue_t opt;
			opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
			m_hConnection = m_SteamNetworkingSockets->ConnectByIPAddress(serverAddr, 1, &opt);
			if (m_hConnection == k_HSteamNetConnection_Invalid)
			{
				PX_CORE_ERROR("Failed to create connection");
				m_ConnectionStatusMessage = "Failed to create connection";
				m_ConnectionStatus = ConnectionStatus::FailedToConnect;
				//GameNetworkingSockets_Kill();
				OnConnectionFailure(m_ConnectionStatusMessage);
				return false;
			}

			//Currently Connecting
			m_ConnectionStatusMessage = "Connecting";
			m_ConnectionStatus = Connecting;
			return true;
		}

		bool ClientInterface::ConnectP2P(SteamNetworkingIdentity& identity, int virtualPort)
		{
			if (m_ConnectionStatus == Connected) return true;

			// Select instance to use.  For now we'll always use the default.
			m_SteamNetworkingSockets = SteamNetworkingSockets();
			m_SteamNetworkingUtils = SteamNetworkingUtils();

			//configureoptions
			SteamNetworkingConfigValue_t opt;
			opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);

			//Start Connecting
			m_hConnection = m_SteamNetworkingSockets->ConnectP2P(identity, virtualPort, 1, &opt);
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

		
		void ClientInterface::UpdateInterface()
		{
			if (m_ConnectionStatus == Connected || m_ConnectionStatus == Connecting)
			{
				//PollIncomingMessages();
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
			PX_CORE_TRACE("Disconnecting from server");
			m_SteamNetworkingSockets->CloseConnection(m_hConnection, 0, "Goodbye", false);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			
			//GameNetworkingSockets_Kill();
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
			Network::Message msg;
			msg.header.id = 0;
			msg.PushData(stringMessage.c_str(), stringMessage.size());
			SendMessageToServer(msg);
		}

		void ClientInterface::SendMessageToServer(Message& message)
		{
			/*message << message.header.id;
			m_SteamNetworkingSockets->SendMessageToConnection(m_hConnection, message.body.data(), (uint32)message.size(), k_nSteamNetworkingSend_Reliable, nullptr);*/

			message << message.header.id;

			std::string inString(message.body.begin(), message.body.end());
			std::string compressedString;
			snappy::Compress(inString.data(), inString.size(), &compressedString);


			Network::Message compressedMsg;
			compressedMsg.PushData(compressedString.data(), compressedString.size());
			m_SteamNetworkingSockets->SendMessageToConnection(m_hConnection, compressedMsg.body.data(), (uint32)compressedMsg.size(), k_nSteamNetworkingSend_Reliable, nullptr);
		}

		void ClientInterface::SendCompressedMessageToServer(Message& message)
		{
			message << message.header.id;
			
			std::string inString(message.body.begin(), message.body.end());
			std::string compressedString;
			snappy::Compress(inString.data(), inString.size(), &compressedString);


			Network::Message compressedMsg;
			compressedMsg.PushData(compressedString.data(), compressedString.size());
			m_SteamNetworkingSockets->SendMessageToConnection(m_hConnection, compressedMsg.body.data(), (uint32)compressedMsg.size(), k_nSteamNetworkingSend_Reliable, nullptr);
		}

		void ClientInterface::SendUnreliableMessageToServer(Message& message)
		{
			message << message.header.id;
			m_SteamNetworkingSockets->SendMessageToConnection(m_hConnection, message.body.data(), (uint32)message.size(), k_nSteamNetworkingSend_Unreliable, nullptr);
		}

		bool ClientInterface::PollMessage(Ref<Message>& MessageOut)
		{
			
			/*ISteamNetworkingMessage* pIncomingMsg = nullptr;
			int numMsgs = m_SteamNetworkingSockets->ReceiveMessagesOnConnection(m_hConnection, &pIncomingMsg, 1);
			if (numMsgs == 0) return false;
			if (numMsgs < 0)
			{
				PX_CORE_ERROR("Error checking for messages");
				return false;
			}
			assert(numMsgs == 1 && pIncomingMsg);

			MessageOut = CreateRef<Message>(pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);
			*MessageOut >> MessageOut->header.id;
			MessageOut->clientHConnection = k_HSteamNetConnection_Invalid;
			pIncomingMsg->Release();
			return true;*/
			ISteamNetworkingMessage* pIncomingMsg = nullptr;
			int numMsgs = m_SteamNetworkingSockets->ReceiveMessagesOnConnection(m_hConnection, &pIncomingMsg, 1);
			if (numMsgs == 0) return false;
			if (numMsgs < 0)
			{
				PX_CORE_ERROR("Error checking for messages");
				return false;
			}
			assert(numMsgs == 1 && pIncomingMsg);

			// '\0'-terminate it to make it easier to parse
			std::string stringCompressed;
			stringCompressed.assign((const char*)pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);


			std::string uncompressed;
			if (!snappy::Uncompress(stringCompressed.data(), stringCompressed.size(), &uncompressed))
			{
				//message failed to uncompress!
				PX_CORE_ERROR("Message failed to uncompress!");
			}
			MessageOut = CreateRef<Message>(uncompressed.data(), uncompressed.size());
			*MessageOut >> MessageOut->header.id;
			MessageOut->clientHConnection = pIncomingMsg->m_conn;
			pIncomingMsg->Release();
			return true;
	
		}

		
		void ClientInterface::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
		{
			if (!(pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid))
			{
				//the callback was for a previos
				return;
			}
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
					OnConnectionLost(m_ConnectionStatusMessage);
				}

				// Clean up the connection.  This is important!
				// The connection is "closed" in the network sense, but
				// it has not been destroyed.  We must close it on our end, too
				// to finish up.  The reason information do not matter in this case,
				// and we cannot linger because it's already closed on the other end,
				// so we just pass 0's.
				m_SteamNetworkingSockets->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
				m_hConnection = k_HSteamNetConnection_Invalid;

				//GameNetworkingSockets_Kill();

				break;
			}

			case k_ESteamNetworkingConnectionState_Connecting:
				// We will get this callback when we start connecting.
				// We can ignore this.
				break;

			case k_ESteamNetworkingConnectionState_Connected:
				m_ConnectionStatus = ConnectionStatus::Connected;
				m_ConnectionStatusMessage = "Connected to server";
				PX_CORE_INFO("Connected to server");
				OnConnectionSuccess();
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
			m_SteamNetworkingSockets->RunCallbacks();
		}
	}
}