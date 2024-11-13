#pragma once

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <Pyxis/Core/Log.h>



namespace Pyxis
{
	class GameServer
	{
	public:

		void Start(uint16 nPort);
		void Update();
		void Close();

	private:

		HSteamListenSocket m_hListenSock;
		HSteamNetPollGroup m_hPollGroup;
		ISteamNetworkingSockets* m_pInterface;

		struct Client_t
		{
			std::string m_sNick;
		};

		std::map<HSteamNetConnection, Client_t > m_mapClients;

		void SendStringToClient(HSteamNetConnection conn, const char* str);
		void SendStringToAllClients(const char* str, HSteamNetConnection except = k_HSteamNetConnection_Invalid);
		void PollIncomingMessages();
		void SetClientNick(HSteamNetConnection hConn, const char* nick);
		void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);
		//static ChatServer* s_pCallbackInstance;
		static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);
		void PollConnectionStateChanges();
		
	};
}