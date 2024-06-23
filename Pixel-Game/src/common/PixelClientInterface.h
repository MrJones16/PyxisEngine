#pragma once

#include <Pyxis/Network/Network.h>

namespace Pyxis
{
	enum class GameMessage : uint32_t
	{
		Ping,

		Server_ClientConnected,
		Server_ClientAccepted,
		Server_ClientAssignID,
		
		Client_RegisterWithServer,
		Client_UnregisterWithServer,

		Game_RequestGameData,
		Game_GameData,
		Game_TickClosure,
		Game_MergedTickClosure,

		Message_All,
		Server_Message,
	};

	class PixelClientInterface : public Network::ClientInterface<GameMessage>
	{
	public:
		uint64_t m_ID = 0;
	};
}